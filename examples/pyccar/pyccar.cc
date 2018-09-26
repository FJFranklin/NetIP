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

#include "pyccar.hh"

#include <cstring>
#include <unistd.h>

#include "TouchInput.hh"
#include "Window.hh"

using namespace PyCCar;

static const char * application_name = "PyCCar";
static const char * script_filename  = "pyccarui"; // without extension

static const char * touch_device = "/dev/input/event0";
static const char * video_device = "/dev/fb0";
static const char * video_driver = "fbcon";

static TouchInput * TI = 0;

#define MENU_ID_Exit     1
#define MENU_ID_Reboot   2
#define MENU_ID_Shutdown 3

#define MENU_ID_Item0   10
#define MENU_ID_Item1   11
#define MENU_ID_Item2   12
#define MENU_ID_Item3   13
#define MENU_ID_Item4   14
#define MENU_ID_Item5   15
#define MENU_ID_Item6   16
#define MENU_ID_Item7   17
#define MENU_ID_Item8   18
#define MENU_ID_Item9   19

#define MENU_ID_Sub0    20
#define MENU_ID_Sub1    21
#define MENU_ID_Sub2    22

static struct Menu::Info s_menu_SubMenu[] = {
  { MENU_ID_Sub0, "§±!@£$%^&*()_+-=`~",         0 },
  { MENU_ID_Sub1, "Lorem ipsum dolor sit amet", 0 },
  { MENU_ID_Sub2, "consectetur adipiscing elit", 0 },
  { 0, 0, 0 }
};

static struct Menu::Info s_menu_Main[] = {
  { MENU_ID_Item0, "The quick",          0 },
  { MENU_ID_Item1, "brown fox jumps",    0 },
  { MENU_ID_Item2, "over the lazy dog.", 0 },
  { MENU_ID_Item3, "Testing,",           0 },
  { MENU_ID_Item4, "testing,",           0 },
  { MENU_ID_Item5, "1, 2, 3,...",        s_menu_SubMenu },
  { MENU_ID_Item6, "0123456789",         0 },
  { MENU_ID_Item7, "qwertyuiop",         0 },
  { MENU_ID_Item8, "QWERTYUIOP",         0 },
  { MENU_ID_Item9, "{}[]:\"|;'\\<>?,./", 0 },
  { 0, 0, 0 }
};

static struct Menu::Info s_menu_Exit[] = {
  { MENU_ID_Exit,     "Exit",     0 },
  { MENU_ID_Reboot,   "Reboot",   0 },
  { MENU_ID_Shutdown, "Shutdown", 0 },
  { 0, 0, 0 }
};

class Canvas : public Window {
public:
  Canvas (Window & parent, int rel_x, int rel_y, unsigned width, unsigned height) :
    Window(parent, rel_x, rel_y, width, height)
  {
    m_bTouchable = true;
    ui().set_property_number ("CT#", 0);
    ui().set_bg_color (127, 127, 127);
    ui().set_fg_color (  0,   0, 255);
  }

  ~Canvas () {
    // ...
  }

  virtual void touch_enter () {
    ui().set_bg_color (0, 0, 0);

    set_dirty (true);
  }

  virtual void touch_leave () {
    ui().set_property_number ("CT#", 0);
    ui().set_bg_color (127, 127, 127);

    set_dirty (true);
  }

  virtual bool touch_event (TouchInput::TouchEvent te, const struct TouchInput::touch_event_data & event_data) {
    unsigned touch_count = 1;

    if (te == TouchInput::te_End) {
      ui().set_bg_color (127, 127, 127);
      touch_count = 0;
    } else {
      ui().set_bg_color (0, 0, 0);
      ui().set_property_coordinate ("CT1", event_data.t1.x, event_data.t1.y);

      if ((event_data.t1.x != event_data.t2.x) || (event_data.t1.y != event_data.t2.y)) {
	ui().set_property_coordinate ("CT2", event_data.t2.x, event_data.t2.y);
	touch_count = 2;
      }
    }
    ui().set_property_number ("CT#", touch_count);

    set_dirty (true);
    return true;
  }
};

class PyCCarMenu : public MenuManager::Handler, public TouchInput::RunTimer {
public:
  MenuManager MM;

  Canvas * m_Canvas;

  unsigned long m_time_stop;

  PyCCarMenu (unsigned long time_stop_ms) :
    MM(this, s_menu_Main, s_menu_Exit),
    m_Canvas(0),
    m_time_stop(time_stop_ms)
  {
    /* Create canvas window using the same bbox as the scrolling menu
     */
    BBox bbox = MM.bbox ();
    m_Canvas = new Canvas (Window::root (), bbox.m_x, bbox.m_y, bbox.m_width, bbox.m_height);
    m_Canvas->ui().set_type ("Canvas:Tracker");
    m_Canvas->set_visible (true);
  }

  virtual ~PyCCarMenu () {
    if (m_Canvas) {
      delete m_Canvas;
    }
  }

  virtual bool run_timer_tick (unsigned long time_in_ms) { // return false to stop timer
    return true; // m_time_stop ? (time_in_ms < m_time_stop) : true;
  }

