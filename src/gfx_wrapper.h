/**
 * @file gfx_wrapper.h 
 * @brief Wrap call to assembler routines
 * @created 2006-12-13 
 * @date 2007-08-06
 * @author Bruno Ethvignot 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: gfx_wrapper.h,v 1.14 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __GFX_WRAPPER__
#define __GFX_WRAPPER__


#ifdef __cplusplus
extern "C"
{
#endif

  void type_routine_gfx (void);
  void draw_sprite_mask (Uint32 color, image * img, Uint32 xcoord,
                         Uint32 ycoord);
  void draw_sprite (image * img, Uint32 xcoord, Uint32 ycoord);
  void draw_bitmap (bitmap * bmp, Uint32 xcoord, Uint32 ycoord);
  void poke_into_memory (char *adresse, unsigned char valeur);
  void put_pixel (char *dest, Sint32 xcoord, Sint32 ycoord, Sint32 color);
  void draw_electrical_shock (char *oscreen, Eclair * shock,
                              Sint32 numof_iterations);
  void draw_empty_rectangle (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                             Sint32 color, Sint32 width, Sint32 height);
  void draw_bitmap_char (unsigned char *dst, unsigned char *src);
  void draw_bitmap_in_options (bitmap * bmp, Uint32 xcoord, Uint32 ycoord);
  void draw_image_in_score (image * img, Uint32 xcoord, Uint32 ycoord);
  void draw_image_in_score_repeat (image * img, Uint32 xcoord, Uint32 ycoord,
                                   Uint32 width);
  void draw_bitmap_in_score (bitmap * bmp, Uint32 xcoord, Uint32 ycoord);
  void copy2X_512x440 (char *src, char *dest, Uint32);
  void copy2X (char *src, char *dest, Uint32 width, Uint32 height,
               Uint32 _iOffset, Uint32 _iOffset2);

#ifdef __cplusplus
}
#endif

#endif
