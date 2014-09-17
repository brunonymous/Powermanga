/** 
 * @file gfxroutines.c 
 * @brief 
 * @created 2003-04-15  
 * @date 2014-09-17 
 * @author Sam Hocevar <sam@zoy.org> 
 */
/* 
 * copyright (c) 2003-2007 Sam Hocevar <sam@zoy.org> 
 * $Id: gfxroutines.c,v 1.22 2010/08/15 10:34:27 gurumeditation Exp $
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
#include "electrical_shock.h"
#include "log_recorder.h"
#include "images.h"
#include "display.h"

void
_type_routine_gfx (Sint32 * addr)
{
  *addr = 20;
}

/* To test these functions: the intro animation */

void
conv8_16 (char *src, char *dest, Uint16 * pal16, Uint32 size)
{
  Uint16 *d = (Uint16 *) dest;
  unsigned char *s = (unsigned char *) src;
  while (size--)
    {
      *d++ = pal16[*s++];
    }
}

void
conv8_24 (char *src, char *dest, Uint32 * pal24, Uint32 size)
{
  /* (!) This code was not tested at all */
  char *p;
  char *d = (char *) dest;
  unsigned char *s = (unsigned char *) src;
  while (size--)
    {
      p = (char *) (pal24 + *(s++));
      *d++ = *p++;
      *d++ = *p++;
      *d++ = *p++;
    }
}

void
conv8_32 (char *src, char *dest, Uint32 * pal32, Uint32 size)
{
  Uint32 *d = (Uint32 *) dest;
  unsigned char *s = (unsigned char *) src;
  while (size--)
    {
      *d++ = pal32[*s++];
    }
}

/**
 * @param src Pointer to the source of data to be copied
 * @param dest Pointer to the destination array where the content is to be copied
 * @param width Number of 32-bit words per line
 * @param height Number of lines
 * @param offset_src Offset of the next source line
 * @param offset_dest Offset of the next destinatino line
 */
void
copie4octets (char *src, char *dest,
              Uint32 width, Uint32 height, Uint32 offset_src,
              Uint32 offset_dest)
{
  /* (!) This code was not tested at all */
  width *= 4;
  while (height--)
    {
      memcpy (src, dest, width);
      src += width + offset_src;
      src += width + offset_dest;
    }
}

/**
 * Clear a offscreen
 * @param oscreen Pointer to a offscreen destination
 * @param with Width of region to clear in 32-bit long words
 * @param height Height of region to clear in lines
 * @param offset Offet of the next line
 */
void
clear_offscreen (char *oscreen, Uint32 width, Uint32 height, Uint32 offset)
{
  width *= 4;
  while (height--)
    {
      memset (oscreen, 0, width);
      oscreen += width + offset;
    }
}

/**
 * Convert a 24-bit RGB palette to a 16-bit RGB palette 
 * @param src 24-bit RGB palette source
 * @param dest 16-bit RGB palette destination 
 */
void
convert_palette_24_to_16 (unsigned char *src, Uint16 * dest)
{
  Sint32 i = 256;
  while (i--)
    {
      Uint16 d = 0;
      d |= (Uint16) ((*src++) & 0x000000f8) << 8;
      d |= (Uint16) ((*src++) & 0xfffffffc) << 3;
      d |= (Uint16) ((*src++) & 0x000000f8) >> 3;
      *dest++ = d;
    }
}

/**
 * Convert a 24-bit RGB palette to a 15-bit RGB palette 
 * @param src 24-bit RGB palette source
 * @param dest 15-bit RGB palette destination 
 */
void
convert_palette_24_to_15 (unsigned char *src, Uint16 * dest)
{
  Sint32 i = 256;
  while (i--)
    {
      Uint16 d = 0;
      d |= ((Uint16) (*src++) >> 3) << 10;
      d |= ((Uint16) (*src++) >> 3) << 5;
      d |= ((Uint16) (*src++) >> 3) << 0;
      *dest++ = d;
    }
}

