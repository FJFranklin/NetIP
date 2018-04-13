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

/*! \file ip_address.hh
    \brief A class representing an internet address.
    
    IPv6 addresses are 16 bytes compared to 4 bytes for an IPv4 address. IPv6 protocol headers are 40 bytes
    vs 20 bytes for IPv4. Support for IPv6 has a significant impact on memory requirements, which is critical
    for devices such as the Arduino Uno. For this reason, there is support for either IPv4 or IPv6, but not
    really both. The IP_Address class is defined for one or other, but not both. Perhaps this could/should be
    rewritten as two distinct classes.
*/

#ifndef __ip_address_hh__
#define __ip_address_hh__

#include "ip_defines.hh"
#include "ip_types.hh"

/** A class for storing and manipulating IP addresses.
 * IPv6 addresses are 16 bytes compared to 4 bytes for an IPv4 address. IPv6 protocol headers are 40 bytes
 * vs 20 bytes for IPv4. Support for IPv6 has a significant impact on memory requirements, which is critical
 * for devices such as the Arduino Uno. For this reason, there is support for either IPv4 or IPv6, but not
 * really both. The IP_Address class is defined for one or other, but not both. Perhaps this could/should be
 * rewritten as two distinct classes.
 */
class IP_Address {
private:
  ns16_t address[IP_Address_WordCount]; ///< byte buffer (4 bytes for IPv4; 16 bytes for IPv6)
public:
  /** Pointer to the internal byte buffer containing the IP address. 
   */
  inline const u8_t * byte_buffer () const {
    return (u8_t *) address;
  }

  /** Number of bytes in the internal byte buffer containing the IP address. 
   */
  inline u8_t byte_length () const {
    return IP_Address_WordCount << 1;
  }

#if IP_USE_IPv6
  /** For IPv6, IP_Address[n] is a reference to the nth two-byte word of the eight two-byte address.
   */
  inline ns16_t & operator[] (int i) {
    return address[i];
  }
#else
  /** For IPv4, IP_Address[n] is a reference to the nth byte of the four-byte address.
   */
  inline u8_t & operator[] (int i) {
    return address[0x01 & (((u8_t) i) >> 1)][i];
  }

  /** For IPv4, IP_Address[n] is a (constant) reference to the nth byte of the four-byte address.
   */
  inline const u8_t & operator[] (int i) const {
    return address[0x01 & (((u8_t) i) >> 1)][i];
  }
#endif

  /** Assignment operator.
   */
  inline IP_Address & operator= (const IP_Address & rhs) {
    memcpy (address, rhs.address, IP_Address_WordCount << 1);
    return *this;
  }

  /** Equality comparison of two IP_Address objects.
   */
  inline bool operator== (const IP_Address & rhs) const {
    return !memcmp (address, rhs.address, IP_Address_WordCount << 1);
  }

  /** Inequality comparison of two IP_Address objects.
   */
  inline bool operator!= (const IP_Address & rhs) const {
    return memcmp (address, rhs.address, IP_Address_WordCount << 1);
  }

  /** Masked equality comparison of two IP_Address objects, i.e., do the addresses match wherever the mask bit is 1.
   * \param rhs  The address to compare with.
   * \param mask The address to use as a bitmask.
   * \return True if the addresses match wherever the mask bit is 1.
   */
  inline bool compare (const IP_Address & rhs, const IP_Address & mask) const {
    bool bMatch = true;
    for (u8_t i = 0; i < IP_Address_WordCount; i++) {
      if (((u16_t) address[i] & (u16_t) mask.address[i]) != ((u16_t) rhs.address[i] & (u16_t) mask.address[i])) {
	bMatch = false;
	break;
      }
    }
    return bMatch;
  }

  /** Add the address to a checksum sequence. (This doesn't clear the Check16 object or calculate the checksum.)
   * \param C  The Check16 object being used to calculate the checksum.
   */
  inline void check (Check16 & C) const {
    for (u8_t i = 0; i < IP_Address_WordCount; i++) {
      C += address[i];
    }
  }

  /** For IPv6, IP_Address::set() will set the address with eight two-byte words, e.g., set (0xfe00, 0x1234, ...).
   * For IPv4, IP_Address::set() will set the address with four bytes, e.g., set (192, 168, 5, 1).
   */
  inline void set (
#if IP_USE_IPv6
		   u16_t addr_0, u16_t addr_1, u16_t addr_2, u16_t addr_3,
		   u16_t addr_4, u16_t addr_5, u16_t addr_6, u16_t addr_7
#else
		   u8_t addr_0, u8_t addr_1, u8_t addr_2, u8_t addr_3
#endif
		   ) {
    (*this)[0] = addr_0;
    (*this)[1] = addr_1;
    (*this)[2] = addr_2;
    (*this)[3] = addr_3;
#if IP_USE_IPv6
    (*this)[4] = addr_4;
    (*this)[5] = addr_5;
    (*this)[6] = addr_6;
    (*this)[7] = addr_7;
#endif
  }

  /** For IPv6, the constructor can set the address with eight two-byte words, e.g., IP_Address(0xfe00, 0x1234, ...).
   * For IPv4, the constructor can set the address with four bytes, e.g., IP_Address(192, 168, 5, 1).
   */
  IP_Address (
#if IP_USE_IPv6
	      u16_t addr_0, u16_t addr_1, u16_t addr_2, u16_t addr_3,
	      u16_t addr_4, u16_t addr_5, u16_t addr_6, u16_t addr_7
#else
	      u8_t addr_0, u8_t addr_1, u8_t addr_2, u8_t addr_3
#endif
	      ) {
    (*this)[0] = addr_0;
    (*this)[1] = addr_1;
    (*this)[2] = addr_2;
    (*this)[3] = addr_3;
#if IP_USE_IPv6
    (*this)[4] = addr_4;
    (*this)[5] = addr_5;
    (*this)[6] = addr_6;
    (*this)[7] = addr_7;
#endif
  }

  /** The default constructor - does not set a default address.
   */
  IP_Address () {
    // ...
  }

  ~IP_Address () {
    // ...
  }

  /** Set the last byte of the address. NetIP uses the last byte of the address (value in range 1-254) to identify devices in the local network.
   */
  inline void set_local_network_id (u8_t id) {
    address[IP_Address_WordCount-1][1] = id; // final byte of address; must be local-network unique in range 1-254
  }

  /** Get the last byte of the address. NetIP uses the last byte of the address (value in range 1-254) to identify devices in the local network.
   */
  inline u8_t local_network_id () const {
    return address[IP_Address_WordCount-1][1]; // final byte of address; must be local-network unique
  }
};

#endif /* ! __ip_address_hh__ */
