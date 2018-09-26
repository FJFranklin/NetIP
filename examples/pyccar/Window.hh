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

#ifndef __Window_hh__
#define __Window_hh__

#include "pyccar.hh"
#include "TouchInput.hh"

namespace PyCCar {

  class Window : public TouchInput::Handler {
  private:
    Window * m_parent;

    Window * m_child_bottom;
    Window * m_child_top;
    Window * m_sibling_lower;
    Window * m_sibling_upper;

    unsigned m_id;

  protected:
    unsigned m_flags;

    bool     m_bTouchable;

  private:
    BBox     m_bbox;
    BBox     m_dirty;

    Window (unsigned width, unsigned height);

  public:
    Window (Window & parent, int rel_x, int rel_y, unsigned width, unsigned height);

    virtual ~Window ();

    static Window & root ();

    static bool init (unsigned width, unsigned height);

    inline unsigned id () const {
      return m_id;
    }

    inline PyCCarUI ui () const {
      return PyCCarUI(m_id);
    }

    inline bool coord_in_bounds (unsigned x, unsigned y) const {
      return m_bbox.includes (x, y);
    }
    inline unsigned origin_x () const { // all coordinates are absolute; origin locates top-left corner of window
      return m_bbox.m_x;
    }
    inline unsigned origin_y () const {
      return m_bbox.m_y;
    }
    inline unsigned window_width () const {
      return m_bbox.m_width;
    }
    inline unsigned window_height () const {
      return m_bbox.m_height;
    }

    inline void set_dirty (const BBox & region) { // mark a region of the window as dirty
      m_dirty.combine (region);
    }
    inline void set_dirty (bool bDirty) { // mark the whole window as dirty or clean
      if (bDirty) {
	m_dirty = m_bbox;
      } else {
	m_dirty.clear ();
      }
    }

    inline bool visible () const {
      return m_flags & PyCCar_VISIBLE;
    }
    void set_visible (bool bVisible);

    virtual TouchInput::Handler * touch_handler (const struct TouchInput::touch_event_data & event_data);

    virtual void touch_enter ();
    virtual void touch_leave ();
    virtual bool touch_event (TouchInput::TouchEvent te, const struct TouchInput::touch_event_data & event_data);

    inline void redraw (bool bForceRedraw = false) {
      BBox dirty;
      root().redraw (dirty, bForceRedraw);
    }
  private:
    void redraw (BBox & dirty, bool bForceRedraw);

    void add_child (Window * child);
    void remove_child (Window * child);
  };

  class Button : public Window {
  public:
    class Handler {
    public:
      virtual bool button_press (unsigned button_id) = 0;
      virtual ~Handler () { }
    };

  private:
    Handler * m_handler;
    unsigned  m_button_id;

  public:
    Button (Window & parent, int rel_x, int rel_y, unsigned width, unsigned height);

    virtual ~Button ();

    inline void set_handler (Handler * handler, unsigned button_id) {
      m_handler   = handler;
      m_button_id = button_id;
    }
    inline unsigned button_id () const {
      return m_button_id;
    }

    inline bool enabled () const {
      return m_flags & PyCCar_ENABLED;
    }
    void set_enabled (bool bEnabled);
  private:
    inline bool active () const {
      return m_flags & PyCCar_ACTIVE;
    }
    void set_active (bool bActive);
  public:
    void set_has_submenu (bool bHasSubmenu);

    virtual void touch_enter ();
    virtual void touch_leave ();
    virtual bool touch_event (TouchInput::TouchEvent te, const struct TouchInput::touch_event_data & event_data);
  };

  class Menu {
  public:
    struct Info {
      unsigned      m_id;
      const char *  m_label;
      struct Info * m_submenu;
    };

    class Item {
    private:
      friend class Menu;

      unsigned  m_id;
      char *    m_label;
      Item *    m_next;
    public:
      Menu *    m_submenu;
      bool      m_bEnabled;

      Item (unsigned id, const char * str);

      ~Item ();

      inline unsigned id () const {
	return m_id;
      }
      inline const char * label () const {
	return m_label;
      }
      void set_label (const char * str);

      void set_submenu (Menu * menu);
    };

  private:
    Menu *   m_parent;

    Item *   m_item_first;
    Item *   m_item_last;

    unsigned m_length;
    unsigned m_offset;

  public:
    Menu (struct Info * info);

    ~Menu ();

    inline unsigned length () const {
      return m_length;
    }

    inline void set_offset (unsigned offset) {
      m_offset = offset;
    }
    inline unsigned offset () const {
      return m_offset;
    }

    inline void set_parent (Menu * menu) {
      m_parent = menu;
    }
    inline Menu * parent () {
      return m_parent;
    }

    Item * add (unsigned id, const char * label);

    Item * item_no (unsigned no); // by order in list
    Item * find_id (unsigned id); // recursive search for item by id
  };

  class ScrollableMenu : public Window, public Button::Handler {
  public:
  private:
    Button * m_Item[6];

    Button * m_Up;
    Button * m_Down;
    Window * m_Scroll;

    Menu *   m_menu;

    Button::Handler * m_app;

  public:
    ScrollableMenu (Window & parent, int rel_x, int rel_y, unsigned width, unsigned height);

    virtual ~ScrollableMenu ();

    void manage_menu (Menu & menu, Button * back, Button::Handler * app_manager);

    virtual bool button_press (unsigned button_id);

  private:
    void configure ();

    bool menu_back ();
    void menu_up ();
    void menu_down ();
  };

  class MenuManager : public Button::Handler {
  public:
    class Handler {
    public:
      virtual bool notify_menu_will_open () = 0;              // return false to cancel menu
      virtual bool notify_menu_closed (unsigned menu_id) = 0; // return false to stop timer

      virtual ~Handler () { }
    };
  private:
    Handler * m_handler;

    Button *  m_Main; // alt. Back
    Button *  m_Exit;

    ScrollableMenu * m_Menu;

    Menu      m_menu_Main;
    Menu      m_menu_Exit;

    unsigned  m_off_x;
    unsigned  m_off_y;
    unsigned  m_W;
    unsigned  m_H;

  public:
    MenuManager (Handler * H, struct Menu::Info * main_info, struct Menu::Info * exit_info);

    virtual ~MenuManager ();

    virtual bool button_press (unsigned button_id);

    inline BBox bbox () const {
      return BBox(m_off_x, m_off_y, m_W, m_H);
    }

    inline Menu::Item * main_menu_find_id (unsigned id) {
      return m_menu_Main.find_id (id);
    }
    inline Menu::Item * exit_menu_find_id (unsigned id) {
      return m_menu_Exit.find_id (id);
    }
  };
} // namespace PyCCar

#endif /* ! __Window_hh__ */