/* To test these functions: the intro animation with the --640 flag */
#define COPY2X(TYPE) \
  Sint32 i; \
  TYPE *src = (TYPE *)source; \
  TYPE *dest = (TYPE *)destination; \
  \
  while(height--) \
    { \
      for(i = width / 8; i--;) \
        { \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
        } \
      for(i = width % 8; i--;) \
        { \
          dest[0] = dest[1] = *src++; \
          dest += 2; \
        } \
      src += offset_src / sizeof(TYPE); \
      dest += offset_dest / sizeof(TYPE); \
    }

void
_COPY2X8BITS (char *source, char *destination,
              Uint32 width, Uint32 height, Uint32 offset_src,
              Uint32 offset_dest)
{
  COPY2X (unsigned char);
}

void
_COPY2X16BITS (char *source, char *destination,
               Uint32 width, Uint32 height, Uint32 offset_src,
               Uint32 offset_dest)
{
  COPY2X (Uint16);
}

void
_COPY2X24BITS (char *source, char *destination,
               Uint32 width, Uint32 height, Uint32 offset_src,
               Uint32 offset_dest)
{
  /* (!) This code was not tested at all */
  _COPY2X8BITS (source, destination, width * 3, height, offset_src,
                offset_dest);
}

void
_COPY2X32BITS (char *source, char *destination,
               Uint32 width, Uint32 height, Uint32 offset_src,
               Uint32 offset_dest)
{
  COPY2X (Uint32);
}

/**
 * Draw a 8x8 font (8 / 16 / 24 or 32-bit depth)
 * @param src
 * @param dest
 * @param with
 * @param height
 * @param src_offset
 * @param dest_offset
 */
void
draw_bitmap_char_8 (unsigned char *src, unsigned char *dest,
                    Uint32 width, Uint32 height,
                    Uint32 src_offset, Uint32 dest_offset)
{
  Uint32 h, l;
  unsigned char p;
  unsigned char *s = src;
  unsigned char *d = dest;
  for (h = 0; h < height; h++)
    {
      for (l = 0; l < width; l++)
        {
          p = *(s++);
          if (p != 0)
            {
              *d = p;
            }
          d++;
        }
      s = s + src_offset;
      d = d + dest_offset;
    }
}

void
draw_bitmap_char_16 (unsigned char *src, unsigned char *dest,
                     Uint32 width, Uint32 height,
                     Uint32 src_offset, Uint32 dest_offset)
{
  Uint32 h, l;
  Uint16 p;
  Uint16 *s = (Uint16 *) src;
  Uint16 *d = (Uint16 *) dest;
  src_offset = src_offset / 2;
  dest_offset = dest_offset / 2;
  for (h = 0; h < height; h++)
    {
      for (l = 0; l < width; l++)
        {
          p = *(s++);
          if (p != 0)
            {
              *d = p;
            }
          d++;
        }
      s = s + src_offset;
      d = d + dest_offset;
    }
}

void
draw_bitmap_char_24 (unsigned char *src, unsigned char *dest,
                     Uint32 width, Uint32 height,
                     Uint32 src_offset, Uint32 dest_offset)
{

#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN ("not implemented: src = %p; dest = %p;  width = %i,"
            "height = %i, src_offset = %i, dest_offset = %i", src, dest,
            width, height, src_offset, dest_offset);
#else
  fprintf (stderr, "not implemented: src = %p; dest = %p;  width = %i,"
           "height = %i, src_offset = %i, dest_offset = %i", src, dest, width,
           height, src_offset, dest_offset);
#endif
}

void
draw_bitmap_char_32 (unsigned char *src, unsigned char *dest,
                     Uint32 width, Uint32 height,
                     Uint32 src_offset, Uint32 dest_offset)
{
  Uint32 h, l, p;
  Uint32 *s = (Uint32 *) src;
  Uint32 *d = (Uint32 *) dest;
  src_offset = src_offset / 4;
  dest_offset = dest_offset / 4;
  for (h = 0; h < height; h++)
    {
      for (l = 0; l < width; l++)
        {
          p = *(s++);
          if (p != 0)
            {
              *d = p;
            }
          d++;
        }
      s = s + src_offset;
      d = d + dest_offset;
    }
}

/* To test these functions: the main menu, the whole game */

