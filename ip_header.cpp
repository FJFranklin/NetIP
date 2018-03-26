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

IP_Header::HeaderSniff IP_Header::sniff (const IP_Buffer & buffer) {
  /* quick version check before we proceed
   */
  if (!buffer.length ()) { // empty buffer
    return hs_FrameError;  // sort-of - shouldn't happen
  }

  u8_t version = buffer[0] >> 4;

  Check16 check;

  ns16_t checksum_sent;
  ns16_t checksum_calc;

  u16_t payload_offset;
  u16_t payload_length;

#if IP_USE_IPv6

  if (version == 4) {
    return hs_IPv4;
  }

  if (buffer.length () < 40) { // buffer needs to be at least 40, just for the IP header
    return hs_IPv6_FrameError;
  }
  const struct IP_Header_IPv6 & ip = buffer.ip ();

  if (ip.total_length () != buffer.length ()) {
    return hs_IPv6_PacketTooShort;
  }

  payload_offset = 40;
  payload_length = ip.length ();

#else // IP_USE_IPv6

  if (version == 6) {
    return hs_IPv6;
  }

  if (buffer.length () < 20) { // buffer needs to be at least 20, just for the IP header
    return hs_IPv4_FrameError;
  }

  const struct IP_Header_IPv4 & ip = buffer.ip ();

  u8_t header_length = ip.header_length ();

  if (header_length > buffer.length ()) {
    return hs_IPv4_FrameError;
  }

  if (ip.length () != buffer.length ()) {
    return hs_IPv4_PacketTooShort;
  }

  payload_offset = header_length;
  payload_length = buffer.length () - payload_offset;

  // TODO: reject fragments as unsupported

  checksum_sent = ip.checksum ();
  ip.header (check);

  if (header_length > 20) {
    buffer.check_16 (check, 20, header_length - 20);
  }
  checksum_calc = check.checksum ();

  if (checksum_sent != checksum_calc) {
    return hs_IPv4_Checksum;
  }

#endif // IP_USE_IPv6

  /* Protocol
   */
  if (ip.is_ICMP ()) {
    if (payload_length < 8) { // minimum size of ICMP header
      return hs_Protocol_PacketTooShort;
    }
    if (payload_length < 12) { // minimum size of ICMP header that we expect
      return hs_Protocol_Unsupported;
    }

    const struct IP_Header_ICMP & icmp = buffer.icmp ();

    if ((icmp.type () == ip.protocol_echo_request ()) || (icmp.type () == ip.protocol_echo_reply ())) {

      /* This is an echo request/reply - ping!
       */
      check.clear ();

      if (ip.is_IPv6 ()) {
	ip.pseudo_header (check);
      }

      checksum_sent = icmp.checksum ();
      icmp.header (check);

      if (payload_length > 12) {
	buffer.check_16 (check, payload_offset + 12);
      }
      checksum_calc = check.checksum ();

      if (checksum_sent != checksum_calc) {
	return hs_Protocol_Checksum;
      }
      return (icmp.type () == ip.protocol_echo_request ()) ? hs_EchoRequest : hs_EchoReply;
    }
    return hs_Protocol_Unsupported;
  }

  if (ip.is_TCP ()) {
    if (payload_length < 20) { // minimum size of TCP header
      return hs_Protocol_PacketTooShort;
    }

    const struct IP_Header_TCP & tcp = buffer.tcp ();

    u8_t tcp_header_length = tcp.header_length ();

    if (tcp_header_length < 20) {
      return hs_Protocol_FrameError;
    }
    if (payload_length < tcp_header_length) {
      return hs_Protocol_PacketTooShort;
    }

    check.clear ();

    ip.pseudo_header (check);

    checksum_sent = tcp.checksum ();
    tcp.header (check);

    if (payload_length > 20) {
      buffer.check_16 (check, payload_offset + 20);
    }
    checksum_calc = check.checksum ();

    if (checksum_sent != checksum_calc) {
      fprintf (stderr, "%x != %x", (unsigned) (u16_t) checksum_sent, (unsigned) (u16_t) checksum_calc);
      return hs_Protocol_Checksum;
    }
    return hs_Okay;
  }

  if (ip.is_UDP ()) {
    if (payload_length < 8) { // minimum size of UDP header
      return hs_Protocol_PacketTooShort;
    }

    const struct IP_Header_UDP & udp = buffer.udp ();

    if (ip.is_IPv6 () || udp.checksum ()) { // optional in IPv4, and mandatory in IPv6; 0 if unused
      check.clear ();

      ip.pseudo_header (check);

      checksum_sent = udp.checksum ();
      udp.header (check);

      if (payload_length > 8) {
	buffer.check_16 (check, payload_offset + 8);
      }
      checksum_calc = check.checksum ();

      if (checksum_sent != checksum_calc) {
	return hs_Protocol_Checksum;
      }
    }
    return hs_Okay;
  }

  return hs_Protocol_Unsupported;
}

void IP_Header::ping_to_pong (IP_Buffer & buffer) { // we do this in-place, i.e., convert in incoming request buffer to an outgoing reply buffer
  buffer.ip().destination() = buffer.ip().source ();
  buffer.ip().source() = IP_Manager::manager().host;

  Check16 check;

#if IP_USE_IPv6
  buffer.ip().pseudo_header (check);
#else
  buffer.ip().header (check);
  buffer.ip().checksum() = check.checksum ();
#endif

  u16_t payload_offset = buffer.ip().header_length ();
  u16_t payload_length = buffer.ip().payload_length ();

  buffer.icmp().type() = buffer.ip().protocol_echo_reply ();
  buffer.icmp().header (check);

  if (payload_length > 12) {
    buffer.check_16 (check, payload_offset + 12);
  }
  buffer.icmp().checksum() = check.checksum ();
}

void IP_Header::defaults (Protocol p) {
  static u16_t seq_no = 0;

  IP_Manager & IP = IP_Manager::manager ();

  ip.defaults ();

  ip.source()   = IP.host;
  ip.protocol() = (u8_t) p;

  switch (p) {
  case p_TCP:
    proto.tcp.defaults ();
    break;

  case p_UDP:
    proto.udp.defaults ();
    break;

  case p_ICMP: // Ping!
    proto.icmp.defaults ();

    proto.icmp.type()    = ip.protocol_echo_request ();
    proto.icmp.seq_no()  = seq_no++;
    proto.icmp.payload() = IP_Manager::manager().milliseconds ();
    break;
  }
}
