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
  flags = (p == p_TCP) ? IP_Connection_Protocol_TCP : 0;

  port_local = port;
}

bool IP_Connection::open () {
  if (flags & IP_Connection_Open) {
    return true;
  }

  if (is_TCP ()) {

    if (port_local) {
      flags |= IP_Connection_Open;
      // TODO: what else?
      return true;
    }
    return false;

  } else { // UDP

    if (port_local || has_remote ()) {
      flags |= IP_Connection_Open;
      // TODO: what else?
      return true;
    }
    return false;

  }
}

void IP_Connection::close () {
  flags &= ~(IP_Connection_Open | IP_Connection_TimeoutSet);
  // TODO: what else?
}

bool IP_Connection::timeout () { // return true if the timer should be reset & retained
  if (!(flags & IP_Connection_TimeoutSet)) {
    // something went wrong
    return false;
  }
  if (!(flags & IP_Connection_Open)) {
    return false;
  }
  // TODO:
  return false;
}

bool IP_Connection::accept (IP_Buffer * buffer) {
  if (!is_open () || is_busy () || !port_local) {
    return false;
  }

  if (is_TCP ()) {

    if (!buffer->ip().is_TCP ()) { // incoming stream isn't TCP
      return false;
    }
    if (has_remote ()) { // make sure remote address & port match
      if (buffer->tcp().destination () != port_local) { // local port mismatch
	return false;
      }
      if (buffer->tcp().source () != port_remote) { // remote port mismatch
	return false;
      }
      if (buffer->ip().source () != remote) { // remote address mismatch
	return false;
      }
      // TODO: what else?
      IP_Manager::manager().add_to_spares (buffer);
      return true;

    } else { // need to establish connection
      // TODO: what else?
      IP_Manager::manager().add_to_spares (buffer);
      return true;
    }
  } else { // UDP

    if (!buffer->ip().is_UDP ()) { // incoming stream isn't UDP
      return false;
    }
    if (buffer->udp().destination () != port_local) { // local port mismatch
      return false;
    }
    // TODO: what else?
    IP_Manager::manager().add_to_spares (buffer);
    return true;

  }
  return false; // FIXME
}

bool IP_Connection::connect (const IP_Address & address, u16_t port) {
  if (is_open ()) {
    return false;
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

      return open ();
    }
    return false;

  }
  return false;
}
