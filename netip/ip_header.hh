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
#include "ip_buffer.hh"

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

  /* IP header - we support one or the other, not both
   */
#if IP_USE_IPv6
    struct IP_Header_IPv6 ip;
#else
    struct IP_Header_IPv4 ip;
#endif

  /* Protocol-specific fields
   */
  union {
    struct IP_Header_TCP  tcp;
    struct IP_Header_UDP  udp;
    struct IP_Header_ICMP icmp;
  } proto;

  enum HeaderSniff {
    hs_Okay = 0,
    hs_FrameError,
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
    hs_Protocol_FrameError,
    hs_Protocol_PacketTooShort,
    hs_Protocol_Checksum
  };

  static HeaderSniff sniff (const IP_Buffer & buffer);

  static void ping_to_pong (IP_Buffer & buffer);

  void defaults (Protocol p);
};

#endif /* ! __ip_header_hh__ */
