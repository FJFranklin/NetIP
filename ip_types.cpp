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

#include "netip/ip_types.hh"

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
