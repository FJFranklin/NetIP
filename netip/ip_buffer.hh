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

#ifndef __ip_buffer_hh__
#define __ip_buffer_hh__

#include "ip_protocol.hh"

class IP_Buffer : public Buffer, public Link {
private:
  u8_t buffer[IP_Buffer_WordCount << 1];

  u8_t source_channel;

public:
  inline void channel (u8_t channel_number) {
    source_channel = channel_number;
  }

  inline u8_t channel () const {
    return source_channel;
  }

  IP_Buffer () :
    Buffer(buffer, IP_Buffer_WordCount << 1),
    source_channel(0)
  {
    // ...
  }

  ~IP_Buffer () {
    // ...
  }

  inline void copy (u16_t byte_offset, const IP_Address & address) {
    write (byte_offset, address.byte_buffer (), address.byte_length ());
  }

#if IP_USE_IPv6
  inline struct IP_Header_IPv6 & ip () {
    return *((struct IP_Header_IPv6 *) buffer);
  }
  inline const struct IP_Header_IPv6 & ip () const {
    return *((const struct IP_Header_IPv6 *) buffer);
  }
#else // IP_USE_IPv6
  inline struct IP_Header_IPv4 & ip () {
    return *((struct IP_Header_IPv4 *) buffer);
  }
  inline const struct IP_Header_IPv4 & ip () const {
    return *((const struct IP_Header_IPv4 *) buffer);
  }
#endif

  inline u8_t protocol_offset () const {
    if ((buffer[0] & 0xF0) == 0x60) {
      return 40;
    }
    if ((buffer[0] & 0xF0) == 0x40) {
      return (buffer[0] & 0x0F) << 2;
    }
    return 0; // hmm...
  }

  inline struct IP_Header_TCP & tcp () {
    return *((struct IP_Header_TCP *) (buffer + protocol_offset ()));
  }
  inline const struct IP_Header_TCP & tcp () const {
    return *((const struct IP_Header_TCP *) (buffer + protocol_offset ()));
  }

  inline struct IP_Header_UDP & udp () {
    return *((struct IP_Header_UDP *) (buffer + protocol_offset ()));
  }
  inline const struct IP_Header_UDP & udp () const {
    return *((const struct IP_Header_UDP *) (buffer + protocol_offset ()));
  }

  inline struct IP_Header_ICMP & icmp () {
    return *((struct IP_Header_ICMP *) (buffer + protocol_offset ()));
  }
  inline const struct IP_Header_ICMP & icmp () const {
    return *((const struct IP_Header_ICMP *) (buffer + protocol_offset ()));
  }

  void defaults (IP_Protocol p) {
    ip().defaults ();
    ip().protocol() = (u8_t) p;

    buffer_used = ip().header_length ();

    switch (p) {
    case p_TCP:
      tcp().defaults ();
      buffer_used += tcp().header_length ();
      break;

    case p_UDP:
      udp().defaults ();
      buffer_used += udp().header_length ();
      break;

    case p_ICMP:
      icmp().defaults ();
      buffer_used += icmp().header_length ();
      break;
    }
  }

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

  HeaderSniff sniff () const;

private:
  void icmp_finalise ();
public:
  void ping (const IP_Address & destination);
  void ping_to_pong ();

  void print () const;
};

#endif /* ! __ip_buffer_hh__ */
