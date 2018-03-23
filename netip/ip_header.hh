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

#ifndef __ip_header_hh__
#define __ip_header_hh__

#include "ip_address.hh"

class IP_Buffer;

struct IP_Header_IPv4 {
  u8_t   IHL;      // Internet Header Length (IHL): 5-15 32-bit fields in header
  ns16_t id;       // Identification: (16 bits) [used for uniquely identifying the group of fragments of a single IP datagram]
  u8_t   flags;    // [bit 0: Reserved (must be zero) | bit 1: Don't Fragment (DF) | bit 2: More Fragments (MF)]
  ns16_t fragoff;  // Fragment Offset: (13 bits) [measured in units of eight-byte blocks]
  ns16_t checksum; // Header Checksum: (16 bits)
};

struct IP_Header_IPv6 {
  ns32_t flow;     // Flow Label: (20 bits) [if non-zero, packets should stay on the same path, so that they will not be reordered]
};

struct IP_Header_TCP {
  /* If the SYN flag is set (1), then this is the initial sequence number. The sequence number of the actual first data byte and the acknowledged
   * number in the corresponding ACK are then this sequence number plus 1. If the SYN flag is clear (0), then this is the accumulated sequence
   * number of the first data byte of this segment for the current session.
   */
  ns32_t sequence;

  /* If the ACK flag is set then the value of this field is the next sequence number that the sender of the ACK is expecting. This acknowledges
   * receipt of all prior bytes (if any). The first ACK sent by each end acknowledges the other end's initial sequence number itself, but no data.
   */
  ns32_t acknowledgement;

  /* Data offset: (4 bits) [Specifies the size of the TCP header in 32-bit words.]
   */
  u8_t offset;

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
#define IP_TCP_FLAG_NS  0x01
#define IP_TCP_FLAG_CWR 0x80
#define IP_TCP_FLAG_ECE 0x40
#define IP_TCP_FLAG_URG 0x20
#define IP_TCP_FLAG_ACK 0x10
#define IP_TCP_FLAG_PSH 0x08
#define IP_TCP_FLAG_RST 0x04
#define IP_TCP_FLAG_SYN 0x02
#define IP_TCP_FLAG_FIN 0x01

  ns16_t flags;

  inline void flag_ns (bool b) {
    if (b)
      flags[0] |=  IP_TCP_FLAG_NS;
    else
      flags[0] &= ~IP_TCP_FLAG_NS;
  }
  inline bool flag_ns () const {
    return flags[0] & IP_TCP_FLAG_NS;
  }

  inline void flag_cwr (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_CWR;
    else
      flags[1] &= ~IP_TCP_FLAG_CWR;
  }
  inline bool flag_cwr () const {
    return flags[1] & IP_TCP_FLAG_CWR;
  }

  inline void flag_ece (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_ECE;
    else
      flags[1] &= ~IP_TCP_FLAG_ECE;
  }
  inline bool flag_ece () const {
    return flags[1] & IP_TCP_FLAG_ECE;
  }

  inline void flag_urg (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_URG;
    else
      flags[1] &= ~IP_TCP_FLAG_URG;
  }
  inline bool flag_urg () const {
    return flags[1] & IP_TCP_FLAG_URG;
  }

  inline void flag_ack (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_ACK;
    else
      flags[1] &= ~IP_TCP_FLAG_ACK;
  }
  inline bool flag_ack () const {
    return flags[1] & IP_TCP_FLAG_ACK;
  }

  inline void flag_psh (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_PSH;
    else
      flags[1] &= ~IP_TCP_FLAG_PSH;
  }
  inline bool flag_psh () const {
    return flags[1] & IP_TCP_FLAG_PSH;
  }

  inline void flag_rst (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_RST;
    else
      flags[1] &= ~IP_TCP_FLAG_RST;
  }
  inline bool flag_rst () const {
    return flags[1] & IP_TCP_FLAG_RST;
  }

  inline void flag_syn (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_SYN;
    else
      flags[1] &= ~IP_TCP_FLAG_SYN;
  }
  inline bool flag_syn () const {
    return flags[1] & IP_TCP_FLAG_SYN;
  }

