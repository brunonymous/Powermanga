/**
 * @file display_sdl.c
 * @brief handle displaying and updating the graphical components of the game
 * @created 2003-07-09
 * @date 2015-01-10 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: display_sdl.c,v 1.44 2012/08/26 19:30:40 gurumeditation Exp $
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
#include "menu_sections.h"
#include "movie.h"
#include "log_recorder.h"
#include "options_panel.h"
#include "gfx_wrapper.h"
#ifdef USE_SCALE2X
#include "scalebit.h"
#endif
#include "script_page.h"
#include "sprites_string.h"
#include "texts.h"

#ifdef POWERMANGA_SDL

#if defined(POWERMANGA_GP2X) || defined(_WIN32_WCE)
static Uint32 display_offset_y = 0;
#endif

#ifdef POWERMANGA_GP2X
/* GP2X button codes, as received through SDL joystick events */
typedef enum
{
  GP2X_BUTTON_UP,
  GP2X_BUTTON_UPLEFT,
  GP2X_BUTTON_LEFT,
  GP2X_BUTTON_DOWNLEFT,
  GP2X_BUTTON_DOWN,
  GP2X_BUTTON_DOWNRIGHT,
  GP2X_BUTTON_RIGHT,
  GP2X_BUTTON_UPRIGHT,
  GP2X_BUTTON_START,
  GP2X_BUTTON_SELECT,
  GP2X_BUTTON_R,
  GP2X_BUTTON_L,
  GP2X_BUTTON_A,
  GP2X_BUTTON_B,
  GP2X_BUTTON_Y,
  GP2X_BUTTON_X,
  GP2X_BUTTON_VOLUP,
  GP2X_BUTTON_VOLDOWN,
  GP2X_BUTTON_CLICK,
  GP2X_NUM_BUTTONS
} GP2X_BUTTONS_CODE;
/* The current state of all the GP2X buttons is stored in
 * this array - used to handle multi-key actions */
static bool gp2x_buttons[GP2X_NUM_BUTTONS];
/* The resolution the GP2X runs at */
static const Sint32 GP2X_VIDEO_HEIGHT = 240;
#endif

#ifdef POWERMANGA_PSP
/* PSP button codes, as received through SDL joystick events */
typedef enum
{
  PSP_BUTTON_Y,
  PSP_BUTTON_B,
  PSP_BUTTON_A,
  PSP_BUTTON_X,
  PSP_BUTTON_L,
  PSP_BUTTON_R,
  PSP_BUTTON_DOWN,
  PSP_BUTTON_LEFT,
  PSP_BUTTON_UP,
  PSP_BUTTON_RIGHT,
  PSP_BUTTON_SELECT,
  PSP_BUTTON_START,
  PSP_NUM_BUTTONS
} PSP_BUTTONS_CODE;
bool psp_buttons[PSP_NUM_BUTTONS];
#endif

/* SDL surfaces */
#define MAX_OF_SURFACES 100
static SDL_Surface *public_surface = NULL;
/** 512x440: game's offscreen  */
static SDL_Surface *game_surface = NULL;
/** offscreen to resize to 640x400, 960x600 or 1280x800 */
static SDL_Surface *scalex_surface = NULL;
/** 64*184: right options panel */
static SDL_Surface *options_surface = NULL;
static SDL_Surface *score_surface = NULL;
/** 320x200: movie animation */
static SDL_Surface *movie_surface = NULL;
static Uint32 surfaces_counter = 0;
static SDL_Surface *surfaces_list[MAX_OF_SURFACES];

#ifdef USE_SDL_JOYSTICK
/** Number of available joysticks */
static Uint32 numof_joysticks = 0;
static SDL_Joystick **sdl_joysticks = NULL;
#endif
#ifdef WIN32
/** Gf (vmode == 1) vmode2 = 0 (640x400) or vmode2 = 1 (640x480) */
Sint32 vmode2 = 1;
#else
Sint32 vmode2 = 0;
#endif

static void display_movie (void);
static void display_320x200 (void);
static void display_640x400 (void);
#ifdef USE_SCALE2X
static void display_scale_x (void);
#endif
static SDL_Surface *create_surface (Uint32 width, Uint32 height);
static void get_rgb_mask (Uint32 * rmask, Uint32 * gmask, Uint32 * bmask);
static void free_surface (SDL_Surface * surface);
static void free_surfaces (void);
#ifdef USE_SDL_JOYSTICK
void display_close_joysticks (void);
#endif
void key_status (Uint8 * k);
/** Color table in 8-bit depth */
SDL_Color *sdl_color_palette = NULL;
static const char window_tile[] = POWERMANGA_VERSION " by TLK Games (SDL)\0";
/** If TRUE reverses the horizontal and vertical controls */
static bool is_reverse_ctrl = FALSE;
static bool pause_will_disable = FALSE;

/**
 * Initialize SDL display
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
display_init (void)
{
  Uint32 i;
  Uint32 sdl_flag;

#ifndef POWERMANGA_HANDHELD_CONSOLE
  const SDL_VideoInfo *vi;
  char driver_name[32];
#endif
  for (i = 0; i < MAX_OF_SURFACES; i++)
    {
      surfaces_list[i] = (SDL_Surface *) NULL;
    }

  switch (vmode)
    {
    case 0:
      window_width = display_width;
      window_height = display_height;
      vmode2 = 0;
      break;
    case 1:
      window_width = display_width * 2;
      window_height = display_height * 2;
#ifdef WIN32
      vmode2 = 1;
#else
      vmode2 = 0;
#endif
      break;
    case 2:
      window_width = display_width * power_conf->scale_x;
      window_height = display_height * power_conf->scale_x;
      vmode2 = 0;
#ifdef WIN32
      if (window_height == 400)
        {
          vmode2 = 1;
        }
#endif
      break;
    }

  /* initialize SDL screen */
  sdl_flag = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
#ifdef USE_SDL_JOYSTICK
  sdl_flag |= SDL_INIT_JOYSTICK;
#endif
  if (SDL_Init (sdl_flag) < 0)
    {
      LOG_ERR ("SDL_Init() failed: %s", SDL_GetError ());
      return FALSE;
    }
#ifdef USE_SDL_JOYSTICK
  if (!display_open_joysticks ())
    {
      return FALSE;
    }
#endif

#ifdef POWERMANGA_HANDHELD_CONSOLE
  /* force 8 bits per pixel if running on the GP2X or PSP */
  bits_per_pixel = 8;
  bytes_per_pixel = 1;
#else
  if (power_conf->extract_to_png)
    {
      bits_per_pixel = 8;
      bytes_per_pixel = 1;
    }
  else
    {
      vi = SDL_GetVideoInfo ();
      bits_per_pixel = vi->vfmt->BitsPerPixel;
      bytes_per_pixel = vi->vfmt->BytesPerPixel;
      if (bits_per_pixel == 16)
        {
          bits_per_pixel =
            vi->vfmt->Rshift + vi->vfmt->Gshift + vi->vfmt->Bshift;
        }
      if (bits_per_pixel < 8)
        {

          LOG_ERR ("Powermanga need 8 bits per pixels minimum"
                   " (256 colors)");
          return FALSE;
        }
      LOG_DBG ("Rmask=%i Gmask=%i Bmask=%i Amask: %i", vi->vfmt->Rmask,
               vi->vfmt->Gmask, vi->vfmt->Bmask, vi->vfmt->Amask);
      LOG_DBG ("Rshift=%i Gshift=%i Bshift=%i Ashift: %i", vi->vfmt->Rshift,
               vi->vfmt->Gshift, vi->vfmt->Bshift, vi->vfmt->Ashift);
      LOG_DBG ("Rloss=%i Gloss=%i Bloss=%i Aloss: %i", vi->vfmt->Rloss,
               vi->vfmt->Gloss, vi->vfmt->Bloss, vi->vfmt->Aloss);
      LOG_DBG ("Pixel value of transparent pixels. colorkey: %i",
               vi->vfmt->colorkey);
      LOG_DBG ("Overall surface alpha value. alpha=%i", vi->vfmt->alpha);
      LOG_DBG ("Is it possible to create hardware surfaces? hw_available: %i",
               vi->hw_available);
      LOG_DBG ("Is there a window manager available wm_available: %i",
               vi->wm_available);
      LOG_DBG ("Are hardware to hardware blits accelerated? blit_hw: %i",
               vi->blit_hw);
      LOG_DBG
        ("Are hardware to hardware colorkey blits accelerated? blit_hw_CC: %i",
         vi->blit_hw_CC);
      LOG_DBG
        ("Are hardware to hardware alpha blits accelerated? blit_hw_A: %i",
         vi->blit_hw_A);
      LOG_DBG ("Are software to hardware blits accelerated? blit_sw: %i",
               vi->blit_sw);
      LOG_DBG
        ("Are software to hardware colorkey blits accelerated? blit_sw_CC: %i",
         vi->blit_sw_CC);
      LOG_DBG
        ("Are software to hardware alpha blits accelerated? blit_sw_A: %i",
         vi->blit_sw_A);
      LOG_DBG ("Are color fills accelerated? blit_fill: %i", vi->blit_fill);
      LOG_DBG ("Total amount of video memory in Kilobytes. video_mem: %i",
               vi->video_mem);
      if (SDL_VideoDriverName (driver_name, 32) != NULL)
        {
          LOG_INF ("the name of the video driver: %s", driver_name);
        }

    }