  virtual bool run_timer_interval () { // return false to stop timer
    Window::root().redraw ();
    return true;
  }

  virtual bool notify_menu_will_open () { // return false to cancel menu
    // fputs ("notify_menu_will_open\n", stderr);

    if (m_Canvas) {
      m_Canvas->set_visible (false);
    }
    return true;
  }

  virtual bool notify_menu_closed (unsigned menu_id) { // return false to stop timer
    // fprintf (stderr, "notify_menu_closed: menu-id = %u\n", menu_id);

    if (m_Canvas) {
      m_Canvas->set_visible (true);
    }

    bool response = true;

    switch (menu_id) {
    case MENU_ID_Exit:
      {
	response = false;
	break;
      }
    case MENU_ID_Reboot:
      {
	execl ("/sbin/reboot", "reboot", (char *) 0);
	break;
      }
    case MENU_ID_Shutdown:
      {
	execl ("/sbin/shutdown", "shutdown", "-h", "now", (char *) 0);
	break;
      }
    case MENU_ID_Item0:
      {
	// ...
	break;
      }
    case MENU_ID_Item1:
      {
	// ...
	break;
      }
    case MENU_ID_Item2:
      {
	// ...
	break;
      }
    case MENU_ID_Item3:
      {
	// ...
	break;
      }
    case MENU_ID_Item4:
      {
	// ...
	break;
      }
    case MENU_ID_Item5:
      {
	// ...
	break;
      }
    case MENU_ID_Item6:
      {
	// ...
	break;
      }
    case MENU_ID_Item7:
      {
	// ...
	break;
      }
    case MENU_ID_Item8:
      {
	// ...
	break;
      }
    case MENU_ID_Item9:
      {
	// ...
	break;
      }
    case MENU_ID_Sub0:
      {
	// ...
	break;
      }
    case MENU_ID_Sub1:
      {
	// ...
	break;
      }
    case MENU_ID_Sub2:
      {
	// ...
	break;
      }
    }

    return response;
  }
};

int main (int argc, char ** argv) {
  WString Name(application_name);
  WStrArr Args(argc, argv);

  bool bTouch = false;

  unsigned long time_stop_ms = 0;

  unsigned screen_width  = 800;
  unsigned screen_height = 480;

  unsigned refresh_interval = 15;

  for (int arg = 1; arg < argc; arg++) {
    if (strcmp (argv[arg], "--timeout") == 0) {
      time_stop_ms = 30000;
    }
    if (strcmp (argv[arg], "--fb-touch") == 0) {
      bTouch = true;
    }
    if (strncmp (argv[arg], "--fb-device=", 12) == 0) {
      video_device = argv[arg] + 12;
    }
    if (strncmp (argv[arg], "--touch-device=", 15) == 0) {
      bTouch = true;
      touch_device = argv[arg] + 15;
    }
    if (strcmp (argv[arg], "--help") == 0) {
      fprintf (stdout, "%s [--timeout] [--fb-touch] [--fb-device=<dev>] [--touch-device=<dev>]\n\n"
	       "  --timeout            Exit automatically after 30 seconds.\n"
	       "  --fb-touch           Use the touch screen in framebuffer mode as the display.\n"
	       "  --fb-device=<dev>    Framebuffer device [default: /dev/fb0].\n"
	       "  --touch-device=<dev> Touch event input device [default: /dev/input/event0].\n"
	       "\n"
	       , argv[0]);
      return 0;
    }
  }

  TouchInput touch(screen_width, screen_height);
  TI = &touch;

  if (bTouch) {
    bool bFound = false;
    if (TI->init (touch_device)) {
      bFound = true;
    } else {
      fprintf (stderr, "Device %s not found, or is not a recognised touch device; scanning...\n", touch_device);
      char devbuf[32];
      for (int d = 0; d < 10; d++) {
	sprintf (devbuf, "/dev/input/event%d", d);
	if (strcmp (devbuf, touch_device)) {
	  fprintf (stderr, "Trying %s: ", devbuf);
	  if (TI->init (devbuf)) {
	    fprintf (stderr, "okay!\n");
	    bFound = true;
	    break;
	  }
	}
      }
    }
    if (!bFound) {
      return -1;
    }
    screen_width  = TI->width ();
    screen_height = TI->height ();
  }

  Py_SetProgramName (const_cast<wchar_t *>(Name.str ()));
  Py_Initialize ();
  PySys_SetArgv (Args.argc (), const_cast<wchar_t **>(Args.argv ()));

  if (PyCCarUI::ui_load (script_filename)) {
    bool bUI = false;

    if (bTouch)
      bUI = PyCCarUI::init (video_driver, video_device, screen_width, screen_height);
    else
      bUI = PyCCarUI::init (screen_width, screen_height);

    if (bUI) {
      if (Window::init (screen_width, screen_height)) {
	// TODO: create UI
	PyCCarMenu Menu(time_stop_ms);

	Window::root().redraw (true);
	TI->run (refresh_interval, &Menu);
      } else {
	fputs ("Failed to initialise window manager!\n", stderr);
      }
    }
  }
  PyCCarUI::ui_free ();

  Py_Finalize ();
  return 0;
}
