/**
 * @file display_x11.c
 * @brief Handle displaying and updating the graphical components of the game
 * @created 1999-08-17
 * @date 2010-01-01
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: display_x11.c,v 1.19 2012/06/03 17:06:14 gurumeditation Exp $
 *
 * Powermanga is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Powermanga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include "config.h"
#include "powermanga.h"
#include "tools.h"
#include "assembler.h"
#include "images.h"
#include "config_file.h"
#include "display.h"
#include "electrical_shock.h"
#include "energy_gauge.h"
#include "log_recorder.h"
#include "movie.h"
#include "gfx_wrapper.h"
#include "options_panel.h"
#include "gfx_wrapper.h"
#include "scalebit.h"
#include "sprites_string.h"
#include "texts.h"
#ifdef POWERMANGA_X11
/* use XSHM Ximage */
#define UTILISE_XSHM 1
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86dga.h>
#ifdef UTILISE_XSHM
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#endif

/** X Window */
static Display *x11_display = NULL;
/** Our main window */
static Window main_window_id = 0;
static GC graphic_contexts = NULL;
/** Maxinum number of ximages */
#define MAXIMUM_OF_XIMAGES 100
/** 512x440: game's offscreen  */
static XImage *game_ximage = NULL;
static XImage *scalex_ximage = NULL;
/** Resize to 640x400 960x600 or 1280x800 */
static XImage *options_ximage = NULL;
/** 64*184: right options panel */
static XImage *scores_ximage = NULL;
/** 320*16: top scores panel */
static XImage *movie_ximage = NULL;
/** 320x200: movie animation */
static Uint32 ximages_counter = 0;
static XImage *ximages_list[MAXIMUM_OF_XIMAGES];
#ifdef UTILISE_XSHM
static XShmSegmentInfo SHMInfo[MAXIMUM_OF_XIMAGES];
#endif
XPixmapFormatValues pixmapFormat;
Atom protocols;
bool dga_enable = FALSE;
Sint32 dga_flags;
static char *dga_base_addr;
static Sint32 dga_width;
static Sint32 dga_bank_size;
static Sint32 dga_ram_size;
static Sint32 dga_viewport_width;
static Sint32 dga_viewport_height;
static void disable_cursor ();
static void display_movie ();
static void display_320x200 ();
static void display_640x400 ();
static void display_scalex ();
static void dga320x200 ();
static void dga640x400 ();
static XImage *create_ximage (Uint32 width, Uint32 height);
static void destroy_ximage (XImage * ximg);
static void destroy_ximages ();
static void keytouch (Uint32 keycode, bool i);
static void Keys_Down (Uint32 keycode);
static void Keys_Up (Uint32 keycode);

const int EVENT_MASKS =
  KeyPressMask | KeyReleaseMask | ExposureMask | FocusChangeMask |
  StructureNotifyMask;
static Colormap x_color_map;
static XColor *x_colors = NULL;

/** 
 * Initialize X11 display
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
display_init (void)
{
  /* the root window for the default screen */
  Window root_window_id = 0;
  /* width of the screen in pixels */
  Uint32 displayw = 0;
  /* height of the screen in pixels */
  Uint32 displayh = 0;
  Sint32 black = 0;
  Sint32 white = 0;
  Sint32 i = 0;
  /* depth of the screen */
  Uint32 depth = 0;
  /* default screen number */
  Sint32 dfscreen;
  Sint32 formatok, formatcnt;
  XPixmapFormatValues *formatlist;
  XGCValues gcvalues;
  XSizeHints *hints;
  XSetWindowAttributes window_attributes;

  /* reset ximages list */
  for (i = 0; i < (Sint32) MAXIMUM_OF_XIMAGES; i++)
    {
      ximages_list[i] = NULL;
    }

  LOG_INF ("Beginning of the initialization of XWindow");
  switch (vmode)
    {
    case 0:
      window_width = display_width;
      window_height = display_height;
      break;
    case 1:
      window_width = display_width * 2;
      window_height = display_height * 2;
      break;
    case 2:
      window_width = display_width * power_conf->scale_x;
      window_height = display_height * power_conf->scale_x;
      break;
    }
  /* connect to X server */
  if (x11_display == NULL)
    {
      x11_display = XOpenDisplay (0);
      if (x11_display == NULL)
        {
          LOG_ERR ("XOpenDisplay() failed!");
          return FALSE;
        }
    }

  /* get the root window for the default screen */
  if (root_window_id == 0)
    {
      root_window_id = DefaultRootWindow (x11_display);
      if (root_window_id == 0)
        {
          LOG_ERR ("DefaultRootWindow() failed!");
          return FALSE;
        }
    }

  /* get default screen number */
  dfscreen = DefaultScreen (x11_display);
  black = BlackPixel (x11_display, dfscreen);
  white = WhitePixel (x11_display, dfscreen);

  /* get the default depth of the screen */
  depth = DefaultDepth (x11_display, dfscreen);
  if (depth < 8)
    {
      LOG_ERR ("this game need 8-bit per pixels minimum (256 colors)!");
      return FALSE;
    }
  bits_per_pixel = depth;

  formatlist = XListPixmapFormats (x11_display, &formatcnt);
  if (formatlist == NULL)
    {
      LOG_ERR ("cannot get pixmap list format");
      return FALSE;
    }
  formatok = -1;
  for (i = 0; i < formatcnt; i++)
    {
      if (formatlist[i].depth == (Sint32) (depth))
        {
          formatok = i;
        }
    }
  if (formatok == -1)
    {
      LOG_ERR ("screen depth error");
      return FALSE;
    }
  memcpy (&pixmapFormat, &formatlist[formatok], sizeof (pixmapFormat));
  XFree (formatlist);
  if (depth == 24)
    {
      depth = pixmapFormat.bits_per_pixel;
    }
  switch (depth)
    {
    case 8:
      bytes_per_pixel = 1;
      break;
    case 15:
      bytes_per_pixel = 2;
      break;
    case 16:
      bytes_per_pixel = 2;
      break;
    case 24:
      bytes_per_pixel = 3;
      break;
    case 32:
      bytes_per_pixel = 4;
      break;
    }
  displayw = DisplayWidth (x11_display, dfscreen);
  displayh = DisplayHeight (x11_display, dfscreen);
  LOG_INF ("depth of screen: %i; bytes per pixel:%i; "
           "size screen: (%i,%i)", depth, bytes_per_pixel, displayw,
           displayh);
  LOG_INF ("depth:%i; formatok:%i; formatcnt:%i; "
           "pixmapFormat.scanline_pad:%i", depth, formatok, formatcnt,
           pixmapFormat.scanline_pad);

  /* function creates a graphics context and returns a Graphic Contexts */
  if (!dga_enable)
    {
      if (graphic_contexts == NULL)
        {
          gcvalues.foreground = white;
          gcvalues.background = black;
          graphic_contexts = XCreateGC (x11_display,
                                        root_window_id,
                                        (GCForeground | GCBackground),
                                        &gcvalues);
          if (graphic_contexts == NULL)
            {
              LOG_ERR ("XCreateGC() failed!");
              return FALSE;
            }
        }
    }

  /* initialize XSetWindowAttributes structure */
  window_attributes.background_pixel = black;
  window_attributes.border_pixel = black;
  window_attributes.event_mask = EVENT_MASKS;

  /* creates our unmapped subwindow for the root  window */
  main_window_id = XCreateWindow (x11_display, root_window_id,
                                  /* x-coordinate */
                                  (displayw - window_width) >> 1,
                                  /* y-coordinate */
                                  (displayh - window_height) >> 1,
                                  window_width, window_height,
                                  /* width of the border in pixels */
                                  5,
                                  /* depth is taken from parent */
                                  bits_per_pixel,
                                  /* receive events and is displayed */
                                  InputOutput,
                                  CopyFromParent,
                                  CWEventMask | CWBackPixel | CWBorderPixel,
                                  /* some values */
                                  &window_attributes);
  if (main_window_id == 0)
    {
      LOG_ERR ("XCreateWindow() failed!");
      return FALSE;
    }
  XStoreName (x11_display, main_window_id, "PowerManga by TLK Games");
  XSelectInput (x11_display, main_window_id, EVENT_MASKS);
  protocols = XInternAtom (x11_display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols (x11_display, main_window_id, &protocols, 1);
  /* initialise window hints property */
  hints = XAllocSizeHints ();
  hints->flags = PPosition | PMinSize | PMaxSize;
  hints->x = (displayw - window_width) >> 1;
  hints->y = (displayh - window_height) >> 1;
  hints->x = (displayw - window_width) >> 1;
  hints->y = (displayh - window_height) >> 1;
  hints->min_width = window_width;
  hints->min_height = window_height;
  hints->max_width = window_width;
  hints->max_height = window_height;
  XSetWMNormalHints (x11_display, main_window_id, hints);
  free (hints);
  XClearWindow (x11_display, main_window_id);
  /* display the window */
  XMapRaised (x11_display, main_window_id);
  XFlush (x11_display);

  /* enable DGA mode */
  if (dga_enable)
    {
      XF86DGAQueryDirectVideo (x11_display, dfscreen, &dga_flags);
      if (!(dga_flags & XF86DGADirectPresent))
        {
          LOG_ERR ("DGA's not available!");
          return FALSE;
        }
      XF86DGAGetVideo (x11_display, dfscreen, &dga_base_addr,
                       &dga_width, &dga_bank_size, &dga_ram_size);
      XF86DGAGetViewPortSize (x11_display, dfscreen, &dga_viewport_width,
                              &dga_viewport_height);
      LOG_INF ("dga_base_addr: %p; dga_width: %i; dga_bank_size: %i; "
               "dga_ram_size: %i; dga_viewport_width: %i; dga_viewport_height: %i",
               dga_base_addr, dga_width, dga_bank_size,
               dga_ram_size, dga_viewport_width, dga_viewport_height);
      XF86DGADirectVideo (x11_display, dfscreen,
                          XF86DGADirectGraphics | XF86DGADirectKeyb);
      memset (dga_base_addr, 0,
              dga_viewport_width * dga_viewport_height * bytes_per_pixel);

    }

  disable_cursor ();
  LOG_INF ("end xwindow intialisation "
           "dga_enable: %i; vmode: %i; window_width: %i",
           dga_enable, vmode, window_width);

  /* 8-bit depth screen */
  if (bytes_per_pixel == 1)
    {
      x_color_map =
        XCreateColormap (x11_display, main_window_id,
                         DefaultVisual (x11_display, dfscreen), AllocAll);
      XSetWindowColormap (x11_display, main_window_id, x_color_map);
    }
  return TRUE;
}

