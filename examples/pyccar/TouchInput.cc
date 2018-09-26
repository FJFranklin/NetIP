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

/*! \file TouchInput.cc
    \brief Manage a Linux event stream for a touchscreen
    
    Developed for use with a HyperPixel 4.0 touchscreen used in console mode.
*/

#include "pyccar.hh"

#include <cstdio>

#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#if HAVE_LINUX_INPUT_H
#include <linux/input.h>
#endif

#include "TouchInput.hh"
#include "Window.hh"

static unsigned long bInit = false;

static unsigned long time_secs;
static unsigned long time_nano;

static unsigned long timer_millis () {
  unsigned long us = 0;

  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  unsigned long new_time_secs = (unsigned long) ts.tv_sec;
  unsigned long new_time_nano = (unsigned long) ts.tv_nsec;

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

using namespace PyCCar;

TouchInput::TouchInput (unsigned width, unsigned height) :
  m_te(te_None),
  m_handler(0),
  m_devfd(-1),
#if HAVE_LINUX_INPUT_H
  m_touch_new(false),
  m_touch_end(false),
  m_touch_yes(false),
#endif
  m_timer_active(false),
#if HAVE_LINUX_INPUT_H
  ev_count(0),
#endif
  m_width(width),
  m_height(height)
#if HAVE_LINUX_INPUT_H
, m_range_min_x(0),
  m_range_max_x(width),
  m_range_min_y(0),
  m_range_max_y(height),
  m_bFlip(false)
#endif
{
  m_touch.t1.x = 0;
  m_touch.t1.y = 0;
  m_touch.t2.x = 0;
  m_touch.t2.y = 0;
}

TouchInput::~TouchInput () {
  if (m_devfd >= 0) {
    close (m_devfd);
  }
}

bool TouchInput::init (const char * device) {
#if HAVE_LINUX_INPUT_H
  if (m_devfd >= 0) {
    return false;
  }

  m_devfd = open (device, O_RDONLY | O_NONBLOCK /* O_NDELAY */);
  if (m_devfd == -1) {
    fprintf (stderr, "Failed to open \"%s\" - exiting.\n", device);
    return false;
  }

  bool bDeviceRecognised = false;

  struct input_id iid;
  if (ioctl (m_devfd, EVIOCGID, &iid)) {
    fprintf (stderr, "Failed to get device info about \"%s\" - exiting.\n", device);
  } else if ((iid.vendor == 0x416) && (iid.product == 0x38f)) { // HyperPixel 800x480 a.k.a. "Goodix Capacitive TouchScreen"
    bDeviceRecognised = true;
    m_width  = 800;
    m_height = 480;
    m_range_min_x = 0;
    m_range_max_x = 480;
    m_range_min_y = 0;
    m_range_max_y = 800;
    m_bFlip = false;
//} else if (...) {
  } else {
    char name[256];
    if (ioctl(m_devfd, EVIOCGNAME (sizeof (name)), name)) {
      if (strcmp (name, "ADS7846 Touchscreen") == 0) {
	bDeviceRecognised = true;
	m_width  = 480;
	m_height = 320;
	m_range_min_x = 3920;
	m_range_max_x = 150;
	m_range_min_y = 220;
	m_range_max_y = 3780;
	m_bFlip = true;
//    } else if (...) {
      } else {
	fprintf (stderr, "Touch device (%s) not recognised:\n", device);
	fprintf (stderr, "-> info: vendor %04hx product %04hx version %04hx\n", iid.vendor, iid.product, iid.version);
	fprintf (stderr, "-> name: '%s'.\n", name);
      }
    }
  }

  if (!bDeviceRecognised) {
    close (m_devfd);
    m_devfd = -1;
    return false;
  }

  unsigned char byte;

  while (read (m_devfd, &byte, 1) > 0) {
    // empty the input buffer
  }
#endif
  return true;
}

void TouchInput::handle (const struct input_event * event) {
#if HAVE_LINUX_INPUT_H
  if (event->type == EV_SYN) {
    if (m_touch_new) {
      m_touch_new = false;
      m_touch_yes = true;
      touch_event_begin ();
    } else if (m_touch_end) {
      m_touch_end = false;
      m_touch_yes = false;
      touch_event_end ();
    } else if (m_touch_yes) {
      touch_event_change ();
    }
  } else if (event->type == EV_ABS) {
    switch (event->code) {
    case 0:
      {
	if (m_bFlip) // this is a y-axis event
	  m_touch.t1.y = (event->value - m_range_min_y) * m_height / (m_range_max_y - m_range_min_y);
	else
	  m_touch.t1.x = (event->value - m_range_min_x) * m_width  / (m_range_max_x - m_range_min_x);
	break;
      }
    case 1:
      {
	if (m_bFlip) // this is a x-axis event
	  m_touch.t1.x = (event->value - m_range_min_x) * m_width  / (m_range_max_x - m_range_min_x);
	else
	  m_touch.t1.y = (event->value - m_range_min_y) * m_height / (m_range_max_y - m_range_min_y);
	break;
      }
    case 24: // touch event with zero-pressure marks end of sequence
      {
	if (event->value == 0) {
	  if (m_touch_yes)
	    m_touch_end = true;
	} else {
	  if (!m_touch_yes)
	    m_touch_new = true;
	}
	break;
      }
    case 53:
      {
	if (m_bFlip) // this is a y-axis event
	  m_touch.t2.y = (event->value - m_range_min_y) * m_height / (m_range_max_y - m_range_min_y);
	else
	  m_touch.t2.x = (event->value - m_range_min_x) * m_width  / (m_range_max_x - m_range_min_x);
	break;
      }
    case 54:
      {
	if (m_bFlip) // this is a x-axis event
	  m_touch.t2.x = (event->value - m_range_min_x) * m_width  / (m_range_max_x - m_range_min_x);
	else
	  m_touch.t2.y = (event->value - m_range_min_y) * m_height / (m_range_max_y - m_range_min_y);
	break;
      }
    case 57: // touch-sequence identifier
      {
	if (event->value == -1) {
	  if (m_touch_yes)
	    m_touch_end = true;
	} else {
	  if (!m_touch_yes)
	    m_touch_new = true;
	}
	break;
      }
    default:
      break;
    }
  }
#endif
}

bool TouchInput::tick () {
  bool response = true;

  if (m_devfd < 0) {
    struct PyCCarUI::event_data data;

    while (PyCCarUI::event (data)) {
      if (data.type == PyCCarUI::et_Quit) {
	response = false;
	break;
      }
      switch (data.type) {
      case PyCCarUI::et_Mouse_Motion:
	{
	  m_touch.t1.x = data.pos.x;
	  m_touch.t1.y = data.pos.y;
	  m_touch.t2 = m_touch.t1;
	  touch_event_change ();
	  break;
	}
      case PyCCarUI::et_Mouse_Up:
	{
	  m_touch.t1.x = data.pos.x;
	  m_touch.t1.y = data.pos.y;
	  m_touch.t2 = m_touch.t1;
	  touch_event_end ();
	  break;
	}
      case PyCCarUI::et_Mouse_Down:
	{
	  m_touch.t1.x = data.pos.x;
	  m_touch.t1.y = data.pos.y;
	  m_touch.t2 = m_touch.t1;
	  touch_event_begin ();
	  break;
	}
      default:
	{
	  break;
	}
      }
    }
  } else {
#if HAVE_LINUX_INPUT_H
    struct input_event * ev = reinterpret_cast<struct input_event *>(ev_buffer);

    int bytes_read = read (m_devfd, ev_buffer + ev_count, TouchInput_BUFSIZE - ev_count);
    if (bytes_read > 0) {
      ev_count += bytes_read;

      int count = ev_count / sizeof (struct input_event);

      for (int c = 0; c < count; c++) {
	handle (ev + c);
	ev_count -= sizeof (struct input_event);
      }
      if (ev_count) {
	ev[0] = ev[count];
      }
    }
#endif
  }
  return response;
}

void TouchInput::event_process () {
  if (m_te) {
    Handler * handler = Window::root().touch_handler (m_touch);

    if (m_handler != handler) {
      if (m_handler)
	m_handler->touch_leave ();

      m_handler = handler;

      if (m_handler)
	m_handler->touch_enter ();
    }
    if (m_handler) {
      m_timer_active = m_handler->touch_event (m_te, m_touch);
    }
  }
  m_te = te_None;

  PyCCarUI::refresh ();
}

void TouchInput::run (unsigned long interval, RunTimer * RT) {
  if (m_timer_active) { // already active; just return
    return;
  }

  unsigned long last_milli = timer_millis ();
  unsigned long last_event = last_milli;
  unsigned long time_start = last_milli;

  m_timer_active = true;

  while (m_timer_active) {
    unsigned long time = timer_millis ();  // just call this the once

    if (last_milli < time) { // time is in milliseconds
      last_milli = time;     // note current time

      if (!tick ()) {
	break;               // break loop if tick() fails
      }
      if (RT && m_timer_active) {
	m_timer_active = RT->run_timer_tick (time - time_start); // break loop if callback~tick() fails
      }
    }
    if (interval) {
      if (last_event + interval <= time) {
	last_event += interval;

	event_process ();

	if (RT && m_timer_active) {
	  m_timer_active = RT->run_timer_interval (); // break loop if callback~interval() fails
	}
      }
    }
    usleep (1);
  }
}
