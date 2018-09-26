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

#ifndef __BBox_hh__
#define __BBox_hh__

namespace PyCCar {

  class BBox {
  public:
    unsigned m_x;
    unsigned m_y;
    unsigned m_width;
    unsigned m_height;

    BBox () :
      m_x(0),
      m_y(0),
      m_width(0),
      m_height(0)
    {
      // ...
    }

    BBox (unsigned x, unsigned y, unsigned width, unsigned height) :
      m_x(x),
      m_y(y),
      m_width(width),
      m_height(height)
    {
      // ...
    }

    ~BBox () {
      // ...
    }

    inline void clear () {
      m_x = 0;
      m_y = 0;
      m_width  = 0;
      m_height = 0;
    }

    inline bool exists () const {
      return m_width && m_height;
    }

    inline bool intersects (const BBox & rhs) const {
      return ((m_x < rhs.m_x + rhs.m_width)  && (rhs.m_x < m_x + m_width) &&
	      (m_y < rhs.m_y + rhs.m_height) && (rhs.m_y < m_y + m_height));
    }

    inline bool includes (unsigned x, unsigned y) const {
      return ((m_x <= x) && (x < m_x + m_width) &&
	      (m_y <= y) && (y < m_y + m_height));
    }

    inline void combine (const BBox & rhs) {
      if (exists () && rhs.exists ()) {
	unsigned x2_lhs = m_x + m_width;
	unsigned y2_lhs = m_y + m_height;

	unsigned x2_rhs = rhs.m_x + rhs.m_width;
	unsigned y2_rhs = rhs.m_y + rhs.m_height;

	m_x = (m_x < rhs.m_x) ? m_x : rhs.m_x;
	m_y = (m_y < rhs.m_y) ? m_y : rhs.m_y;

	m_width  = ((x2_rhs > x2_lhs) ? x2_rhs : x2_lhs) - m_x;
	m_height = ((y2_rhs > y2_lhs) ? y2_rhs : y2_lhs) - m_y;
      } else if (rhs.exists ()) {
	*this = rhs;
      }
    }
  };

} // namespace PyCCar

#endif /* ! __BBox_hh__ */