#define PUTSPRITE(TYPE) \
    Uint32 *pCompression = (Uint32 *)repeats_table; \
    \
    while(size--) \
    { \
        dest += *pCompression++; \
        \
        Uint32 data = *pCompression++; \
        Uint32 i = ((data & 0x0000ffff) << 2) /* 32 bits words */ \
                       | ((data & 0xffff0000) \
                           / (0x10000 / sizeof(TYPE))); /* 8 bits words */ \
        \
        memcpy( dest, src, i ); \
        dest += i; \
        src += i; \
    }

void
put_sprite_8 (char *src, char *dest, char *repeats_table, Uint32 size)
{
  Uint16 z;
  Uint32 *s2, *p2;
  unsigned char *s = (unsigned char *) src;
  unsigned char *p = (unsigned char *) dest;
  _compress *t = (_compress *) repeats_table;
  do
    {
      register Uint32 d = t->offset;
      p = p + d;
      z = t->r1;
      s2 = (Uint32 *) s;
      p2 = (Uint32 *) p;
      memcpy (p2, s2, z * 4);
      p2 = p2 + z;
      s2 = s2 + z;
      s = (unsigned char *) s2;
      p = (unsigned char *) p2;
      z = t->r2;
      memcpy (p2, s2, z);
      p = p + z;
      s = s + z;
      t++;
      size = size - 1;
    }
  while (size > 0);
}

void
put_sprite_16 (char *src, char *dest, char *repeats_table, Uint32 size)
{
  Uint16 z;
  Uint32 *s2, *p2;
  Uint16 *s = (Uint16 *) src;
  Uint16 *p = (Uint16 *) dest;
  _compress *t = (_compress *) repeats_table;
  do
    {
      register Uint32 d = t->offset;
      d = d >> 1;
      p = p + d;
      z = t->r1;
      s2 = (Uint32 *) s;
      p2 = (Uint32 *) p;
      memcpy (p2, s2, z * 4);
      p2 = p2 + z;
      s2 = s2 + z;
      s = (Uint16 *) s2;
      p = (Uint16 *) p2;
      z = t->r2;
      memcpy (p2, s2, z * 2);
      p = p + z;
      s = s + z;
      t++;
      size = size - 1;
    }
  while (size > 0);
}

/**
 *
 */
void
put_sprite_24 (char *src, char *dest, char *repeats_table, Uint32 size)
{
  register Uint16 z;
  unsigned char *s8;
  unsigned char *p8;
  Uint32 *s = (Uint32 *) src;
  Uint32 *p = (Uint32 *) dest;
  _compress *t = (_compress *) repeats_table;
  do
    {
      register Uint32 d = t->offset;
      d = d >> 2;
      p = p + d;
      z = t->r1;
      memcpy (p, s, z * 4);
      p = p + z;
      s = s + z;
      s8 = (unsigned char *) s;
      p8 = (unsigned char *) p;
      z = t->r2;
      memcpy (p8, s8, z);
      p8 = p8 + z;
      s8 = s8 + z;
      s = (Uint32 *) s8;
      p = (Uint32 *) p8;
      t++;
      size = size - 1;
    }
  while (size > 0);
}

/**
 *
 */
void
put_sprite_32 (char *src, char *dest, char *repeats_table, Uint32 size)
{
  register Uint16 z;
  Uint32 *s = (Uint32 *) src;
  Uint32 *p = (Uint32 *) dest;
  _compress *t = (_compress *) repeats_table;
  do
    {
      register Uint32 d = t->offset;
      d = d >> 2;
      p = p + d;
      z = t->r1;
      memcpy (p, s, z * 4);
      p = p + z;
      s = s + z;
      z = t->r2;
      memcpy (p, s, z * 4);
      p = p + z;
      s = s + z;
      t++;
      size = size - 1;
    }
  while (size > 0);
}


/* To test these functions: when an enemy gets killed */

#define PUTCOLOR(TYPE) \
    Uint32 data, i; \
    TYPE *d = (TYPE *)dest; \
    Uint32 *pCompression = (Uint32 *)repeats_table; \
    \
    while(size--) \
    { \
        d += *pCompression++ / sizeof(TYPE); \
        \
        data = *pCompression++; \
        i = ((data & 0x0000ffff) \
                    * (4 / sizeof(TYPE))) /* 32 bits words */ \
                | ((data & 0xffff0000) >> 16); /* remaining */ \
        \
        if( sizeof(TYPE) == 1 ) \
        { \
            memset(d, _iColor, i); \
            d += i; \
        } \
        else \
        { \
            while(i--) *d++ = _iColor; \
        } \
    }

