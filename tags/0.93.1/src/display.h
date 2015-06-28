/**
 * @file display.h
 * @brief handle displaying and updating the graphical components of the game
 * @created 2006-12-03
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: display.h,v 1.43 2012/08/26 15:44:26 gurumeditation Exp $
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
#ifndef __DISPLAY__
#define __DISPLAY__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(POWERMANGA_SDL)  && !defined(_WIN32_WCE)
#define USE_SDL_JOYSTICK
#endif

/** Width of option right side panel */
#define OPTIONS_WIDTH 64
/** Height of option right side panel */
#define OPTIONS_HEIGHT 184
/** Width of cores top side panel */
#define SCORES_WIDTH 320
/** Height of cores top side panel */
#define SCORES_HEIGHT 16
/** Abscissa visible surface area */
#define OFFSCREEN_STARTX 128
/** Ordinate visible surface area */
#define OFFSCREEN_STARTY 128

  /** Key codes of Powermanga */
  typedef enum
  {
    K_ESCAPE,
    K_F1,
    K_F2,
    K_F3,
    K_F4,
    K_F5,
    K_F6,
    K_F7,
    K_F8,
    K_F9,
    K_F10,
    K_F11,
    K_F12,
    K_INSERT,
    K_1,
    K_2,
    K_3,
    K_4,
    K_5,
    K_6,
    K_7,
    K_8,
    K_9,
    K_0,
    K_A,
    K_F,
    K_E,
    K_T,
    K_O,
    K_P,
    K_RETURN,
    K_Q,
    K_S,
    K_G,
    K_L,
    K_C,
    K_V,
    K_B,
    K_N,
    K_UP,
    K_LEFT,
    K_RIGHT,
    K_SHIFT,
    K_DOWN,
    K_CTRL,
    K_SPACE,
    K_PAUSE,
    K_PAGEUP,
    K_PAGEDOWN,
    MAX_OF_KEYS_DOWN
  }
  KEYS_CODES;

  unsigned char search_color (unsigned char r, unsigned char g,
                              unsigned char b);

  bool display_initialize (void);
  void display_release (void);
  void display_colors_init (void);

  bool display_init (void);
  bool create_offscreens (void);
  bool create_palettes (void);
  void display_handle_events (void);
  void display_update_window (void);
  void display_free (void);
  void display_clear_offscreen (void);
  char *load_pcx_file (const char *filename);
  bool load_pcx_into_buffer (const char *filename, char *buffer);
  bool create_movie_offscreen (void);
  void destroy_movie_offscreen (void);
#ifdef POWERMANGA_SDL
  bool init_video_mode (void);
#ifdef USE_SDL_JOYSTICK
  bool display_open_joysticks (void);
#endif
#endif
  void clear_keymap (void);

#ifdef SHAREWARE_VERSION
  void show_page_order (int num, char *lang, int cpt);
#endif


  extern Sint32 display_width;
  extern Sint32 display_height;
  extern Sint32 offscreen_width;
  extern Sint32 offscreen_height;
  extern Sint32 offscreen_width_visible;
  extern Sint32 offscreen_height_visible;
  extern Sint32 offscreen_startx;
  extern Sint32 offscreen_starty;
  extern Sint32 offscreen_clipsize;
  extern Sint32 pixel_size;
  extern Sint32 screen_pixel_size;

  extern Sint32 score_offscreen_width;
  extern Sint32 score_offscreen_height;

  typedef enum
  {
    BLACK,
    GRIS,
    LIGHT_GRAY,
    WHITE,
    RED,
    GREEN,
    COLORS_ENUM_NUMOF
  }
  COLORS_ENUM;

  typedef enum
  {
    PORTRAIT,
    RIGHT_LANDSCAPE,
    LEFT_LANDSCAPE
  }
  SCREEN_ORIENTATION;

  extern Uint32 bytes_per_pixel;
  extern Uint32 bits_per_pixel;
  extern unsigned char *palette_24;
  extern Uint32 *pal32;
  extern Uint16 *pal16;
  extern Uint16 *pal16PlayAnim;
  extern Uint32 *pal32PlayAnim;
  extern Uint32 offscreen_pitch;
  extern Uint32 score_offscreen_pitch;
  extern Sint32 vmode;
  extern char *scores_offscreen;
  extern char *options_offscreen;
  extern char *movie_offscreen;
  extern char *scalex_offscreen;
  extern char *game_offscreen;
  extern bool *keys_down;
  extern Uint32 key_code_down;
  extern bool fire_button_down;
  extern bool option_button_down;
  extern bool start_button_down;
  extern bool joy_left;
  extern bool joy_right;
  extern bool joy_top;
  extern bool joy_down;
  extern Sint32 mouse_b;
  extern Sint32 mouse_x;
  extern Sint32 mouse_y;
  extern unsigned char coulor[COLORS_ENUM_NUMOF];
  extern Uint32 real_black_color;

/* common */
  extern bool update_all;
  extern Uint32 window_width;
  extern Uint32 window_height;
  extern bool is_iconified;
  extern SCREEN_ORIENTATION screen_orientation;

#ifdef __cplusplus
}
#endif
#endif