/**
 * Destroy off screen for start and end movies
 */
void
destroy_movie_offscreen (void)
{
  if (movie_offscreen != NULL)
    {
      destroy_ximage (movie_ximage);
      movie_offscreen = NULL;
    }
  movie_ximage = NULL;
}

/**
 * Create off screen surface for the start and end movies
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_movie_offscreen (void)
{
  if (!(movie_ximage = create_ximage (display_width, display_height)))
    {
      return FALSE;
    }
  movie_offscreen = movie_ximage->data;
  return TRUE;
}

/**
 * Create 3 or 4 off screens XImages for the game
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_offscreens (void)
{
  /* create XImage "game_offscreen" 512*440 */
  if ((game_ximage = create_ximage (offscreen_width, display_height)) == NULL)
    {
      return FALSE;
    }
  game_offscreen = game_ximage->data;
  offscreen_pitch = offscreen_width * bytes_per_pixel;

  /* create XImage 640x400 (window 640x400) */
  if (vmode > 0 && !dga_enable)
    {
      if (!(scalex_ximage = create_ximage (window_width, window_height)))
        {
          return FALSE;
        }
      scalex_offscreen = scalex_ximage->data;
    }
  if (!(options_ximage = create_ximage (OPTIONS_WIDTH, OPTIONS_HEIGHT)))
    {
      return FALSE;
    }
  options_offscreen = options_ximage->data;
  if (!
      (scores_ximage =
       create_ximage (score_offscreen_width, score_offscreen_height)))
    {
      return FALSE;
    }
  scores_offscreen = scores_ximage->data;
  score_offscreen_pitch = score_offscreen_width * bytes_per_pixel;
  return TRUE;
}

/**
 * Create 16-bit and 24-bit palettes
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_palettes (void)
{
  Uint32 i;
  unsigned char *_pPal;
  unsigned char *_p;

  /* 8-bit depth with a 256-color palette */
  if (bytes_per_pixel == 1)
    {
      if (x_colors == NULL)
        {
          x_colors = (XColor *) memory_allocation (sizeof (XColor) * 256);
        }
      if (x_colors == NULL)
        {
          LOG_ERR ("'x_colors' out of memory");
          return FALSE;
        }
      _p = (unsigned char *) palette_24;

      for (i = 0; i < 256; i++)
        {
          x_colors[i].red = _p[0] << 8;
          x_colors[i].green = _p[1] << 8;
          x_colors[i].blue = _p[2] << 8;
          x_colors[i].pixel = i;
          x_colors[i].flags = DoRed | DoGreen | DoBlue;
          _p += 3;
        }
      XStoreColors (x11_display, x_color_map, x_colors, 256);
      XInstallColormap (x11_display, x_color_map);
    }
  else
    /* 16-bit depth with 65336 colors */
    {
      if (bytes_per_pixel == 2)
        {
          if (pal16 == NULL)
            {
              pal16 = (Uint16 *) memory_allocation (256 * 2);
              if (pal16 == NULL)
                {
                  LOG_ERR ("not enough memory to allocate 512 bytes!");
                  return FALSE;
                }
            }
          if (bits_per_pixel == 15)
            {
              convert_palette_24_to_15 (palette_24, pal16);
            }
          else
            {
              convert_palette_24_to_16 (palette_24, pal16);
            }
        }
      else
        /* 24-bit or 32-bit depth */
        {
          if (bytes_per_pixel > 2)
            {
              if (pal32 == NULL)
                {
                  pal32 = (Uint32 *) memory_allocation (256 * 4);
                }
              if (pal32 == NULL)
                {
                  LOG_ERR ("not enough memory to allocate 1024 bytes!");
                  return FALSE;
                }
              _p = (unsigned char *) pal32;

              _pPal = palette_24;
              for (i = 0; i < 256; i++)
                {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                  _p[3] = _pPal[2];
                  _p[2] = _pPal[1];
                  _p[1] = _pPal[0];
                  _p[0] = 0;
#else
                  _p[0] = _pPal[2];     /* blue */
                  _p[1] = _pPal[1];     /* green */
                  _p[2] = _pPal[0];     /* red */
                  _p[3] = 0;
#endif
                  _p += 4;
                  _pPal += 3;
                }
            }
        }
    }
  return 1;
}