#endif

  LOG_INF ("depth of screen: %i; bytes per pixel: %i",
           bits_per_pixel, bytes_per_pixel);
  if (!init_video_mode ())
    {
      return FALSE;
    }
  SDL_EnableUNICODE (1);
  LOG_INF ("video has been successfully initialized");
  return TRUE;
}

/**
 * Initialize video mode
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
init_video_mode (void)
{
  Uint32 i;
#ifndef POWERMANGA_HANDHELD_CONSOLE
  Uint32 bpp;
  Uint32 width, height;
  SDL_Rect **modes;
#endif
  Uint32 flag;

  /* 640x480 instead of 640x400 on win32 only */
  if ((vmode) && (vmode2) && power_conf->scale_x == 2)
    {
      window_height = 480;
    }


#ifdef POWERMANGA_HANDHELD_CONSOLE
  /* Ignore any video mode options if running on the GP2X or PSP
   * - force a 320x240 full screen resolution, with 8 bits per pixel,
   * as well as specific SDL surface flags */
  flag = SDL_SWSURFACE | SDL_FULLSCREEN;
#ifdef POWERMANGA_GP2X
  window_height = GP2X_VIDEO_HEIGHT;
  /* Use a 20px y offset to center the output on the GP2X's screen */
  display_offset_y = (GP2X_VIDEO_HEIGHT - display_height) / 2;
  for (i = 0; i < GP2X_NUM_BUTTONS; i++)
    {
      gp2x_buttons[i] = FALSE;
    }
#endif
#ifdef POWERMANGA_PSP
  for (i = 0; i < PSP_NUM_BUTTONS; i++)
    {
      psp_buttons[i] = FALSE;
    }
#endif
#else
  /* check if video mode is available */
  flag = SDL_ANYFORMAT;
  if (bytes_per_pixel == 1)
    {
      flag = flag | SDL_HWPALETTE;
    }
  if (power_conf->fullscreen > 0)
    {
      flag = flag | SDL_FULLSCREEN;
    }

  modes = SDL_ListModes (NULL, flag);
  if (modes == NULL)
    {
      LOG_ERR (" SDL_ListModes return %s", SDL_GetError ());
      return FALSE;
    }
  if (modes != (SDL_Rect **) - 1)
    {
      for (i = 0; modes[i]; i++)
        {
          LOG_INF ("mode %i; width: %i; height: %i",
                   i, modes[i]->w, modes[i]->h);
        }
    }
  else
    {
      LOG_DBG ("any dimension is okay for the given format");
    }

  width = window_width;
  height = window_height;
#if defined(_WIN32_WCE)
  /* Samsung - SGH- i900 - Player Addict Omnia
   * mode 0; width: 400; height:240
   * mode 1; width: 240; height: 400 */
  /* HTC Universal
   * mode 0; width: 640; height: 480
   * mode 1; width: 480; height: 640
   * mode 2; width: 320; height: 240
   * mode 3; width: 240; height: 320  
   */

  width = 320;
  height = 240;
  display_offset_y = (240 - display_height) / 2;
  LOG_INF ("display_offset_y: %i", display_offset_y);
  is_reverse_ctrl = TRUE;
#endif
  bpp = SDL_VideoModeOK (width, height, bits_per_pixel, flag);
  if (bpp == 0)
    {
      if (!power_conf->fullscreen)
        {
          LOG_ERR ("SDL_VideoModeOK() failed");
          return FALSE;
        }
      else
        {
          /* fullscreen fail, try in window mode */
          power_conf->fullscreen = 0;
          flag = SDL_ANYFORMAT;
          if (bytes_per_pixel == 1)
            {
              flag = flag | SDL_HWPALETTE;
            }
          bpp = SDL_VideoModeOK (width, height, bits_per_pixel, flag);
          if (bpp == 0)
            {
              LOG_ERR ("SDL_VideoModeOK() failed");
              return FALSE;
            }
        }
    }
#endif

  /* initialize video mode */
  public_surface = SDL_SetVideoMode (width, height, bits_per_pixel, flag);
  if (public_surface == NULL)
    {
      LOG_ERR ("SDL_SetVideoMode() return %s", SDL_GetError ());
      return FALSE;
    }
  LOG_INF ("SDL_SetVideoMode() successful window_width: %i;"
           " window_height: %i; bits_per_pixel: %i",
           width, height, bits_per_pixel);
#ifdef POWERMANGA_GP2X
  /* The native resolution is 320x200, so we scale up to 320x240
   * when updating the screen */
  window_height = display_height;
#endif

  /* restore valid height screen (win32 only) */
  if ((vmode) && (vmode2) && power_conf->scale_x == 2)
    {
      window_height = display_height * 2;
    }

  SDL_WM_SetCaption (window_tile, window_tile);
  /* force redraw entirely the screen */
  update_all = TRUE;
  if (power_conf->fullscreen > 0)
    {
      SDL_ShowCursor (SDL_DISABLE);
    }
  else
    {
      SDL_ShowCursor (SDL_ENABLE);
    }
  return TRUE;
}

/**
 * Destroy off screen surface for start and end movies
 */
void
destroy_movie_offscreen (void)
{
  if (movie_offscreen != NULL)
    {
      free_surface (movie_surface);
      movie_offscreen = NULL;
    }
  movie_surface = NULL;
}

/**
 * Create off screen surface for the start and end movies
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_movie_offscreen (void)
{
  movie_surface = create_surface (display_width, display_height);
  if (movie_surface == NULL)
    {
      return FALSE;
    }
  movie_offscreen = (char *) movie_surface->pixels;
  return TRUE;
}

/**
 * Create 3 or 4 off screens surfaces for the game
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_offscreens (void)
{
  /* create surface "game_offscreen" 512*440 */
  game_surface = create_surface (offscreen_width, offscreen_height);
  if (game_surface == NULL)
    {
      return FALSE;
    }
  game_offscreen = (char *) game_surface->pixels;
  offscreen_pitch = offscreen_width * bytes_per_pixel;

  /* create surface 640x400, 960x600 or 1280x800 */
  if (vmode == 1)
    {
      scalex_surface = create_surface (window_width, window_height);
      if (scalex_surface == NULL)
        {
          return FALSE;
        }
      scalex_offscreen = (char *) scalex_surface->pixels;
    }
  options_surface = create_surface (OPTIONS_WIDTH, OPTIONS_HEIGHT);
  if (options_surface == NULL)
    {
      return FALSE;
    }
  options_offscreen = (char *) options_surface->pixels;
  score_surface =
    create_surface (score_offscreen_width, score_offscreen_height);
  if (score_surface == NULL)
    {
      return FALSE;
    }
  scores_offscreen = (char *) score_surface->pixels;
  score_offscreen_pitch = score_offscreen_width * bytes_per_pixel;
  return TRUE;
}

