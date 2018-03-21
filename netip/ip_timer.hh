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

#ifndef __ip_timer_hh__
#define __ip_timer_hh__

#include "ip_types.hh"

class IP_Clock;
class IP_Timer;

class IP_TimerClient {
public:
  virtual bool timeout () = 0; // return true if the timer should be reset & retained

  virtual ~IP_TimerClient () {
    //
  }
};

class IP_Timer : public Link {
private:
  IP_TimerClient * timer_client;

  u32_t timer_target;
  u32_t timer_interval;

public:
  IP_Timer (IP_TimerClient * client) :
    timer_client(client),
    timer_target(0),
    timer_interval(0)
  {
    // ...
  }

  ~IP_Timer () {
    // ...
  }

  void start (IP_Clock & clock, u32_t interval);

  inline bool check (u32_t current_time) { // return true if we're finished with the timer
    bool bDetachTimer = false;

    if (timer_target < current_time) { // trigger the callback
      if (timer_client->timeout ()) {  // returned true, so reset the timer
	timer_target += timer_interval;
      } else {                         // returned false; need to detach the timer - return true
	bDetachTimer = true;
      }
    }
    return bDetachTimer;
  }
};

class IP_Clock {
private:
  Chain<IP_Timer>  chain_timers;

  u32_t last_timer_check; // very little point in checking the timers if the time hasn't changed

  void timer_checks ();

  bool bStop;

public:
  inline void timer_add (IP_Timer * timer) {
    chain_timers.chain_prepend (timer);
  }

  inline u32_t milliseconds () {
    return ip_arch_millis ();
  }

  IP_Clock () :
    bStop(false)
  {
    last_timer_check = milliseconds ();
  }

  virtual ~IP_Clock () {
    // ...
  }

  virtual void tick () {
    timer_checks ();
  }

  inline void stop () {
    bStop = true;
  }

  inline void run () {
    while (!bStop) {
      tick ();
      ip_arch_usleep (1);
    }
  }
};

#endif /* ! __ip_timer_hh__ */
