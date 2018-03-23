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
  IP_Header::HeaderSniff hs = hs_Okay;

  IP_BufferIterator I(buffer);

  Check16 check;

  /* IP header; 
   */
  ns16_t h; // temporary variable

#if IP_USE_IPv6
  if (I.remaining () < 40) { // minimum size of IPv6 header
    return hs_IPv6_PacketTooShort;
  }

  I.ns16 (h);

  // Version: 4 or 6
  version = h[0] >> 4;

  if (version == 4) {
    return hs_IPv4;
  }

  // Differentiated Services Code Point (DSCP): (6 bits) // ??
  DSCP = ((h[0] & 0x0F) << 2) | (h[1] >> 6);

  // Explicit Congestion Notification (ECN): (2 bits) // ??
  ECN  = (h[1] >> 4) & 0x03;

  // Flow Label: (20 bits) [if non-zero, packets should stay on the same path, so that they will not be reordered]
  ipv6.flow[1] = h[1] & 0x0F;
  I.ns16 (h);
  ipv6.flow[2] = h[0];
  ipv6.flow[3] = h[1];

  I.ns16 (length); // IPv6: Payload Length: (16 bits; not including the header) [set to zero when a Hop-by-Hop extension header carries a Jumbo Payload option]
  I.ns16 (h);
  protocol = h[0]; // IPv6: Next Header / Protocol: (8 bits) [TCP=0x06, UDP=0x11]
  ttl = h[1];      // Time To Live (TTL) / Hop Limit: (8 bits) [when the datagram arrives at a router, the router decrements the TTL field by one]

  I.address (address_source);
  I.address (address_destination);

  if (I.remaining () != length) {
    return hs_IPv6_FrameError;
  }

#else // IP_USE_IPv6
  if (I.remaining () < 20) { // minimum size of IPv4 header
    return hs_IPv4_PacketTooShort;
  }

  I.ns16 (h);
  check += h;

  version  = h[0] >> 4;

  if (version == 6) {
    return hs_IPv6;
  }

  ipv4.IHL = h[0] & 0x0F; // Internet Header Length (IHL): 5-15 32-bit fields in header

  if (ipv4.IHL < 5) {
    return hs_IPv4_FrameError;
  }

  u8_t header_length = ipv4.IHL << 2;

  if (I.remaining () < (header_length - 4)) {
    return hs_IPv4_PacketTooShort;
  }

  DSCP = h[1] >> 2;       // Differentiated Services Code Point (DSCP): (6 bits) // ??
  ECN  = h[1] & 0x03;     // Explicit Congestion Notification (ECN): (2 bits) // ??

  I.ns16 (length);        // IPv4: Total Length: (16 bits; min=20, i.e., just the minimum header)
                          // [All hosts are required to be able to reassemble datagrams of size up to 576 bytes]
  check += length;

  if (length < header_length) {
    return hs_IPv4_FrameError;
  }

  I.ns16 (ipv4.id);       // Identification: (16 bits) [used for uniquely identifying the group of fragments of a single IP datagram]
  check += ipv4.id;

  I.ns16 (h);
  check += h;

  // [bit 0: Reserved (must be zero) | bit 1: Don't Fragment (DF) | bit 2: More Fragments (MF)]
  ipv4.flags = h[0] >> 5;

  // Fragment Offset: (13 bits) [measured in units of eight-byte blocks]
  ipv4.fragoff[0] = h[0] & 0x1F;
  ipv4.fragoff[1] = h[1];

  I.ns16 (h);
  check += h;

  ttl = h[0];             // Time To Live (TTL) / Hop Limit: (8 bits) [when the datagram arrives at a router, the router decrements the TTL field by one]
  protocol = h[1];        // IPv4: Protocol: (8 bits) [TCP=0x06, UDP=0x11]

  I.ns16 (ipv4.checksum); // Header Checksum: (16 bits)
  // check += 0;

  I.address (address_source);
  address_source.check (check);

  I.address (address_destination);
  address_destination.check (check);

  for (u8_t i = 5; i < ipv4.IHL; i++) { // skip optional headers
    I.ns16 (h);
    check += h;
    I.ns16 (h);
    check += h;
  }

  if (ipv4.checksum != check.checksum ()) {
    return hs_IPv4_Checksum;
  }

  length = (u16_t) length - (u16_t) header_length; // to match the IPv6 payload length, which is more useful anyway

  if (I.remaining () != length) {
    return hs_IPv4_FrameError;
  }