/**
 * Recopy 8-bit palette or create 16-bit or 24-bit palette
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
create_palettes (void)
{
  Uint32 i;
  unsigned char *dest;
  unsigned char *src;

  /* 8-bit displays support 256 colors */
  if (bytes_per_pixel == 1)
    {
      if (sdl_color_palette == NULL)
        {
          sdl_color_palette =
            (SDL_Color *) memory_allocation (sizeof (SDL_Color) * 256);
          if (sdl_color_palette == NULL)
            {
              LOG_ERR ("'sdl_color_palette' out of memory");
              return FALSE;
            }
        }
      src = (unsigned char *) palette_24;
      for (i = 0; i < 256; i++)
        {
          sdl_color_palette[i].r = src[0];
          sdl_color_palette[i].g = src[1];
          sdl_color_palette[i].b = src[2];
          src += 3;
        }
      SDL_SetPalette (public_surface,
                      SDL_PHYSPAL | SDL_LOGPAL, sdl_color_palette, 0, 256);
    }
  else
    /* 16-bit depth with 65336 colors */
    {
      if (bytes_per_pixel == 2)
        {
          if (pal16 == NULL)
            {
              pal16 = (unsigned short *) memory_allocation (256 * 2);
              if (pal16 == NULL)
                {
                  LOG_ERR ("'pal16' out of memory");
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
                  if (pal32 == NULL)
                    {
                      LOG_ERR ("'pal32' out of memory");
                      return FALSE;
                    }
                }
              dest = (unsigned char *) pal32;
              src = palette_24;
              for (i = 0; i < 256; i++)
                {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                  dest[3] = src[2];
                  dest[2] = src[1];
                  dest[1] = src[0];
                  dest[0] = 0;
#else
                  if (power_conf->scale_x >= 2)
                    {
                      dest[0] = src[2];
                      dest[1] = src[1];
                      dest[2] = src[0];
                      dest[3] = 0;
                    }
                  else
                    {
                      dest[2] = src[2];
                      dest[1] = src[1];
                      dest[0] = src[0];
                      dest[3] = 0;
                    }
#endif
                  dest += 4;
                  src += 3;
                }
            }
        }
    }
  return TRUE;
}

/**
 * Handle buttons of handheld video game console
 * @param event Pointer to a SDL_Event structure
 */
#ifdef POWERMANGA_HANDHELD_CONSOLE
static void
display_handle_console_buttons (SDL_Event * event)
{
#ifdef POWERMANGA_GP2X
  if (event->jbutton.button >= GP2X_NUM_BUTTONS)
    {
      return;
    }
  if (event->type == SDL_JOYBUTTONDOWN)
    {
      gp2x_buttons[event->jbutton.button] = TRUE;
      if (event->jbutton.button == GP2X_BUTTON_UP
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT)
        {
          sprites_string_set_joy (IJOY_TOP);
        }
      if (event->jbutton.button == GP2X_BUTTON_DOWN
          || event->jbutton.button == GP2X_BUTTON_DOWNLEFT
          || event->jbutton.button == GP2X_BUTTON_DOWNRIGHT)
        {
          sprites_string_set_joy (IJOY_DOWN);
        }

      if (event->jbutton.button == GP2X_BUTTON_LEFT
          || event->jbutton.button == GP2X_BUTTON_UPLEFT
          || event->jbutton.button == GP2X_BUTTON_DOWNLEFT
          || event->jbutton.button == GP2X_BUTTON_L)
        {
          sprites_string_set_joy (IJOY_LEFT);
        }

      if (event->jbutton.button == GP2X_BUTTON_RIGHT
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT
          || event->jbutton.button == GP2X_BUTTON_DOWNRIGHT
          || event->jbutton.button == GP2X_BUTTON_R)
        {
          sprites_string_set_joy (IJOY_RIGHT);
        }
      if (event->jbutton.button == GP2X_BUTTON_A
          || event->jbutton.button == GP2X_BUTTON_B
          || event->jbutton.button == GP2X_BUTTON_CLICK)
        {
          sprites_string_set_joy (IJOY_FIRE);
        }
      if (event->jbutton.button == GP2X_BUTTON_X
          || event->jbutton.button == GP2X_BUTTON_Y)
        {
          sprites_string_set_joy (IJOY_OPT);
        }
    }
  else
    {
      gp2x_buttons[event->jbutton.button] = FALSE;
      if (event->jbutton.button == GP2X_BUTTON_UP
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT)
        {
          sprites_string_clr_joy (IJOY_TOP);
        }
      if (event->jbutton.button == GP2X_BUTTON_DOWN
          || event->jbutton.button == GP2X_BUTTON_DOWNLEFT
          || event->jbutton.button == GP2X_BUTTON_DOWNRIGHT)
        {
          sprites_string_clr_joy (IJOY_DOWN);
        }

      if (event->jbutton.button == GP2X_BUTTON_LEFT
          || event->jbutton.button == GP2X_BUTTON_UPLEFT
          || event->jbutton.button == GP2X_BUTTON_DOWNLEFT
          || event->jbutton.button == GP2X_BUTTON_L)
        {
          sprites_string_clr_joy (IJOY_LEFT);
        }

      if (event->jbutton.button == GP2X_BUTTON_RIGHT
          || event->jbutton.button == GP2X_BUTTON_UPRIGHT
          || event->jbutton.button == GP2X_BUTTON_DOWNRIGHT
          || event->jbutton.button == GP2X_BUTTON_R)
        {
          sprites_string_clr_joy (IJOY_RIGHT);
        }
      if (event->jbutton.button == GP2X_BUTTON_A
          || event->jbutton.button == GP2X_BUTTON_B
          || event->jbutton.button == GP2X_BUTTON_CLICK)
        {
          sprites_string_clr_joy (IJOY_FIRE);
        }
      if (event->jbutton.button == GP2X_BUTTON_X
          || event->jbutton.button == GP2X_BUTTON_Y)
        {
          sprites_string_clr_joy (IJOY_OPT);
        }
    }
  /* This button mapping conforms to the GP2X Common User Interface
     Recommendations, as of 2006-07-29, available from
     http://wiki.gp2x.org/wiki/Common_User_Interface_Recommendations */

  /* Directions (UDLR) */
  joy_top = gp2x_buttons[GP2X_BUTTON_UP]
    | gp2x_buttons[GP2X_BUTTON_UPLEFT] | gp2x_buttons[GP2X_BUTTON_UPRIGHT];
  joy_down = gp2x_buttons[GP2X_BUTTON_DOWN]
    | gp2x_buttons[GP2X_BUTTON_DOWNLEFT]
    | gp2x_buttons[GP2X_BUTTON_DOWNRIGHT];
  joy_left = gp2x_buttons[GP2X_BUTTON_LEFT]
    | gp2x_buttons[GP2X_BUTTON_UPLEFT]
    | gp2x_buttons[GP2X_BUTTON_DOWNLEFT] | gp2x_buttons[GP2X_BUTTON_L];
  joy_right = gp2x_buttons[GP2X_BUTTON_RIGHT]
    | gp2x_buttons[GP2X_BUTTON_UPRIGHT]
    | gp2x_buttons[GP2X_BUTTON_DOWNRIGHT] | gp2x_buttons[GP2X_BUTTON_R];

  /* pause (in game) / Return (in menu) */
  keys_down[K_PAUSE] = keys_down[K_RETURN] = gp2x_buttons[GP2X_BUTTON_START];

  /* escape (exit to menu) */
  keys_down[K_ESCAPE] = gp2x_buttons[GP2X_BUTTON_SELECT];

  /* volume ctrl */
  keys_down[K_PAGEUP] = gp2x_buttons[GP2X_BUTTON_VOLUP];
  keys_down[K_PAGEDOWN] = gp2x_buttons[GP2X_BUTTON_VOLDOWN];

  fire_button_down = gp2x_buttons[GP2X_BUTTON_A]
    | gp2x_buttons[GP2X_BUTTON_B] | gp2x_buttons[GP2X_BUTTON_CLICK];

  /* special (powerup), also control for multi-key commands */
  option_button_down = gp2x_buttons[GP2X_BUTTON_X]
    | gp2x_buttons[GP2X_BUTTON_Y];

  /* Quit */
  keys_down[K_Q] = FALSE;
  quit_game = gp2x_buttons[GP2X_BUTTON_CLICK]
    & gp2x_buttons[GP2X_BUTTON_START];
#endif
#ifdef POWERMANGA_PSP
  if (event->jbutton.button >= PSP_NUM_BUTTONS)
    {
      return;
    }
  if (event->type == SDL_JOYBUTTONDOWN)
    {
      psp_buttons[event->jbutton.button] = TRUE;
      if (event->jbutton.button == PSP_BUTTON_UP)
        {
          sprites_string_set_joy (IJOY_TOP);
        }
      if (event->jbutton.button == PSP_BUTTON_DOWN)
        {
          sprites_string_set_joy (IJOY_DOWN);
        }

      if (event->jbutton.button == PSP_BUTTON_LEFT)
        {
          sprites_string_set_joy (IJOY_LEFT);
        }

      if (event->jbutton.button == PSP_BUTTON_RIGHT)
        {
          sprites_string_set_joy (IJOY_RIGHT);
        }
      if (event->jbutton.button == PSP_BUTTON_A)
        {
          sprites_string_set_joy (IJOY_FIRE);
        }
      if (event->jbutton.button == PSP_BUTTON_X)
        {
          sprites_string_set_joy (IJOY_OPT);
        }
    }
  else
    {
      psp_buttons[event->jbutton.button] = FALSE;
      if (event->jbutton.button == PSP_BUTTON_UP)
        {
          sprites_string_clr_joy (IJOY_TOP);
        }
      if (event->jbutton.button == PSP_BUTTON_DOWN)
        {
          sprites_string_clr_joy (IJOY_DOWN);
        }

      if (event->jbutton.button == PSP_BUTTON_LEFT)
        {
          sprites_string_clr_joy (IJOY_LEFT);
        }

      if (event->jbutton.button == PSP_BUTTON_RIGHT)
        {
          sprites_string_clr_joy (IJOY_RIGHT);
        }
      if (event->jbutton.button == PSP_BUTTON_A)
        {
          sprites_string_clr_joy (IJOY_FIRE);
        }
      if (event->jbutton.button == PSP_BUTTON_X)
        {
          sprites_string_clr_joy (IJOY_OPT);
        }
    }

  /* Directions (UDLR) */
  joy_top = psp_buttons[PSP_BUTTON_UP];
  joy_down = psp_buttons[PSP_BUTTON_DOWN];
  joy_left = psp_buttons[PSP_BUTTON_LEFT];
  joy_right = psp_buttons[PSP_BUTTON_RIGHT];

  /* pause (in game) / Return (in menu) */
  keys_down[K_PAUSE] = keys_down[K_RETURN] = psp_buttons[PSP_BUTTON_START];

  /* escape (exit to menu) */
  keys_down[K_ESCAPE] = psp_buttons[PSP_BUTTON_SELECT];

  fire_button_down = psp_buttons[PSP_BUTTON_A];

  /* special (powerup), also control for multi-key commands */
  option_button_down = psp_buttons[PSP_BUTTON_X];
#endif
}
#endif