/**
 * Handle events
 */
void
display_handle_events (void)
{
  Sint32 num_events, asciikey;
  char ascii;
  XKeyEvent *xkeyevent;
  KeySym keysym;
  XEvent xevent;

  XFlush (x11_display);
  num_events = XPending (x11_display);
  while ((num_events != 0))
    {
      num_events--;
      XNextEvent (x11_display, &xevent);

      switch (xevent.type)
        {
        case KeyPress:
          {
            xkeyevent = (XKeyEvent *) & xevent;
            asciikey =
              XLookupString ((XKeyEvent *) & xevent, (char *) &ascii,
                             1, &keysym, NULL);
            if (keysym == XK_F10)
              quit_game = TRUE;
            Keys_Down (keysym);
          }
          break;

        case KeyRelease:
          {
            xkeyevent = (XKeyEvent *) & xevent;
            asciikey =
              XLookupString ((XKeyEvent *) & xevent, (char *) &ascii,
                             1, &keysym, NULL);
            Keys_Up (keysym);
          }
          break;

        case Expose:
          update_all = TRUE;
          break;

        case FocusIn:
          break;

        case FocusOut:
          if (dga_enable)
            {
              /* in  DGA if leave the focus then quit the game! */
              quit_game = TRUE;
            }
          break;

        case ClientMessage:
          protocols = XInternAtom (x11_display, "WM_PROTOCOLS", FALSE);
          if (xevent.xclient.message_type == protocols)
            {
              quit_game = TRUE;
            }
          break;

        default:
          LOG_DBG ("evenement type: %i", xevent.type);
          break;
        }
    }
}

/**
 * Display game window
 */
void
display_update_window (void)
{
  /* currently play a movie? */
  if (movie_ximage)
    {
      update_all = TRUE;
      display_movie ();
    }
  else
    {
      /* display with Direct Graphics Access  */
      if (dga_enable)
        {
          switch (vmode)
            {
            case 0:
              dga320x200 ();
              break;
            case 1:
              dga640x400 ();
              break;
            }
        }
      else
        /* display in a window */
        {
          switch (vmode)
            {
            case 0:
              display_320x200 ();
              break;
            case 1:
              display_640x400 ();
              break;
            case 2:
              display_scalex ();
            }
        }
    }
}

/**
 * Draw in DGA
 */
void
dga320x200 ()
{
  Sint32 _iOffset;
  Sint32 optx, opty;
  char *_pSource;
  char *_pDestination, *_pDestination2;
  _pSource =
    game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);
  _iOffset = ((dga_viewport_width - display_width) / 2) * bytes_per_pixel;
  _pDestination =
    dga_base_addr + _iOffset +
    ((dga_viewport_height -
      display_height) / 2 * dga_viewport_width) * bytes_per_pixel;
  _pDestination2 = _pDestination + 16 * dga_viewport_width * bytes_per_pixel;
  copie4octets (_pSource, _pDestination2,
                (offscreen_width_visible * bytes_per_pixel) / 4,
                offscreen_height_visible,
                offscreen_pitch - (offscreen_width_visible * bytes_per_pixel),
                dga_viewport_width * bytes_per_pixel -
                (offscreen_width_visible * bytes_per_pixel));
  if (!update_all)
    {
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          copie4octets (scores_offscreen + (68 * bytes_per_pixel),
                        _pDestination + (68 * bytes_per_pixel),
                        128 / 4 * bytes_per_pixel, 16,
                        score_offscreen_pitch - 128 * bytes_per_pixel,
                        (dga_viewport_width * bytes_per_pixel) -
                        128 * bytes_per_pixel);
        }
      /* display X2 score multiplier */
      if (score_x2_refresh || !update_all)
        {
          copie4octets (options_offscreen + (41 * bytes_per_pixel) +
                        (171 * OPTIONS_WIDTH * bytes_per_pixel),
                        _pDestination +
                        ((offscreen_width_visible + 41) * bytes_per_pixel) +
                        ((score_offscreen_height +
                          171) * dga_viewport_width * bytes_per_pixel),
                        16 / 4 * bytes_per_pixel, 8,
                        OPTIONS_WIDTH * bytes_per_pixel -
                        16 * bytes_per_pixel,
                        (dga_viewport_width * bytes_per_pixel) -
                        16 * bytes_per_pixel);
          score_x2_refresh = FALSE;
        }
      /* display X4 score multiplier */
      if (score_x4_refresh || !update_all)
        {
          copie4octets (options_offscreen + (41 * bytes_per_pixel) +
                        (5 * OPTIONS_WIDTH * bytes_per_pixel),
                        _pDestination +
                        ((offscreen_width_visible + 41) * bytes_per_pixel) +
                        ((score_offscreen_height +
                          5) * dga_viewport_width * bytes_per_pixel),
                        16 / 4 * bytes_per_pixel, 8,
                        OPTIONS_WIDTH * bytes_per_pixel -
                        16 * bytes_per_pixel,
                        (dga_viewport_width * bytes_per_pixel) -
                        16 * bytes_per_pixel);
          score_x4_refresh = FALSE;
        }
      /* display option boxes on the left option panel */
      while (opt_refresh_index >= 0)
        {
          optx = options_refresh[opt_refresh_index].coord_x;
          opty = options_refresh[opt_refresh_index--].coord_y;

          copie4octets (options_offscreen + (optx * bytes_per_pixel) +
                        (opty * OPTIONS_WIDTH * bytes_per_pixel),
                        _pDestination +
                        ((offscreen_width_visible + optx) * bytes_per_pixel) +
                        ((score_offscreen_height +
                          opty) * dga_viewport_width * bytes_per_pixel),
                        28 / 4 * bytes_per_pixel, 28,
                        (OPTIONS_WIDTH - 28) * bytes_per_pixel,
                        dga_viewport_width * bytes_per_pixel -
                        (28 * bytes_per_pixel));
        }
      /* display player's energy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = 0;
          copie4octets (scores_offscreen + (210 * bytes_per_pixel) +
                        (3 * score_offscreen_pitch),
                        _pDestination + (210 * bytes_per_pixel) +
                        (3 * dga_viewport_width * bytes_per_pixel),
                        100 / 4 * bytes_per_pixel, 9,
                        score_offscreen_pitch - 100 * bytes_per_pixel,
                        (dga_viewport_width * bytes_per_pixel) -
                        100 * bytes_per_pixel);
        }
      /* display big-boss's energy */
      if (energy_gauge_guard_is_update)
        {
          energy_gauge_guard_is_update = FALSE;
          copie4octets (scores_offscreen + (10 * bytes_per_pixel) +
                        (3 * score_offscreen_pitch),
                        _pDestination + (10 * bytes_per_pixel) +
                        (3 * dga_viewport_width * bytes_per_pixel),
                        48 / 4 * bytes_per_pixel, 9,
                        score_offscreen_pitch - 48 * bytes_per_pixel,
                        (dga_viewport_width * bytes_per_pixel) -
                        48 * bytes_per_pixel);
        }
    }
  else
    {
      copie4octets (scores_offscreen, _pDestination,
                    score_offscreen_width / 4 * bytes_per_pixel,
                    score_offscreen_height, 0,
                    dga_viewport_width * bytes_per_pixel -
                    (score_offscreen_pitch));
      copie4octets (options_offscreen,
                    _pDestination +
                    (offscreen_width_visible * bytes_per_pixel) +
                    (dga_viewport_width * bytes_per_pixel * 16),
                    OPTIONS_WIDTH / 4 * bytes_per_pixel, OPTIONS_HEIGHT, 0,
                    dga_viewport_width * bytes_per_pixel -
                    (OPTIONS_WIDTH * bytes_per_pixel));
      update_all = false;
      opt_refresh_index = -1;
    }
}

