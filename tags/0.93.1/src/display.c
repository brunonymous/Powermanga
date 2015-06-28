/**
 * @file display.c
 * @brief handle displaying and updating the graphical components of the game
 * @created 2006-12-03
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: display.c,v 1.27 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "config_file.h"
#include "display.h"
#include "log_recorder.h"

/** Width of the main window */
const Sint32 DISPLAY_WIDTH = 320;
/** Height of the main window */
const Sint32 DISPLAY_HEIGHT = 200;
/* 
 * size of the surface, screen, and pannels
 */
/** Size of clipping regions */
const Sint32 OFFSCREEN_CLIPSIZE = 128;
/** Width of the main surface  */
const Sint32 OFFSCREEN_WIDTH = 512;
/** Height of the main surface  */
const Sint32 OFFSCREEN_HEIGHT = 440;
/* Width of the visible surface area */
const Sint32 OFFSCREEN_WIDTH_VISIBLE = 256;
/* Height of the visible surface area */
const Sint32 OFFSCREEN_HEIGHT_VISIBLE = 184;
Sint32 display_width = 0;
Sint32 display_height = 0;
Sint32 offscreen_width = 0;
Sint32 offscreen_height = 0;
Sint32 offscreen_width_visible = 0;
Sint32 offscreen_height_visible = 0;
Sint32 offscreen_startx = 0;
Sint32 offscreen_starty = 0;
Sint32 offscreen_clipsize = 0;
Sint32 score_offscreen_width = 0;
Sint32 score_offscreen_height = 0;
/** Size of a pixel, always equal to 1. Will be able to be 2 or 3
 * in a further version */
Sint32 pixel_size = 1;
/** Size of a pixel in the screen, used for mouse or stylus 
 * collision: 1, 2, 3 or 4 */
Sint32 screen_pixel_size = 1;
/** Number of bytes per pixel 1=256 colors */
Uint32 bytes_per_pixel = 0;
/** Depth of the screen 8, 15, 16, 24 or 32 */
Uint32 bits_per_pixel = 0;
/*
 * color palettes
 */
/** 24-bit color palette: 3 bytes per color */
unsigned char *palette_24 = NULL;
/** 32-bit color palette: 4 bytes per color */
Uint32 *pal32 = NULL;
/** 16-bit color palette: 2 bytes per color */
Uint16 *pal16 = NULL;
Uint16 *pal16PlayAnim = NULL;
Uint32 *pal32PlayAnim = NULL;
/** Size of a line of the game offscreen (in bytes) */
Uint32 offscreen_pitch = 0;
Uint32 score_offscreen_pitch = 0;
/** 0=resolution de 320x200 / 1=640*400 / 2=640*400 (double pixels) */
Sint32 vmode = 1;
/** Pointer to the game offscreen memory buffer */
char *game_offscreen = NULL;
/** Memory buffer 320*016 (score bar-line) */
char *scores_offscreen = NULL;
/** Memory buffer 064*184 (option panel) */
char *options_offscreen = NULL;
/** Offscreen for movie */
char *movie_offscreen = NULL;
/** Offscreen for scaling */
char *scalex_offscreen = NULL;
/** Keys flags table */
bool *keys_down = NULL;
/** Last code key pressed */
Uint32 key_code_down = 0;
bool fire_button_down = FALSE;
bool option_button_down = FALSE;
bool start_button_down = FALSE;
bool joy_left = FALSE;
bool joy_right = FALSE;
bool joy_top = FALSE;
bool joy_down = FALSE;
Sint32 mouse_x = 0;
Sint32 mouse_y = 0;
/** 1 = left mouse pressed */
Sint32 mouse_b;
bool is_iconified = FALSE;

/* preselected colors */
unsigned char coulor[COLORS_ENUM_NUMOF];
Uint32 real_black_color = 0;
SCREEN_ORIENTATION screen_orientation = PORTRAIT;
static char *convert_16_or_24 (bitmap_desc * bmp);

/* 
 * common 
 */
/** Main window width */
Uint32 window_width = 0;
/** Main window height */
Uint32 window_height = 0;
/** TRUE = update display option panel and bareline's score */
bool update_all = TRUE;

/** 
 * Initialize SDL or X11 display
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
display_initialize (void)
{
  pixel_size = 1;
  /* width and height of the main window */
  display_width = DISPLAY_WIDTH * pixel_size;
  display_height = DISPLAY_HEIGHT * pixel_size;
  /* width and hieght of the main offscreen  */
  offscreen_width = OFFSCREEN_WIDTH * pixel_size;
  offscreen_height = OFFSCREEN_HEIGHT * pixel_size;
  /* width and height of the visible offscreen area */
  offscreen_width_visible = OFFSCREEN_WIDTH_VISIBLE * pixel_size;
  offscreen_height_visible = OFFSCREEN_HEIGHT_VISIBLE * pixel_size;
  /* x and y coordinates visible offscreen area */
  offscreen_startx = OFFSCREEN_STARTX * pixel_size;
  offscreen_starty = OFFSCREEN_STARTY * pixel_size;
  /* size of border clipping regions */
  offscreen_clipsize = OFFSCREEN_CLIPSIZE * pixel_size;

  /* 
   * top score panel offscreen 
   */
  score_offscreen_width = SCORES_WIDTH * pixel_size;
  score_offscreen_height = SCORES_HEIGHT * pixel_size;
  /* initialize X11 or SDL display */
  if (!display_init ())
    {
      return FALSE;
    }
  /* load our 256 colors palette */
  if (palette_24 == NULL)
    {
      palette_24 =
        (unsigned char *) load_file ("graphics/256_colors_palette.bin");
      if (palette_24 == NULL)
        {
          return FALSE;
        }
    }
  if (!create_palettes ())
    {
      return FALSE;
    }

  /* allocate table for keys flags down/up */
  if (keys_down == NULL)
    {
      keys_down =
        (bool *) memory_allocation (sizeof (bool *) * MAX_OF_KEYS_DOWN);
      if (keys_down == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes!",
                   (int) (sizeof (bool) * MAX_OF_KEYS_DOWN));
        }
    }

  if (power_conf->scale_x > 1)
    {
      screen_pixel_size = power_conf->scale_x;
    }
  else
    {
      if (power_conf->resolution == 640)
        {
          screen_pixel_size = 2;
        }
    }
  return TRUE;
}

