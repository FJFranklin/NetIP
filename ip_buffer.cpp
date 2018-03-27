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

IP_Buffer::HeaderSniff IP_Buffer::sniff () const {
  /* quick version check before we proceed
   */
  if (!length ()) { // empty buffer
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

  if (length () < 40) { // buffer needs to be at least 40, just for the IP header
    return hs_IPv6_FrameError;
  }

  if (ip().total_length () != length ()) {
    return hs_IPv6_PacketTooShort;
  }

  payload_offset = 40;
  payload_length = ip().length ();

#else // IP_USE_IPv6

  if (version == 6) {
    return hs_IPv6;
  }

  if (length () < 20) { // buffer needs to be at least 20, just for the IP header
    return hs_IPv4_FrameError;
  }

  u8_t header_length = ip().header_length ();

  if (header_length > length ()) {
    return hs_IPv4_FrameError;
  }

  if (ip().length () != length ()) {
    return hs_IPv4_PacketTooShort;
  }

  payload_offset = header_length;
  payload_length = length () - payload_offset;

  // TODO: reject fragments as unsupported

  checksum_sent = ip().checksum ();
  ip().header (check);

  if (header_length > 20) {
    check_16 (check, 20, header_length - 20);
  }
  checksum_calc = check.checksum ();

  if (checksum_sent != checksum_calc) {
    return hs_IPv4_Checksum;
  }

#endif // IP_USE_IPv6

  /* Protocol
   */
  if (ip().is_ICMP ()) {
    if (payload_length < 8) { // minimum size of ICMP header
      return hs_Protocol_PacketTooShort;
    }
    if (payload_length < 12) { // minimum size of ICMP header that we expect
      return hs_Protocol_Unsupported;
    }

    if ((icmp().type () == ip().protocol_echo_request ()) || (icmp().type () == ip().protocol_echo_reply ())) {

      /* This is an echo request/reply - ping!
       */
      check.clear ();

      if (ip().is_IPv6 ()) {
	ip().pseudo_header (check);
      }

      checksum_sent = icmp().checksum ();
      icmp().header (check);

      if (payload_length > 12) {
	check_16 (check, payload_offset + 12);
      }
      checksum_calc = check.checksum ();

      if (checksum_sent != checksum_calc) {
	return hs_Protocol_Checksum;
      }
      return (icmp().type () == ip().protocol_echo_request ()) ? hs_EchoRequest : hs_EchoReply;
    }
    return hs_Protocol_Unsupported;
  }

  if (ip().is_TCP ()) {
    if (payload_length < 20) { // minimum size of TCP header
      return hs_Protocol_PacketTooShort;
    }

    u8_t tcp_header_length = tcp().header_length ();

    if (tcp_header_length < 20) {
      return hs_Protocol_FrameError;
    }
    if (payload_length < tcp_header_length) {
      return hs_Protocol_PacketTooShort;
    }

    check.clear ();

    ip().pseudo_header (check);

    checksum_sent = tcp().checksum ();
    tcp().header (check);

    if (payload_length > 20) {
      check_16 (check, payload_offset + 20);
    }
    checksum_calc = check.checksum ();

    if (checksum_sent != checksum_calc) {
      return hs_Protocol_Checksum;
    }
    return hs_Okay;
  }

  if (ip().is_UDP ()) {
    if (payload_length < 8) { // minimum size of UDP header
      return hs_Protocol_PacketTooShort;
    }

    if (ip().is_IPv6 () || udp().checksum ()) { // optional in IPv4, and mandatory in IPv6; 0 if unused
      check.clear ();

      ip().pseudo_header (check);

      checksum_sent = udp().checksum ();
      udp().header (check);

      if (payload_length > 8) {
	check_16 (check, payload_offset + 8);
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

void IP_Buffer::ping_to_pong () { // we do this in-place, i.e., convert in incoming request buffer to an outgoing reply buffer
  ip().destination() = ip().source ();
  ip().source() = IP_Manager::manager().host;

  Check16 check;

#if IP_USE_IPv6
  ip().pseudo_header (check);
#else
  ip().header (check);
  ip().checksum() = check.checksum ();
#endif

  u16_t payload_offset = ip().header_length ();
  u16_t payload_length = ip().payload_length ();

  icmp().type() = ip().protocol_echo_reply ();
  icmp().header (check);

  if (payload_length > 12) {
    check_16 (check, payload_offset + 12);
  }
  icmp().checksum() = check.checksum ();
}
