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

#ifndef __ip_protocol_hh__
#define __ip_protocol_hh__

#include "ip_defines.hh"
#include "ip_address.hh"

enum IP_Protocol {
#if IP_USE_IPv6
  p_ICMP = 0x3A,  // IPv6-ICMP
#else
  p_ICMP = 0x01,  // Internet Control Message Protocol
#endif
  p_TCP  = 0x06,
  p_UDP  = 0x11
};

struct IP_Header_IPv4 {
  u8_t buffer[20]; // there may be upto 10 extra header fields, but the first 20 bytes are required

  inline void clear () {
    memset (buffer, 0, 20);
  }

  /* methods for interrogating & editing the header
   */

  inline u8_t get_version () const {
    return buffer[0] >> 4;
  }
  inline void set_version () {
    buffer[0] = (buffer[0] & 0x0F) | 0x40;
  }

  /* Internet Header Length (IHL): 5-15 32-bit fields in header
   */
  inline u8_t get_IHL () const {
    return buffer[0] & 0x0F;
  }
  inline void set_IHL (u8_t ihl) {
    ihl = (ihl < 5) ? 5 : ((ihl <= 15) ? ihl : 15); // 5 <= ihl <= 15
    buffer[0] = (buffer[0] & 0xF0) | ihl;
  }

  inline u8_t get_DSCP () const {
    return buffer[1] >> 2;
  }
  inline void set_DSCP (u8_t dscp) {
    dscp = (dscp < 64) ? dscp : 0;
    buffer[1] = (buffer[1] & 0x03) | (dscp << 2);
  }

  inline u8_t get_ECN () const {
    return buffer[1] & 0x03;
  }
  inline void set_ECN (u8_t ecn) {
    ecn = (ecn < 4) ? ecn : 0;
    buffer[1] = (buffer[1] & 0xFC) | ecn;
  }

  inline ns16_t & length () {
    return *((ns16_t *) (buffer + 2));
  }
  inline const ns16_t & length () const {
    return *((const ns16_t *) (buffer + 2));
  }

  /* Identification: (16 bits) [used for uniquely identifying the group of fragments of a single IP datagram]
   */
  inline ns16_t & id () {
    return *((ns16_t *) (buffer + 4));
  }
  inline const ns16_t & id () const {
    return *((const ns16_t *) (buffer + 4));
  }

  /* [bit 0: Reserved (must be zero) | bit 1: Don't Fragment (DF) | bit 2: More Fragments (MF)]
   */
  inline void get_flags (bool & DF, bool & MF) const {
    DF = buffer[6] & 0x40;
    MF = buffer[6] & 0x20;
  }
  inline void set_flags_DF () {
    buffer[6] = (buffer[6] & 0x1F) | 0x40;
  }
  inline void set_flags_MF () {
    buffer[6] = (buffer[6] & 0x1F) | 0x20;
  }

  /* Fragment Offset: (13 bits) [measured in units of eight-byte blocks]
   */
  inline u16_t get_fragment_offset () const {
    return ((buffer[6] & 0x1F) << 8) | buffer[7];
  }
  inline void set_fragment_offset (u16_t offset) {
    ns16_t o = ns16_t::convert (offset);
    buffer[6] = (buffer[6] & 0xE0) | (o[0] & 0x1F);
    buffer[7] = o[1];
  }

  inline u8_t & ttl () {
    return buffer[8];
  }
  inline const u8_t & ttl () const {
    return buffer[8];
  }

  inline u8_t & protocol () {
    return buffer[9];
  }
  inline const u8_t & protocol () const {
    return buffer[9];
  }

  /* Header Checksum: (16 bits)
   */
  inline ns16_t & checksum () {
    return *((ns16_t *) (buffer + 10));
  }
  inline const ns16_t & checksum () const {
    return *((const ns16_t *) (buffer + 10));
  }

  inline IP_Address & source () {
    return *((IP_Address *) (buffer + 12));
  }
  inline const IP_Address & source () const {
    return *((const IP_Address *) (buffer + 12));
  }

  inline IP_Address & destination () {
    return *((IP_Address *) (buffer + 16));
  }
  inline const IP_Address & destination () const {
    return *((const IP_Address *) (buffer + 16));
  }

  /* utility methods
   */

  inline u8_t version () const {
    return 4;
  }

  inline u8_t header_length () const {
    return get_IHL () << 2;
  }