#endif

  /* Protocol header
   */
  check.clear ();

#if IP_USE_IPv6
  if (is_ICMP () || is_TCP () || is_UDP ())
#else
  if (is_TCP () || is_UDP ())
#endif
  {
    /* Checksum has pseudo-header
     */
    address_source.check (check);
    address_destination.check (check);

#if IP_USE_IPv6
    // check += 0;
    check += length;
    // check += 0;
    check += protocol;
#else
    check += protocol;
    check += length;
#endif
  }

  if (is_ICMP ()) {

    if (I.remaining () < 8) { // minimum size of ICMP header
      return hs_Protocol_PacketTooShort;
    }

    I.ns16 (h);
    check += h;

#if IP_USE_IPv6
    if ((h[0] == 128 /* echo request */) || (h[0] == 129 /* echo reply*/))
#else
    if ((h[0] ==   8 /* echo request */) || (h[0] ==   0 /* echo reply*/))
#endif
    {
#if IP_USE_IPv6
      hs = (h[0] == 128) ? hs_EchoRequest : hs_EchoReply;
#else
      hs = (h[0] ==   8) ? hs_EchoRequest : hs_EchoReply;
#endif

      /* This is an echo request/reply - ping!
       */
      I.ns16 (checksum);
      // check += 0;

      I.ns16 (h); // identifier
      check += h;

      I.ns16 (h); // sequence number
      check += h;
    } else {
      return hs_Protocol_Unsupported;
    }
  } else if (is_TCP ()) {

    if (I.remaining () < 20) { // minimum size of TCP header
      return hs_Protocol_PacketTooShort;
    }

    I.ns16 (port_source);
    check += port_source;

    I.ns16 (port_destination);
    check += port_destination;

    /* If the SYN flag is set (1), then this is the initial sequence number. The sequence number of the actual first data byte and the acknowledged
     * number in the corresponding ACK are then this sequence number plus 1. If the SYN flag is clear (0), then this is the accumulated sequence
     * number of the first data byte of this segment for the current session.
     */
    I.ns32 (tcp.sequence);

    check += tcp.sequence.hi ();
    check += tcp.sequence.lo ();
    
    /* If the ACK flag is set then the value of this field is the next sequence number that the sender of the ACK is expecting. This acknowledges
     * receipt of all prior bytes (if any). The first ACK sent by each end acknowledges the other end's initial sequence number itself, but no data.
     */
    I.ns32 (tcp.acknowledgement);

    check += tcp.acknowledgement.hi ();
    check += tcp.acknowledgement.lo ();

    /* Data offset: (4 bits) [Specifies the size of the TCP header in 32-bit words.]
     */
    I.ns16 (h);
    check += h;

    tcp.offset = h[0] >> 4;

    /* NS (1 bit):  ECN-nonce - concealment protection (experimental: see RFC 3540).
     * CWR (1 bit): Congestion Window Reduced (CWR) flag is set by the sending host to indicate that it received a TCP segment with the ECE flag
     *              set and had responded in congestion control mechanism (added to header by RFC 3168).
     * ECE (1 bit): ECN-Echo has a dual role, depending on the value of the SYN flag. It indicates:
     * 
     *  - If the SYN flag is set (1), that the TCP peer is ECN capable.
     *  - If the SYN flag is clear (0), that a packet with Congestion Experienced flag set (ECN=11) in IP header was received during normal
     *    transmission (added to header by RFC 3168). This serves as an indication of network congestion (or impending congestion) to the TCP sender.
     * 
     * URG (1 bit): indicates that the Urgent pointer field is significant
     * ACK (1 bit): indicates that the Acknowledgment field is significant. All packets after the initial SYN packet sent by the client should have this flag set.
     * PSH (1 bit): Push function. Asks to push the buffered data to the receiving application.
     * RST (1 bit): Reset the connection
     * SYN (1 bit): Synchronize sequence numbers. Only the first packet sent from each end should have this flag set. Some other flags and fields change meaning
     *              based on this flag, and some are only valid for when it is set, and others when it is clear.
     * FIN (1 bit): Last packet from sender.
     */
    tcp.flags[0] = h[0] & 0x01;
    tcp.flags[1] = h[1];

    /* The size of the receive window, which specifies the number of window size units (by default, bytes) (beyond the segment identified by the sequence number
     * in the acknowledgment field) that the sender of this segment is currently willing to receive.
     */
    I.ns16 (tcp.size);
    check += tcp.size;

    I.ns16 (checksum);
    check += 0;

    /* Urgent pointer: (16 bits) [if the URG flag is set, then this 16-bit field is an offset from the sequence number indicating the last urgent data byte]
     */
    I.ns16 (tcp.urgent);
    check += tcp.urgent;

    u8_t tcp_header_length = tcp.offset << 2;

    if (I.remaining () < (tcp_header_length - 20)) {
      return hs_Protocol_PacketTooShort;
    }

    for (u8_t i = 5; i < tcp.offset; i++) { // optional headers
      I.ns16 (h);
      check += h;
    }

  } else if (is_UDP ()) {

    if (I.remaining () < 8) { // minimum size of UDP header
      return hs_Protocol_PacketTooShort;
    }

    I.ns16 (port_source);
    check += port_source;

    I.ns16 (port_destination);
    check += port_destination;

    I.ns16 (udp.length); // Length: (16 bits) [the length in bytes of the UDP header and UDP data]
    check += udp.length;

    I.ns16 (checksum);   // Checksum: (16 bits) [optional in IPv4, and mandatory in IPv6]
    check += 0;

  } else {
    return hs_Protocol_Unsupported;
  }

  /* checksum the data
   */
  while (I.remaining () > 1) {
    I.ns16 (h);
    check += h;
  }
  if (I.remaining ()) { // an odd byte
    h[0] = *(*I);
    h[1] = 0;
    check += h;
  }

  if (checksum && (checksum != check.checksum ())) {
    return hs_Protocol_Checksum;
  }

  return hs;
}

