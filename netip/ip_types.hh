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

#ifndef __ip_types_hh__
#define __ip_types_hh__

#include "ip_config.hh"

class BufferIterator {
private:
  const u8_t * ptr;
  const u8_t * end;

public:
  BufferIterator (const u8_t * buffer, u16_t length) :
    ptr(buffer),
    end(buffer + length)
  {
    // ...
  }

  ~BufferIterator () {
    // ...
  }

  inline u16_t remaining () const {
    return (u16_t) (end - ptr);
  }

  inline const u8_t * operator* () const {
    return ptr;
  }

  inline BufferIterator & operator++ () {
    ++ptr;
    return *this;
  }

  inline BufferIterator & operator+= (u16_t count) {
    ptr += count;

    if (ptr > end) {
      ptr = end;
    }
    return *this;
  }
};

class ns16_t {
private:
  u8_t byte[2];

public:
  inline ns16_t & operator= (const BufferIterator & I) {
    memcpy (byte, *I, 2);
    return *this;
  }

  inline u8_t & operator[] (int i) {
    return byte[0x01 & (u8_t) i];
  }
  inline const u8_t & operator[] (int i) const {
    return byte[0x01 & (u8_t) i];
  }

  inline ns16_t & operator= (u16_t i) {
    byte[0] = 0xFF & (i >> 8);
    byte[1] = 0xFF & i;
    return *this;
  }

  inline operator u16_t () const {
    return (((u16_t) byte[0]) << 8) | ((u16_t) byte[1]);
  }

  inline bool compare (ns16_t i) const {
    return *((u16_t *) byte) == *((u16_t *) i.byte);
  }
  inline bool compare (u16_t i) const {
    return (byte[0] == (u8_t) ((i >> 8) & 0xFF)) && (byte[1] == (u8_t) (i & 0xFF));
  }

  static inline ns16_t convert (u16_t i) {
    ns16_t c;
    c = i;
    return c;
  }
};

inline bool operator== (const ns16_t & lhs, const ns16_t & rhs) {
  return lhs.compare (rhs);
}

inline bool operator== (const ns16_t & lhs, const u16_t & rhs) {
  return lhs.compare (rhs);
}

inline bool operator== (const u16_t & lhs, const ns16_t & rhs) {
  return rhs.compare (lhs);
}

inline bool operator!= (const ns16_t & lhs, const ns16_t & rhs) {
  return !lhs.compare (rhs);
}

inline bool operator!= (const ns16_t & lhs, const u16_t & rhs) {
  return !lhs.compare (rhs);
}

inline bool operator!= (const u16_t & lhs, const ns16_t & rhs) {
  return !rhs.compare (lhs);
}

class ns32_t {
private:
  union {
    u8_t   byte[4];
    ns16_t sword[2];
    u32_t  lword;
  } u;

public:
  inline ns32_t & operator= (const BufferIterator & I) {
    memcpy (u.byte, *I, 4);
    return *this;
  }

  inline u8_t & operator[] (u8_t i) {
    return u.byte[i & 0x03];
  }
  inline const u8_t & operator[] (u8_t i) const {
    return u.byte[i & 0x03];
  }

  inline ns16_t & hi () {
    return u.sword[0];
  }
  inline const ns16_t & hi () const {
    return u.sword[0];
  }

  inline ns16_t & lo () {
    return u.sword[1];
  }
  inline const ns16_t & lo () const {
    return u.sword[1];
  }

  ns32_t () {
    u.lword = 0;
  }

  ns32_t (int i) {
    *this = (u32_t) i;
  }

  ~ns32_t () {
  }

  inline ns32_t & operator= (u32_t i) {
    u.byte[3] =  i & 0xFF;
    i >>= 8;
    u.byte[2] =  i & 0xFF;
    i >>= 8;
    u.byte[1] =  i & 0xFF;
    i >>= 8;
    u.byte[0] =  i;
    return *this;
  }

  inline operator u32_t () const {
    return (((((((u32_t) u.byte[0]) << 8) | ((u32_t) u.byte[1])) << 8) | ((u32_t) u.byte[2])) << 8) | ((u32_t) u.byte[3]);
  }

  inline ns32_t & operator++ () {
    if (!++u.byte[3])
      if (!++u.byte[2])
	if (!++u.byte[1])
	  ++u.byte[0]; // carry loss

    return *this;
  }

