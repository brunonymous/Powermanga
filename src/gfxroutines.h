/**
 * @file gfxroutines.h
 * @brief handle memory allocation and file access
 * @created 2003-04-15 
 * @date 2007-09-02
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: gfxroutines.h,v 1.12 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __PLAIN_C_H__
#define __PLAIN_C_H__

#ifdef __cplusplus
extern "C"
{
#endif


  void _type_routine_gfx (Sint32 * adresse);
  void conv8_16 (char *, char *, unsigned short *, Uint32);
  void conv8_24 (char *, char *, Uint32 *, Uint32);
  void conv8_32 (char *, char *, Uint32 *, Uint32);

  void copie4octets (char *, char *, Uint32, Uint32, Uint32, Uint32);
  void clear_offscreen (char *offscreen, Uint32 widht, Uint32 height,
                        Uint32 offset);

  void convert_palette_24_to_16 (unsigned char *, unsigned short *);
  void convert_palette_24_to_15 (unsigned char *, unsigned short *);

  void _COPY2X8BITS (char *, char *, Uint32, Uint32, Uint32, Uint32);
  void _COPY2X16BITS (char *, char *, Uint32, Uint32, Uint32, Uint32);
  void _COPY2X24BITS (char *, char *, Uint32, Uint32, Uint32, Uint32);
  void _COPY2X32BITS (char *, char *, Uint32, Uint32, Uint32, Uint32);

  void draw_bitmap_char_8 (unsigned char *source, unsigned char *dest,
                           Uint32 width, Uint32 height, Uint32 offset_s,
                           Uint32 offset_d);
  void draw_bitmap_char_16 (unsigned char *source, unsigned char *dest,
                            Uint32 width, Uint32 height, Uint32 offset_s,
                            Uint32 offset_d);
  void draw_bitmap_char_24 (unsigned char *source, unsigned char *dest,
                            Uint32 width, Uint32 height, Uint32 offset_s,
                            Uint32 offset_d);
  void draw_bitmap_char_32 (unsigned char *source, unsigned char *dest,
                            Uint32 width, Uint32 height, Uint32 offset_s,
                            Uint32 offset_d);

  void put_sprite_8 (char *oscreen, char *dest, char *repeats, Uint32 size);
  void put_sprite_16 (char *oscreen, char *dest, char *repeats, Uint32 size);
  void put_sprite_24 (char *oscreen, char *dest, char *repeats, Uint32 size);
  void put_sprite_32 (char *oscreen, char *dest, char *repeats, Uint32 size);

  void put_sprite_mask_8bits (Uint32 color, char *oscreen, char *repeats,
                              Uint32 size);
  void put_sprite_mask_16bits (Uint32 color, char *oscreen, char *repeats,
                               Uint32 size);
  void put_sprite_mask_24bits (Uint32 color, char *oscreen, char *repeats,
                               Uint32 size);
  void put_sprite_mask_32bits (Uint32 color, char *oscreen, char *repeats,
                               Uint32 size);

  void poly8bits (char *, Sint32, Sint32);
  void poly16bits (char *, Sint32, Sint32);
  void poly24bits (char *, Sint32, Sint32);
  void poly32bits (char *, Sint32, Sint32);

  void draw_electrical_shock_8 (char *, Eclair *, Sint32);
  void draw_electrical_shock_16 (char *, Eclair *, Sint32);
  void draw_electrical_shock_24 (char *, Eclair *, Sint32);
  void draw_electrical_shock_32 (char *, Eclair *, Sint32);

  void _COPY2X8BITS_512x440 (char *, char *, Uint32);
  void _COPY2X16BITS_512x440 (char *, char *, Uint32);
  void _COPY2X24BITS_512x440 (char *, char *, Uint32);
  void _COPY2X32BITS_512x440 (char *, char *, Uint32);

  void draw_rectangle_8 (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                         Sint32 color, Sint32 width, Sint32 height);
  void draw_rectangle_16 (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                          Sint32 color, Sint32 width, Sint32 height);
  void draw_rectangle_24 (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                          Sint32 color, Sint32 width, Sint32 height);
  void draw_rectangle_32 (char *oscreen, Sint32 xcoord, Sint32 ycoord,
                          Sint32 color, Sint32 width, Sint32 height);

#ifdef __cplusplus
}
#endif
#endif