/**
 * Display in DGA mode 640*400
 */
void
dga640x400 ()
{
  Sint32 optx, opty;
  char *_pSource;
  char *_pDestination, *_pDestination2;

  _pSource =
    game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);
  _pSource =
    game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);
  {
    _pDestination =
      dga_base_addr + (((dga_viewport_width - display_width * 2) / 2) +
                       (dga_viewport_height -
                        display_height * 2) / 2 * dga_viewport_width) *
      bytes_per_pixel;
    _pDestination2 =
      _pDestination + 32 * dga_viewport_width * bytes_per_pixel;
    copy2X (_pSource, _pDestination2, offscreen_width_visible,
            offscreen_height_visible,
            offscreen_pitch - (offscreen_width_visible * bytes_per_pixel),
            dga_viewport_width * bytes_per_pixel * 2 -
            (offscreen_width_visible * 2 * bytes_per_pixel));
    if (!update_all)
      {
        if (is_player_score_displayed)
          {
            is_player_score_displayed = FALSE;
            copy2X (scores_offscreen + (68 * bytes_per_pixel),
                    _pDestination + (68 * bytes_per_pixel * 2), 128, 16,
                    score_offscreen_pitch - 128 * bytes_per_pixel,
                    (dga_viewport_width * 2 * bytes_per_pixel) -
                    128 * bytes_per_pixel * 2);
          }
        /* display X2 score multiplier */
        if (score_x2_refresh || !update_all)
          {
            copy2X (options_offscreen + (41 * bytes_per_pixel) +
                    (171 * OPTIONS_WIDTH * bytes_per_pixel),
                    _pDestination +
                    ((offscreen_width_visible + 41) * bytes_per_pixel * 2) +
                    ((score_offscreen_height +
                      171) * dga_viewport_width * bytes_per_pixel * 2), 14, 8,
                    OPTIONS_WIDTH * bytes_per_pixel - 14 * bytes_per_pixel,
                    (dga_viewport_width * bytes_per_pixel * 2) -
                    14 * bytes_per_pixel * 2);
            score_x2_refresh = FALSE;
          }
        /* display X4 score multiplier */
        if (score_x4_refresh || !update_all)
          {
            copy2X (options_offscreen + (41 * bytes_per_pixel) +
                    (5 * OPTIONS_WIDTH * bytes_per_pixel),
                    _pDestination +
                    ((offscreen_width_visible + 41) * bytes_per_pixel * 2) +
                    ((score_offscreen_height +
                      5) * dga_viewport_width * bytes_per_pixel * 2), 14, 8,
                    OPTIONS_WIDTH * bytes_per_pixel - 14 * bytes_per_pixel,
                    (dga_viewport_width * bytes_per_pixel * 2) -
                    14 * bytes_per_pixel * 2);
            score_x4_refresh = FALSE;
          }
        /* display options from option panel */
        while (opt_refresh_index >= 0)
          {
            optx = options_refresh[opt_refresh_index].coord_x;
            opty = options_refresh[opt_refresh_index--].coord_y;
            copy2X (options_offscreen + (optx * bytes_per_pixel) +
                    (opty * OPTIONS_WIDTH * bytes_per_pixel),
                    _pDestination +
                    ((offscreen_width_visible + optx) * bytes_per_pixel * 2) +
                    ((score_offscreen_height +
                      opty) * dga_viewport_width * bytes_per_pixel * 2), 28,
                    28,
                    OPTIONS_WIDTH * bytes_per_pixel - 28 * bytes_per_pixel,
                    (dga_viewport_width * bytes_per_pixel * 2) -
                    28 * bytes_per_pixel * 2);
          }
        /* display player's energy */
        if (energy_gauge_spaceship_is_update)
          {
            energy_gauge_spaceship_is_update = 0;
            copy2X (scores_offscreen + (210 * bytes_per_pixel) +
                    (3 * score_offscreen_pitch),
                    _pDestination + (210 * bytes_per_pixel * 2) +
                    (3 * dga_viewport_width * bytes_per_pixel * 2), 100, 9,
                    score_offscreen_pitch - 100 * bytes_per_pixel,
                    (dga_viewport_width * bytes_per_pixel * 2) -
                    100 * bytes_per_pixel * 2);
          }
        /* display big-boss's energy */
        if (energy_gauge_guard_is_update)
          {
            energy_gauge_guard_is_update = FALSE;
            copy2X (scores_offscreen + (10 * bytes_per_pixel) +
                    (3 * score_offscreen_pitch),
                    _pDestination + (10 * bytes_per_pixel * 2) +
                    (3 * dga_viewport_width * bytes_per_pixel * 2), 45, 9,
                    score_offscreen_pitch - 45 * bytes_per_pixel,
                    (dga_viewport_width * bytes_per_pixel * 2) -
                    45 * bytes_per_pixel * 2);
          }
      }
    else
      {
        copy2X (scores_offscreen, _pDestination, score_offscreen_width,
                score_offscreen_height, 0,
                dga_viewport_width * bytes_per_pixel * 2 -
                (score_offscreen_width * 2 * bytes_per_pixel));
        copy2X (options_offscreen,
                _pDestination +
                (offscreen_width_visible * 2 * bytes_per_pixel) +
                (dga_viewport_width * bytes_per_pixel * 32), OPTIONS_WIDTH,
                OPTIONS_HEIGHT, 0,
                dga_viewport_width * bytes_per_pixel * 2 -
                (OPTIONS_WIDTH * 2 * bytes_per_pixel));
        update_all = false;
        opt_refresh_index = -1;
      }
  }
}