  inline void set_total_length (u16_t total) {
    length() = total;
  }
  inline u16_t total_length () const {
    return length ();
  }

  inline u16_t payload_length () const {
    u16_t total_length = length ();
    return total_length - header_length ();
  }

  inline void defaults () {
    clear ();
    set_version ();
    set_IHL (5);
    ttl() = IP_TimeToLive;
  }

  inline u8_t protocol_echo_request () const {
    return 8;
  }

  inline u8_t protocol_echo_reply () const {
    return 0;
  }

  inline bool is_IPv6 () const {
    return false;
  }

  inline bool is_ICMP () const {
    return protocol () == 0x01;
  }
  inline bool is_TCP () const {
    return protocol () == 0x06;
  }
  inline bool is_UDP () const {
    return protocol () == 0x11;
  }

  inline void pseudo_header (Check16 & check) const {
    source().check (check);
    destination().check (check);

    check += protocol ();
    check += payload_length ();
  }

  inline void header (Check16 & check) const {
    const Buffer B((u8_t *) buffer, 20, true /* full buffer */);
    B.check_16 (check,  0, 10);
    B.check_16 (check, 12,  8);
  }
};

struct IP_Header_IPv6 {
  u8_t buffer[40];

  inline void clear () {
    memset (buffer, 0, 40);
  }

  /* methods for interrogating & editing the header
   */

  inline u8_t get_version () const {
    return buffer[0] >> 4;
  }
  inline void set_version () {
    buffer[0] = (buffer[0] & 0x0F) | 0x60;
  }

  inline u8_t get_DSCP () const {
    return ((buffer[0] & 0x0F) << 2) | (buffer[1] >> 6);
  }
  inline void set_DSCP (u8_t dscp) {
    dscp = (dscp < 64) ? dscp : 0;
    buffer[0] = (buffer[0] & 0xF0) | (dscp >> 2);
    buffer[1] = (buffer[1] & 0x3F) | ((dscp & 0x03) << 6);
  }

  inline u8_t get_ECN () const {
    return (buffer[1] & 0x30) >> 4;
  }
  inline void set_ECN (u8_t ecn) {
    ecn = (ecn < 4) ? ecn : 0;
    buffer[1] = (buffer[1] & 0xCF) | (ecn << 4);
  }

  /* Flow Label
   */
  inline ns32_t get_flow () const {
    ns32_t flow = *((ns32_t *) buffer);
    flow[0] = 0;
    flow[1] &= 0x0F;
    return flow;
  }
  inline void set_flow (const ns32_t & flow) {
    buffer[1] = (buffer[1] & 0xF0) | (flow[1] & 0x0F);
    buffer[2] = flow[2];
    buffer[3] = flow[3];
  }

  inline ns16_t & length () {
    return *((ns16_t *) (buffer + 4));
  }
  inline const ns16_t & length () const {
    return *((const ns16_t *) (buffer + 4));
  }

  inline u8_t & protocol () { // Next Header
    return buffer[6];
  }
  inline const u8_t & protocol () const { // Next Header
    return buffer[6];
  }

  inline u8_t & ttl () { // Hop Limit
    return buffer[7];
  }
  inline const u8_t & ttl () const { // Hop Limit
    return buffer[7];
  }

  inline IP_Address & source () {
    return *((IP_Address *) (buffer + 8));
  }
  inline const IP_Address & source () const {
    return *((const IP_Address *) (buffer + 8));
  }

  inline IP_Address & destination () {
    return *((IP_Address *) (buffer + 24));
  }
  inline const IP_Address & destination () const {
    return *((const IP_Address *) (buffer + 24));
  }

  /* utility methods
   */

  inline u8_t version () const {
    return 6;
  }

  inline u8_t header_length () const {
    return 40;
  }

  inline void set_total_length (u16_t total) {
    length() = (total > 40) ? (total - 40) : 0;
  }
  inline u16_t total_length () const {
    u16_t payload = length ();
    return payload + 40;
  }

  inline u16_t payload_length () const {
    return length ();
  }

  inline void defaults () {
    clear ();
    set_version ();
    ttl() = IP_TimeToLive;
  }

  inline u8_t protocol_echo_request () const {
    return 128;
  }

  inline u8_t protocol_echo_reply () const {
    return 129;
  }

  inline bool is_IPv6 () const {
    return true;
  }

