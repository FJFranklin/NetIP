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

#ifndef __PyCCar_hh__
#define __PyCCar_hh__

#include <Python.h>

#include "BBox.hh"

/* interface flags
 */
#define PyCCar_VISIBLE 0x01u // whether the window/button is visible
#define PyCCar_BORDER  0x02u // whether the window/button has a border
#define PyCCar_ENABLED 0x04u // whether the window/button is enabled
#define PyCCar_ACTIVE  0x08u // whether the window/button is active
#define PyCCar_SUBMENU 0x10u // whether the window/button has a submenu
#define PyCCar_BLANK   0x20u // whether the window/button is blank (if visible, but regardless of type)

class PyCCarUI {
public:
  enum EventType {
    et_None = 0,
    et_Quit,
    et_Mouse_Motion,
    et_Mouse_Up,
    et_Mouse_Down
  };
  struct event_data {
    EventType type;
    struct {
      int x, y;
    } pos;
  };

private:
  unsigned m_id;
public:
  PyCCarUI (unsigned win_id) :
    m_id(win_id)
  {
    // ...
  }

  ~PyCCarUI () {
    // ...
  }

  static bool ui_load (const char * script_filename);
  static void ui_free ();

  static bool init (const char * driver, const char * device, unsigned screen_width, unsigned screen_height);
  static bool init (unsigned screen_width, unsigned screen_height);

  static EventType event (struct event_data & data);

  static bool refresh ();

  bool draw ();
  bool set_property (const char * property, PyObject * value);

  bool set_property_number (const char * property, unsigned number);
  bool set_property_coordinate (const char * property, unsigned x, unsigned y);

  /* Set properties
   */
  bool set_bbox (const PyCCar::BBox & bbox);
  bool set_flags (unsigned flags);
  bool set_type (const char * window_type);
  bool set_bg_color (unsigned char r, unsigned char g, unsigned char b);
  bool set_fg_color (unsigned char r, unsigned char g, unsigned char b);
  bool set_fg_disabled (unsigned char r, unsigned char g, unsigned char b);
  bool set_font_size (unsigned size);
  bool set_label (const char * text);
  bool set_spacing (unsigned inset);
  bool set_border_active (unsigned thickness);
  bool set_border_inactive (unsigned thickness);
  bool set_thickness (unsigned thickness);
  bool set_scroll (unsigned s_min, unsigned s_max);
};

namespace PyCCar {
  class WString {
  private:
    wchar_t * m_str;

  public:
    inline const wchar_t * str () const {
      return m_str;
    }

    WString (const char * c_str);

    ~WString ();
  };

  class WStrArr {
  private:
    int m_argc;

    wchar_t ** m_argv;

  public:
    inline int argc () const {
      return m_argc;
    }
    inline const wchar_t * const * argv () const {
      return m_argv;
    }

    WStrArr (int c_argc, const char * const * c_argv);

    ~WStrArr ();
  };
}

#endif /* ! __PyCCar_hh__ */
