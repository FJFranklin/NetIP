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

#ifndef __ip_defines_hh__
#define __ip_defines_hh__

#include "ip_config.hh"

#define IP_Buffer_SafeSize    (IP_Buffer_WordCount * 2 - 2) // UIP_BUFSIZE // uip_buf: The packet buffer that contains incoming packets. // Why -2?

/* Header sizes. */
#define IP_Header_Length_IPv6 40
#define IP_Header_Length_IPv4 20
#define IP_Header_Length_UDP   8
#define IP_Header_Length_TCP  20

/* Different header types for IPv4 & IPv6
 */
#if IP_USE_IPv6
#define IP_Address_WordCount   8
#define IP_Header_Length_IP    IP_Header_Length_IPv6
#else
#define IP_Address_WordCount   2
#define IP_Header_Length_IP    IP_Header_Length_IPv4
#endif

/* TCP (or UDP) is framed within IP...
 */
#define IP_Header_UDP_IP       (IP_Header_Length_IP + IP_Header_Length_UDP)
#define IP_Header_TCP_IP       (IP_Header_Length_IP + IP_Header_Length_TCP)

/* Maximum data length for TCP/IP connections
 */
#define IP_TCP_MaxSegmentSize  (IP_Buffer_SafeSize - IP_Header_TCP_IP)

/*
 * ======== uIP definitions ========
 */

/* TCP/IP connection status
 */
#define UIP_CLOSED        0
#define UIP_SYN_RCVD      1
#define UIP_SYN_SENT      2
#define UIP_ESTABLISHED   3
#define UIP_FIN_WAIT_1    4
#define UIP_FIN_WAIT_2    5
#define UIP_CLOSING       6
#define UIP_TIME_WAIT     7
#define UIP_LAST_ACK      8
#define UIP_TS_MASK      15
#define UIP_STOPPED      16

/**
 * The initial retransmission timeout counted in timer pulses.
 *
 * This should not be changed.
 */
#define UIP_RTO           3

#endif /* ! __ip_defines_hh__ */