/**
 * Display start movie and end movie
 */
void
display_movie (void)
{
  Sint32 i;
  char *_pDestination;
  switch (bytes_per_pixel)
    {
    case 1:
      for (i = 0; i < (display_width * display_height); i++)
        {
          movie_offscreen[i] = movie_buffer[i];
        }
      break;
    case 2:
      conv8_16 ((char *) movie_buffer, movie_offscreen, pal16,
                display_width * display_height);
      break;
    case 3:
      conv8_24 ((char *) movie_buffer, movie_offscreen, pal32PlayAnim,
                display_width * display_height);
      break;
    case 4:
      conv8_32 ((char *) movie_buffer, movie_offscreen, pal32PlayAnim,
                display_width * display_height);
      break;
    }
  /* display with Direct Graphics Access  */
  if (dga_enable)
    {
      switch (vmode)
        {
          /* 320x200 mode */
        case 0:
          {
            int _iOffset =
              ((dga_viewport_width - display_width) / 2) * bytes_per_pixel;
            _pDestination =
              dga_base_addr + _iOffset +
              ((dga_viewport_height -
                display_height) / 2 * dga_viewport_width) * bytes_per_pixel;
            copie4octets (movie_offscreen, _pDestination,
                          320 / 4 * bytes_per_pixel, 200, 0,
                          dga_viewport_width * bytes_per_pixel -
                          (window_width * bytes_per_pixel));
          }
          break;
          /* 640x400 mode */
        case 1:
          {
            _pDestination =
              dga_base_addr +
              (((dga_viewport_width - display_width * 2) / 2) +
               (dga_viewport_height -
                display_height * 2) / 2 * dga_viewport_width) *
              bytes_per_pixel;
            copy2X (movie_offscreen, _pDestination, 320, 200, 0,
                    dga_viewport_width * bytes_per_pixel * 2 -
                    (window_width * bytes_per_pixel));
          }
          break;
        }
    }

  /* window mode */
  else
    {
      switch (vmode)
        {
          /* 320x200 */
        case 0:
          {
            XPutImage (x11_display, main_window_id, graphic_contexts,
                       movie_ximage, 0, 0, 0, 0, display_width,
                       display_height);
          }
          break;
          /* 640x480 */
        case 1:
          {
            copy2X (movie_offscreen, scalex_offscreen, 320, 200, 0,
                    window_width * bytes_per_pixel * 2 -
                    (window_width * bytes_per_pixel));
            XPutImage (x11_display, main_window_id, graphic_contexts,
                       scalex_ximage, 0, 0, 0, 0, window_width,
                       window_height);
          }
          /* scale2x */
        case 2:
          {
            scale (power_conf->scale_x, scalex_offscreen,
                   window_width * bytes_per_pixel, movie_offscreen,
                   display_width * bytes_per_pixel, bytes_per_pixel,
                   display_width, display_height);
            XPutImage (x11_display, main_window_id, graphic_contexts,
                       scalex_ximage, 0, 0, 0, 0, window_width,
                       window_height);
          }
          break;
        }
    }
}

/**
 * Display window in 320*200
 */
void
display_320x200 ()
{
  Sint32 optx, opty;
  XPutImage (x11_display, main_window_id, graphic_contexts, game_ximage,
             offscreen_clipsize, offscreen_clipsize, 0, 16,
             offscreen_width_visible, offscreen_height_visible);
  if (update_all)
    {
      XPutImage (x11_display, main_window_id, graphic_contexts,
                 options_ximage, 0, 0, offscreen_width_visible, 16,
                 OPTIONS_WIDTH, OPTIONS_HEIGHT);
      /* display score panel */
      XPutImage (x11_display, main_window_id, graphic_contexts,
                 scores_ximage, 0, 0, 0, 0, score_offscreen_width,
                 score_offscreen_height);
      opt_refresh_index = -1;
      update_all = false;
    }
  else
    {

      /* display options from option panel */
      while (opt_refresh_index >= 0)
        {
          optx = options_refresh[opt_refresh_index].coord_x;
          opty = options_refresh[opt_refresh_index--].coord_y;
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     options_ximage, optx, opty,
                     offscreen_width_visible + optx, 16 + opty, 28, 28);
        }
      if (score_x2_refresh)
        {
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     options_ximage, 41, 171, 297, 187, 14, 8);
          score_x2_refresh = FALSE;
        }
      if (score_x4_refresh)
        {
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     options_ximage, 41, 5, 297, 21, 14, 8);
          score_x4_refresh = FALSE;
        }
      /* display player's energy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = 0;
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scores_ximage, 210, 3, 210, 3, 100, 9);
        }
      /* display guardian's energy */
      if (energy_gauge_guard_is_update)
        {
          energy_gauge_guard_is_update = FALSE;
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scores_ximage, 10, 3, 10, 3, 45, 9);
        }
      /* display score number */
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scores_ximage, 68, 0, 68, 0, 128, 16);
        }
    }
}

/**
 * Increase the size of the bitmaps guessing the missing pixels
 */
