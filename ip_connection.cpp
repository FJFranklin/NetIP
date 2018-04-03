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

  if (buffer_tcp) {
    IP_Manager::manager().add_to_spares (buffer_tcp);
    buffer_tcp = 0;
  }

  flags = 0;
  is_TCP (p == p_TCP);

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

      if (tcp_send_ack ()) {
	if (tcp_ack ()) {
	  tcp_send_ack (false);
	}
      }

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
  } else if (is_busy ()) { // connection recently closed and still active, or attempting to establish connection
    
    if (is_TCP ()) {

      if (tcp_send_syn ()) {  // we wish to set up a new connection
	DEBUG_PRINT ("IP_Connection::update: send SYN\n");
	if (!buffer_tcp) {    // we haven't send a SYN yet
	  buffer_tcp = IP_Manager::manager().get_from_spares ();
	  buffer_tcp->ref (); // don't return to spares after sending
	}
	if (buffer_tcp) {     // set up a new connection
	  if (!tcp_syn_sent ()) { // this is the very first attempt
	    tcp_prepare (buffer_tcp);

	    buffer_tcp->tcp().flag_syn (true);

	    tcp.seq_no = IP_Manager::manager().milliseconds (); // just a random (-ish) number

	    buffer_tcp->tcp().seq_no() = tcp.seq_no++;

	    buffer_tcp->tcp_finalise ();

	    tcp.attempts = 1;
	    tcp.send_time = tcp.seq_no;
	  } else {
	    tcp.attempts++;
	    tcp.send_time = IP_Manager::manager().milliseconds ();
	  }

	  IP_Manager::manager().forward (buffer_tcp); // send it

	  timer.start (IP_Manager::manager (), 500); // TODO: 500?
	  timeout_set (true);

	  tcp_send_syn (false);
	  tcp_syn_sent (true);
	}
      }

      if (tcp_send_syn_ack ()) {  // we wish to respond to a new connection
	DEBUG_PRINT ("IP_Connection::update: send SYN-ACK\n");
	if (!buffer_tcp) {        // we haven't send a SYN ACK yet
	  buffer_tcp = IP_Manager::manager().get_from_spares ();
	  buffer_tcp->ref ();     // don't return to spares after sending
	}
	if (buffer_tcp) {         // respond to a new connection
	  if (!tcp_syn_ack_sent ()) { // this is the very first attempt
	    tcp_prepare (buffer_tcp);

	    tcp.seq_no = IP_Manager::manager().milliseconds (); // just a random (-ish) number

	    buffer_tcp->tcp().seq_no() =   tcp.seq_no++;
	    buffer_tcp->tcp().ack_no() = ++tcp.ack_no;

	    buffer_tcp->tcp().flag_syn (true);
	    buffer_tcp->tcp().flag_ack (true);

	    buffer_tcp->tcp_finalise ();

	    tcp.attempts = 1;
	    tcp.send_time = tcp.seq_no;
	  } else {
	    tcp.attempts++;
	    tcp.send_time = IP_Manager::manager().milliseconds ();
	  }

	  IP_Manager::manager().forward (buffer_tcp); // send it

	  timer.start (IP_Manager::manager (), 500); // TODO: 500?
	  timeout_set (true);

	  tcp_send_syn_ack (false);
	  tcp_syn_ack_sent (true);
	}
      }

    } else { // UDP

      if (!has_remote ()) { // this connection is only listening; nothing further to do
	is_busy (false);

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
	  is_busy (false);

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

bool IP_Connection::open () {
  if (is_open ()) {
    return true;
  }
  if (is_busy ()) { // this connection hasn't finished closing yet
    return false;
  }
  if (is_TCP ()) {
    if (!is_open () && !is_busy ()) {
      tcp_server (true);
      return true;
    }
    // use connect for TCP remote connections
    return false;
  }

  // UDP

  if (port_local || has_remote ()) {
    is_open (true);

    if (EL) {
      EL->connection_has_opened ();
    }
    return true;
  }
  return false;
}

void IP_Connection::close () {
  is_open (false);
  is_busy (true);

  /* the only reason we would have buffer_in is if there is more data for the user
   * - who has closed the connection now
   */
  if (buffer_in) {
    IP_Manager::manager().add_to_spares (buffer_in);
    buffer_in = 0;
  }

  /* similarly for the read buffer
   */
  fifo_read.clear ();
}

bool IP_Connection::timeout () { // return true if the timer should be reset & retained
  if (!timeout_set ()) {
    // was the connection reset?
    return false;
  }
  if (tcp_syn_sent ()) {
    fprintf (stderr, "timeout while sending SYN.\n");
  } else {
    fprintf (stderr, "other timeout (unhandled).\n");
  }
  return false;
}

bool IP_Connection::accept_tcp (IP_Buffer * buffer) {
  DEBUG_PRINT ("IP_Connection::accept_tcp\n");
  if (!buffer->tcp().source ()) { // remote port cannot be 0
    return false;
  }

  /* At this point, we know it's a TCP packet and the incoming port is correct.
   */
  if (!has_remote ()) {
    DEBUG_PRINT ("IP_Connection::accept_tcp: !has_remote()\n");

    /* No remote address & port identified yet; the only thing we accept at this
     * point is a connection request.
     */
    if (buffer->tcp().flag_syn () && !buffer->tcp().flag_ack ()) { // connection request
      is_busy (true);

      remote = buffer->ip().source ();

      port_remote = buffer->tcp().source ();

      tcp.ack_no = buffer->tcp().seq_no ();

      tcp_send_syn_ack (true); // let update() handle it

      IP_Manager::manager().add_to_spares (buffer);
      return true;
    }
    // not a connection request; reject
    return false;
  }

  /* we have a remote port that is non-zero, and incoming packets must match 
   */
  if (buffer->tcp().source () != port_remote) { // remote port mismatch
    return false;
  }
  if (buffer->ip().source () != remote) { // remote address mismatch
    return false;
  }
  DEBUG_PRINT ("IP_Connection::accept_tcp: remote matched\n");
  /* Strictly speaking, this isn't a TCP requirement, but we're going to
   * enforce it.
   */
  if (buffer->tcp().ack_no() == tcp.seq_no) {

    if (buffer->tcp().flag_syn () && buffer->tcp().flag_ack ()) {
      if (tcp_syn_sent ()) {
	DEBUG_PRINT ("This is a response to a SYN we sent.\n");

	/* This is a response to a SYN we sent.
	 */
	buffer_tcp->unref ();
	IP_Manager::manager().add_to_spares (buffer_tcp);
	buffer_tcp = 0;

	tcp.ack_no = buffer->tcp().seq_no ();
	tcp.ack_no++;

	timeout_set (false);

	tcp_syn_sent (false);
	tcp_send_ack (true); // let update() handle it

	is_open (true);

	return true;
      }
    }

    if (buffer->tcp().flag_ack ()) {
      if (tcp_syn_ack_sent ()) {
	DEBUG_PRINT ("This is a response to a SYN-ACK we sent.\n");

	/* This is a response to a SYN we sent.
	 */
	buffer_tcp->unref ();
	IP_Manager::manager().add_to_spares (buffer_tcp);
	buffer_tcp = 0;

	timeout_set (false);

	tcp_syn_ack_sent (false);

	is_open (true);

	return true;
      }
    }
  }

  // unexpected, or packet re-sent; send an ack  
  tcp_send_ack (true); // let update() handle it

  IP_Manager::manager().add_to_spares (buffer);
  return true;
#if 0
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
#endif
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
  if (buffer_in || !port_local) {
    return false;
  }

  if (is_TCP ()) {
    if (!buffer->ip().is_TCP ()) { // incoming stream isn't TCP
      return false;
    }
    if (buffer->tcp().destination () != port_local) { // local port mismatch
      return false;
    }
    if (!is_open () && !is_busy () && !tcp_server ()) {
      // we're in client mode, and not accepting incoming packets
      return false;
    }
    return accept_tcp (buffer);
  }

  // Not TCP so should be UDP

  if (!is_open () || is_busy ()) {
    return false;
  }
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
  if (is_open () || is_busy () || tcp_server ()) {
    return;
  }
  if (!port) { // not allowed to connect to port 0
    has_remote (false);
    return;
  }
  has_remote (true);

  port_remote = port;
  remote = address;

  if (is_TCP ()) {
    if (port_local) { // need a non-zero local port to establish a TCP connection

      tcp_reset_flags ();
      tcp_send_syn (true);

      is_busy (true);

      // let update() && accept_tcp() manage the connection
    }
  } else { // UDP
    open ();
  }
}

void IP_Connection::tcp_prepare (IP_Buffer * buffer) {
  buffer->channel (0);

  buffer->defaults (p_TCP);

  buffer->ip().destination() = remote;

  buffer->tcp().source() = port_local;
  buffer->tcp().destination() = port_remote;
}

bool IP_Connection::tcp_ack () {
  DEBUG_PRINT ("IP_Connection::tcp_ack: send ACK\n");
  IP_Buffer * buffer = IP_Manager::manager().get_from_spares ();

  if (buffer) {
    tcp_prepare (buffer);

    buffer->tcp().flag_ack (true);

    buffer->tcp().seq_no() = tcp.seq_no;
    buffer->tcp().ack_no() = tcp.ack_no;

    buffer->tcp_finalise ();

    IP_Manager::manager().forward (buffer); // send it

    return true;
  }
  return false;
}