void
put_sprite_mask_8bits (Uint32 _iColor, char *dest,
                       char *repeats_table, Uint32 size)
{
  Uint16 z;
  Uint32 *p2;
  unsigned char *p = (unsigned char *) dest;
  _compress *t = (_compress *) repeats_table;
  do
    {
      Uint32 d = t->offset;
      p = p + d;
      z = t->r1;
      p2 = (Uint32 *) p;
      memset (p2, _iColor, z * 4);
      p2 = p2 + z;
      p = (unsigned char *) p2;
      z = t->r2;
      memset (p2, _iColor, z);
      p = p + z;
      t++;
      size = size - 1;
    }
  while (size > 0);
}

void
put_sprite_mask_16bits (Uint32 _iColor, char *dest,
                        char *repeats_table, Uint32 size)
{
  Uint16 z;
  Uint32 *p2;
  _compress *t;
  Uint16 *p;
  Uint32 d = _iColor << 16;
  _iColor = _iColor | d;
  p = (Uint16 *) dest;
  t = (_compress *) repeats_table;
  do
    {
      d = t->offset;
      d = d >> 1;
      p = p + d;
      z = t->r1;
      p2 = (Uint32 *) p;
      memset (p2, _iColor, z * 4);
      p2 = p2 + z;
      p = (Uint16 *) p2;
      z = t->r2;
      memset (p2, _iColor, z * 2);
      p = p + z;
      t++;
      size = size - 1;
    }
  while (size > 0);

}

void
put_sprite_mask_24bits (Uint32 _iColor, char *dest,
                        char *repeats_table, Uint32 size)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN ("not implemented: _iColor = %i; dest = %p;"
            "repeats_table = %p; size = %i", _iColor, dest, repeats_table,
            size);
#else
  fprintf (stderr, "not implemented: _iColor = %i; dest = %p;"
           "repeats_table = %p; size = %i", _iColor, dest, repeats_table,
           size);
#endif
}

void
put_sprite_mask_32bits (Uint32 _iColor, char *dest,
                        char *repeats_table, Uint32 size)
{
  PUTCOLOR (Uint32);
}

/* To test these functions: the nuke (purple gem) */

#define POLY(TYPE) \
  TYPE *pDestination = (TYPE *)dest; \
  \
  if(sizeof(TYPE) == 1) \
    { \
      memset(pDestination, color, numof_pixels); \
    } \
  else \
    { \
      while(numof_pixels--) \
        { \
          *pDestination++ = (TYPE)color; \
        } \
    }

void
poly8bits (char *dest, Sint32 numof_pixels, Sint32 color)
{
  POLY (unsigned char);
}

void
poly16bits (char *dest, Sint32 numof_pixels, Sint32 color)
{
  POLY (Uint16);
}

void
poly24bits (char *dest, Sint32 numof_pixels, Sint32 color)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN ("not implemented: dest = %p; numof_pixels = %i; color = %i", dest,
            numof_pixels, color);
#else
  fprintf (stderr,
           "not implemented: dest = %p; numof_pixels = %i; color = %i", dest,
           numof_pixels, color);
#endif
}

void
poly32bits (char *dest, Sint32 numof_pixels, Sint32 color)
{
  POLY (Uint32);
}

/* To test these functions: the lightning (6 yellow gems) */

#define BRESENHAM(TYPE, cur1, cur2, inc1, inc2, d1, d2) \
  do \
    { \
      Sint32 dp = d2 << 1; \
      Sint32 dpu = dp - (d1 << 1); \
      Sint32 p = dp - d1; \
      \
      while(d1--) \
        { \
          pDestination[cur1 + cur2 - inc2] = (TYPE)shock->col2; \
          pDestination[cur1 + cur2] = (TYPE)shock->col1; \
          pDestination[cur1 + cur2 + inc2] = (TYPE)shock->col2; \
          cur1 += inc1; \
          if (p > 0) \
            { \
              cur2 += inc2; \
              p += dpu; \
            } \
          else \
            { \
              p += dp; \
            } \
        } \
  } \
  while(0)