static void
display_scalex (void)
{
  char *src;
  Sint32 optx, opty;
  Sint32 pitch = window_width * bytes_per_pixel;
  Sint32 scalex = power_conf->scale_x;
  char *pixels = scalex_offscreen;

  /* scale main screen */
  src = game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);
  scale (scalex, pixels + (pitch * score_offscreen_height * scalex), pitch,
         src, offscreen_width * bytes_per_pixel, bytes_per_pixel,
         offscreen_width_visible, offscreen_height_visible);

  /* whole screen will be redisplayed? */
  if (update_all)
    {
      /* display score panel */
      scale (scalex, pixels, pitch, scores_offscreen, score_offscreen_pitch,
             bytes_per_pixel, score_offscreen_width, score_offscreen_height);
      /* display option panel */
      scale (scalex,
             pixels + (offscreen_width_visible * bytes_per_pixel * scalex) +
             (pitch * score_offscreen_height * scalex), pitch,
             options_offscreen, OPTIONS_WIDTH * bytes_per_pixel,
             bytes_per_pixel, OPTIONS_WIDTH, OPTIONS_HEIGHT);
      XPutImage (x11_display, main_window_id, graphic_contexts, scalex_ximage,
                 /* offset source from the top left edge of the image */
                 0, 0,
                 /* offset destination the top left edge of the image */
                 0, 0, window_width, window_height);
      update_all = false;
      opt_refresh_index = -1;
    }
  else
    {
      XPutImage (x11_display, main_window_id, graphic_contexts, scalex_ximage,
                 0, score_offscreen_height * scalex, 0,
                 score_offscreen_height * scalex,
                 offscreen_width_visible * scalex,
                 offscreen_height_visible * scalex);
      /* display player score */
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          scale (scalex, pixels + (68 * bytes_per_pixel * scalex), pitch,
                 scores_offscreen + (68 * bytes_per_pixel),
                 score_offscreen_pitch, bytes_per_pixel, 128, 16);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 68 * scalex, 0, 68 * scalex, 0,
                     128 * scalex, 16 * scalex);
        }
      if (score_x2_refresh)
        {
          score_x2_refresh = FALSE;
          scale (scalex,
                 pixels +
                 ((offscreen_width_visible + 41) * bytes_per_pixel * scalex) +
                 (score_offscreen_height + 171) * pitch * scalex, pitch,
                 options_offscreen + (41 * bytes_per_pixel) +
                 (171 * OPTIONS_WIDTH * bytes_per_pixel),
                 OPTIONS_WIDTH * bytes_per_pixel, bytes_per_pixel, 14, 8);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + 41) * scalex,
                     (score_offscreen_height + 171) * scalex,
                     (offscreen_width_visible + 41) * scalex,
                     (score_offscreen_height + 171) * scalex, 14 * scalex,
                     8 * scalex);
        }
      if (score_x4_refresh)
        {
          score_x4_refresh = FALSE;
          scale (scalex,
                 pixels +
                 ((offscreen_width_visible + 41) * bytes_per_pixel * scalex) +
                 (score_offscreen_height + 5) * pitch * scalex, pitch,
                 options_offscreen + (41 * bytes_per_pixel) +
                 (5 * OPTIONS_WIDTH * bytes_per_pixel),
                 OPTIONS_WIDTH * bytes_per_pixel, bytes_per_pixel, 14, 8);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + 41) * scalex,
                     (score_offscreen_height + 5) * scalex,
                     (offscreen_width_visible + 41) * scalex,
                     (score_offscreen_height + 5) * scalex, 14 * scalex,
                     8 * scalex);
        }
      while (opt_refresh_index >= 0)
        {
          optx = options_refresh[opt_refresh_index].coord_x;
          opty = options_refresh[opt_refresh_index--].coord_y;
          scale (scalex,
                 pixels + (offscreen_width_visible +
                           optx) * bytes_per_pixel * scalex +
                 (score_offscreen_height + opty) * pitch * scalex, pitch,
                 options_offscreen + (optx * bytes_per_pixel) +
                 (opty * OPTIONS_WIDTH * bytes_per_pixel),
                 OPTIONS_WIDTH * bytes_per_pixel, bytes_per_pixel, 28, 28);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + optx) * scalex,
                     (score_offscreen_height + opty) * scalex,
                     (offscreen_width_visible + optx) * scalex,
                     (score_offscreen_height + opty) * scalex, 28 * scalex,
                     28 * scalex);
        }

      /* display player's ernergy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = 0;
          scale (scalex,
                 pixels + 210 * bytes_per_pixel * scalex + 3 * pitch * scalex,
                 pitch,
                 scores_offscreen + 210 * bytes_per_pixel +
                 3 * score_offscreen_pitch, score_offscreen_pitch,
                 bytes_per_pixel, 100, 9);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 210 * scalex, 3 * scalex, 210 * scalex,
                     3 * scalex, 100 * scalex, 9 * scalex);
        }

      /* display guardian's energy */
      if (energy_gauge_guard_is_update)
        {
          energy_gauge_guard_is_update = FALSE;
          scale (scalex,
                 pixels + 10 * bytes_per_pixel * scalex + 3 * pitch * scalex,
                 pitch,
                 scores_offscreen + 10 * bytes_per_pixel +
                 3 * score_offscreen_pitch, score_offscreen_pitch,
                 bytes_per_pixel, 45, 9);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 10 * scalex, 3 * scalex, 10 * scalex,
                     3 * scalex, 45 * scalex, 9 * scalex);
        }
    }
}

/**
 * Display window in 640*400
 * playfield 512x440 ; score panel 320x16 ; option panel 64x184
 * update_all == TRUE then display whole 640x400 window
 */
static void
display_640x400 (void)
{
  Sint32 optx, opty;
  char *_pSource =
    game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);

  /* recopy the main screen by it doubling */
  copy2X_512x440 (_pSource,
                  scalex_offscreen + (window_width * bytes_per_pixel * 32),
                  offscreen_height_visible);

  /** whole screen will be redisplayed? */
  if (update_all)
    {
      copy2X (scores_offscreen, scalex_offscreen, score_offscreen_width,
              score_offscreen_height, 0,
              window_width * bytes_per_pixel * 2 -
              (score_offscreen_width * 2 * bytes_per_pixel));
      copy2X (options_offscreen,
              scalex_offscreen +
              (offscreen_width_visible * 2 * bytes_per_pixel) +
              (window_width * bytes_per_pixel * 32), OPTIONS_WIDTH,
              OPTIONS_HEIGHT, 0,
              window_width * bytes_per_pixel * 2 -
              (OPTIONS_WIDTH * 2 * bytes_per_pixel));
      XPutImage (x11_display, main_window_id, graphic_contexts, scalex_ximage,
                 0, 0, 0, 0, window_width, window_height);
      update_all = false;
      opt_refresh_index = -1;
    }
  else
    {
      XPutImage (x11_display, main_window_id, graphic_contexts,
                 scalex_ximage, 0, 32, 0, 32,
                 offscreen_width_visible * 2, offscreen_height_visible * 2);

      /* display player score */
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          copy2X (scores_offscreen + (68 * bytes_per_pixel),
                  scalex_offscreen + (68 * bytes_per_pixel * 2), 128, 16,
                  score_offscreen_pitch - 128 * bytes_per_pixel,
                  (window_width * 2 * bytes_per_pixel) -
                  128 * bytes_per_pixel * 2);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 68 * 2, 0, 68 * 2, 0, 128 * 2, 16 * 2);
        }
      if (score_x2_refresh)
        {
          copy2X (options_offscreen + (41 * bytes_per_pixel) +
                  (171 * OPTIONS_WIDTH * bytes_per_pixel),
                  scalex_offscreen +
                  ((offscreen_width_visible + 41) * bytes_per_pixel * 2) +
                  ((score_offscreen_height +
                    171) * window_width * bytes_per_pixel * 2), 14, 8,
                  OPTIONS_WIDTH * bytes_per_pixel - 14 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  14 * bytes_per_pixel * 2);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + 41) * 2,
                     (score_offscreen_height + 171) * 2,
                     (offscreen_width_visible + 41) * 2,
                     (score_offscreen_height + 171) * 2, 14 * 2, 8 * 2);
          score_x2_refresh = FALSE;
        }

      if (score_x4_refresh)
        {
          copy2X (options_offscreen + (41 * bytes_per_pixel) +
                  (5 * OPTIONS_WIDTH * bytes_per_pixel),
                  scalex_offscreen +
                  ((offscreen_width_visible + 41) * bytes_per_pixel * 2) +
                  ((score_offscreen_height +
                    5) * window_width * bytes_per_pixel * 2), 14, 8,
                  OPTIONS_WIDTH * bytes_per_pixel - 14 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  14 * bytes_per_pixel * 2);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + 41) * 2,
                     (score_offscreen_height + 5) * 2,
                     (offscreen_width_visible + 41) * 2,
                     (score_offscreen_height + 5) * 2, 14 * 2, 8 * 2);
          score_x4_refresh = FALSE;
        }

      /* display options from option panel */
      while (opt_refresh_index >= 0)
        {
          optx = options_refresh[opt_refresh_index].coord_x;
          opty = options_refresh[opt_refresh_index--].coord_y;

          copy2X (options_offscreen + (optx * bytes_per_pixel) +
                  (opty * OPTIONS_WIDTH * bytes_per_pixel),
                  scalex_offscreen +
                  ((offscreen_width_visible + optx) * bytes_per_pixel * 2) +
                  ((score_offscreen_height +
                    opty) * window_width * bytes_per_pixel * 2), 28, 28,
                  OPTIONS_WIDTH * bytes_per_pixel - 28 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  28 * bytes_per_pixel * 2);

          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, (offscreen_width_visible + optx) * 2,
                     (score_offscreen_height + opty) * 2,
                     (offscreen_width_visible + optx) * 2,
                     (score_offscreen_height + opty) * 2, 28 * 2, 28 * 2);
        }

      /* display player's ernergy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = 0;
          copy2X (scores_offscreen + (210 * bytes_per_pixel) +
                  (3 * score_offscreen_pitch),
                  scalex_offscreen + (210 * bytes_per_pixel * 2) +
                  (3 * window_width * bytes_per_pixel * 2), 100, 9,
                  score_offscreen_pitch - 100 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  100 * bytes_per_pixel * 2);

          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 210 * 2, 3 * 2, 210 * 2, 3 * 2, 100 * 2,
                     9 * 2);
        }
      /* display guardian's energy */
      if (energy_gauge_guard_is_update)
        {
          energy_gauge_guard_is_update = FALSE;
          copy2X (scores_offscreen + (10 * bytes_per_pixel) +
                  (3 * score_offscreen_pitch),
                  scalex_offscreen + (10 * bytes_per_pixel * 2) +
                  (3 * window_width * bytes_per_pixel * 2), 45, 9,
                  score_offscreen_pitch - 45 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  45 * bytes_per_pixel * 2);
          XPutImage (x11_display, main_window_id, graphic_contexts,
                     scalex_ximage, 10 * 2, 3 * 2, 10 * 2, 3 * 2, 45 * 2,
                     9 * 2);
        }
    }
}

