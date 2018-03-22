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

#ifndef __ip_channel_hh__
#define __ip_channel_hh__

#include "ip_buffer.hh"

/* SLIP encoding special bytes
 */
#define IP_SLIP_END           0xC0
#define IP_SLIP_ESC           0xDB
#define IP_SLIP_ESC_END       0xDC
#define IP_SLIP_ESC_ESC       0xDD

/* internal-only flags for slip_receive() / slip_can_receive()
 */
#define IP_SLIP_READ_ESCAPE   1 // last byte read was an escape character
#define IP_SLIP_READ_ERROR    2 // framing error or too long for buffer
#define IP_SLIP_READ_COMPLETE 4 // the buffer has a complete packet

/* flags returned by slip_next_to_send()
 */
#define IP_SLIP_NONE          0 // idle; nothing to send
#define IP_SLIP_SINGLE        1 // send the next byte
#define IP_SLIP_ESCAPE        2 // .. and the one after - it's a two-byte escape sequence
#define IP_SLIP_PACKET_FIRST  4 // this begins a new packet
#define IP_SLIP_PACKET_LAST   8 // this ends the packet

class IP_Channel : public Link {
private:
  IP_Buffer initial_buffer;

  Chain<IP_Buffer> chain_out;

  IP_Buffer * buffer_in;
  IP_Buffer * buffer_out;

  u16_t bytes_sent;

  u8_t channel_number;

  u8_t slip_read_flags;

public:
  inline u8_t number () const {
    return channel_number;
  }

  inline void set_number (u8_t number) {
    channel_number = number;
  }

  IP_Channel () :
    buffer_in(&initial_buffer),
    buffer_out(0),
    bytes_sent(0),
    channel_number(0),
    slip_read_flags(0)
  {
    // ...
  }

  virtual ~IP_Channel () {
    // ...
  }

  inline void send (IP_Buffer * buffer, bool bUrgent = false) {
    if (buffer) {
      buffer->ref ();

      if (bUrgent)
	chain_out.chain_prepend (buffer);
      else
	chain_out.chain_append (buffer);
    }
  }

protected:
  /* returns true if there is a byte (or two) to be sent; check flags for IP_SLIP_ESCAPE
   */
  bool slip_next_to_send (const u8_t *& byte, u8_t & flags);

  bool slip_can_receive (); // call this before trying slip_receive().
  void slip_receive (u8_t byte);

public:
  virtual void update () {
    // 
  }
};

#endif /* ! __ip_channel_hh__ */