#define DRAW_ECLAIR(TYPE,CALLBACK) \
  TYPE *pDestination = (TYPE *)dest; \
  \
  if(numof_iterations--) \
    { \
      Sint32 dx, dy, midx, midy, r, oldx, oldy; \
      \
      dx = (shock->dx - shock->sx) / 2; \
      dy = (shock->dy - shock->sy) / 2; \
      \
      /* Add a little random normal deviation */ \
      r = randomize_eclair (shock) / (1 << 24); \
      midx = shock->sx + dx + ((dy * r) / (1 << 8)); \
      midy = shock->sy + dy - ((dx * r) / (1 << 8)); \
      \
      /* Recurse on both halves */ \
      oldx = shock->sx; shock->sx = midx; \
      oldy = shock->sy; shock->sy = midy; \
      CALLBACK (dest, shock, numof_iterations); \
      shock->sx = oldx; \
      shock->sy = oldy; \
      \
      oldx = shock->dx; shock->dx = midx; \
      oldy = shock->dy; shock->dy = midy; \
      CALLBACK (dest, shock, numof_iterations); \
      shock->dx = oldx; \
      shock->dy = oldy; \
    } \
  else \
    { \
      /* Draw a line using Bresenham */ \
      Sint32 dx = abs(shock->dx - shock->sx); \
      Sint32 dy = abs(shock->dy - shock->sy); \
      \
      Sint32 xcur = shock->sx; \
      Sint32 ycur = shock->sy * 512; \
      \
      Sint32 xinc = shock->sx > shock->dx ? -1 : 1; \
      Sint32 yinc = shock->sy > shock->dy ? -512 : 512; \
      \
      if(dx >= dy) \
        { \
          BRESENHAM(TYPE, xcur, ycur, xinc, yinc, dx, dy); \
        } \
      else \
        { \
          BRESENHAM(TYPE, ycur, xcur, yinc, xinc, dy, dx); \
        } \
    }

static Sint32
randomize_eclair (Eclair * shock)
{
  Sint32 a = shock->r1, b = shock->r2, c = shock->r3;

  a = (a << 13) | (a >> 3);
  a = a ^ c;
  b = b ^ c;
  b = (b >> 7) | (b << 9);
  c += 27;
  a = a ^ c;
  b = b ^ c;

  shock->r1 = c;
  shock->r2 = a;
  shock->r3 = b;

  return c;
}

static void
draw_electrical_shock_8_in (char *dest, Eclair * shock,
                            Sint32 numof_iterations)
{
  DRAW_ECLAIR (unsigned char, draw_electrical_shock_8_in);
}

static void
draw_electrical_shock_16_in (char *dest, Eclair * shock,
                             Sint32 numof_iterations)
{
  DRAW_ECLAIR (Uint16, draw_electrical_shock_16_in);
}

static void
draw_electrical_shock_32_in (char *dest, Eclair * shock,
                             Sint32 numof_iterations)
{
  DRAW_ECLAIR (Uint32, draw_electrical_shock_32_in);
}

void
draw_electrical_shock_8 (char *dest, Eclair * shock, Sint32 numof_iterations)
{
  Sint32 a = shock->r1, b = shock->r2, c = shock->r3;
  draw_electrical_shock_8_in (dest, shock, numof_iterations);
  shock->r1 = a;
  shock->r2 = b;
  shock->r3 = c;
}

void
draw_electrical_shock_16 (char *dest, Eclair * shock, Sint32 numof_iterations)
{
  Sint32 a = shock->r1, b = shock->r2, c = shock->r3;
  draw_electrical_shock_16_in (dest, shock, numof_iterations);
  shock->r1 = a;
  shock->r2 = b;
  shock->r3 = c;
}

void
draw_electrical_shock_24 (char *dest, Eclair * shock, Sint32 numof_iterations)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN ("not implemented: dest = %p; shock = %p; numof_iterations = %i",
            dest, shock, numof_iterations);
#else
  fprintf (stderr,
           "not implemented: dest = %p; shock = %p; numof_iterations = %i",
           dest, shock, numof_iterations);
