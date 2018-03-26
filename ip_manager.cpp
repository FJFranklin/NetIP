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

void IP_Manager::connection_handover (IP_Buffer * buffer) {
  bool bHandedOver = false;

  Chain<IP_Connection>::iterator I = chain_connection.begin ();

  while (*I) {
    if ((*I)->accept (buffer)) {
      bHandedOver = true;
      break;
    }
    ++I;
  }
  if (!bHandedOver) {
    add_to_spares (buffer);
  }
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
  u8_t channel_number;

  IP_Channel * ch = 0;

  switch (channel_for_destination (channel_number, buffer->ip().destination ())) {

  case ri_Destination_Local:  // route through local network to final destination
  case ri_Gateway_Local:      // route through local network to gateway
    ch = channel (channel_number);
    if (ch) {
      ch->send (buffer);
    } else {
      add_to_spares (buffer);
    }
    break;

  case ri_Destination_Self:   // that's us!
    queue (buffer);
    break;

  case ri_Gateway_Self:       // we're the gateway - route to external network
    // TODO: route to external network

  case ri_InvalidAddress:     // reserved network address, or channel not registered
    add_to_spares (buffer);
    break;
  }
}

void IP_Manager::register_source (u8_t channel, const IP_Address & source) {
  if (channel > 0x0F) { // allowed a maximum of 15 external channels; and 0 = self
    return;
  }
  if (!is_local_network (source)) {
    return;
  }

  u8_t id = source.local_network_id ();

  if ((id == 0 /* reserved as a network identifier */) || (id == 255 /* reserved for broadcasts */)) {
    return;
  }

  if ((--id) & 1) {
    id >>= 1;
    channel_register[id] &= 0xF0;
    channel_register[id] |= channel;
  } else {
    id >>= 1;
    channel_register[id] &= 0x0F;
    channel_register[id] |= (channel << 4);
  }
}

IP_Manager::RoutingInfo IP_Manager::channel_for_destination (u8_t & channel, const IP_Address & destination) const {
  u8_t id;

  RoutingInfo ri;

  if (is_local_network (destination)) {
    id = destination.local_network_id ();

    if (id == host.local_network_id ()) {
      channel = 0;
      return ri_Destination_Self;
    }
    ri = ri_Destination_Local;
  } else {
    id = gateway.local_network_id ();

    if (id == host.local_network_id ()) {
      channel = 0;
      return ri_Gateway_Self;
    }
    ri = ri_Gateway_Local;
  }

  if ((id == 0 /* reserved as a network identifier */) || (id == 255 /* reserved for broadcasts */)) {
    return ri_InvalidAddress;
  }

  if ((--id) & 1) {
    id >>= 1;
    channel = channel_register[id] & 0x0F;
  } else {
    id >>= 1;
    channel = channel_register[id] >> 4;
  }

  if (!channel) {
    return ri_InvalidAddress;
  }
  return ri;
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
	switch (IP_Header::sniff (*pending)) {

	case IP_Header::hs_Okay:
	  register_source (pending->channel (), pending->ip().source ());

	  if (pending->ip().destination () == host) { // it's for us
	    connection_handover (pending); // hand over to appropriate connection
	  } else { // forward it
	    forward (pending);
	  }
	  break;

	case IP_Header::hs_EchoRequest:
	  register_source (pending->channel (), pending->ip().source ());

	  if (pending->ip().destination () == host) { // it's for us; we don't respond to broadcast pings
	    IP_Header::ping_to_pong (*pending);
	    forward (pending);
	  } else { // forward it
	    if (!chain_buffers_spare.chain_first ()) { // check if there is a spare - affects priorities
	      add_to_spares (pending);                 // drop it; more important to have spare buffers than broadcast packets (trying to avoid storms)
	    } else {
	      forward (pending);
	    }
	  }
	  break;

	case IP_Header::hs_EchoReply:
	  register_source (pending->channel (), pending->ip().source ());

	  if (pending->ip().destination () == host) { // it's for us - but we don't (yet) use it
	    add_to_spares (pending);
	  } else { // forward it
	    forward (pending);
	  }
	  break;

	case IP_Header::hs_Protocol_Unsupported:
	  register_source (pending->channel (), pending->ip().source ());

	  if (pending->ip().destination () == host) { // it's for us - but we can't use it
	    add_to_spares (pending);
	  } else { // forward it
	    forward (pending);
	  }
	  break;

	  /* Unable to handle this packet; don't handle or even forward it
	   */
	case IP_Header::hs_FrameError:
	case IP_Header::hs_IPv4:
	case IP_Header::hs_IPv4_FrameError:
	case IP_Header::hs_IPv4_PacketTooShort:
	case IP_Header::hs_IPv4_Checksum:
	case IP_Header::hs_IPv6:
	case IP_Header::hs_IPv6_FrameError:
	case IP_Header::hs_IPv6_PacketTooShort:
	case IP_Header::hs_Protocol_PacketTooShort:
	case IP_Header::hs_Protocol_FrameError:
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