/**
 * Switch to pause when the application loses focus or disables the
 * pause if the application gains focus.
 * @param gain Whether given states were gained or lost (1/0)
 */
void
display_toggle_pause (Uint8 gain)
{
  if (gain == 0)
    {
      if (!player_pause)
        {
          if (toggle_pause ())
            {
              pause_will_disable = TRUE;
            }
        }
    }
  else
    {
      if (player_pause && pause_will_disable)
        {
          toggle_pause ();
        }
      pause_will_disable = FALSE;
    }
}

/**
 * Handle input events
 */
void
display_handle_events (void)
{
  Uint8 *keys;
  SDL_Event event;
  SDL_KeyboardEvent *ke;
  while (SDL_PollEvent (&event) > 0)
    {
      switch (event.type)
        {
        case SDL_KEYDOWN:
          {
            ke = (SDL_KeyboardEvent *) & event;
            /* LOG_INF ("SDL_KEYDOWN: "
               "%i %i %i %i", ke->type, ke->keysym.sym,
               ke->keysym.unicode, ke->state); */
            keys = SDL_GetKeyState (NULL);
            key_status (keys);
            if (ke->keysym.unicode > 0)
              {
                sprites_string_key_down (ke->keysym.unicode, ke->keysym.sym);
              }
            else
              {
                sprites_string_key_down (ke->keysym.sym, ke->keysym.sym);
              }
            /* save key code pressed */
            key_code_down = ke->keysym.sym;
          }
          break;

        case SDL_KEYUP:
          {
            ke = (SDL_KeyboardEvent *) & event;
            /* LOG_INF ("SDL_KEYUP: "
               "%i %i %i %i\n", ke->type, ke->keysym.sym,
               ke->keysym.unicode, ke->state); */
            keys = SDL_GetKeyState (NULL);
            if (ke->keysym.unicode > 0)
              {
                sprites_string_key_up (ke->keysym.unicode, ke->keysym.sym);
              }
            else
              {
                sprites_string_key_up (ke->keysym.sym, ke->keysym.sym);
              }
            if (key_code_down == (Uint32) ke->keysym.sym)
              {
                /* clear key code */
                key_code_down = 0;
              }
            key_status (keys);
          }
          break;
        case SDL_JOYHATMOTION:
          if (event.jhat.value == SDL_HAT_RIGHTUP)
            {
              joy_top = 1;
              joy_right = 1;
              joy_down = 0;
              joy_left = 0;
            }
          else if (event.jhat.value == SDL_HAT_RIGHTDOWN)
            {
              joy_top = 0;
              joy_right = 1;
              joy_down = 1;
              joy_left = 0;
            }
          else if (event.jhat.value == SDL_HAT_LEFTDOWN)
            {
              joy_top = 0;
              joy_right = 0;
              joy_down = 1;
              joy_left = 1;
            }
          else if (event.jhat.value == SDL_HAT_LEFTUP)
            {
              joy_top = 1;
              joy_right = 0;
              joy_down = 0;
              joy_left = 1;
            }
          else if (event.jhat.value == SDL_HAT_UP)
            {
              joy_top = 1;
              joy_right = 0;
              joy_down = 0;
              joy_left = 0;
            }
          else if (event.jhat.value == SDL_HAT_RIGHT)
            {
              joy_top = 0;
              joy_right = 1;
              joy_down = 0;
              joy_left = 0;
            }
          else if (event.jhat.value == SDL_HAT_DOWN)
            {
              joy_top = 0;
              joy_right = 0;
              joy_down = 1;
              joy_left = 0;
            }
          else if (event.jhat.value == SDL_HAT_LEFT)
            {
              joy_top = 0;
              joy_right = 0;
              joy_down = 0;
              joy_left = 1;
            }
          else if (event.jhat.value == SDL_HAT_CENTERED)
            {
              joy_top = 0;
              joy_right = 0;
              joy_down = 0;
              joy_left = 0;
            }
          break;
        case SDL_JOYAXISMOTION:
          {
            Sint32 deadzone = 4096;
            /* x axis */
            if (event.jaxis.axis == power_conf->joy_x_axis)
              {
                if (event.jaxis.value < -deadzone)
                  {
                    joy_left = TRUE;
                    sprites_string_set_joy (IJOY_LEFT);
                    joy_right = FALSE;
                  }
                else if (event.jaxis.value > deadzone)
                  {
                    joy_left = FALSE;
                    joy_right = TRUE;
                    sprites_string_set_joy (IJOY_RIGHT);
                  }
                else
                  {
                    joy_left = FALSE;
                    joy_right = FALSE;
                    sprites_string_clr_joy (IJOY_RIGHT);
                    sprites_string_clr_joy (IJOY_LEFT);
                  }
              }
            /* y axis */
            else if (event.jaxis.axis == power_conf->joy_y_axis)
              {
                if (event.jaxis.value < -deadzone)
                  {
                    joy_down = FALSE;
                    joy_top = TRUE;
                    sprites_string_set_joy (IJOY_TOP);
                  }
                else if (event.jaxis.value > deadzone)
                  {
                    joy_down = TRUE;
                    joy_top = FALSE;
                    sprites_string_set_joy (IJOY_DOWN);
                  }
                else
                  {
                    joy_down = FALSE;
                    joy_top = FALSE;
                    sprites_string_clr_joy (IJOY_TOP);
                    sprites_string_clr_joy (IJOY_DOWN);
                  }
              }
          }
          break;
        case SDL_JOYBUTTONDOWN:
#ifdef POWERMANGA_HANDHELD_CONSOLE
          display_handle_console_buttons (&event);
#else
          if (event.jbutton.button == power_conf->joy_start)
            {
              start_button_down = TRUE;
            }
          else if (event.jbutton.button == power_conf->joy_fire)
            {
              fire_button_down = TRUE;
              sprites_string_set_joy (IJOY_FIRE);
            }
          else if (event.jbutton.button == power_conf->joy_option)
            {
              option_button_down = TRUE;
              sprites_string_set_joy (IJOY_OPT);
            }
          break;
#endif
        case SDL_JOYBUTTONUP:
#ifdef POWERMANGA_HANDHELD_CONSOLE
          display_handle_console_buttons (&event);
#else
          if (event.jbutton.button == power_conf->joy_start)
            {
              start_button_down = FALSE;
            }
          else if (event.jbutton.button == power_conf->joy_fire)
            {
              fire_button_down = FALSE;
              sprites_string_clr_joy (IJOY_FIRE);
            }
          else if (event.jbutton.button == power_conf->joy_option)
            {
              option_button_down = FALSE;
              sprites_string_clr_joy (IJOY_OPT);
            }
#endif
          break;
        case SDL_QUIT:
          quit_game = TRUE;

          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
              mouse_b = 1;
              mouse_x = event.button.x / screen_pixel_size;
              mouse_y = event.button.y / screen_pixel_size;
              LOG_INF ("mouse_x = %i mouse_y = %i", mouse_x, mouse_y);
              break;
            }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
              mouse_b = 0;
              break;
            }

          /* application loses/gains visibility */
        case SDL_ACTIVEEVENT:
          //LOG_INF ("  > SDL_ACTIVEEVENT: %i <", event.active.state);
          if (event.active.state & SDL_APPMOUSEFOCUS)
            {
              LOG_DBG ("[SDL_APPMOUSEFOCUS] The app has mouse coverage; "
                       " event.active.gain: %i", event.active.gain);
            }
          if (event.active.state & SDL_APPINPUTFOCUS)
            {
              LOG_DBG ("[SDL_APPINPUTFOCUS] The app has input focus; "
                       "event.active.gain: %i", event.active.gain);
              display_toggle_pause (event.active.gain);
            }
          if (event.active.state & SDL_APPACTIVE)
            {
              is_iconified = event.active.gain ? TRUE : FALSE;
              LOG_DBG ("[SDL_APPACTIVE]The application is active; "
                       "event.active.gain: %i, is_iconified: %i",
                       event.active.gain, is_iconified);
              display_toggle_pause (event.active.gain);
            }
          break;

          /* screen needs to be redrawn */
        case SDL_VIDEOEXPOSE:
          update_all = TRUE;
          if (SDL_FillRect (public_surface, NULL, real_black_color) < 0)
            {
              LOG_ERR ("SDL_FillRect(public_surface) return %s",
                       SDL_GetError ());
            }
          break;

          /* mouse moved */
        case SDL_MOUSEMOTION:
          break;

        default:
          LOG_INF ("not supported event type: %i", event.type);
          break;
        }
    }
}

