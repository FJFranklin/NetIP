/* Copyright (c) 2018 Francis James Franklin
 * 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and
 *    the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netip/ip_manager.hh"

bool IP_Channel::slip_next_to_send (const u8_t *& byte, u8_t & flags) { // returns true if there are byte(s) to be sent
  static const u8_t END = IP_SLIP_END;
  static const u8_t ESC_END[2] = { IP_SLIP_ESC, IP_SLIP_ESC_END };
  static const u8_t ESC_ESC[2] = { IP_SLIP_ESC, IP_SLIP_ESC_ESC };

  flags = IP_SLIP_NONE;

  if (!buffer_out) {
    buffer_out = chain_out.chain_pop (); // which may still be 0

    if (buffer_out) { // we have a new buffer; reset
      bytes_sent = 0;
    }
    return false; // even if we have data to send, return now; this is a low priority job
  }

  if (bytes_sent == buffer_out->length ()) { // we've finished sending the buffer; add the frame end
    flags = IP_SLIP_SINGLE | IP_SLIP_PACKET_LAST;
    byte = &END;

    /* don't need the buffer any more; set it free...
     */
    if (!buffer_out->unref ()) { // ... so long as no one else is ising it
      IP_Manager::manager().add_to_spares (buffer_out);
    }
    buffer_out = 0;

    return true; // there's data to send
  }

  byte = buffer_out->bytes () + bytes_sent;

  if (!bytes_sent) { // first byte of a new buffer!
    flags |= IP_SLIP_PACKET_FIRST;
  }

  switch (*byte) {
  case IP_SLIP_END:
    flags |= IP_SLIP_ESCAPE;
    byte = ESC_END;
    break;

  case IP_SLIP_ESC:
    flags |= IP_SLIP_ESCAPE;
    byte = ESC_ESC;
    break;

  default:
    flags |= IP_SLIP_SINGLE;
    break;
  }
  ++bytes_sent;

  return true; // there's data to send
}

bool IP_Channel::slip_can_receive () {
  if (slip_read_flags == IP_SLIP_READ_COMPLETE) {
    buffer_in->channel (channel_number); // note the buffer's originating channel

    if (IP_Manager::manager().queue (buffer_in)) {
      slip_read_flags = 0;
      buffer_in->clear (); // this is now actually a different buffer
    } else {
      return false; // oops, need to hang onto the buffer
    }
  }
  return true;
}

void IP_Channel::slip_receive (u8_t byte) {
  bool bAddByte = false;
  bool bPacketComplete = false;

  if (slip_read_flags == IP_SLIP_READ_COMPLETE) { // should call slip_can_receive() to check before calling slip_receive().
    return;
  }

  if (slip_read_flags & IP_SLIP_READ_ERROR) {
    if (byte == IP_SLIP_END) { // end of discarded packet; reset
      slip_read_flags = 0;
      buffer_in->clear ();
    }
  } else if (slip_read_flags & IP_SLIP_READ_ESCAPE) {
    slip_read_flags = 0;

    switch (byte) {
    case IP_SLIP_END: // well, this is wrong; quietly discard packet
      slip_read_flags = 0;
      buffer_in->clear ();
      break;

    case IP_SLIP_ESC_END:
      byte = IP_SLIP_END;
      bAddByte = true;
      break;

    case IP_SLIP_ESC_ESC:
      byte = IP_SLIP_ESC;
      bAddByte = true;
      break;

    default: // set error flag
      slip_read_flags = IP_SLIP_READ_ERROR;
      break;
    }
  } else {
    switch (byte) {
    case IP_SLIP_END:
      bPacketComplete = true;
      break;

    case IP_SLIP_ESC:
      slip_read_flags = IP_SLIP_READ_ESCAPE;
      break;

    default:
      bAddByte = true;
      break;
    }
  }

  if (bAddByte) {
    // TODO
  }
  if (bPacketComplete) {
    if (IP_Manager::manager().queue (buffer_in)) {
      slip_read_flags = 0;
      buffer_in->clear ();
    } else { // oops, need to hang onto the buffer
      slip_read_flags = IP_SLIP_READ_COMPLETE;
    }
  }
}
