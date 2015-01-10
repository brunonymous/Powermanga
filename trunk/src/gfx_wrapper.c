/**
 * @file gfx_wrapper.c
 * @brief Wrap call to assembler routines in x86 or C routines in other
 * architectures
 * @created 1999-08-30 
 * @date 2014-09-17 
 * @author Bruno Ethvignot 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: gfx_wrapper.c,v 1.19 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "explosions.h"
#include "gfx_wrapper.h"
#include "gfxroutines.h"
#include "log_recorder.h"
#include "text_overlay.h"

/**
 * Check type of graphics routine C or assembler X86
 */
void
type_routine_gfx (void)
{
  Sint32 flagcode = 0;
  _type_routine_gfx (&flagcode);
  switch (flagcode)
    {
    case 10:
      LOG_INF ("powered by assembler routines");
      break;
    case 20:
      LOG_INF ("powered by C routines");
      break;
    default:
      LOG_ERR ("type_routine_gfx() error");
      break;
    }
}

/**
 *
 */
void
put_pixel (char *dest, Sint32 xcoord, Sint32 ycoord, Sint32 color)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_ERR ("dest = %p, xcoord = %i, ycoord = %i, color = %i "
           "not implemented", dest, xcoord, ycoord, color);
#else
  fprintf (stderr, "dest = %p, xcoord = %i, ycoord = %i, color = %i "
           "not implemented", dest, xcoord, ycoord, color);
#endif
}

/** 
 * Display sprite's mask (from an 'image' structure)
 * @param color Color of the mask
 * @param img Pointer to an 'image' structure
 * @param xcoord X-coordinate in game offscreen
 * @param ycoord Y-coordinate in game offscreen
 */
void
draw_sprite_mask (Uint32 color, image * img, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *dest, *repeats;
  dest =
    game_offscreen + (ycoord * offscreen_width + xcoord) * bytes_per_pixel;
  repeats = img->compress;
  size = img->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_mask_8bits (color, dest, repeats, size);
      break;
    case 2:
      put_sprite_mask_16bits ((Sint32) pal16[color], dest, repeats, size);
      break;
    case 3:
      put_sprite_mask_24bits ((Sint32) pal32[color], dest, repeats, size);
      break;
    case 4:
      put_sprite_mask_32bits ((Sint32) pal32[color], dest, repeats, size);
      break;
    }
}

/** 
 * Display sprite from an 'image' structure
 * @param img Pointer to an 'image' structure
 * @param xcoord X-coordinate in game offscreen
 * @param ycoord Y-coordinate in game offscreen
 */
void
draw_sprite (image * img, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *source, *dest, *repeats;
  source = img->img;
  dest =
    game_offscreen + (ycoord * offscreen_pitch + xcoord * bytes_per_pixel);
  repeats = img->compress;
  size = img->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_8 (source, dest, repeats, size);
      break;
    case 2:
      put_sprite_16 (source, dest, repeats, size);
      break;
    case 3:
      put_sprite_24 (source, dest, repeats, size);
      break;
    case 4:
      put_sprite_32 (source, dest, repeats, size);
      break;
    }
}

/** 
 * Display a sprite in game offscreen (from an 'bitmap' structure)
 * @param bmp Pointer to a 'bitmap' structure
 * @param xcoord X-coordinate in the game offscreen
 * @param ycoord Y-coordinate in the game offscreen
 */
void
draw_bitmap (bitmap * bmp, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *source, *dest, *repeats;
  source = bmp->img;
  dest =
    game_offscreen + (ycoord * offscreen_width + xcoord) * bytes_per_pixel;
  repeats = bmp->compress;
  size = bmp->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_8 (source, dest, repeats, size);
      break;
    case 2:
      put_sprite_16 (source, dest, repeats, size);
      break;
    case 3:
      put_sprite_24 (source, dest, repeats, size);
      break;
    case 4:
      put_sprite_32 (source, dest, repeats, size);
      break;
    }
}

/** 
 * Display sprite into score panel (from an 'bitmap' structure)
 * @param bmp Pointer to a 'bitmap' structure
 * @param xcoord X-coordinate in the score panel offscreen
 * @param ycoord Y-coordinate in the score panel offscreen
 */