void IP_Header::ping_to_pong (IP_Buffer & buffer) {
  Check16 check;

  address_destination = address_source;
  address_source = IP_Manager::manager().host;

  u8_t header_length;

#if IP_USE_IPv6
  header_length = 40;

  buffer.copy_address ( 8, address_source);
  buffer.copy_address (12, address_destination);
#else
  header_length = ipv4.IHL << 2;

  buffer.copy_address (12, address_source);
  buffer.copy_address (16, address_destination);

  ipv4.checksum = 0;
  buffer.copy_ns16 (10, ipv4.checksum);

  buffer.check (check, 0, header_length);

  ipv4.checksum = check.checksum ();
  buffer.copy_ns16 (10, ipv4.checksum);

  check.clear ();
#endif

  /* Checksum has pseudo-header
   */
  address_source.check (check);
  address_destination.check (check);

#if IP_USE_IPv6
  // check += 0;
  check += length;
  // check += 0;
  check += protocol;

  buffer[header_length] = 129; // type
#else
  check += protocol;
  check += length;

  buffer[header_length] =   0; // type
#endif

  checksum = 0;
  buffer.copy_ns16 (header_length + 2, checksum);

  buffer.check (check, header_length, 0 /* to end */);

  checksum = check.checksum ();
  buffer.copy_ns16 (header_length + 2, checksum);
}

void IP_Header::defaults (Protocol p) {
  IP_Manager & IP = IP_Manager::manager ();

  address_source = IP.host;

#if IP_USE_IPv6
  version = 6;
  ipv6.flow = 0;
#else
  version = 4;
  ipv4.IHL = 5;
  ipv4.id = 0;
  ipv4.flags = 0;
  ipv4.fragoff = 0;
#endif
  DSCP = 0;
  ECN = 0;
  ttl = 64;
  protocol = (u8_t) p;

  switch (p) {
  case p_TCP:
    tcp.sequence = 0;
    tcp.acknowledgement = 0;
    tcp.offset = 5;
    tcp.flags = 0;
    tcp.size = IP_TCP_MaxSegmentSize; // ??
    tcp.urgent = 0;
    break;

  case p_UDP:
    // ...
    break;

  case p_ICMP: // Ping!
    // ...
    break;
  }
}