  inline bool is_ICMP () const {
    return protocol () == 0x3A;
  }
  inline bool is_TCP () const {
    return protocol () == 0x06;
  }
  inline bool is_UDP () const {
    return protocol () == 0x11;
  }

  inline void pseudo_header (Check16 & check) const {
    source().check (check);
    destination().check (check);

    check += length ();
    check += protocol ();
  }
};

struct IP_Header_TCP {
  u8_t buffer[20]; // there may be upto 10 extra header fields, but the first 20 bytes are required

  inline void clear () {
    memset (buffer, 0, 20);
  }

  /* methods for interrogating & editing the header
   */

  inline ns16_t & source () {
    return *((ns16_t *) buffer);
  }
  inline const ns16_t & source () const {
    return *((const ns16_t *) buffer);
  }

  inline ns16_t & destination () {
    return *((ns16_t *) (buffer + 2));
  }
  inline const ns16_t & destination () const {
    return *((const ns16_t *) (buffer + 2));
  }

  /* If the SYN flag is set (1), then this is the initial sequence number. The sequence number of the actual first data byte and the acknowledged
   * number in the corresponding ACK are then this sequence number plus 1. If the SYN flag is clear (0), then this is the accumulated sequence
   * number of the first data byte of this segment for the current session.
   */
  inline ns32_t & seq_no () {
    return *((ns32_t *) (buffer + 4));
  }
  inline const ns32_t & seq_no () const {
    return *((const ns32_t *) (buffer + 4));
  }

  /* If the ACK flag is set then the value of this field is the next sequence number that the sender of the ACK is expecting. This acknowledges
   * receipt of all prior bytes (if any). The first ACK sent by each end acknowledges the other end's initial sequence number itself, but no data.
   */
  inline ns32_t & ack_no () {
    return *((ns32_t *) (buffer + 8));
  }
  inline const ns32_t & ack_no () const {
    return *((const ns32_t *) (buffer + 8));
  }

  /* Data offset: (4 bits) [Specifies the size of the TCP header in 32-bit words.]
   */
  inline u8_t get_data_offset () const {
    return buffer[12] >> 4;
  }
  inline void set_data_offset (u8_t offset) {
    offset = (offset < 5) ? 5 : ((offset <= 15) ? offset : 15); // 5 <= offset <= 15
    buffer[12] = (buffer[12] & 0x0F) | (offset << 4);
  }

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
      buffer[12] = (buffer[12] & 0xF0) | IP_TCP_FLAG_NS;
    else
      buffer[12] &= 0xF0;
  }
  inline bool flag_ns () const {
    return buffer[12] & IP_TCP_FLAG_NS;
  }

  inline void flag_cwr (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_CWR;
    else
      buffer[13] &= ~IP_TCP_FLAG_CWR;
  }
  inline bool flag_cwr () const {
    return buffer[13] & IP_TCP_FLAG_CWR;
  }

  inline void flag_ece (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_ECE;
    else
      buffer[13] &= ~IP_TCP_FLAG_ECE;
  }
  inline bool flag_ece () const {
    return buffer[13] & IP_TCP_FLAG_ECE;
  }

  inline void flag_urg (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_URG;
    else
      buffer[13] &= ~IP_TCP_FLAG_URG;
  }
  inline bool flag_urg () const {
    return buffer[13] & IP_TCP_FLAG_URG;
  }

  inline void flag_ack (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_ACK;
    else
      buffer[13] &= ~IP_TCP_FLAG_ACK;
  }
  inline bool flag_ack () const {
    return buffer[13] & IP_TCP_FLAG_ACK;
  }

  inline void flag_psh (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_PSH;
    else
      buffer[13] &= ~IP_TCP_FLAG_PSH;
  }
  inline bool flag_psh () const {
    return buffer[13] & IP_TCP_FLAG_PSH;
  }

  inline void flag_rst (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_RST;
    else
      buffer[13] &= ~IP_TCP_FLAG_RST;
  }
  inline bool flag_rst () const {
    return buffer[13] & IP_TCP_FLAG_RST;
  }

  inline void flag_syn (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_SYN;
    else
      buffer[13] &= ~IP_TCP_FLAG_SYN;
  }
  inline bool flag_syn () const {
    return buffer[13] & IP_TCP_FLAG_SYN;
  }

  inline void flag_fin (bool b) {
    if (b)
      buffer[13] |=  IP_TCP_FLAG_FIN;
    else
      buffer[13] &= ~IP_TCP_FLAG_FIN;
  }
  inline bool flag_fin () const {
    return buffer[13] & IP_TCP_FLAG_FIN;
  }

  /* The size of the receive window, which specifies the number of window size units (by default, bytes) (beyond the segment identified by the sequence number
   * in the acknowledgment field) that the sender of this segment is currently willing to receive.
   */
  inline ns16_t & window_size () {
    return *((ns16_t *) (buffer + 14));
  }
  inline const ns16_t & window_size () const {
    return *((const ns16_t *) (buffer + 14));
  }

  inline ns16_t & checksum () {
    return *((ns16_t *) (buffer + 16));
  }
  inline const ns16_t & checksum () const {
    return *((const ns16_t *) (buffer + 16));
  }

  /* Urgent pointer: (16 bits) [if the URG flag is set, then this 16-bit field is an offset from the sequence number indicating the last urgent data byte]
   */
  inline ns16_t & urgent () {
    return *((ns16_t *) (buffer + 18));
  }
  inline const ns16_t & urgent () const {
    return *((const ns16_t *) (buffer + 18));
  }

  /* utility methods
   */

  inline u8_t header_length () const {
    return get_data_offset () << 2;
  }

  inline void defaults () {
    clear ();
    set_data_offset (5);
    window_size () = IP_TCP_MaxSegmentSize; // ??
  }

  inline void header (Check16 & check) const {
    const Buffer B((u8_t *) buffer, 20, true /* full buffer */);
    B.check_16 (check,  0, 16);
    B.check_16 (check, 18,  2);
  }
};