void
draw_bitmap_in_score (bitmap * bmp, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *source, *dest, *repeats;
  source = bmp->img;
  dest =
    scores_offscreen + ycoord * score_offscreen_pitch +
    xcoord * bytes_per_pixel;
  repeats = bmp->compress;
  size = bmp->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_8 (source, dest, repeats, size);
      break;
    case 2:
      put_sprite_16 (source, dest, repeats, size);
      break;
    case 3:
      put_sprite_24 (source, dest, repeats, size);
      break;
    case 4:
      put_sprite_32 (source, dest, repeats, size);
      break;
    }
}

/** 
 * Display sprite into options panel (from an 'bitmap' structure)
 * @param bmp Pointer to a 'bitmap' structure
 * @param xcoord X-coordinate in the option panel offscreen
 * @param ycoord Y-coordinate in the option panel offscreen
 */
void
draw_bitmap_in_options (bitmap * bmp, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *source, *dest, *repeats;
  source = bmp->img;
  dest =
    options_offscreen + (ycoord * OPTIONS_WIDTH + xcoord) * bytes_per_pixel;
  repeats = bmp->compress;
  size = bmp->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_8 (source, dest, repeats, size);
      break;
    case 2:
      put_sprite_16 (source, dest, repeats, size);
      break;
    case 3:
      put_sprite_24 (source, dest, repeats, size);
      break;
    case 4:
      put_sprite_32 (source, dest, repeats, size);
      break;
    }
}

/** 
 * Display a sprite into score panel (from an 'image' structure)
 * @param img Pointer to a image structure
 * @param xcoord X-coordinate in the top panel 
 * @param ycoord Y-coordinate in the top panel
 */
void
draw_image_in_score (image * img, Uint32 xcoord, Uint32 ycoord)
{
  Uint32 size;
  char *source, *dest, *repeats;
  source = img->img;
  dest =
    scores_offscreen + ycoord * score_offscreen_pitch +
    xcoord * bytes_per_pixel;
  repeats = img->compress;
  size = img->nbr_data_comp >> 2;
  switch (bytes_per_pixel)
    {
    case 1:
      put_sprite_8 (source, dest, repeats, size);
      break;
    case 2:
      put_sprite_16 (source, dest, repeats, size);
      break;
    case 3:
      put_sprite_24 (source, dest, repeats, size);
      break;
    case 4:
      put_sprite_32 (source, dest, repeats, size);
      break;
    }
}

/**
 * Repeat the same image horizontally
 * @param img Pointer to a image structure
 * @param xcoord X-coordinate in the top panel 
 * @param ycoord Y-coordinate in the top panel
 * @param repeat_count The number of repetitions
 */
void
draw_image_in_score_repeat (image * img, Uint32 xcoord, Uint32 ycoord,
                            Uint32 repeat_count)
{
  Uint32 i, step, size;
  char *source, *dest, *repeats;
  source = img->img;
  dest =
    scores_offscreen + ycoord * score_offscreen_pitch +
    xcoord * bytes_per_pixel;
  repeats = img->compress;
  size = img->nbr_data_comp >> 2;
  step = 1 * pixel_size * bytes_per_pixel;
  switch (bytes_per_pixel)
    {
    case 1:
      for (i = 0; i < repeat_count; i++)
        {
          put_sprite_8 (source, dest, repeats, size);
          dest += step;
        }
      break;
    case 2:
      for (i = 0; i < repeat_count; i++)
        {
          put_sprite_16 (source, dest, repeats, size);
          dest += step;
        }
      break;
    case 3:
      for (i = 0; i < repeat_count; i++)
        {
          put_sprite_24 (source, dest, repeats, size);
          dest += step;
        }
      break;
    case 4:
      for (i = 0; i < repeat_count; i++)
        {
          put_sprite_32 (source, dest, repeats, size);
          dest += step;
        }
      break;
    }
}

/**
 * Draw electrical_shock
 * @param oscreen Offscreen to draw
 * @param shock Electrical shock structure
 * @param numof_iterations Maximum number of iterations
 */