  inline ns32_t & operator+= (const ns32_t & rhs) {
    u16_t sum = (u16_t) u.byte[3] + (u16_t) rhs.u.byte[3];
    u.byte[3] = sum & 0xFF;
    sum = (sum >> 8) + (u16_t) u.byte[2] + (u16_t) rhs.u.byte[2];
    u.byte[2] = sum & 0xFF;
    sum = (sum >> 8) + (u16_t) u.byte[1] + (u16_t) rhs.u.byte[1];
    u.byte[1] = sum & 0xFF;
    sum = (sum >> 8) + (u16_t) u.byte[0] + (u16_t) rhs.u.byte[0];
    u.byte[0] = sum & 0xFF; // carry loss

    return *this;
  }

  inline bool operator== (const ns32_t & rhs) const {
    return u.lword == rhs.u.lword;
  }

  inline bool operator< (const ns32_t & rhs) const {
    return ((u.byte[0] < rhs.u.byte[0]) || ((u.byte[0] == rhs.u.byte[0]) &&
					    ((u.byte[1] < rhs.u.byte[1]) || ((u.byte[1] == rhs.u.byte[1]) &&
									     ((u.byte[2] < rhs.u.byte[2]) || ((u.byte[2] == rhs.u.byte[2]) &&
													      (u.byte[3] < rhs.u.byte[3])))))));
  }
};

class Link {
public:
  Link * link_next;

  Link () :
    link_next(0)
  {
    // ...
  }

  virtual ~Link () {
    // ...
  }

  bool in_chain (const Link * link_first) const;

  static void remove_from_chain (Link *& link_first, Link *& link_last, Link * link);
};

template<class T>
class Chain {
public:
  class iterator {
  private:
    Link * link;

  public:
    iterator (Link * start) :
      link(start)
    {
      // ...
    }

    ~iterator () {
      // ...
    }

    inline iterator & operator++ () {
      if (link) {
	link = link->link_next;
      }
      return *this;
    }

    inline T * operator* () {
      return static_cast<T *>(link);
    }
  };

private:
  Link * link_first;
  Link * link_last;

public:
  inline iterator begin () {
    return iterator(link_first);
  }

  inline void chain_prepend (T * link) {
    if (!link->in_chain (link_first)) {
      link->link_next = link_first;
      link_first = link;

      if (!link_last) {
	link_last = link;
      }
    }
  }

  inline void chain_append (T * link) {
    if (!link->in_chain (link_first)) {
      if (!link_last) {
	link_first = link;
      } else {
	link_last->link_next = link;
      }
      link_last = link;
      link_last->link_next = 0;
    }
  }

  inline void chain_remove (Link * link) {
    Link::remove_from_chain (link_first, link_last, link);
  }

  inline T * chain_first (bool bRemove = false) {
    Link * first = link_first;

    if (bRemove && first) {
      chain_remove (first);
    }
    return static_cast<T *>(first);
  }

  inline void chain_push (T * link, bool bFIFO = false) {
    if (bFIFO) {
      chain_append (link); // first in, first out
    } else {
      chain_prepend (link); // last in, first out - default to stack behaviour
    }
  }

  inline T * chain_pop () {
    return chain_first (true);
  }

  Chain () :
    link_first(0),
    link_last(0)
  {
    // ...
  }

  ~Chain () {
    // ...
  }
};

class Check16 {
private:
  u32_t sum;
public:
  Check16 () :
    sum(0)
  {
    // ...
  }

  ~Check16 () {
    // ...
  }

  inline void clear () {
    sum = 0;
  }

  inline Check16 & operator+= (u16_t rhs) {
    sum += rhs;
    return *this;
  }

  inline u16_t checksum () {
    /* Fold to get the ones-complement result */
    while (sum >> 16) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    /* Invert to get the negative in ones-complement arithmetic */
    return ~((u16_t) sum);
  }
};

class FIFO {
private:
  u8_t * buffer_start;
  u8_t * buffer_end;
  u8_t * data_start; // if start == end, then no data
  u8_t * data_end;   // must point to a writable byte

public:
  inline void clear () {
    data_start = buffer_start;
    data_end   = buffer_start;
  }

