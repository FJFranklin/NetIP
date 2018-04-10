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

/*! \file ip_types.cpp
    \brief The more fundamental classes used within NetIP
    
    Although types are mostly defined as inline, the more complex and loopy bits are defined
    here instead.
*/

#include "netip/ip_types.hh"

ns32_t Buffer::fake_word; // static member variable

/** Returns true if this link is in the specified chain of links.
 * \param link The first link in the chain to search through.
 * \return True if found.
 */
bool Link::in_chain (const Link * link) const {
  bool bMatched = false;

  while (link) {
    if (link == this) {
      bMatched = true;
      break;
    }
    link = link->link_next;
  }
  return bMatched;
}

/** Remove a link from a specified chain of links, updating the first and last links if necessary.
 * \param link_first The first link in the chain to search through.
 * \param link_last  The last link in the chain to search through.
 * \param link The link to be removed.
 */
void Link::remove_from_chain (Link *& link_first, Link *& link_last, Link * link) {
  if (link && link_first) {
    if (link_first == link) {
      link_first = link_first->link_next;
      link->link_next = 0;

      if (!link_first) {
	link_last = 0;
      }
    } else {
      Link * L = link_first;

      while (true) {
	Link * N = L->link_next;

	if (!N) {
	  break;
	}
	if (N == link) {
	  L->link_next = N->link_next;
	  link->link_next = 0;

	  if (!L->link_next) {
	    link_last = L;
	  }
	  break;
	}
	L = N;
      }
    }
  }
}

/** Read (and remove) multiple bytes from the buffer.
 * \param ptr    Pointer to an external byte array where the data should be written.
 * \param length Number of bytes to read from the buffer, if possible.
 * \return The number of bytes actually read from the buffer.
 */
u16_t FIFO::read (u8_t * ptr, u16_t length) {
  u16_t count = 0;

  if (ptr && length) {
    if (data_end > data_start) {

      count = data_end - data_start;             // i.e., bytes in FIFO
      count = (count > length) ? length : count; // or length, if less

      memcpy (ptr, data_start, count);
      data_start += count;

    } else if (data_end < data_start) {

      count = buffer_end - data_start;           // i.e., bytes in FIFO *at the end*
      count = (count > length) ? length : count; // or length, if less

      memcpy (ptr, data_start, count);
      data_start += count;

      if (data_start == buffer_end) { // wrap-around
	data_start = buffer_start;

	length -= count;                         // how much we still want to read

	u16_t extra = data_end - data_start;     // i.e., bytes in FIFO

	if (length && extra) {                       // we can read more...
	  extra = (extra > length) ? length : extra; // or length, if less

	  memcpy (ptr, data_start, extra);
	  data_start += extra;

	  count += extra;
	}
      }

    } // else (data_end == data_start) => FIFO is empty
  }
  return count;
}

/** Write multiple bytes to the buffer.
 * \param ptr    Pointer to an external byte array where the data should be read from.
 * \param length Number of bytes to write to the buffer, if possible.
 * \return The number of bytes actually written to the buffer.
 */
u16_t FIFO::write (const u8_t * ptr, u16_t length) {
  u16_t count = 0;

  if (ptr && length) {
    if (data_end > data_start) {

      /* this is where we need to worry about wrap-around
       */

      if (data_start == buffer_start) { // we're *not* able to wrap-around

	count = buffer_end - data_end - 1;         // i.e., usable free space in FIFO *at the end*
	count = (count > length) ? length : count; // or length, if less

	memcpy (data_end, ptr, count);
	data_end += count;

      } else { // we *are* able to wrap-around

	count = buffer_end - data_end;             // i.e., usable free space in FIFO *at the end*
	count = (count > length) ? length : count; // or length, if less

	memcpy (data_end, ptr, count);
	data_end += count;

	if (data_end == buffer_end) { // wrap-around
	  data_end = buffer_start;

	  length -= count;                             // how much we still want to write

	  u16_t extra = data_start - data_end - 1;     // i.e., usable free space in FIFO

	  if (length && extra) {                       // we can write more...
	    extra = (extra > length) ? length : extra; // or length, if less

	    memcpy (data_end, ptr, extra);
	    data_end += extra;

	    count += extra;
	  }
	}

      }

    } else if (data_end < data_start) {

      count = data_start - data_end - 1;         // i.e., usable free space in FIFO
      count = (count > length) ? length : count; // or length, if less

      /* don't need to worry about wrap-around
       */
      memcpy (data_end, ptr, count);
      data_end += count;

    } else { // (data_end == data_start)

      /* the FIFO is empty - we can move the pointers for convenience
       */
      data_start = buffer_start;
      data_end   = buffer_start;

      count = buffer_end - buffer_start - 1;     // i.e., maximum number of bytes the FIFO can hold
      count = (count > length) ? length : count; // or length, if less

      /* don't need to worry about wrap-around
       */
      memcpy (data_end, ptr, count);
      data_end += count;

    }
  }
  return count;
}
