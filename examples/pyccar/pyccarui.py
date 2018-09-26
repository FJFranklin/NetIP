import os
import pygame

# Colors
black = (  0,  0,  0)
white = (255,255,255)
red   = (255,  0,  0)
green = (  0,255,  0)
blue  = (  0,  0,255)
grey  = (127,127,127)

windows = {}
deffont = {}
defprop = {}

defprop['BG']      = black    # background color
defprop['FG']      = white    # foreground color
defprop['Label']   = '(none)' # label, if any
#defprop['Scroll'] = (10,90)        # scroll percentages
defprop['a-thick'] = 9        # border thickness, if active
#defprop['bbox']   = (0,0,800,480)  # bounding box
defprop['dis_FG']  = grey     # foreground color if disabled
defprop['flags']   = 0
defprop['inset']   = 10       # border-inset, if any
defprop['line']    = 5        # general line thickness
defprop['n-thin']  = 3        # border thickness, if not
defprop['size']    = 24       # font size
defprop['type']    = 'blank'  # window type

# flags, as follows:
# 0x01 - whether the window/button is visible
# 0x02 - whether the window/button has a border
# 0x04 - whether the window/button is enabled
# 0x08 - whether the window/button is active
# 0x10 - whether the window/button has a submenu
# 0x20 - whether the window/button is blank (if visible, but regardless of type)

def get_font(size):
    if size not in deffont:
        deffont[size] = pygame.font.Font(None, size)
    return deffont[size]

def get_property(win_id, property):
    value = None
    if win_id in windows:
        W = windows[win_id]
        if property in W:
            value = W[property]
        elif property in defprop:
            value = defprop[property]
    return value

def ui_set_property(win_id, property, value):
    if win_id not in windows:
        windows[win_id] = {}
    W = windows[win_id]
    W[property] = value

def draw_border(win_id, bbox, flags):
    BG = get_property(win_id, 'BG')
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
        if flags & 0x08: # active
            thickness = get_property(win_id, 'a-thick')
        else: # inactive
            thickness = get_property(win_id, 'n-thin')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
        thickness = get_property(win_id, 'n-thin')
    d = get_property(win_id, 'inset')
    if thickness > d:
        thickness = d
    (x, y, w, h) = bbox
    pygame.draw.rect(screen, FG, (x+d-thickness, y+d-thickness, w-2*d+2*thickness, h-2*d+2*thickness), 0)
    pygame.draw.rect(screen, BG, (x+d, y+d, w-2*d, h-2*d), 0)

def draw_back(win_id, bbox, flags):
    BG = get_property(win_id, 'BG')
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
    thickness = get_property(win_id, 'line')
    d         = get_property(win_id, 'inset')
    (x, y, w, h) = bbox
    x = x + 3 * d
    y = y + 3 * d
    w = w - 6 * d
    h = h - 6 * d
    hw = int(round(w/2))
    hh = int(round(h/2))
    ht = int(round(thickness/2))
    centre = (x+hw, y+hh)
    radius = hw
    pygame.draw.circle(screen, FG, centre, radius, 0)
    pygame.draw.circle(screen, BG, centre, radius-thickness, 0)
    bboxi = (x, y+hh, hw, hh)
    pygame.draw.rect(screen, BG, bboxi, 0)
    xarr = x + ht
    yarr = y + hh
    pygame.draw.polygon(screen, FG, [(xarr-d,yarr), (xarr+d,yarr), (xarr,yarr+d)], 0)

def draw_main(win_id, bbox, flags):
    BG = get_property(win_id, 'BG')
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
    thickness = get_property(win_id, 'line')
    d         = get_property(win_id, 'inset')
    (x, y, w, h) = bbox
    x = x + 3 * d
    y = y + 3 * d
    w = w - 6 * d
    h = h - 6 * d
    t = int(round(h/7))
    for i in range(0, 4):
        bboxi = (x, y+2*i*t, w, t)
        pygame.draw.rect(screen, FG, bboxi, 0)

def draw_exit(win_id, bbox, flags):
    BG = get_property(win_id, 'BG')
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
    thickness = get_property(win_id, 'line')
    d         = get_property(win_id, 'inset')
    (x, y, w, h) = bbox
    x = x + 3 * d
    y = y + 3 * d
    w = w - 6 * d
    h = h - 6 * d
    hw = int(round(w/2))
    hh = int(round(h/2))
    ht = int(round(thickness/2))
    centre = (x+hw, y+hh)
    radius = hw
    pygame.draw.circle(screen, FG, centre, radius, 0)
    pygame.draw.circle(screen, BG, centre, radius-thickness, 0)
    bboxi = (x+hw-thickness-ht, y, 3*thickness, hh)
    pygame.draw.rect(screen, BG, bboxi, 0)
    bboxi = (x+hw-ht, y-thickness, thickness, hh+thickness)
    pygame.draw.rect(screen, FG, bboxi, 0)

def draw_up(win_id, bbox, flags):
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
    thickness = get_property(win_id, 'line')
    d         = get_property(win_id, 'inset')
    (x, y, w, h) = bbox
    hw = int(round(w/2))
    xl = x + 3*d
    yt = y + 3*d
    x0 = x + hw
    xr = x + w - 3*d
    yb = y + h - 3*d
    pygame.draw.polygon(screen, FG, [(xl,yb), (x0,yt), (xr,yb)], 0)

