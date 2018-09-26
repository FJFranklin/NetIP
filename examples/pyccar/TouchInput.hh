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

/*! \file TouchInput.hh
    \brief Manage a Linux event stream for a touchscreen
    
    Developed for use with a HyperPixel 4.0 touchscreen used in console mode.
*/

#ifndef __TouchInput_hh__
#define __TouchInput_hh__

#include "pyccar.hh"

#define TouchInput_BUFSIZE 1024

struct input_event;

namespace PyCCar {

  class TouchInput {
  public:
    enum TouchEvent {
      te_None = 0,
      te_Begin,
      te_Change,
      te_End
    } m_te;

    struct touch_event_data {
      struct {
	int x, y;
      } t1, t2;
    } m_touch;

    class Handler {
    public:
      virtual Handler * touch_handler (const struct touch_event_data & event_data) = 0;

      virtual void touch_enter () = 0;
      virtual void touch_leave () = 0;

      virtual bool touch_event (TouchEvent te, const struct touch_event_data & event_data) = 0; // return false to stop the timer

      virtual ~Handler () { }
    };

    class RunTimer {
    public:
      virtual bool run_timer_tick (unsigned long time_in_ms) = 0;
      virtual bool run_timer_interval () = 0;

      virtual ~RunTimer () { }
    };

  private:
    Handler * m_handler;

    int m_devfd;
#if HAVE_LINUX_INPUT_H
    bool m_touch_new;
    bool m_touch_end;
    bool m_touch_yes;
#endif
    bool m_timer_active;

#if HAVE_LINUX_INPUT_H
    unsigned long ev_count;
    unsigned char ev_buffer[TouchInput_BUFSIZE];
#endif
    int  m_width;
    int  m_height;

#if HAVE_LINUX_INPUT_H
    int  m_range_min_x;
    int  m_range_max_x;
    int  m_range_min_y;
    int  m_range_max_y;
    bool m_bFlip;
#endif
  public:
    TouchInput (unsigned width, unsigned height);

    ~TouchInput ();

    bool init (const char * device);

    inline unsigned width () const {
      return m_width;
    }
    inline unsigned height () const {
      return m_height;
    }

  private:
    inline void touch_event_begin () {
      m_te = (m_te == te_End)   ? te_Change : te_Begin;
    }
    inline void touch_event_end () {
      m_te = (m_te == te_Begin) ? te_None   : te_End;
    }
    inline void touch_event_change () {
      m_te = (m_te == te_None)  ? te_Change : m_te;
    }

    void handle (const struct input_event * event);

  public:
    inline bool event_pending () const {
      return m_te;
    }
    void event_process ();

    /* start a simple event loop to check for touch events and process them
     * 
     * calls tick() every millisecond, and event_process() every <interval>
     * (interval specified in milliseconds; must be non-zero)
     */
    void run (unsigned long interval, RunTimer * RT = 0);

    /* stop the event loop
     */
    inline void stop () {
      m_timer_active = false;
    }

    bool tick (); // to be called more frequently than the screen refresh rate; returns false if program-end is requested
  };

} // namespace PyCCar

#endif /* ! __TouchInput_hh__ */
