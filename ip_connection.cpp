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

void IP_Connection::reset (IP_Header::Protocol p, u16_t port) {
  flags = 0;

  channel = 0;

  header.defaults (p);

  if (port) {
    flags |= IP_Connection_LocalSpecified;
    header.port_source = port;
  } else {
    header.port_source = IP_Manager::manager().available_port ();
  }
  // TODO
}

void IP_Connection::open () {
  if (flags & IP_Connection_Open) {
    return;
  }
  if (flags & (IP_Connection_LocalSpecified | IP_Connection_RemoteSpecified)) {
    flags |= IP_Connection_Open;
    // TODO: what else?
    return;
  }
}

void IP_Connection::close () {
  flags &= ~IP_Connection_Open;
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

bool IP_Connection::connect (const IP_Address & address, u16_t port) {
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
}