/**
 * Destroy Window , free graphics contexts and close X11 connection
 */
void
display_free (void)
{
  destroy_ximages ();
  game_offscreen = NULL;
  game_ximage = NULL;
  scalex_offscreen = NULL;
  scalex_ximage = NULL;
  scores_offscreen = NULL;
  scores_ximage = NULL;
  options_offscreen = NULL;
  options_ximage = NULL;
  if (main_window_id != 0)
    {
      XDestroyWindow (x11_display, main_window_id);
      main_window_id = 0;
      LOG_DBG ("XDestroyWindow()");
    }
  if (graphic_contexts != NULL)
    {
      XFreeGC (x11_display, graphic_contexts);
      graphic_contexts = NULL;
      LOG_DBG ("XFreeGC()");
    }
  if (x11_display != NULL)
    {
      XCloseDisplay (x11_display);
      x11_display = NULL;
      LOG_DBG ("XCloseDisplay()");
    }
  if (x_colors != NULL)
    {
      free_memory ((char *) x_colors);
      x_colors = NULL;
    }
  if (pal16 != NULL)
    {
      free_memory ((char *) pal16);
      pal16 = NULL;
    }
  if (pal32 != NULL)
    {
      free_memory ((char *) pal32);
      pal32 = NULL;
    }
}

/**
 * Clear the main offscreen
 */
void
display_clear_offscreen (void)
{
  /* clear logical screen */
  clear_offscreen (game_offscreen +
                   (offscreen_clipsize * offscreen_pitch) +
                   (offscreen_clipsize * bytes_per_pixel),
                   (offscreen_width_visible * bytes_per_pixel) >> 2,
                   offscreen_height_visible,
                   (offscreen_width -
                    offscreen_width_visible) * bytes_per_pixel);
}

/**
 * Initialize small cursor (one pixel)
 */
static void
disable_cursor (void)
{
  Sint32 dfscreen = DefaultScreen (x11_display);
  Cursor cursor;
  XColor color;
  Pixmap cursorPixmap;
  color.pixel = WhitePixel (x11_display, dfscreen);
  cursorPixmap = XCreatePixmap (x11_display, main_window_id, 1, 1, 1);
  cursor =
    XCreatePixmapCursor (x11_display, cursorPixmap, cursorPixmap,
                         &color, &color, 0, 0);
  if (cursorPixmap != 0)
    {
      XFreePixmap (x11_display, cursorPixmap);
    }
  XDefineCursor (x11_display, main_window_id, cursor);
}

/**
 * Create a XImage
 * @param width Width of the XImage is pixels
 * @param height Height of the XImage is pixels
 * @return Pointer to a XImage
 */
#ifdef UTILISE_XSHM
XImage *
create_ximage (Uint32 width, Uint32 height)
{
  XImage *ximg;
  Uint32 i;
  Sint32 index = -1;
  for (i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] == 0)
        {
          index = i;
          break;
        }
    }
  if (index < 0)
    {
      LOG_ERR ("ximages list overflows!");
      return NULL;
    }

  ximg =
    XShmCreateImage (x11_display, CopyFromParent, bits_per_pixel,
                     ZPixmap, 0, &SHMInfo[index], width, height);
  if (ximg == NULL)
    {
      LOG_ERR ("XShmCreateImage() failed!");
      return NULL;
    }
  SHMInfo[index].shmid =
    shmget (IPC_PRIVATE, ximg->bytes_per_line * ximg->height,
            IPC_CREAT | 0777);
  ximg->data = SHMInfo[index].shmaddr =
    (char *) shmat (SHMInfo[index].shmid, 0, 0);
  SHMInfo[index].readOnly = False;
  XShmAttach (x11_display, &SHMInfo[index]);
  ximages_list[index] = ximg;
  ximages_counter++;
  LOG_DBG ("XShmCreateImage(%i, %i)", width, height);
  return ximg;
}
#else
XImage *
create_ximage (Uint32 width, Uint32 height)
{
  XImage *ximg;
  char *data;
  Sint32 pad;
  Sint32 lenght;
  Sint32 index = -1;

  for (Uint32 i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] == 0)
        {
          index = i;
          break;
        }
    }
  if (index < 0)
    {
      LOG_ERR ("ximages list overflows!");
      return NULL;
    }
  pad = pixmapFormat.scanline_pad;
  lenght = pixmapFormat.bits_per_pixel * offscreen_width / 8;
  if ((lenght & (pixmapFormat.scanline_pad / 8 - 1)) != 0)
    {
      lenght &= ~(pixmapFormat.scanline_pad / 8 - 1);
      lenght += pixmapFormat.scanline_pad / 8;
    }

  data = memory_allocation (width * height * bytes_per_pixel);
  if (data == NULL)
    {
      LOG_ERR ("out of memory!");
      return NULL;
    }

  *ximg =
    XCreateImage (x11_display, CopyFromParent, bits_per_pixel, ZPixmap,
                  0, data, width, height, pad, width * bytes_per_pixel);
  if (ximg == NULL)
    {
      LOG_ERR ("XCreateImage() failed!");
      return NULL;
    }
  ximages_list[index] = ximg;
  ximages_counter++;
  LOG_DBG ("XCreateImage() successful!");
  return ximg;
}
#endif

