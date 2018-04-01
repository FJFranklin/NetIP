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

void IP_Connection::reset (IP_Protocol p, u16_t port) {
  if (is_open ()) {
    close ();
  }
  fifo_write.clear ();

  flags = (p == p_TCP) ? IP_Connection_Protocol_TCP : 0;

  port_local = port;
}

void IP_Connection::update () {
  if (is_open ()) {

    if (buffer_in) {
      data_in_length -= buffer_in->push (fifo_read, data_in_offset);

      if (!data_in_length) {
	IP_Manager::manager().add_to_spares (buffer_in);
	buffer_in = 0;
      }
    }

    if (is_TCP ()) {

      // TODO: // FIXME

    } else if (has_remote ()) { // UDP

      if (!fifo_write.is_empty () || (EL && bSendRequested)) {
	IP_Buffer * buffer_out = IP_Manager::manager().get_from_spares ();

	if (buffer_out) {
	  buffer_out->defaults (p_UDP);

	  if (!fifo_write.is_empty ()) {
	    buffer_out->pull (fifo_write);
	  } else {
	    bSendRequested = false;

	    if (!EL->buffer_to_send (*this, *buffer_out)) { // it was asked for, but not used...
	      IP_Manager::manager().add_to_spares (buffer_out);
	      buffer_out = 0;
	    } else if (buffer_out->length () <= buffer_out->udp_data_offset ()) { // make sure something was added
	      IP_Manager::manager().add_to_spares (buffer_out);
	      buffer_out = 0;
	    }
	  }
	}
	if (buffer_out) {
	  buffer_out->channel (0);

	  buffer_out->ip().destination() = remote;

	  buffer_out->udp().source() = port_local;
	  buffer_out->udp().destination() = port_remote;

	  buffer_out->udp_finalise ();

	  IP_Manager::manager().forward (buffer_out); // send it
	}
      }
    }
  } else if (is_busy ()) { // connection recently closed and still active
    
    if (is_TCP ()) {

      // TODO: // FIXME

    } else { // UDP

      if (!has_remote ()) { // this connection is only listening; nothing further to do
	flags &= ~IP_Connection_Busy;

	if (EL) {
	  EL->connection_has_closed ();
	}
      } else {
	if (!fifo_write.is_empty ()) { // finish writing the buffered output
	  IP_Buffer * buffer_out = IP_Manager::manager().get_from_spares ();

	  if (buffer_out) {
	    buffer_out->defaults (p_UDP);
	    buffer_out->pull (fifo_write);
	    buffer_out->channel (0);

	    buffer_out->ip().destination() = remote;

	    buffer_out->udp().source() = port_local;
	    buffer_out->udp().destination() = port_remote;

	    buffer_out->udp_finalise ();

	    IP_Manager::manager().forward (buffer_out); // send it
	  }
	}
	if (fifo_write.is_empty ()) { // finished writing; nothing else to do
	  flags &= ~IP_Connection_Busy;

	  if (EL) {
	    EL->connection_has_closed ();
	  }
	}
      }
    }
  }
}

u16_t IP_Connection::read (u8_t * ptr, u16_t length) {
  if (!is_open ()) {
    return 0;
  }

  u16_t count = fifo_read.read (ptr, length);

  while ((count < length) && buffer_in) {
    data_in_length -= buffer_in->push (fifo_read, data_in_offset);

    if (!data_in_length) {
      IP_Manager::manager().add_to_spares (buffer_in);
      buffer_in = 0;
    }
    count += fifo_read.read (ptr + count, length - count);
  }
  return count;
}

u16_t IP_Connection::write (const u8_t * ptr, u16_t length) {
  if (!is_open () || !has_remote ()) {
    return 0;
  }

  u16_t count = fifo_write.write (ptr, length);

  if (count < length) {
    IP_Buffer * buffer_out = IP_Manager::manager().get_from_spares ();

    if (buffer_out) {
      if (is_TCP ()) {

	// TODO: // FIXME

      } else { // UDP

	buffer_out->defaults (p_UDP);
	buffer_out->pull (fifo_write);

	if (length - count > buffer_out->available ()) {
	  length = count + buffer_out->available ();
	}
	count += buffer_out->append (ptr + count, length - count);

	buffer_out->channel (0);

	buffer_out->ip().destination() = remote;

	buffer_out->udp().source() = port_local;
	buffer_out->udp().destination() = port_remote;

	buffer_out->udp_finalise ();

	IP_Manager::manager().forward (buffer_out); // send it
      }
    }
  }
  return count;
}

bool IP_Connection::open_tcp () {
  flags |= IP_Connection_Open;
  // TODO: what else?
  return true;
}

bool IP_Connection::open_udp () {
  flags |= IP_Connection_Open;

  if (EL) {
    EL->connection_has_opened ();
  }
  return true;
}

