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

#ifndef __ip_address_hh__
#define __ip_address_hh__

#include "ip_defines.hh"
#include "ip_types.hh"

class IP_Address {
private:
  ns16_t address[IP_Address_WordCount];
public:
  inline const u8_t * byte_buffer () const {
    return (u8_t *) address;
  }
  inline u8_t byte_length () const {
    return IP_Address_WordCount << 1;
  }

#if IP_USE_IPv6
  inline ns16_t & operator[] (int i) {
    return address[i];
  }
#else
  inline u8_t & operator[] (int i) {
    return address[0x01 & (((u8_t) i) >> 1)][i];
  }
  inline const u8_t & operator[] (int i) const {
    return address[0x01 & (((u8_t) i) >> 1)][i];
  }
#endif

  inline IP_Address & operator= (const IP_Address & rhs) {
    memcpy (address, rhs.address, IP_Address_WordCount << 1);
    return *this;
  }

  inline bool compare (const IP_Address & rhs) const {
    return !memcmp (address, rhs.address, IP_Address_WordCount << 1);
  }

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

  inline bool compare_all (const ns16_t & rhs) const {
    bool bMatch = true;
    for (u8_t i = 0; i < IP_Address_WordCount; i++) {
      if (address[i] != rhs) {
	bMatch = false;
	break;
      }
    }
    return bMatch;
  }
  inline bool is_zeros () const {
    return compare_all (0x0000);
  }
  inline bool is_ones () const {
    return compare_all (0xFFFF);
  }

  inline void check (Check16 & C) const {
    for (u8_t i = 0; i < IP_Address_WordCount; i++) {
      C += address[i];
    }
  }

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

  IP_Address () {
    // ...
  }

  ~IP_Address () {
    // ...
  }

  inline void set_local_network_id (u8_t id) {
    address[IP_Address_WordCount-1][1] = id; // final byte of address; must be local-network unique in range 1-254
  }

  inline u8_t local_network_id () const {
    return address[IP_Address_WordCount-1][1]; // final byte of address; must be local-network unique
  }
};

inline bool operator== (const IP_Address & lhs, const IP_Address & rhs) {
  return lhs.compare (rhs);
}

inline bool operator!= (const IP_Address & lhs, const IP_Address & rhs) {
  return !lhs.compare (rhs);
}

#endif /* ! __ip_address_hh__ */
