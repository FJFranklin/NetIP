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

static IP_Manager s_manager;

IP_Manager & IP_Manager::manager () {
  return s_manager;
}

/*
 * Use IP_Manager::manager() to get the global instance
 */
IP_Manager::IP_Manager () :
  last_port(4096),
  host(IP_Address_DefaultHost),
  gateway(IP_Address_DefaultGateway),
  netmask(IP_Address_DefaultNetmask),
  ticker(0)
{
  for (int i = 0; i < IP_Buffer_Extras; i++) {
    chain_buffers_spare.chain_prepend (buffers + i);
  }
}

IP_Connection * IP_Manager::connection_for_port (const ns16_t & port) {
  Chain<IP_Connection>::iterator I = chain_connection.begin ();

  while (*I) {
    if ((*I)->listening (port)) {
      break;
    }
    ++I;
  }
  return *I;
}

ns16_t IP_Manager::available_port () {
  ns16_t lp;

  while (true) {
    lp = last_port;

    if (!connection_for_port (lp)) {
      break;
    }
    if (++last_port >= 32000) {
      last_port = 4096;
    }
  }
  return lp;
}

void IP_Manager::channel_add (IP_Channel * channel) {
  if (channel) {
    IP_Channel * C = chain_channel.chain_first ();

    if (C) {
      channel->set_number (C->number () + 1);
    } else {
      channel->set_number (1); // first channel is #1; reserve 0 for ourself
    }
    chain_channel.chain_prepend (channel);
  }
}

IP_Channel * IP_Manager::channel (u8_t number) {
  Chain<IP_Channel>::iterator I = chain_channel.begin ();

  while (*I) {
    if ((*I)->number () == number) {
      break;
    }
    ++I;
  }
  return *I;
}

bool IP_Manager::queue (IP_Buffer *& buffer) {
  bool bQueued = false;

  if (buffer) {
    IP_Buffer * pending = buffer;
    IP_Buffer * spare = chain_buffers_spare.chain_pop ();

    if (spare) {
      buffer = spare;
      chain_buffers_pending.chain_push (pending, true /* FIFO */);
      bQueued = true;
    }
  }
  return bQueued;
}

void IP_Manager::forward (IP_Buffer * buffer) {
  // TODO
  add_to_spares (buffer); // FIXME
}

void IP_Manager::tick () {
  /* Update I/O channels
   */
  Chain<IP_Channel>::iterator C = chain_channel.begin ();

  while (*C) {
    (*C)->update ();
    ++C;
  }

  switch (ticker) { // try to balance processor load to allow the timers to function properly
  case 0:
    {
      /* Update IP connections
       */
      Chain<IP_Connection>::iterator I = chain_connection.begin ();

      while (*I) {
	(*I)->update ();
	++I;
      }
    }
    ++ticker;
    break;

  case 1: // Handle next pending buffer, if any
    {
      IP_Buffer * pending = chain_buffers_pending.chain_pop ();

      if (pending) {
	switch (header.sniff (*pending)) {
	case IP_Header::hs_Okay:
	  if (header.address_destination == host) { // it's for us
	    add_to_spares (pending); // FIXME // TODO: hand over to appropriate connection
	  } else { // forward it
	    forward (pending);
	  }
	  break;

	case IP_Header::hs_EchoRequest:
	  if (header.address_destination == host) { // it's for us
	    add_to_spares (pending); // FIXME // TODO: add support for Echo Reply
	  } else { // forward it
	    if (!chain_buffers_spare.chain_first ()) { // check if there is a spare - affects priorities
	      add_to_spares (pending);                 // drop it; more important to have spare buffers than broadcast packets (trying to avoid storms)
	    } else {
	      forward (pending);
	    }
	  }
	  break;

	case IP_Header::hs_Protocol_Unsupported:
	  if (header.address_destination == host) { // it's for us - but we can't use it
	    add_to_spares (pending);
	  } else { // forward it
	    forward (pending);
	  }
	  break;

	  /* Unable to handle this packet; don't handle or even forward it
	   */
	case IP_Header::hs_IPv4:
	case IP_Header::hs_IPv4_FrameError:
	case IP_Header::hs_IPv4_PacketTooShort:
	case IP_Header::hs_IPv4_Checksum:
	case IP_Header::hs_IPv6:
	case IP_Header::hs_IPv6_FrameError:
	case IP_Header::hs_IPv6_PacketTooShort:
	case IP_Header::hs_Protocol_PacketTooShort:
	case IP_Header::hs_Protocol_Checksum:
	  add_to_spares (pending);
	  break;
	}
      }
    }
    ++ticker;
    break;

  case 2:
  default:
    {
      // TODO: Handle next pending task, if any
    }
    ticker = 0;
    break;
  }
}

void IP_Manager::every_millisecond () {
  // ...
}

void IP_Manager::every_second () {
  // TODO: broadcast ping - i.e., set a flag to broadcast a ping as and when a spare buffer is available
}