void
draw_electrical_shock (char *oscreen, Eclair * shock, Sint32 numof_iterations)
{
  switch (bytes_per_pixel)
    {
    case 1:
      draw_electrical_shock_8 (oscreen, shock, numof_iterations);
      break;
    case 2:
      shock->col1 = (Sint32) pal16[shock->col1];
      shock->col2 = (Sint32) pal16[shock->col2];
      draw_electrical_shock_16 (oscreen, shock, numof_iterations);
      break;
    case 3:
      draw_electrical_shock_24 (oscreen, shock, numof_iterations);
      break;
    case 4:
      shock->col1 = (Sint32) pal32[shock->col1];
      shock->col2 = (Sint32) pal32[shock->col2];
      draw_electrical_shock_32 (oscreen, shock, numof_iterations);
      break;
    }
}

/**
 * Draw en empty box (cursor text)
 * @param oscreen Offscreen destination
 * @param xcoord X-coordinate
 * @param ycoord Y-coordinate
 * @param color The color of the rectangle 
 * @param width The width of the rectangle, in pixels
 * @param height The height of the rectangle, in pixels
 */
void
draw_empty_rectangle (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                      Sint32 color, Sint32 width, Sint32 height)
{
  switch (bytes_per_pixel)
    {
    case 1:
      draw_rectangle_8 (oscreen, xcoord, ycoord, color, width, height);
      break;
    case 2:
      draw_rectangle_16 (oscreen, xcoord, ycoord, (Sint32) (pal16[color]),
                         width, height);
      break;
    case 3:
      draw_rectangle_24 (oscreen, xcoord, ycoord, (Sint32) (pal32[color]),
                         width, height);
      break;
    case 4:
      draw_rectangle_32 (oscreen, xcoord, ycoord, (Sint32) (pal32[color]),
                         width, height);
      break;
    }
}

/** 
 * Display char 8*8 pixels (used for text overlay)
 * @param dest Pointer to offscreen where draw
 * @param source Pointer to pixels source
 */
void
draw_bitmap_char (unsigned char *dest, unsigned char *source)
{
  switch (bytes_per_pixel)
    {
    case 1:
      draw_bitmap_char_8 (source, dest, 8, 8, FONT_OVERLAY_WIDTH - 8,
                          offscreen_pitch - 8);
      break;
    case 2:
      draw_bitmap_char_16 (source, dest, 8, 8,
                           FONT_OVERLAY_WIDTH * 2 - 16, offscreen_pitch - 16);
      break;
    case 3:
      draw_bitmap_char_24 (source, dest, 8, 8,
                           FONT_OVERLAY_WIDTH * 3 - 24, offscreen_pitch - 24);
      break;
    case 4:
      draw_bitmap_char_32 (source, dest, 8, 8,
                           FONT_OVERLAY_WIDTH * 4 - 32, offscreen_pitch - 32);
      break;
    }
}

/**
 * Resize 256x184 zone into 512x328 zone. Double pixels horizontally,
 *     interlaced with empty line vertically
 * @param source Pointer to the source of image in low-res
 * @param dest Ponter  to the destination image in hi-res
 * @param height Number of lines of the source image 
 */
void
copy2X_512x440 (char *source, char *dest, Uint32 height)
{
  switch (bytes_per_pixel)
    {
    case 1:
      _COPY2X8BITS_512x440 (source, dest, height);
      break;
    case 2:
      _COPY2X16BITS_512x440 (source, dest, height);
      break;
    case 3:
      _COPY2X24BITS_512x440 (source, dest, height);
      break;
    case 4:
      _COPY2X32BITS_512x440 (source, dest, height);
      break;
    }
}

/**
 * Double pixels horizontally and vertically
 * @param source
 * @param dest
 * @param width
 * @param height
 * @param _iOffset
 * @param _iOffset2
 */
void
copy2X (char *source, char *dest, Uint32 width,
        Uint32 height, Uint32 _iOffset, Uint32 _iOffset2)
{
  switch (bytes_per_pixel)
    {
    case 1:
      _COPY2X8BITS (source, dest, width, height, _iOffset, _iOffset2);
      break;
    case 2:
      _COPY2X16BITS (source, dest, width, height, _iOffset, _iOffset2);
      break;
    case 3:
      _COPY2X24BITS (source, dest, width, height, _iOffset, _iOffset2);
      break;
    case 4:
      _COPY2X32BITS (source, dest, width, height, _iOffset, _iOffset2);
      break;
    }
}
