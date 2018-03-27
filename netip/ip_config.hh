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

#ifndef __ip_config_hh__
#define __ip_config_hh__

/* Protocol options
 */
#define IP_USE_IPv6 0

/* Architecture build options
 */
#ifndef IP_ARCH_UNIX
#define IP_ARCH_ARDUINO      1
#define IP_ARCH_UNIX         0
#endif

#if IP_ARCH_UNIX
#define IP_DEBUG    1
#include "unix/ip_arch.hh"
#endif
#if IP_ARCH_ARDUINO
#define IP_DEBUG    0
#include "arduino/ip_arch.hh"
#endif

/* (comma-separated) default host & gateway (default router) addresses and netmask
 */
#if IP_USE_IPv6
#define IP_Address_DefaultHost    0xfd00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0501
#define IP_Address_DefaultGateway 0xfd00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x05fe
#define IP_Address_DefaultNetmask 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xff00
#else
#define IP_Address_DefaultHost    192,168,  5,  1
#define IP_Address_DefaultGateway 192,168,  5,  1 // .255 should be broadcast; .0 reserved as a network identifier;
#define IP_Address_DefaultNetmask 255,255,255,  0
#endif

/* parameters affecting memory use
 */
#define IP_Buffer_WordCount  64   // buffer size in (2-byte) words; one included per channel - affects TCP/IP data size
#define IP_Buffer_Extras      1   // how many extra buffers (1 minimum) to use to increase flexibility and responsiveness
#define IP_Connection_FIFO   32   // size of FIFO in bytes per connection

#endif /* ! __ip_config_hh__ */