/**
 * Copy SDL flags buttons
 * @param k Pointer to an array of snapshot of the current keyboard state
 */
void
key_status (Uint8 * k)
{
  keys_down[K_ESCAPE] = k[SDLK_ESCAPE];
  keys_down[K_CTRL] = k[SDLK_LCTRL];
  keys_down[K_CTRL] |= k[SDLK_RCTRL];
  keys_down[K_RETURN] = k[SDLK_RETURN];
  keys_down[K_PAUSE] = k[SDLK_PAUSE];
  keys_down[K_SHIFT] = k[SDLK_LSHIFT];
  keys_down[K_SHIFT] |= k[SDLK_RSHIFT];
  keys_down[K_1] = k[SDLK_1] | k[SDLK_KP1];
  keys_down[K_2] = k[SDLK_2] | k[SDLK_KP2];
  keys_down[K_3] = k[SDLK_3] | k[SDLK_KP3];
  keys_down[K_4] = k[SDLK_4] | k[SDLK_KP4];
  keys_down[K_5] = k[SDLK_5] | k[SDLK_KP5];
  keys_down[K_6] = k[SDLK_6] | k[SDLK_KP6];
  keys_down[K_7] = k[SDLK_7] | k[SDLK_KP7];
  keys_down[K_8] = k[SDLK_8] | k[SDLK_KP8];
  keys_down[K_9] = k[SDLK_9] | k[SDLK_KP9];
  keys_down[K_0] = k[SDLK_0] | k[SDLK_KP0];
  keys_down[K_F1] = k[SDLK_F1];
  keys_down[K_F2] = k[SDLK_F2];
  keys_down[K_F3] = k[SDLK_F3];
  keys_down[K_F4] = k[SDLK_F4];
  keys_down[K_F5] = k[SDLK_F5];
  keys_down[K_F6] = k[SDLK_F6];
  keys_down[K_F7] = k[SDLK_F7];
  keys_down[K_F8] = k[SDLK_F8];
  keys_down[K_F9] = k[SDLK_F9];
  keys_down[K_F10] = k[SDLK_F10];
  keys_down[K_F11] = k[SDLK_F11];
  keys_down[K_F12] = k[SDLK_F12];
  keys_down[K_INSERT] = k[SDLK_INSERT];
  keys_down[K_SPACE] = k[SDLK_SPACE];
  if (is_reverse_ctrl)
    {
      keys_down[K_LEFT] = k[SDLK_DOWN];
      keys_down[K_RIGHT] = k[SDLK_UP];
      keys_down[K_UP] = k[SDLK_LEFT];
      keys_down[K_DOWN] = k[SDLK_RIGHT];
    }
  else
    {
      keys_down[K_LEFT] = k[SDLK_LEFT];
      keys_down[K_RIGHT] = k[SDLK_RIGHT];
      keys_down[K_UP] = k[SDLK_UP];
      keys_down[K_DOWN] = k[SDLK_DOWN];
    }
  /* [Ctrl] + [A]: ABOUT */
  keys_down[K_A] = k[SDLK_a];
  /* switch between full screen and windowed mode */
  keys_down[K_F] = k[SDLK_f];
  keys_down[K_V] = k[SDLK_v];
  keys_down[K_B] = k[SDLK_b];
  /* enable/disable pause */
  keys_down[K_P] = k[SDLK_p];
  /* [Ctrl] + [Q]: force "Game Over" */
  keys_down[K_Q] = k[SDLK_q];
  /* [Ctrl] + [S]: enable/disable the music */
  keys_down[K_S] = k[SDLK_s];
  /* right */
  keys_down[K_RIGHT] |= k[SDLK_KP6];
  /* left */
  keys_down[K_LEFT] |= k[SDLK_KP4];
  /* up */
  keys_down[K_UP] |= k[SDLK_KP8];
  /* down */
  keys_down[K_DOWN] |= k[SDLK_KP5];
  /* power-up (aka Ctrl key) */
  keys_down[K_CTRL] |= k[SDLK_KP2];
  /* fire (aka space bar) */
  keys_down[K_SPACE] |= k[SDLK_KP0];
  /* fire (aka space ENTER) */
  if (k[SDLK_RETURN] && !is_playername_input () &&
      menu_section != SECTION_ORDER)
    {
      keys_down[K_SPACE] |= k[SDLK_RETURN];
    }
  keys_down[K_C] = k[SDLK_c];
  keys_down[K_G] = k[SDLK_g];
  keys_down[K_E] = k[SDLK_e];
  /* Volume control */
  keys_down[K_PAGEUP] = k[SDLK_PAGEUP];
  keys_down[K_PAGEDOWN] = k[SDLK_PAGEDOWN];
}

/**
 * Display on the screen
 */
void
display_update_window (void)
{
  /* movie is playing? */
  if (movie_surface != NULL)
    {
      update_all = TRUE;
      display_movie ();
    }
  else
    {
      switch (vmode)
        {
        case 0:
          display_320x200 ();
          break;
          /* Double pixels horizontally,
           * interlaced with empty line vertically */
        case 1:
          display_640x400 ();
          break;
        case 2:
#ifdef USE_SCALE2X
          display_scale_x ();
#endif
          break;
        }
    }
}

/**
 * Return a pointer to a rectangular area
 * @param x X-coordintate of the upper-left corner of the rectangle
 * @param y Y-coordintate of the upper-left corner of the rectangle
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 * @return Pointer to SDL_Rect structure
 */
static void
get_rect (SDL_Rect * rect, Sint16 x, Sint16 y, Sint16 w, Sint16 h)
{
  rect->x = x;
#if defined (POWERMANGA_GP2X) || defined (_WIN32_WCE)
  rect->y = y + (Sint16) display_offset_y;
#else
  rect->y = y;
#endif
  rect->w = w;
  rect->h = h;
}

/** 
 * Display start movie and end movie
 */
static void
display_movie (void)
{
  SDL_Rect rsour;
  Sint32 i;

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

  /* 320x200 mode */
  switch (vmode)
    {
    case 0:
      {
        get_rect (&rsour, 0, 0, (Sint16) display_width,
                  (Sint16) display_height);
        if (SDL_BlitSurface (movie_surface, &rsour, public_surface, &rsour) <
            0)
          {
            LOG_ERR ("SDL_BlitSurface() return %s", SDL_GetError ());
          }
        SDL_UpdateRect (public_surface, 0, 0,
                        public_surface->w, public_surface->h);
      }
      break;

      /* 640x400 mode */
    case 1:
      {
        copy2X (movie_offscreen, scalex_offscreen, 320, 200, 0,
                window_width * bytes_per_pixel * 2 -
                (window_width * bytes_per_pixel));
        rsour.x = 0;

        if (window_height == 480)
          {
            rsour.y = 40;
          }
        else
          {
            rsour.y = 0;
          }
        rsour.w = (Uint16) window_width;
        rsour.h = (Uint16) window_height;
        if (SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rsour) <
            0)
          {
            LOG_ERR ("SDL_BlitSurface() return %s", SDL_GetError ());
          }
      }

      if (window_height == 480)
        {
          SDL_UpdateRect (public_surface, 0, 40,
                          public_surface->w, public_surface->h - 40);
        }
      else
        SDL_UpdateRect (public_surface, 0, 0,
                        public_surface->w, public_surface->h);
      break;

      /* scale 2x, 3x or 4x mode */
    case 2:
#ifdef USE_SCALE2X
      scale (power_conf->scale_x, public_surface->pixels,
             public_surface->pitch, movie_offscreen,
             display_width * bytes_per_pixel, bytes_per_pixel, display_width,
             display_height);
#endif
      SDL_UpdateRect (public_surface, 0, 0,
                      public_surface->w, public_surface->h);
      break;
    }
}

