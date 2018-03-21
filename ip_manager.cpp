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
  netmask(IP_Address_DefaultNetmask)
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
      channel->set_number (0);
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

void IP_Manager::tick () {
  /* Check timers to see if any connections, etc., need updating
   */
  IP_Clock::tick ();

  /* Upodate I/O buffers & streams
   */
  Chain<IP_Channel>::iterator I = chain_channel.begin ();

  while (*I) {
    (*I)->update ();
    ++I;
  }

  // ...
}