def draw_down(win_id, bbox, flags):
    if flags & 0x04: # enabled
        FG = get_property(win_id, 'FG')
    else: # disabled
        FG = get_property(win_id, 'dis_FG')
    thickness = get_property(win_id, 'line')
    d         = get_property(win_id, 'inset')
    (x, y, w, h) = bbox
    hw = int(round(w/2))
    xl = x + 3*d
    yt = y + 3*d
    x0 = x + hw
    xr = x + w - 3*d
    yb = y + h - 3*d
    pygame.draw.polygon(screen, FG, [(xl,yt), (x0,yb), (xr,yt)], 0)

def draw_scroll(win_id, bbox, flags):
    d      = get_property(win_id, 'inset')
    scroll = get_property(win_id, 'Scroll')
    (x, y, w, h) = bbox
    pygame.draw.rect(screen, grey, (x+d, y, w-2*d, h), 0)
    if scroll:
        (s_min, s_max) = scroll
        bboxi = (x+2*d, int(round(y+(h*s_min)/100)), w-4*d, int(round((h*(s_max-s_min))/100)))
        pygame.draw.rect(screen, blue, bboxi, 0)

def draw_menu_item(win_id, bbox, flags):
    d    = get_property(win_id, 'inset')
    FG   = get_property(win_id, 'FG')
    text = get_property(win_id, 'Label')
    size = get_property(win_id, 'size')
    font = get_font(size)
    label = font.render(str(text), 1, (FG))
    (x, y, w, h) = bbox
    screen.blit(label, (x+2*d,y+2*d))
    if flags & 0x10: # we have a submenu
        thickness = get_property(win_id, 'line')
        xl = x + w - 6*d
        yt = y + 2*d
        y0 = y + h/2
        xr = x + w - 2*d
        yb = y + h - 2*d
        pygame.draw.polygon(screen, FG, [(xl,yt), (xl,yb), (xr,y0)], 0)

def draw_canvas_tracker_point(bbox, location, BG, FG):
    (x, y, w, h) = bbox
    half_size = int(round(min(w,h)/12)) # half-size of square to draw around touch point
    thickness = 2
    (tx, ty) = location
    if x + half_size > tx:
        sx = x
        sw = half_size + tx - x
    elif tx + half_size > x + w:
        sx = tx - half_size
        sw = x + w - sx
    else:
        sx = tx - half_size
        sw = 2 * half_size
    if y + half_size > ty:
        sy = y
        sh = half_size + ty - y
    elif ty + half_size > y + h:
        sy = ty - half_size
        sh = y + h - sy
    else:
        sy = ty - half_size
        sh = 2 * half_size
    pygame.draw.rect(screen, FG, (sx, sy, sw, sh), 0)
    pygame.draw.rect(screen, BG, (sx+thickness, sy+thickness, sw-2*thickness, sh-2*thickness), 0)

def draw_canvas_tracker(win_id, bbox, flags):
    count = get_property(win_id, 'CT#')
    loc_1 = None
    loc_2 = None
    if count is not None:
        if count > 0:
            loc_1 = get_property(win_id, 'CT1')
        if count > 1:
            loc_2 = get_property(win_id, 'CT2')
    if loc_1 is not None:
        FG = get_property(win_id, 'FG')
        BG = get_property(win_id, 'BG')
        draw_canvas_tracker_point(bbox, loc_1, BG, FG)
        if loc_2 is not None:
            draw_canvas_tracker_point(bbox, loc_2, BG, FG)

def ui_draw(win_id):
    flags = get_property(win_id, 'flags')
    if flags & 0x01: # visible
        bbox = get_property(win_id, 'bbox')
        BG   = get_property(win_id, 'BG')
        pygame.draw.rect(screen, BG, bbox, 0) # 0-thickness = fill
    if (flags & 0x21) == 0x01: # visible, but not blank
        if flags & 0x02: # there's a border
            draw_border(win_id, bbox, flags)
        type = get_property(win_id, 'type')
        if type == 'Back':
            draw_back(win_id, bbox, flags)
        elif type == 'Main':
            draw_main(win_id, bbox, flags)
        elif type == 'Exit':
            draw_exit(win_id, bbox, flags)
        elif type == 'Up':
            draw_up(win_id, bbox, flags)
        elif type == 'Down':
            draw_down(win_id, bbox, flags)
        elif type == 'Scroll':
            draw_scroll(win_id, bbox, flags)
        elif type == 'Menu Item':
            draw_menu_item(win_id, bbox, flags)
        elif type == 'Canvas:Tracker':
            draw_canvas_tracker(win_id, bbox, flags)

def ui_refresh():
    pygame.display.update()

def ui_init(DD, screen_wh):
    if DD:
        (driver, device) = DD
        os.environ["SDL_VIDEODRIVER"] = driver
        os.environ["SDL_FBDEV"] = device
    pygame.init()
    pygame.display.init()
    pygame.font.init()
    global screen
    if DD:
        pygame.mouse.set_visible(0)
        screen = pygame.display.set_mode(screen_wh, pygame.FULLSCREEN)
    else:
        screen = pygame.display.set_mode(screen_wh)
    screen.fill(red)
    pygame.display.update()

def ui_event():
    next = ('-', None)
    pygame.event.pump()
    while True:
        event = pygame.event.poll()
        if event.type == pygame.NOEVENT:
            break
        if event.type == pygame.QUIT:
            next = ('Q', None)
            break
        pos = pygame.mouse.get_pos()
        if event.type == pygame.MOUSEMOTION: #      pos, rel, buttons
            next = ('M', pos)
            break
        if event.type == pygame.MOUSEBUTTONUP: #    pos, button
            next = ('U', pos)
            break
        if event.type == pygame.MOUSEBUTTONDOWN: #  pos, button
            next = ('D', pos)
            break
    return next