/**
 * Display window in 320*200, orignal size of the game
 */
static void
display_320x200 (void)
{
  SDL_Rect rdest;
  SDL_Rect rsour;
  Sint32 optx, opty;
  rsour.x = (Sint16) offscreen_clipsize;
  rsour.y = (Sint16) offscreen_clipsize;
  rsour.w = (Uint16) offscreen_width_visible;
  rsour.h = (Uint16) offscreen_height_visible;
  get_rect (&rdest, 0, 16, (Sint16) display_width, (Sint16) display_height);
  if (SDL_BlitSurface (game_surface, &rsour, public_surface, &rdest) < 0)
    {
      LOG_ERR ("SDL_BlitSurface(game_surface) return %s", SDL_GetError ());
    }
  if (update_all)
    {
      rsour.x = 0;
      rsour.y = 0;
      rsour.w = OPTIONS_WIDTH;
      rsour.h = OPTIONS_HEIGHT;
      get_rect (&rdest, (Sint16) offscreen_width_visible, 16, OPTIONS_WIDTH,
                OPTIONS_HEIGHT);
      if (SDL_BlitSurface (options_surface, &rsour, public_surface, &rdest))
        {
          LOG_ERR ("SDL_BlitSurface(options_surface) return %s",
                   SDL_GetError ());
        }

      /* display score panel */
      rsour.x = 0;
      rsour.y = 0;
      rsour.w = (Uint16) score_offscreen_width;
      rsour.h = SCORES_HEIGHT;
      get_rect (&rdest, 0, 0, (Sint16) display_width, SCORES_HEIGHT);
      if (SDL_BlitSurface (score_surface, &rsour, public_surface, &rdest) < 0)
        {
          LOG_ERR ("SDL_BlitSurface(score_surface) return %s",
                   SDL_GetError ());
        }
      opt_refresh_index = -1;
      update_all = FALSE;
    }
  else
    {
      /* display options from option panel */
      while (opt_refresh_index >= 0)
        {
          optx = options_refresh[opt_refresh_index].coord_x;
          opty = options_refresh[opt_refresh_index--].coord_y;
          rsour.x = (Sint16) optx;
          rsour.y = (Sint16) opty;
          rsour.w = 28;
          rsour.h = 28;
          get_rect (&rdest, (Sint16) (offscreen_width_visible + optx),
                    (Sint16) (16 + opty), 28, 28);
          if (SDL_BlitSurface
              (options_surface, &rsour, public_surface, &rdest) < 0)
            {
              LOG_ERR ("SDL_BlitSurface(options_surface) return %s",
                       SDL_GetError ());
            }
        }
      if (score_x2_refresh || !update_all)
        {
          rsour.x = 41;
          rsour.y = 171;
          rsour.w = 14;
          rsour.h = 8;
          get_rect (&rdest, 297, 187, 14, 8);
          if (SDL_BlitSurface
              (options_surface, &rsour, public_surface, &rdest) < 0)
            {
              LOG_ERR ("SDL_BlitSurface(options_surface) return %s",
                       SDL_GetError ());
            }
          score_x2_refresh = FALSE;
        }
      if (score_x4_refresh || !update_all)
        {
          rsour.x = 41;
          rsour.y = 5;
          rsour.w = 14;
          rsour.h = 8;
          get_rect (&rdest, 297, 21, 14, 8);
          if (SDL_BlitSurface
              (options_surface, &rsour, public_surface, &rdest) < 0)
            {
              LOG_ERR ("SDL_BlitSurface(options_surface) return %s",
                       SDL_GetError ());
            }
          score_x4_refresh = FALSE;
        }

      /* display player's energy */
      if (energy_gauge_spaceship_is_update)
        {
          rsour.x = 210;
          rsour.y = 3;
          rsour.w = 100;
          rsour.h = 9;
          get_rect (&rdest, 210, 3, 100, 9);
          optx =
            SDL_BlitSurface (score_surface, &rsour, public_surface, &rdest);
          if (SDL_BlitSurface (score_surface, &rsour, public_surface, &rdest)
              < 0)
            {
              LOG_ERR ("SDL_BlitSurface(score_surface) return %s",
                       SDL_GetError ());
            }
          energy_gauge_spaceship_is_update = FALSE;
        }

      /* display big-boss's energy */
      if (energy_gauge_guard_is_update)
        {
          rsour.x = 10;
          rsour.y = 3;
          rsour.w = 45;
          rsour.h = 9;
          get_rect (&rdest, 10, 3, 45, 9);
          if (SDL_BlitSurface (score_surface, &rsour, public_surface, &rdest)
              < 0)
            {
              LOG_ERR ("SDL_BlitSurface(score_surface) return %s",
                       SDL_GetError ());
            }
          energy_gauge_guard_is_update = FALSE;
        }

      /* display score number */
      if (is_player_score_displayed)
        {
          rsour.x = 68;
          rsour.y = 0;
          rsour.w = 128;
          rsour.h = 16;
          get_rect (&rdest, 68, 0, 128, 16);
          if (SDL_BlitSurface (score_surface, &rsour, public_surface, &rdest)
              < 0)
            {
              LOG_ERR ("SDL_BlitSurface(score_surface) return %s",
                       SDL_GetError ());
            }
          is_player_score_displayed = FALSE;
        }
    }
  SDL_UpdateRect (public_surface, 0, 0, public_surface->w, public_surface->h);
  /* SDL_UpdateRect (public_surface, 0, 16, 256, 184); */

}

/**
 * Increase the size of the offscreens,
 * use scale2x effect developed by Andrea Mazzoleni
 */
#ifdef USE_SCALE2X
static void
display_scale_x (void)
{
  char *src;
  Sint32 scalex = power_conf->scale_x;
  char *pixels = (char *) public_surface->pixels;
  Sint32 pitch = public_surface->pitch;
  Sint32 optx, opty;
  if (vmode2)
    {
      pixels += 20 * scalex * pitch;
    }

  /* scale main screen */
  src = game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);
  scale (scalex, pixels + (pitch * score_offscreen_height * scalex), pitch,
         src, 512 * bytes_per_pixel, bytes_per_pixel, offscreen_width_visible,
         offscreen_height_visible);

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
      opt_refresh_index = -1;
    }
  else
    {
      /* display player score */
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          scale (scalex, pixels + (68 * bytes_per_pixel * scalex), pitch,
                 scores_offscreen + (68 * bytes_per_pixel),
                 score_offscreen_pitch, bytes_per_pixel, 128, 16);
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
        }

      /* display player's ernergy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = FALSE;
          scale (scalex,
                 pixels + 210 * bytes_per_pixel * scalex + 3 * pitch * scalex,
                 pitch,
                 scores_offscreen + 210 * bytes_per_pixel +
                 3 * score_offscreen_pitch, score_offscreen_pitch,
                 bytes_per_pixel, 100, 9);
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
        }
    }
  SDL_UpdateRect (public_surface, 0, 0, public_surface->w, public_surface->h);
}
#endif

/**
 * Display window in 640*400. Double pixels horizontally, interlaced
 *     with empty line vertically. Playfield: 512x440;
 *     score panel: 320x16; option panel 64x184
 */