/**
 * Free a XImage
 * @param ximg Pointer to a XImage
 */
#ifdef UTILISE_XSHM
static void
destroy_ximage (XImage * ximg)
{
  for (Uint32 i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] == ximg)
        {
          XShmDetach (x11_display, &SHMInfo[i]);
          if (SHMInfo[i].shmaddr)
            {
              shmdt (SHMInfo[i].shmaddr);
            }
          if (SHMInfo[i].shmid >= 0)
            {
              shmctl (SHMInfo[i].shmid, IPC_RMID, 0);
            }
          XDestroyImage (ximg);
          ximages_list[i] = NULL;
          ximages_counter--;
          LOG_DBG ("XShmDetach() and XDestroyImage()");
          break;
        }
    }
}
#else
void
destroy_ximage (XImage * ximg)
{
  for (Uint32 i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] == ximg)
        {
          free_memory (ximg->data);
          ximg->data = NULL;
          XDestroyImage (ximg);
          ximages_list[i] = NULL;
          ximages_counter--;
          LOG_INF ("XDestroyImage()");
          break;
        }
    }
}
#endif

/** 
 * Free all XImages
 */
#ifdef UTILISE_XSHM
static void
destroy_ximages (void)
{
  Uint32 i;
  for (i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] != NULL)
        {
          XShmDetach (x11_display, &SHMInfo[i]);
          if (SHMInfo[i].shmaddr)
            {
              shmdt (SHMInfo[i].shmaddr);
            }
          if (SHMInfo[i].shmid >= 0)
            {
              shmctl (SHMInfo[i].shmid, IPC_RMID, 0);
            }
          XDestroyImage (ximages_list[i]);
          ximages_list[i] = NULL;
          ximages_counter--;
          LOG_DBG ("XShmDetach and XDestroyImage");
        }
    }
}
#else
static void
destroy_ximages (void)
{
  Uint32 i;
  for (i = 0; i < MAXIMUM_OF_XIMAGES; i++)
    {
      if (ximages_list[i] != NULL)
        {
          free_memory (ximages_list[i]->data);
          ximages_list[i]->data = NULL;
          XDestroyImage (ximages_list[i]);
          ximages_list[i] = NULL;
          ximages_counter--;
          LOG_INF ("XDestroyImage()");
        }
    }
}
#endif

/**
 * Key pressed
 * @prama keycode
 */
void
Keys_Down (Uint32 keycode)
{
  /* save down key code */
  sprites_string_key_down (keycode, keycode);
  key_code_down = keycode;
  keytouch (keycode, TRUE);
}

/**
 * Key released
 * @param keycode
 */
void
Keys_Up (Uint32 keycode)
{
  /* save up key code */
  sprites_string_key_up (keycode, keycode);
  if (key_code_down == keycode)
    {
      key_code_down = 0;
    }
  keytouch (keycode, FALSE);
}

/** 
 * Down or up key
 * @param keycode
 * @param i FALSE = up key, otherwise TRUE = down key
 */
static void
keytouch (Uint32 keycode, bool i)
{
  if (keycode >= 'a' && keycode <= 'z')
    {
      keycode = keycode - 32;
    }
  switch (keycode)
    {
    case XK_Escape:
      keys_down[K_ESCAPE] = i;
      break;
    case XK_Control_L:
      keys_down[K_CTRL] = i;
      break;
    case XK_Control_R:
      keys_down[K_CTRL] = i;
      break;
    case XK_Return:
      keys_down[K_RETURN] = i;
      break;
    case XK_Pause:
      keys_down[K_PAUSE] = i;
      break;
    case XK_Shift_L:
      keys_down[K_SHIFT] = i;
      break;
    case XK_Shift_R:
      keys_down[K_SHIFT] = i;
      break;
    case XK_F1:
      keys_down[K_F1] = i;
      break;
    case XK_F2:
      keys_down[K_F2] = i;
      break;
    case XK_F3:
      keys_down[K_F3] = i;
      break;
    case XK_F4:
      keys_down[K_F4] = i;
      break;
    case XK_F5:
      keys_down[K_F5] = i;
      break;
    case XK_F6:
      keys_down[K_F6] = i;
      break;
    case XK_F7:
      keys_down[K_F7] = i;
      break;
    case XK_F8:
      keys_down[K_F8] = i;
      break;
    case XK_F9:
      keys_down[K_F9] = i;
      break;
    case XK_F10:
      keys_down[K_F10] = i;
      break;
    case XK_F11:
      keys_down[K_F11] = i;
      break;
    case XK_F12:
      keys_down[K_F12] = i;
      break;
    case XK_Insert:
      keys_down[K_INSERT] = i;
      break;
    case XK_space:
      keys_down[K_SPACE] = i;
      break;
    case XK_Left:
      keys_down[K_LEFT] = i;
      break;
    case XK_Right:
      keys_down[K_RIGHT] = i;
      break;
    case XK_Up:
      keys_down[K_UP] = i;
      break;
    case XK_Down:
      keys_down[K_DOWN] = i;
      break;
      /* [Ctrl] + [A]: ABOUT */
    case 'A':
      keys_down[K_A] = i;
      break;
    case 'V':
      keys_down[K_V] = i;
      break;
    case 'B':
      keys_down[K_B] = i;
      break;
      /* [P]: enable/disable pause */
    case 'P':
      keys_down[K_P] = i;
      break;
      /* [Ctrl] + [Q]: force "Game Over */
    case 'Q':
      keys_down[K_Q] = i;
      break;
      /* [Ctrl] + [S]: enable/disable the music */
    case 'S':
      keys_down[K_S] = i;
      break;
    }
}


/*
dga_enable=0 && vmode=0
- display sprites in "game_offscreen" (512*440)
- put "game_offscreen" in window (256*184) with XPutImage function

dga_enable=0 && vmode=1
- display sprites in "game_offscreen" (512*440)
- copy "game_offscreen" in "scalex_offscreen" (double pixels horizontal and skip a line)
*/
#endif