bool IP_Connection::open () {
  if (is_open ()) {
    return true;
  }
  if (is_busy ()) { // this connection hasn't finished closing yet
    return false;
  }

  if (is_TCP ()) {
    if (port_local && has_remote ()) {
      return open_tcp ();
    }
    return false;
  }

  // UDP

  if (port_local || has_remote ()) {
    return open_udp ();
  }
  return false;
}

void IP_Connection::close () {
  flags &= ~(IP_Connection_Open | IP_Connection_TimeoutSet);
  flags |=   IP_Connection_Busy;

  if (buffer_in) {
    IP_Manager::manager().add_to_spares (buffer_in);
    buffer_in = 0;
  }

  fifo_read.clear ();
}

bool IP_Connection::timeout () { // return true if the timer should be reset & retained
  if (!timeout_set () || !is_open ()) {
    // something went wrong
    return false;
  }
  // TODO:
  return false;
}

bool IP_Connection::accept_tcp (IP_Buffer * buffer, bool bNewConnection) {
  if (is_busy () && bNewConnection) {
    return false; // this connection hasn't finished closing yet; don't create a new one
  }

  // TODO: update TCP state

  bool bAcknowledge = false;

  if (EL) { // we have an event listener
    if (EL->buffer_received (*this, *buffer)) { // the new buffer has now been handled by the listener
      bAcknowledge = true;
    }
  }

  if (!bAcknowledge) {
    // TODO: Process data stream
    buffer_in = buffer;

    data_in_offset = buffer->tcp_data_offset ();
    data_in_length = buffer->tcp_data_length ();
  }
  if (bAcknowledge) {
    // TODO: Send acknowledgement
  }
  // FIXME!!!
  return true;
}

bool IP_Connection::accept_udp (IP_Buffer * buffer) {
  if (is_busy ()) {
    return false; // this connection hasn't finished closing yet
  }

  if (EL) { // we have an event listener
    if (EL->buffer_received (*this, *buffer)) { // the new buffer has now been handled by the listener
      IP_Manager::manager().add_to_spares (buffer);
      return true;
    }
  }

  data_in_offset = buffer->udp_data_offset ();
  data_in_length = buffer->udp_data_length ();

  data_in_length -= buffer->push (fifo_read, data_in_offset);

  if (data_in_length) {
    buffer_in = buffer; // save for later processing
  } else {
    IP_Manager::manager().add_to_spares (buffer);
  }
  return true;
}

bool IP_Connection::accept (IP_Buffer * buffer) {
  if (!is_open () || buffer_in || !port_local) {
    return false;
  }

  if (is_TCP ()) {
    if (!buffer->ip().is_TCP ()) { // incoming stream isn't TCP
      return false;
    }
    if (buffer->tcp().destination () != port_local) { // local port mismatch
      return false;
    }
    if (has_remote ()) { // make sure remote address & port match
      if (buffer->tcp().source () != port_remote) { // remote port mismatch
	return false;
      }
      if (buffer->ip().source () != remote) { // remote address mismatch
	return false;
      }
      return accept_tcp (buffer, false);
    }
    return accept_tcp (buffer, true); // need to establish connection
  }

  // Not TCP so should be UDP

  if (!buffer->ip().is_UDP ()) { // incoming stream isn't UDP
    return false;
  }
  if (buffer->udp().destination () != port_local) { // local port mismatch
    return false;
  }
  if (has_remote ()) { // make sure remote address & port match
    if (buffer->udp().source () != port_remote) { // remote port mismatch
      return false;
    }
    if (buffer->ip().source () != remote) { // remote address mismatch
      return false;
    }
  }
  return accept_udp (buffer);
}

void IP_Connection::connect (const IP_Address & address, u16_t port) {
  if (is_open ()) {
    return;
  }

  if (is_TCP ()) {

#if 0
  if (tcp.tcpstateflags != UIP_CLOSED) {
    return false;
  }
  if (tcp.tcpstateflags == UIP_TIME_WAIT) {
    // TODO: then... ??
#if 0// UIP_ACTIVE_OPEN
    if(cconn->tcpstateflags == UIP_TIME_WAIT) {
      if(conn == 0 ||
	 cconn->timer > conn->timer) {
	conn = cconn;
      }
    }
#endif /* UIP_ACTIVE_OPEN */
  }

  // mac = mac;
  // remote = address;
  // port_remote = port;

  tcp.mss_initial   = IP_TCP_MaxSegmentSize;
  tcp.tcpstateflags = UIP_SYN_SENT;

  tcp.timer  = 1; /* Send the SYN next time around. */
  tcp.length = 1; /* TCP length of the SYN is one. */

  tcp.rcv_nxt = 0;
  tcp.snd_nxt = IP_Manager::manager().iss ();

  tcp.nrtx = 0;
  tcp.rto = UIP_RTO;
  tcp.sa = 0;
  tcp.sv = 16;   /* Initial value of the RTT variance. */

    return true;
#endif

  } else { // UDP

    if (port) {
      flags |= IP_Connection_RemoteSpecified;

      remote = address;
      port_remote = port;

      open ();
    }
  }
}