#endif
}

void
draw_electrical_shock_32 (char *dest, Eclair * shock, Sint32 numof_iterations)
{
  Sint32 a = shock->r1, b = shock->r2, c = shock->r3;
  draw_electrical_shock_32_in (dest, shock, numof_iterations);
  shock->r1 = a;
  shock->r2 = b;
  shock->r3 = c;
}

/* To test these functions: the main menu, the game */

#define COPY2X_512x440(TYPE) \
  Sint32 i, j; \
  TYPE *src = (TYPE *)_src; \
  TYPE *dest = (TYPE *)_dest; \
  \
  for(i = height; i--;) \
    { \
      for(j = 256 / 8; j--;) \
        { \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
          dest[0] = dest[1] = *src++; dest += 2; \
        } \
      src += 256; \
      dest += 640 + (640 - 512); \
    }

void
_COPY2X8BITS_512x440 (char *_src, char *_dest, Uint32 height)
{
  COPY2X_512x440 (unsigned char);
}

void
_COPY2X16BITS_512x440 (char *_src, char *_dest, Uint32 height)
{
  COPY2X_512x440 (Uint16);
}

void
_COPY2X24BITS_512x440 (char *_src, char *_dest, Uint32 height)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN ("not implemented. _src = %p; _dest = %p; height = %i", _src,
            _dest, height);
#else
  fprintf (stderr, "not implemented. _src = %p; _dest = %p; height = %i",
           _src, _dest, height);
#endif
}

void
_COPY2X32BITS_512x440 (char *_src, char *_dest, Uint32 height)
{
  COPY2X_512x440 (Uint32);
}

/* To test these functions: the cursor in the "ORDER" menu and name input */

#define PUTRECT(TYPE) \
  TYPE *dest = (TYPE *)adresse + (offscreen_width * y + x); \
  Sint32 i; \
  \
  /* Top line */ \
  i = width; \
  while(i--) \
    { \
      *dest++ = (TYPE)coul; \
    } \
  \
  dest += offscreen_width - width; \
  height -= 2; \
  \
  /* Side lines */ \
  while(height-- > 0) \
  { \
      dest[0] = dest[width-1] = (TYPE)coul; \
      dest += offscreen_width; \
  } \
  \
  /* Bottom line */ \
  i = width; \
  while(i--) \
    { \
      *dest++ = (TYPE)coul; \
    } \

#define OOOOPUTRECT(TYPE) \
  TYPE *dest = (TYPE *)adresse + (offscreen_width * y + x); \
  Sint32 i; \
  \
  /* Top line */ \
  i = width; \
  while(i--) \
    { \
      *dest++ = (TYPE)coul; \
    } \
  \
  dest += offscreen_width - width; \
  height -= 2; \
  \
  /* Side lines */ \
  while(height-- > 0) \
  { \
      dest[0] = dest[width-1] = (TYPE)coul; \
      dest += offscreen_width; \
  } \
  \
  /* Bottom line */ \
  i = width; \
  while(i--) \
    { \
      *dest++ = (TYPE)coul; \
    } \

void
draw_rectangle_8 (char *adresse, Sint32 x, Sint32 y,
                  Sint32 coul, Sint32 width, Sint32 height)
{
  PUTRECT (unsigned char);
}

void
draw_rectangle_16 (char *adresse, Sint32 x, Sint32 y,
                   Sint32 coul, Sint32 width, Sint32 height)
{
  PUTRECT (Uint16);
}

void
draw_rectangle_24 (char *adresse, Sint32 x, Sint32 y,
                   Sint32 coul, Sint32 width, Sint32 height)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_WARN
    ("not implemented. adresse = %p, x = %i, y = %i, coul = %i, width = %i, height = %i",
     adresse, x, y, coul, width, height);
#else
  fprintf
    (stderr,
     "not implemented. adresse = %p, x = %i, y = %i, coul = %i, width = %i, height = %i",
     adresse, x, y, coul, width, height);
#endif
}

void
draw_rectangle_32 (char *adresse, Sint32 x, Sint32 y,
                   Sint32 coul, Sint32 width, Sint32 height)
{
  PUTRECT (Uint32);
}
