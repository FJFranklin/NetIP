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

/*! \file ip_config.hh
    \brief Defaults and settings that affect NetIP.
    
    Some settings affect network communication and should be kept constant for all devices on the network,
    in particular IP_USE_IPv6 and IP_Buffer_WordCount, but the rest could be (and arguably *should* be)
    moved to device-specific application source folders / sketches.
*/

#ifndef __ip_config_hh__
#define __ip_config_hh__

/* Protocol options
 */
#define IP_USE_IPv6          0 ///< Use IPv6 protocol headers; default is IPv4.

/* Architecture build options
 */
#ifndef IP_ARCH_UNIX
#define IP_ARCH_ARDUINO      1 ///< Build for Arduino-type device (this is the default behaviour).
#define IP_ARCH_UNIX         0 ///< Build for Unix-type device; should be defined on the compile line.
#endif

#if IP_ARCH_UNIX
#define IP_DEBUG             1 ///< Enable debug feedback - potentially very noisy.
#include "unix/ip_arch.hh"
#endif

#if IP_ARCH_ARDUINO
#define IP_DEBUG             0 ///< Enable debug feedback - potentially very noisy.
#include "arduino/ip_arch.hh"
#endif

#define IP_HostID_Default    0x01 ///< One-byte local network ID of the host device
#define IP_GatewayID_Default 0x01 ///< One-byte local network ID of the gateway device

/* (Comma-separated) Default host & gateway (default router) addresses and netmask, for IPv4 and IPv6.
 * .255 should be broadcast; .0 reserved as a network identifier.
 */
#if IP_USE_IPv6
#define IP_Address_DefaultHost    0xfd00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,(0x0500|IP_HostID_Default)
#define IP_Address_DefaultGateway 0xfd00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,(0x0500|IP_GatewayID_Default)
#define IP_Address_DefaultNetmask 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xff00
#else
#define IP_Address_DefaultHost    192,168,  5,IP_HostID_Default    ///< Default host address; for both IPv4 and IPv6, the last byte (1-254) identifies the device on NetIP's local network.
#define IP_Address_DefaultGateway 192,168,  5,IP_GatewayID_Default ///< Default gateway address; attempts to connect to external addresses will route to the gateway device.
#define IP_Address_DefaultNetmask 255,255,255,0                    ///< Network mask.
#endif

/* Parameters affecting memory use. A high IP_Buffer_Extras increases performance but significantly impacts memory use.
 */
#define IP_Buffer_WordCount  64   ///< Buffer size in (2-byte) words; one included per channel - affects TCP/IP data size.
#define IP_Buffer_Extras      2   ///< The number of extra buffers (1 minimum) to include to increase flexibility and responsiveness.
#define IP_Connection_FIFO   32   ///< Size of FIFO in bytes; there are two FIFO per connection.

/* Other network parameters.
 */
#define IP_TimeToLive        64   ///< the hop count / time to live of IP packets; not actually relevant to NetIP's local network.

#endif /* ! __ip_config_hh__ */
