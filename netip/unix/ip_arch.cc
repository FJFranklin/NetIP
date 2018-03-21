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

#include <time.h>
#include <unistd.h>

#include "ip_arch.hh"

static u32_t bInit = false;

static u32_t time_secs;
static u32_t time_nano;

u32_t ip_arch_millis () {
  u32_t us = 0;

  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  u32_t new_time_secs = (u32_t) ts.tv_sec;
  u32_t new_time_nano = (u32_t) ts.tv_nsec;

  if (!bInit) {
    time_secs = new_time_secs;
    time_nano = new_time_nano;
    bInit = true;
  } else {
    if (new_time_nano < time_nano) {
      us  = (1000000000UL + new_time_nano - time_nano) / 1000000UL;
      us += 1000UL * (new_time_secs - time_secs - 1);
    } else {
      us  = (new_time_nano - time_nano) / 1000000UL;
      us += 1000UL * (new_time_secs - time_secs);
    }
  }
  return us;
}

void ip_arch_usleep (u16_t us) {
  usleep (us);
}