  inline bool push (u8_t byte) {
    bool bCanPush = true;

    if (data_start < data_end) {
      if ((data_start == buffer_start) && (data_end + 1 == buffer_end)) {
	bCanPush = false;
      }
    } else if (data_start > data_end) {
      if (data_end + 1 == data_start) {
	bCanPush = false;
      }
    } // else (data_start == data_end) // buffer must be empty

    if (bCanPush) {
      *data_end = byte;

      if (++data_end == buffer_end) {
	data_end = buffer_start;
      }
    }
    return bCanPush;
  }

  inline bool pop (u8_t & byte) {
    if (data_start == data_end) { // buffer must be empty
      return false;
    }
    byte = *data_start;

    if (++data_start == buffer_end) {
      data_start = buffer_start;
    }
    return true;
  }

  FIFO (u8_t * byte_buffer, u16_t capacity) :
    buffer_start(byte_buffer),
    buffer_end(byte_buffer+capacity),
    data_start(byte_buffer),
    data_end(byte_buffer)
  {
    // ...
  }

  ~FIFO () {
    // ...
  }

  u16_t read (u8_t * ptr, u16_t length);

  u16_t write (const u8_t * ptr, u16_t length);
};

class Buffer {
private:
  static ns32_t fake_word;

  u8_t * buffer;

  u16_t  buffer_max; // maximum length
  u16_t  buffer_used;

  u8_t   ref_count;

public:
  inline const u8_t * bytes () const {
    return buffer;
  }
  inline u16_t length () const {
    return buffer_used;
  }
  inline u16_t available () const {
    return buffer_max - buffer_used;
  }

  inline u8_t & operator[] (u16_t index) {
    if (index < buffer_max) {
      if (buffer_used < (index + 1)) {
	buffer_used = index + 1;
      }
      return buffer[index];
    }

    // bad indexing!
    return fake_word[0];
  }

  inline ns16_t & ns16 (u16_t index) {
    if ((index + 1) < buffer_max) {
      if (buffer_used < (index + 2)) {
	buffer_used = index + 2;
      }
      return *((ns16_t *) (buffer + index));
    }

    // bad indexing!
    return fake_word.hi ();
  }

  inline ns32_t & ns32 (u16_t index) {
    if ((index + 3) < buffer_max) {
      if (buffer_used < (index + 4)) {
	buffer_used = index + 4;
      }
      return *((ns32_t *) (buffer + index));
    }

    // bad indexing!
    return fake_word;
  }

  inline void clear () {
    buffer_used = 0;
  }

  inline void ref () {
    ++ref_count;
  }
  inline u8_t unref () {
    return --ref_count;
  }

  Buffer (u8_t * byte_buffer, u16_t capacity) :
    buffer(byte_buffer),
    buffer_max(capacity),
    buffer_used(0),
    ref_count(0)
  {
    // ...
  }

  virtual ~Buffer () {
    // ...
  }

  inline u16_t append (const u8_t * ptr, u16_t length) {
    u16_t count = 0;

    if (ptr && length) {
      count = (buffer_used + length <= buffer_max) ? length : (buffer_max - buffer_used);
      memcpy (buffer + buffer_used, ptr, count);

      if (buffer_used + count <= buffer_max) {
	buffer_used += count;
      }     
    }
    return count;
  }

  inline u16_t write (u16_t offset, const u8_t * ptr, u16_t length) {
    u16_t count = 0;

    if (ptr && length) {
      count = (offset + length <= buffer_max) ? length : (buffer_max - offset);
      memcpy (buffer + offset, ptr, count);

      if (buffer_used < offset + count) {
	buffer_used = offset + count;
      }
    }
    return count;
  }

  inline u16_t read (u16_t offset, u8_t * ptr, u16_t length) const {
    u16_t count = 0;

    if (ptr && length) {
      count = (offset + length <= buffer_used) ? length : (buffer_used - offset);
      memcpy (ptr, buffer + offset, count);
    }
    return count;
  }

  inline void check_16 (Check16 & check, u16_t offset, u16_t length = 0) const { // where 0 = 'to the end'
    if (offset < buffer_used) {
      if ((!length) || (length > buffer_used - offset)) {
	length = buffer_used - offset;
      }

      const u8_t * ptr = buffer + offset;

      while (length > 1) {
	u16_t next = *ptr++ << 8;
	next |= *ptr++;
	check += next;
	--length;
	--length;
      }
      if (length) {
	u16_t next = *ptr << 8;
	check += next;
      }
    }
  }
};

#endif /* ! __ip_types_hh__ */
