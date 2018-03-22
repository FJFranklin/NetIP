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

#include "netip/ip_timer.hh"

void IP_Timer::start (IP_Clock & clock, u32_t interval) {
  timer_target   = interval + clock.milliseconds ();
  timer_interval = interval;

  clock.timer_add (this);
}

bool IP_Clock::timer_checks (u32_t current_time) {
  Chain<IP_Timer>::iterator I = chain_timers.begin ();

  while (*I) {
    if ((*I)->check (current_time)) { // the timeout was called, and is no longer needed
      chain_timers.chain_remove (*I); // we just broke the chain; it's not safe to continue...
      break;
    }
    ++I;
  }
  return !*I;
}

void IP_Clock::run () {
  while (!bStop) {
    u32_t time = milliseconds ();  // just call this the once

    if (last_timer_check < time) { // time is in milliseconds; timer_checks() should be called once a millisecond
      while (!timer_checks (time));
      last_timer_check = time;     // finished time checks; note current time

      every_millisecond ();
    }

    if (time - last_timer_second > 999) {
      last_timer_second += 1000;

      every_second ();
    }

    tick ();

    ip_arch_usleep (1);
  }
}