static void
display_640x400 (void)
{
  Sint32 v, starty, optx, opty;
  SDL_Rect rdest;
  SDL_Rect rsour;
  char *src =
    game_offscreen + (offscreen_clipsize * offscreen_pitch) +
    (offscreen_clipsize * bytes_per_pixel);

  /* recopy on the screen: doubling pixels horizontally
     "assembler_opt.S" or "gfxroutine.c" */
  copy2X_512x440 (src,
                  scalex_offscreen + (window_width * bytes_per_pixel * 32),
                  offscreen_height_visible);

  /* mode 640x480 on Windows */
  if (window_height == 480)
    {
      starty = 40;
    }
  else
    {
      starty = 0;
    }

  if (!update_all)
    {
      rsour.x = 0;
      rsour.y = 32;
      rsour.w = (Sint16) (offscreen_width_visible * 2);
      rsour.h = (Sint16) (offscreen_height_visible * 2);
      rdest.x = 0;
      rdest.y = (Sint16) (32 + starty);
      rdest.w = (Uint16) (offscreen_width_visible * 2);
      rdest.h = (Uint16) (offscreen_height_visible * 2);
      v = SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
      if (v < 0)
        {
          LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                   SDL_GetError ());
        }

      /* display score number */
      if (is_player_score_displayed)
        {
          is_player_score_displayed = FALSE;
          copy2X (scores_offscreen + (68 * bytes_per_pixel),
                  scalex_offscreen + (68 * bytes_per_pixel * 2), 128, 16,
                  score_offscreen_pitch - 128 * bytes_per_pixel,
                  (window_width * 2 * bytes_per_pixel) -
                  128 * bytes_per_pixel * 2);
          rsour.x = 68 * 2;
          rsour.y = 0;
          rsour.w = 128 * 2;
          rsour.h = 16 * 2;
          rdest.x = 68 * 2;
          rdest.y = (Sint16) starty;
          rdest.w = 128 * 2;
          rdest.h = 16 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
        }

      if (score_x2_refresh || !update_all)
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
          rsour.x = (Sint16) ((offscreen_width_visible + 41) * 2);
          rsour.y = (Sint16) ((score_offscreen_height + 171) * 2);
          rsour.w = 14 * 2;
          rsour.h = 8 * 2;

          rdest.x = (Sint16) ((offscreen_width_visible + 41) * 2);
          rdest.y = (Sint16) ((score_offscreen_height + 171) * 2 + starty);

          rdest.w = 14 * 2;
          rdest.h = 8 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
          score_x2_refresh = FALSE;
        }

      if (score_x4_refresh || !update_all)
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
          rsour.x = (Sint16) ((offscreen_width_visible + 41) * 2);
          rsour.y = (Sint16) ((score_offscreen_height + 5) * 2);
          rsour.w = 14 * 2;
          rsour.h = 8 * 2;

          rdest.x = (Sint16) ((offscreen_width_visible + 41) * 2);
          rdest.y = (Sint16) ((score_offscreen_height + 5) * 2 + starty);

          rdest.w = 14 * 2;
          rdest.h = 8 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
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
          rsour.x = (Sint16) ((offscreen_width_visible + optx) * 2);
          rsour.y = (Sint16) ((score_offscreen_height + opty) * 2);
          rsour.w = 28 * 2;
          rsour.h = 28 * 2;

          rdest.x = (Sint16) ((offscreen_width_visible + optx) * 2);
          rdest.y = (Sint16) ((score_offscreen_height + opty) * 2 + starty);

          rdest.w = 28 * 2;
          rdest.h = 28 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
        }

      /* display player's ernergy */
      if (energy_gauge_spaceship_is_update)
        {
          energy_gauge_spaceship_is_update = FALSE;
          copy2X (scores_offscreen + (210 * bytes_per_pixel) +
                  (3 * score_offscreen_pitch),
                  scalex_offscreen + (210 * bytes_per_pixel * 2) +
                  (3 * window_width * bytes_per_pixel * 2), 100, 9,
                  score_offscreen_pitch - 100 * bytes_per_pixel,
                  (window_width * bytes_per_pixel * 2) -
                  100 * bytes_per_pixel * 2);
          rsour.x = 210 * 2;
          rsour.y = 3 * 2;
          rsour.w = 100 * 2;
          rsour.h = 9 * 2;

          rdest.x = 210 * 2;
          rdest.y = 3 * 2 + (Sint16) starty;

          rdest.w = 100 * 2;
          rdest.h = 9 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
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
          rsour.x = 10 * 2;
          rsour.y = 3 * 2;
          rsour.w = 45 * 2;
          rsour.h = 9 * 2;
          rdest.x = 10 * 2;
          rdest.y = 3 * 2 + (Sint16) starty;
          rdest.w = 45 * 2;
          rdest.h = 9 * 2;
          v =
            SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
          if (v < 0)
            {
              LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                       SDL_GetError ());
            }
        }


    }
  else
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
      rsour.x = 0;
      rsour.y = 0;
      rsour.w = (Uint16) window_width;
      rsour.h = (Uint16) window_height;
      rdest.x = 0;
      rdest.y = (Sint16) starty;
      rdest.w = (Uint16) window_width;
      rdest.h = (Uint16) window_height;
      v = SDL_BlitSurface (scalex_surface, &rsour, public_surface, &rdest);
      if (v < 0)
        {
          LOG_ERR ("SDL_BlitSurface(scalex_surface) return %s",
                   SDL_GetError ());
        }
      update_all = FALSE;
      opt_refresh_index = -1;
    }

  SDL_UpdateRect (public_surface, 0, starty, public_surface->w,
                  public_surface->h - starty);
}

#ifdef USE_SDL_JOYSTICK
/**
 * Opens all joysticks available
 */
bool
display_open_joysticks (void)
{
  Uint32 i;
  display_close_joysticks ();
  numof_joysticks = SDL_NumJoysticks ();
  LOG_INF ("number of joysticks available: %i", numof_joysticks);
  if (numof_joysticks < 1)
    {
      return TRUE;
    }
  sdl_joysticks =
    (SDL_Joystick **) memory_allocation (sizeof (SDL_Joystick *) *
                                         numof_joysticks);
  if (sdl_joysticks == NULL)
    {
      LOG_ERR ("'sdl_joysticks' out of memory");
      return FALSE;
    }
  for (i = 0; i < numof_joysticks; i++)
    {
      sdl_joysticks[i] = SDL_JoystickOpen (i);
      if (sdl_joysticks[i] == NULL)
        {
          LOG_ERR ("couldn't open joystick 0: %s", SDL_GetError ());
        }
      else
        {
          LOG_DBG ("- joystick  : %s", SDL_JoystickName (0));
          LOG_DBG ("- axes      : %d",
                   SDL_JoystickNumAxes (sdl_joysticks[i]));
          LOG_DBG ("- buttons   : %d",
                   SDL_JoystickNumButtons (sdl_joysticks[i]));
          LOG_DBG ("- trackballs: %d",
                   SDL_JoystickNumBalls (sdl_joysticks[i]));
          LOG_DBG ("- hats      : %d",
                   SDL_JoystickNumHats (sdl_joysticks[i]));
        }
    }
  return TRUE;
}

/**
 * Closes all open joysticks previously
 */
void
display_close_joysticks (void)
{
  Uint32 i;
  if (numof_joysticks < 1 || sdl_joysticks == NULL)
    {
      return;
    }
  for (i = 0; i < numof_joysticks; i++)
    {
      if (sdl_joysticks[i] != NULL)
        {
          SDL_JoystickClose (sdl_joysticks[i]);
          sdl_joysticks[i] = NULL;
        }
    }
  free_memory ((char *) sdl_joysticks);
  sdl_joysticks = NULL;
  numof_joysticks = 0;
}
#endif

/**
 * Shuts down all SDL subsystems and frees the resources allocated to them
 */
void
display_free (void)
{
  free_surfaces ();
  game_offscreen = NULL;
  game_surface = NULL;
  scalex_offscreen = NULL;
  scalex_surface = NULL;
  scores_offscreen = NULL;
  options_surface = NULL;
  options_offscreen = NULL;
  score_surface = NULL;
#ifdef USE_SDL_JOYSTICK
  display_close_joysticks ();
#endif
  SDL_Quit ();
  LOG_INF ("SDL_Quit()");
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
  if (sdl_color_palette != NULL)
    {
      free_memory ((char *) sdl_color_palette);
      sdl_color_palette = NULL;
    }
}

/**
 * Clear the main offscreen
 */
void
display_clear_offscreen (void)
{
  SDL_Rect rect;
  rect.x = (Sint16) offscreen_clipsize;
  rect.y = (Sint16) offscreen_clipsize;
  rect.w = (Uint16) offscreen_width_visible;
  rect.h = (Uint16) offscreen_height_visible;
  if (SDL_FillRect (game_surface, &rect, real_black_color) < 0)
    {
      LOG_ERR ("SDL_FillRect(game_surface) return %s", SDL_GetError ());
    }
}

/**
 * Create an empty SDL surface
 * @param width
 * @param height
 * @return SDL Surface
 */