struct IP_Header_UDP {
  u8_t buffer[8];

  inline void clear () {
    memset (buffer, 0, 8);
  }

  /* methods for interrogating & editing the header
   */

  inline ns16_t & source () {
    return *((ns16_t *) buffer);
  }
  inline const ns16_t & source () const {
    return *((const ns16_t *) buffer);
  }

  inline ns16_t & destination () {
    return *((ns16_t *) (buffer + 2));
  }
  inline const ns16_t & destination () const {
    return *((const ns16_t *) (buffer + 2));
  }

  inline ns16_t & length () {
    return *((ns16_t *) (buffer + 4));
  }
  inline const ns16_t & length () const {
    return *((const ns16_t *) (buffer + 4));
  }

  inline ns16_t & checksum () {
    return *((ns16_t *) (buffer + 6));
  }
  inline const ns16_t & checksum () const {
    return *((const ns16_t *) (buffer + 6));
  }

  /* utility methods
   */

  inline u8_t header_length () const {
    return 8;
  }

  inline void defaults () {
    clear ();
  }

  inline void header (Check16 & check) const {
    const Buffer B((u8_t *) buffer, 8, true /* full buffer */);
    B.check_16 (check, 0, 6);
  }
};

struct IP_Header_ICMP { // currently we only really support echo request, which we can fit into 12 bytes
  u8_t buffer[12];

  inline void clear () {
    memset (buffer, 0, 12);
  }

  /* methods for interrogating & editing the header
   */

  inline u8_t & type () {
    return buffer[0];
  }
  inline const u8_t & type () const {
    return buffer[0];
  }

  inline u8_t & code () {
    return buffer[1];
  }
  inline const u8_t & code () const {
    return buffer[1];
  }

  inline ns16_t & checksum () {
    return *((ns16_t *) (buffer + 2));
  }
  inline const ns16_t & checksum () const {
    return *((const ns16_t *) (buffer + 2));
  }

  inline ns16_t & id () {
    return *((ns16_t *) (buffer + 4));
  }
  inline const ns16_t & id () const {
    return *((const ns16_t *) (buffer + 4));
  }

  inline ns16_t & seq_no () {
    return *((ns16_t *) (buffer + 6));
  }
  inline const ns16_t & seq_no () const {
    return *((const ns16_t *) (buffer + 6));
  }

  inline ns32_t & payload () {
    return *((ns32_t *) (buffer + 8));
  }
  inline const ns32_t & payload () const {
    return *((const ns32_t *) (buffer + 8));
  }

  /* utility methods
   */

  inline u8_t header_length () const {
    return 12;
  }

  inline void defaults () {
    clear ();
  }

  inline void header (Check16 & check) const {
    const Buffer B((u8_t *) buffer, 12, true /* full buffer */);
    B.check_16 (check, 0, 2);
    B.check_16 (check, 4, 8);
  }
};

#endif /* ! __ip_protocol_hh__ */
