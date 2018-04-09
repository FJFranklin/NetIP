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

/*! \file ip_timer.hh
    \brief Class definitions for IP_Clock and IP_Timer.
    
    Clock and timer-related class implementations. IP_Timer is used for one-off or periodic callbacks with
    roughly millisecond precision. IP_Clock cycles indefinitely, checking the time and calling IP_Timer
    callbacks as and when required; there should only be one IP_Clock instance running per application.
*/

#ifndef __ip_timer_hh__
#define __ip_timer_hh__

#include "ip_types.hh"

class IP_Clock;
class IP_Timer;

/** Virtual interface defining callback for IP_Timer.
 */
class IP_TimerClient {
public:
  virtual bool timeout () = 0; ///< Callback function; returns true if the timer should be reset & retained.

  virtual ~IP_TimerClient () {
    //
  }
};

/** Any number of IP_Timer instances may be registered with an IP_Clock. Once the clock is running, it will
 * monitor the time and provide callbacks when appropriate. If the callback returns false, it is removed from
 * the list of active timers.
 */
class IP_Timer : public Link {
private:
  IP_TimerClient * timer_client; ///< Pointer to a class object that implements the IP_TimerClient interface.

  u32_t timer_target;   ///< Time (in milliseconds) when the callback should be called.
  u32_t timer_interval; ///< Interval (in milliseconds) between callbacks, ir periodic.

public:
  /** Create an IP_Timer object.
   * \param client A class object that implements the IP_TimerClient interface (required for the callback).
   */
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

  /** Register the timer with a clock, giving the interval (in milliseconds) until the callback should be called.
   */
  void start (IP_Clock & clock, u32_t interval);

  /** Check to see whether the timer should be triggered; if so, call the callback.
   * \param current_time The current time.
   * \return True if the timer is periodic and should be kept active.
   */
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

/** Creates an infinite loop, replacing Arduino's traditional loop().
 */
class IP_Clock {
private:
  Chain<IP_Timer>  chain_timers; ///< Linked list of all the active timers.

  u32_t last_timer_second; ///< Time (in seconds) the last time we checked.
  u32_t last_timer_check;  ///< Time (in milliseconds) the last time we checked; very little point in checking the timers if the time hasn't changed.

  bool bStop; ///< Whether the clock should be stopped.

  /** Returns true if all timers checked; false if one was triggered & removed.
   */
  bool timer_checks (u32_t current_time);

public:
  /** Add an IP_Timer instance to the list of active timers; use IP_Timer::start().
   * \param timer A timer.
   */
  inline void timer_add (IP_Timer * timer) {
    chain_timers.chain_prepend (timer);
  }

  /** Current clock time.
   * \return Current time (in milliseconds).
   */
  inline u32_t milliseconds () {
    return ip_arch_millis ();
  }

  IP_Clock () :
    bStop(false)
  {
    last_timer_check = milliseconds ();
    last_timer_second = last_timer_check;
  }

  virtual ~IP_Clock () {
    // ...
  }

  /** Virtual function, called once per millisecond.
   */
  virtual void every_millisecond () {
    // 
  }

  /** Virtual function, called once per second.
   */
  virtual void every_second () {
    // ...
  }

  /** Virtual function, called once per cycle of the infinite loop.
   */
  virtual void tick () {
    // ...
  }

  /** Stop the infinite loop; primarily for Unix-based apps to exit neatly.
   */
  inline void stop () {
    bStop = true;
  }

  /** Start the infinite loop.
   */
  void run ();
};

#endif /* ! __ip_timer_hh__ */