/**
 * Shuts down all display subsystems and frees
 * the resources allocated to them
 */
void
display_release (void)
{
  display_free ();
  if (palette_24 != NULL)
    {
      free_memory ((char *) palette_24);
      palette_24 = NULL;
    }
  if (keys_down != NULL)
    {
      free_memory ((char *) keys_down);
      keys_down = NULL;
    }
}

/**
 * Search a color in the palette
 * @input r red intensity from 0 to 255
 * @input g green intensity from 0 to 255
 * @input b blue intensity from 0 to 255
 * @return palette entry from 0 to 255
 */
unsigned char
search_color (unsigned char r, unsigned char g, unsigned char b)
{
  unsigned char cr, cg, cb, indice_couleur;
  Uint32 i;
  double norme[256], min_norme;
  for (i = 0; i < 767; i += 3)
    {
      cr = palette_24[i];
      cg = palette_24[i + 1];
      cb = palette_24[i + 2];
      norme[i / 3] =
        sqrt (pow (r - cr, 2) + pow (g - cg, 2) + pow (b - cb, 2));
    }
  min_norme = norme[0];
  indice_couleur = 0;
  for (i = 1; i < 256; i++)
    {
      if (norme[i] < min_norme)
        {
          min_norme = norme[i];
          indice_couleur = (unsigned char) i;
        }
    }
  return indice_couleur;
}

/**
 * Initialize some predefined colors
 */
void
display_colors_init (void)
{
  coulor[BLACK] = search_color (0, 0, 0);
  coulor[LIGHT_GRAY] = search_color (212, 212, 212);
  coulor[WHITE] = search_color (255, 255, 255);
  coulor[RED] = search_color (255, 0, 0);
  coulor[GREEN] = search_color (0, 255, 0);
  real_black_color = coulor[BLACK];
  if (bytes_per_pixel == 2)
    {
      real_black_color = (Sint32) pal16[real_black_color];
    }
  else
    {
      if (bytes_per_pixel > 3)
        {
          real_black_color = (Sint32) pal32[real_black_color];
        }
    }
}

/**
 * Clear keyboard table
 */
void
clear_keymap (void)
{
  Uint32 i;
  for (i = 0; i < MAX_OF_KEYS_DOWN; i++)
    {
      keys_down[i] = FALSE;
    }
}

/** 
 * Color conversion 8-bit to 15-bits 16-bit, 24 bit or 32-bit
 * @param Pointer to a bitmap structure
 * @return Pointer to a pixel buffer
 */
static char *
convert_16_or_24 (bitmap_desc * gfx)
{
  Uint32 size;
  char *buffer;
  if (bytes_per_pixel == 1)
    {
      return gfx->pixel;
    }
  size = gfx->width * gfx->height;
  buffer = memory_allocation (size * bytes_per_pixel);
  if (buffer == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes!",
               size * bytes_per_pixel);
      return NULL;
    }
  switch (bytes_per_pixel)
    {
    case 2:
      conv8_16 (gfx->pixel, buffer, pal16, size);
      break;
    case 3:
      conv8_24 (gfx->pixel, buffer, pal32, size);
      break;
    case 4:
      conv8_32 (gfx->pixel, buffer, pal32, size);
      break;
    }
  free_memory (gfx->pixel);
  gfx->pixel = buffer;
  gfx->size = size * bytes_per_pixel;
  return buffer;
}

/** 
 * Decompress pcx file and convert to 8 bits (no change),
 * or 16 bits or 24 bits
 * @param filename the file which should be loaded
 * @return file data buffer pointer
 */
char *
load_pcx_file (const char *filename)
{
  char *buffer;
  bitmap_desc *gfx = load_pcx (filename);
  if (gfx == NULL)
    {
      return NULL;
    }
  if (bytes_per_pixel > 0)
    {
      if (!convert_16_or_24 (gfx))
        {
          LOG_ERR ("convert_16_or_24() failed!");
          return NULL;
        }
    }
  buffer = gfx->pixel;
  free_memory ((char *) gfx);
  return buffer;
}

/**
 * Decompress pcx file and convert to 8 bits (no change),
 * or 16 bits or 24 bits
 * @param filename the file which should be loaded
 * @param buffer pointer to the buffer where data are stored
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
load_pcx_into_buffer (const char *filename, char *buffer)
{
  Uint32 i;
  char *data;
  bitmap_desc *gfx = load_pcx (filename);
  if (gfx == NULL)
    {
      return FALSE;
    }
  if (bytes_per_pixel > 0)
    {
      if (!convert_16_or_24 (gfx))
        {
          LOG_ERR ("convert_16_or_24() failed!");
          return TRUE;
        }
    }
  data = gfx->pixel;
  for (i = 0; i < gfx->size; i++)
    {
      buffer[i] = data[i];
    }
  free_memory (data);
  free_memory ((char *) gfx);
  return TRUE;
}