static SDL_Surface *
create_surface (Uint32 width, Uint32 height)
{
  Uint32 i, rmask, gmask, bmask;
  SDL_Surface *surface;
  Sint32 index = -1;
  for (i = 0; i < MAX_OF_SURFACES; i++)
    {
      if (surfaces_list[i] == NULL)
        {
          index = i;
          break;
        }
    }
  if (index < 0)
    {
      LOG_ERR ("out of 'surfaces_list' list");
      return NULL;
    }
  get_rgb_mask (&rmask, &gmask, &bmask);
  surface =
    SDL_CreateRGBSurface (SDL_ANYFORMAT, width, height, bits_per_pixel,
                          rmask, gmask, bmask, 0);
  if (surface == NULL)
    {
      LOG_ERR ("SDL_CreateRGBSurface() return %s", SDL_GetError ());
      return NULL;
    }
  if (bytes_per_pixel == 1)
    SDL_SetPalette (surface, SDL_PHYSPAL | SDL_LOGPAL, sdl_color_palette, 0,
                    256);
  surfaces_list[index] = surface;
  surfaces_counter++;
  LOG_DBG ("SDL_CreateRGBSurface(%i,%i,%i)", width, height, bits_per_pixel);
  return surface;
}

static void
get_rgb_mask (Uint32 * rmask, Uint32 * gmask, Uint32 * bmask)
{
  switch (bits_per_pixel)
    {
    case 15:
      *rmask = 0x7c00;
      *gmask = 0x03e0;
      *bmask = 0x001f;
      break;
    case 16:
      *rmask = 0xf800;
      *gmask = 0x03e0;
      *bmask = 0x001f;
      break;
    default:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      *rmask = 0x00ff0000;
      *gmask = 0x0000ff00;
      *bmask = 0x000000ff;
#else
      *rmask = 0x000000ff;
      *gmask = 0x0000ff00;
      *bmask = 0x00ff0000;
#endif
      break;
    }
}

/**
 * Release a SDL surface
 * @param surface pointer to surface to release
 */
static void
free_surface (SDL_Surface * surface)
{
  Uint32 w;
  Uint32 h;
  Uint32 i;
  for (i = 0; i < MAX_OF_SURFACES; i++)
    {
      if (surfaces_list[i] == surface)
        {
          w = surface->w;
          h = surface->h;
          SDL_FreeSurface (surface);
          surfaces_list[i] = NULL;
          surfaces_counter--;
          LOG_DBG ("SDL_FreeSurface: %ix%i", w, h);
          break;
        }
    }
}

/**
 * Release all SDL surfaces
 */
static void
free_surfaces (void)
{
  Uint32 i;
  Uint32 w;
  Uint32 h;
  for (i = 0; i < MAX_OF_SURFACES; i++)
    {
      if (surfaces_list[i] == NULL)
        {
          continue;
        }
      w = surfaces_list[i]->w;
      h = surfaces_list[i]->h;
      SDL_FreeSurface (surfaces_list[i]);
      surfaces_list[i] = NULL;
      surfaces_counter--;
      LOG_DBG ("SDL_FreeSurface: %ix%i", w, h);
    }
}


#ifdef SHAREWARE_VERSION
/**
 * Display the page order for the Windows shareware version
 * run only in 640x400 mode
 * @param num
 * @param lang
 * @param cpt
 */
void
show_page_order (Sint32 num, char *lang, Sint32 page_num)
{
  Page *script_page;
  SDL_Surface *surface_page;
  Sint32 cptPosX, cptPosY;
  Uint32 tm1, tm2;
  char tmp[256];
  char str[256];
  TTF_Font *fnt = NULL;
  SDL_Surface *counter_surface = NULL;
  bool is_pageorder_displayed = TRUE;
  LOG_DBG ("num: %i; lang: %s, page_num: %i", num, lang, page_num);

  /* load and create page */
  script_initialize (1024, 768, "order/");
  sprintf (tmp, "scriptorder%d_%s.txt", num, lang);
  script_page = script_read_file (tmp);
  if (NULL == script_page)
    {
      LOG_ERR ("script_read_file() failed!");
      return;
    }
  if (page_num == 0)
    {
      script_set_number_of_launch (script_page, 1);
    }
  else
    {
      script_set_number_of_launch (script_page, page_num);
    }

  if (!script_get_counter_pos (script_page, &cptPosX, &cptPosY))
    {
      cptPosX = 16;
      cptPosY = 640 - 16 - 32;
    }
  else
    {
      cptPosX = cptPosX * 640 / 1024;
      cptPosY = cptPosY * 400 / 768;
    }


  surface_page = script_generate_page (script_page);
  if (surface_page == NULL)
    {
      LOG_ERR ("script_generate_page() failed!");
      return;
    }
  if (page_num != 0)
    {
      fnt = TTF_OpenFont ("order/tlk.fnt", 24);
      if (NULL == fnt)
        {
          LOG_ERR (" TTF_OpenFont() return %s", TTF_GetError ());
          script_free (script_page);
          return;
        }
    }

  SDL_Color white = { 255, 255, 255, 0 };

  Sint32 n = 0;

  if (page_num != 0)
    {
      n = (page_num - 8) * 5 + 5;
      if (n > 500)
        {
          n = 500;
        }
      sprintf (str, "%d", n);
      counter_surface = TTF_RenderUTF8_Blended (fnt, str, white);
    }

  SDL_Rect r, r2;

  r.x = 0;
  r.y = 0;
  r.w = 640;
  r.h = 400;

  r2.x = (public_surface->w - 640) / 2;
  r2.y = (public_surface->h - 400) / 2;
  r2.w = r.w;
  r2.h = r.h;

  /* clear screen */
  SDL_FillRect (public_surface, 0, 0);
  SDL_Flip (public_surface);


  float nbsec, lastsec;

  nbsec = n;
  lastsec = n;

  tm1 = SDL_GetTicks ();
  tm2 = SDL_GetTicks ();

  /* empty previous events */
  SDL_Event event;
  while (SDL_PollEvent (&event))
    {
    }

  while (is_pageorder_displayed)
    {
      tm2 = SDL_GetTicks ();
      float dt = (tm2 - tm1) / 1000.f;
      tm1 = tm2;
      /* wait key */
      while (SDL_PollEvent (&event))
        {
          switch (event.type)
            {
            case SDL_MOUSEBUTTONUP:
            case SDL_KEYUP:
              if (nbsec == 0.0f)
                {
                  is_pageorder_displayed = FALSE;
                }
              break;
            }
        }

      if (page_num != 0)
        {
          if (nbsec != 0.0f)
            {
              nbsec -= dt;
              if (nbsec <= 0.0f)
                {
                  nbsec = 0.0f;
                }
            }

          if (lastsec != ceil (nbsec))
            {
              sprintf (str, "%d", (Sint32) nbsec);
              SDL_FreeSurface (counter_surface);
              counter_surface = TTF_RenderUTF8_Blended (fnt, str, white);
              lastsec = ceil (nbsec);
            }
        }

      SDL_BlitSurface (surface_page, &r, public_surface, &r2);
      if (page_num != 0)
        {
          SDL_Rect rc1, rc2;
          rc1.x = 0;
          rc1.y = 0;
          rc1.w = counter_surface->w;
          rc1.h = counter_surface->h;
          rc2.x = cptPosX;
          rc2.y = cptPosY;
          rc2.w = rc1.w;
          rc2.h = rc1.h;
          SDL_BlitSurface (counter_surface, &rc1, public_surface, &rc2);
        }

      SDL_Flip (public_surface);
    }

  /* clear screen */
  SDL_FillRect (public_surface, 0, 0);
  SDL_Flip (public_surface);
  if (page_num != 0)
    {
      SDL_FreeSurface (counter_surface);
      TTF_CloseFont (fnt);
    }

  SDL_FreeSurface (surface_page);
  script_free (script_page);
  LOG_DBG ("End!");
}
#endif





/*

320x200 mode

+------------------------------+
!  ^         512               !
!<-!- - - - - - - - - - - -  ->!
!                              !
!  !  +------------------+     !
!     !      score       !     !
! 4!  !--------------+---!     !
! 4   !  ^           ! o !     !
! 0!  !  !   256     ! p !     !
!     !<- - - - - -> ! t !     !
!  !  !  !           ! i !     !
!     !   1          ! o !     !
!  !  !  !8          ! n !     !
!     !   4          ! s !     !
!  !  !  !           !<64!     !
!     +--------------+---+     !
!  !   <- - - - - - - - >      !
!            320               !
!  !                           !
+------------------------------+

*/



#endif