  inline void flag_fin (bool b) {
    if (b)
      flags[1] |=  IP_TCP_FLAG_FIN;
    else
      flags[1] &= ~IP_TCP_FLAG_FIN;
  }
  inline bool flag_fin () const {
    return flags[1] & IP_TCP_FLAG_FIN;
  }

  /* The size of the receive window, which specifies the number of window size units (by default, bytes) (beyond the segment identified by the sequence number
   * in the acknowledgment field) that the sender of this segment is currently willing to receive.
   */
  ns16_t size;     // Window size: (16 bits)

  ns16_t urgent;   // Urgent pointer: (16 bits) [if the URG flag is set, then this 16-bit field is an offset from the sequence number indicating the last urgent data byte]
};

struct IP_Header_UDP {
  ns16_t length;   // Length: (16 bits) [the length in bytes of the UDP header and UDP data]
};

class IP_Header {
public:
  enum Protocol {
#if IP_USE_IPv6
    p_ICMP = 0x3A,  // IPv6-ICMP
#else
    p_ICMP = 0x01,  // Internet Control Message Protocol
#endif
    p_TCP  = 0x06,
    p_UDP  = 0x11
  };

  /* IP fields common to IPv4 & IPv6
   */
  u8_t   version;  // Version: 4 or 6
  u8_t   DSCP;     // Differentiated Services Code Point (DSCP): (6 bits) // ??
  u8_t   ECN;      // Explicit Congestion Notification (ECN): (2 bits) // ??
  u8_t   ttl;      // Time To Live (TTL) / Hop Limit: (8 bits) [when the datagram arrives at a router, the router decrements the TTL field by one]
  ns16_t length;   // IPv4: Total Length: (16 bits; min=20, i.e., just the minimum header) [All hosts are required to be able to reassemble datagrams of size up to 576 bytes]
                   // IPv6: Payload Length: (16 bits; not including the header) [set to zero when a Hop-by-Hop extension header carries a Jumbo Payload option]
  u8_t   protocol; // IPv4: Protocol: (8 bits) [TCP=0x06, UDP=0x11]
                   // IPv6: Next Header / Protocol: (8 bits) [TCP=0x06, UDP=0x11]

  inline bool is_ICMP () const {
    return protocol == (u8_t) p_ICMP;
  }
  inline bool is_TCP () const {
    return protocol == (u8_t) p_TCP;
  }
  inline bool is_UDP () const {
    return protocol == (u8_t) p_UDP;
  }

  IP_Address address_source;
  IP_Address address_destination;

  /* IP version-specific fields
   */
#if IP_USE_IPv6
    struct IP_Header_IPv6 ipv6;
#else
    struct IP_Header_IPv4 ipv4;
#endif

  /* Protocol fields common to TCP & UDP
   */
  ns16_t port_source;
  ns16_t port_destination;

  /* TCP: The 16-bit checksum field is used for error-checking of the header, the Payload and a Pseudo-Header. The Pseudo-Header consist of the Source IP Address,
   *      the Destination IP Address, the protocol number for the TCP-Protocol (0x0006) and the length of the TCP-Headers including Payload (in Bytes).
   * UDP: optional in IPv4, and mandatory in IPv6
   */
  ns16_t checksum; // Checksum: (16 bits)

  /* Protocol-specific fields
   */
  struct IP_Header_TCP tcp;
  struct IP_Header_UDP udp;

  enum HeaderSniff {
    hs_Okay = 0,
    hs_EchoRequest,
    hs_EchoReply,
    hs_IPv4,
    hs_IPv4_FrameError,
    hs_IPv4_PacketTooShort,
    hs_IPv4_Checksum,
    hs_IPv6,
    hs_IPv6_FrameError,
    hs_IPv6_PacketTooShort,
    hs_Protocol_Unsupported,
    hs_Protocol_PacketTooShort,
    hs_Protocol_Checksum
  };

  HeaderSniff sniff (const IP_Buffer & buffer);

  void ping_to_pong (IP_Buffer & buffer);

  void defaults (Protocol p);
};

#endif /* ! __ip_header_hh__ */
