/** 
 * @file images.c
 * @brief Read and extract data from sprite and bitmap file 
 * @created 2006-12-13 
 * @date 2014-09-17 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: images.c,v 1.34 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "display.h"
#include "images.h"
#include "log_recorder.h"
#ifdef PNG_EXPORT_ENABLE
#include <zlib.h>
#include <png.h>
#endif

static char *bitmap_read (bitmap * bmp, Uint32 num_of_obj,
                          Uint32 num_of_images, char *addr,
                          Uint32 max_of_anims);
static char *image_extract (image * img, const char *filename);
static char *bitmap_extract (bitmap * bmp, char *filedata);
static char *read_pixels (Uint32 numofpixels, char *source,
                          char *destination);
static char *read_compress (Uint32 filesize, char *filedata, char *compress);

/** 
 * Load and extract a file *.spr into 'image' structure
 * @param fname The file *.spr which should be loaded 
 * @param img Pointer to destination 'image' structure 
 * @param num_of_sprites Number different sprites
 * @param num_of_anims Number of animations for a same sprite
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
image_load (const char *fname, image * img, Uint32 num_of_sprites,
            Uint32 num_of_anims)
{
  char *data;
  char *file = load_file (fname);
  if (file == NULL)
    {
      return FALSE;
    }
  data = file;
  data = images_read (img, num_of_sprites, num_of_anims, data, num_of_anims);
  if (data == NULL)
    {
      free_memory (file);
      return FALSE;
    }
  free_memory (file);
  return TRUE;
}

/** 
 * Load and extract a file with number *.spr with into 'image' structure
 * @param fname filename The file *.spr (ie "STAR%1d.SPR") 
 * @param num The file number from 0 to n
 * @param img Pointer to destination 'image' structure 
 * @param num_of_sprites Number different sprites
 * @param num_of_anims Number of animations for a same sprite
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
image_load_num (const char *fname, Sint32 num, image * img,
                Uint32 num_of_sprites, Uint32 num_of_anims)
{
  char *data;
  char *file = loadfile_num (fname, num);
  if (file == NULL)
    {
      return FALSE;
    }
  data = images_read (img, num_of_sprites, num_of_anims, file, num_of_anims);
  free_memory (file);
  if (data == NULL)
    {
      return FALSE;
    }
  return TRUE;
}

/** 
 *  Load and extract a file *.spr into 'fonte' structure
 */
bool
bitmap_load (const char *fname, bitmap * fonte, Uint32 num_of_obj,
             Uint32 num_of_images)
{
  char *addr;
  char *file = load_file (fname);
  if (file == NULL)
    {
      return FALSE;
    }
  addr = bitmap_read (fonte, num_of_obj, num_of_images, file, num_of_images);
  if (addr == NULL)
    {
      free_memory (file);
      return FALSE;
    }
  free_memory (file);
  return TRUE;
}

/**
 * Read a sprite filedata to an 'image' structure
 * @param img Pointer to destination 'image' structure 
 * @param num_of_sprites Number different sprites
 * @param num_of_anims Number of animations for a same sprite
 * @param addr Pointer to the source sprite filedata 
 * @param max_of_anims Maximum number of images per type of sprite
 * @return Pointer to source sprite filedata 
 */
char *
images_read (image * img, Uint32 num_of_sprites, Uint32 num_of_anims,
             char *addr, Uint32 max_of_anims)
{
  Uint32 i, j;
  for (i = 0; i < num_of_sprites; i++)
    {
      for (j = 0; j < num_of_anims; j++)
        {
          addr = image_extract (img + (i * max_of_anims) + j, addr);
          if (addr == NULL)
            {
              LOG_ERR ("image_extract failed!");
              return NULL;
            }
        }
    }
  return addr;
}

/**
 * Release images data 
 * @param first_image Pointer to the first 'image' structure 
 * @param num_of_sprites Number different sprites
 * @param num_of_anims Number of animations for a same sprite
 * @param max_of_anims Maximum number of images per type of sprite
 */
void
images_free (image * first_image, Uint32 num_of_sprites, Uint32 num_of_anims,
             Uint32 max_of_anims)
{
  Uint32 i, j;
  image *img;
  for (i = 0; i < num_of_sprites; i++)
    {
      for (j = 0; j < num_of_anims; j++)
        {
          img = first_image + (i * max_of_anims) + j;
          if (img->img != NULL)
            {
              free_memory (img->img);
              img->img = NULL;
            }
          if (img->compress != NULL)
            {
              free_memory (img->compress);
              img->compress = NULL;
            }
        }
    }
}

/**
 * Read a bitmap filedata to an 'bitmap' structure
 * @param bmp Pointer to destination 'bitmap' structure 
 * @param num_of_obj Number different bitmap objects
 * @param num_of_images Number of images for a same bitmap
 * @param addr Pointer to the source bitmap filedata 
 * @param width Width of the bitmap in pixels
 * @return The source pointer incremented
 */
static char *
bitmap_read (bitmap * bmp, Uint32 num_of_obj, Uint32 num_of_images,
             char *addr, Uint32 max_of_anims)
{
  Uint32 i, j;
  for (i = 0; i < num_of_obj; i++)
    {
      for (j = 0; j < num_of_images; j++)
        {
          addr = bitmap_extract (bmp + (i * max_of_anims) + j, addr);
          if (addr == NULL)
            {
              LOG_ERR ("bitmap_extract() failed!");
              return NULL;
            }
        }
    }
  return addr;
}

/**
 * Release bitmap data 
 */
void
bitmap_free (bitmap * first_bitmap, Uint32 num_of_bitmap, Uint32 num_of_anims,
             Uint32 max_of_anims)
{
  Uint32 i, j;
  bitmap *bmp;
  for (i = 0; i < num_of_bitmap; i++)
    {
      for (j = 0; j < num_of_anims; j++)
        {
          bmp = first_bitmap + (i * max_of_anims) + j;
          if (bmp->img != NULL)
            {
              free_memory (bmp->img);
              bmp->img = NULL;
            }
          if (bmp->compress != NULL)
            {
              free_memory (bmp->compress);
              bmp->compress = NULL;
            }
        }
    }
}

/**
 * Load a sprite file (*.spr) with a single image
 * @param filename Filename the file *.spr which should be loaded 
 * @param img Pointer to destination 'image' structure 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
image_load_single (const char *filename, image * img)
{
  char *addr;
  char *filedata = load_file (filename);
  if (filedata == NULL)
    {
      return FALSE;
    }
  addr = image_extract (img, filedata);
  if (addr == NULL)
    {
      LOG_ERR ("image_extract() failed!");
      free_memory (filedata);
      return FALSE;
    }
  free_memory (filedata);
  return TRUE;
}

/**
 * Convert from sprite filedata to 'image' stucture 
 * Values from filedata are stored in little-endian
 * - x-coordinate, y-coordinate, width and height of the sprite
 * - List of the collision coordinates
 * - list of the collisions areas coordinates and areas sizes (width/height)
 * - list of the cannons coordinates and shots angles
 * - list of the pixels of the sprite without the transparent pixels (0 value)
 * - list of offsets and repeats values (compress table) for displaying sprite
 *
 * @param img Pointer to destination 'image' structure 
 * @param file Pointer to source sprite filedata
 * @return Pointer to the end of source sprite filedata
 */
static char *
image_extract (image * img, const char *file)
{
  Uint32 i;
  Sint16 *dest;
  /* pointer to the file (8/16/32-bit access */
  char *ptr8;
  Sint16 *ptr16;
  Sint32 *ptr32;

  /* 
   * read coordinates and size of the sprite 
   */
  /* 16-bit access */
  ptr16 = (Sint16 *) file;
  /* x and y coordinates of the centre of gravity  */
  img->x_gc = little_endian_to_short (ptr16++);
  img->y_gc = little_endian_to_short (ptr16++);
  /* image width and height */
  img->w = little_endian_to_short (ptr16++);
  img->h = little_endian_to_short (ptr16++);

  /* 
   * read zones and points of collision
   */
  /* number of points of collision */
  img->numof_collisions_points = little_endian_to_short (ptr16++);
  dest = &img->collisions_points[0][0];
  for (i = 0; i < MAX_OF_COLLISION_POINTS * 2; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }
  /* number of zones of collision */
  img->numof_collisions_zones = little_endian_to_short (ptr16++);
  dest = &img->collisions_coords[0][0];
  for (i = 0; i < MAX_OF_COLLISION_ZONES * 2; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }
  dest = &img->collisions_sizes[0][0];
  for (i = 0; i < MAX_OF_COLLISION_ZONES * 2; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }

  /* 
   * read origins of shots (location of the cannons) and angle shots 
   */
  /* number of cannons */
  img->numof_cannons = little_endian_to_short (ptr16++);
  dest = &img->cannons_coords[0][0];
  for (i = 0; i < MAX_OF_CANNONS * 2; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }
  for (i = 0; i < MAX_OF_CANNONS; i++)
    {
      img->cannons_angles[i] = little_endian_to_short (ptr16++);
    }

  /*
   * read the pixel data (8 bits per pixel)
   */
  /* 32-bit access */
  ptr32 = (Sint32 *) (ptr16);
  /* number of pixels */
  img->numof_pixels = little_endian_to_int (ptr32++);
  img->img = memory_allocation (img->numof_pixels * bytes_per_pixel);
  if (img->img == NULL)
    {
      return NULL;
    }
  /* 8-bit access */
  ptr8 = (char *) ptr32;
  ptr8 = read_pixels (img->numof_pixels, ptr8, img->img);

  /* 
   * read offsets and repeat values 
   */
  /* 32-bit access */
  ptr32 = (Sint32 *) (ptr8);
  /* size of the table in bytes */
  img->nbr_data_comp = little_endian_to_int (ptr32++);
  img->compress = memory_allocation (img->nbr_data_comp * 2);
  if (img->compress == NULL)
    {
      return NULL;
    }
  /* 8-bit access */
  ptr8 = (char *) ptr32;
  ptr8 = read_compress (img->nbr_data_comp, ptr8, img->compress);
  return ptr8;
}

/** 
 * Extract a "*.spr" file data into a 'bitmap' structure 
 * @param bmp Pointer to a bitmap structure
 * @param filedata Pointer to the file data
 * @return The source pointer incremented
 */
static char *
bitmap_extract (bitmap * bmp, char *filedata)
{
  char *ptr8;
  Sint32 *ptr32;
  ptr8 = filedata;

  /*
   * read the pixel data (8 bits per pixel)
   */
  /* 32-bit access */
  ptr32 = (Sint32 *) (ptr8);
  /* number of pixels */
  bmp->numof_pixels = little_endian_to_int (ptr32++);
  bmp->img = memory_allocation (bmp->numof_pixels * bytes_per_pixel);
  if (bmp->img == NULL)
    {
      return NULL;
    }
  /* 8-bit access */
  ptr8 = (char *) ptr32;
  ptr8 = read_pixels (bmp->numof_pixels, ptr8, bmp->img);

  /* 
   * read offsets and repeat values 
   */
  /* 32-bit access */
  ptr32 = (Sint32 *) (ptr8);
  /* size of the table in bytes */
  bmp->nbr_data_comp = little_endian_to_int (ptr32++);
  bmp->compress = memory_allocation (bmp->nbr_data_comp * 2);
  if (bmp->compress == NULL)
    {
      return NULL;
    }
  /* 8-bit access */
  ptr8 = (char *) ptr32;
  ptr8 = read_compress (bmp->nbr_data_comp, ptr8, bmp->compress);
  return ptr8;
}

/**
 * Read 8-bit pixels from the data file,
 *   and copy or convert to 16/24/32-bit
 * @input numofpixels Number of 8-bit pixels
 * @input source 8-bit Data from the file
 * @input destination Pixel data from image structure 
 * @return The source pointer incremented
 * */
static char *
read_pixels (Uint32 numofpixels, char *source, char *destination)
{
  Uint32 i;
  Uint16 *dest16;
  unsigned char pixbyte;
  unsigned char *dest;
  unsigned char *pal;
  switch (bytes_per_pixel)
    {
      /* screen depth of 8-bit */
    case 1:
      {
        for (i = 0; i < numofpixels; i++)
          {
            destination[i] = *(source++);
          }
      }
      break;
      /* screen depth of 16-bit */
    case 2:
      {
        dest16 = (Uint16 *) destination;
        for (i = 0; i < numofpixels; i++)
          {
            pixbyte = (unsigned char) *(source++);
            *(dest16++) = pal16[pixbyte];
          }
      }
      break;
      /*  screen depth of 24-bit */
    case 3:
      dest = (unsigned char *) destination;
      for (i = 0; i < numofpixels; i++)
        {
          pixbyte = (unsigned char) *(source++);
          pal = (unsigned char *) &(pal32[pixbyte]);
          /* blue */
          *(dest++) = pal[0];
          /* green */
          *(dest++) = pal[1];
          /* red */
          *(dest++) = pal[2];
        }
      break;
      /*  screen depth of 32-bit */
    case 4:
      dest = (unsigned char *) destination;
      for (i = 0; i < numofpixels; i++)
        {
          pixbyte = (unsigned char) *(source++);
          pal = (unsigned char *) &(pal32[pixbyte]);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
          *(dest++) = pal[0];
          *(dest++) = pal[1];
          *(dest++) = pal[2];
          *(dest++) = pal[3];
#else
          *(dest++) = pal[0];
          *(dest++) = pal[1];
          *(dest++) = pal[2];
          *(dest++) = pal[3];
#endif
        }
      break;
    }
  return source;
}

/**
 * Read 'compress' table: offsets and repeats values (rep;stos)
 * of the sprite's pixels 
 * The table original is intended for a displaying in a 512 pixels wide screen
 * of 256 colors
 * Structure of the source filedata:
 * - filedata + 0: (short) destination offset
 * - filedata + 2: (unsigned char) number of consecutives 32-bit words
 *                 rep;stosl
 * - filedata + 3: (unsigned char) number of consecutives bytes (rep;stosb)
 * - ...   
 * @param filesize Size of the source filedata in bytes
 * @param filedata Source filedata of the sprite
 * @param destination Pointer to 'compress' structure of sprite or bitmap 
 * @return The destination pointer incremented
 */
static char *
read_compress (Uint32 filesize, char *filedata, char *destination)
{
  Uint32 i = 0;
  Uint32 offset;
  /* filedata 8-bit access */
  char *src8;
  /* filedata 16-bit access */
  Sint16 *src16;
  Uint32 *dest32;
  Uint16 *dest16;
  Uint16 repeat, repeat1, repeat2;
  dest32 = (Uint32 *) destination;
  src16 = (Sint16 *) filedata;
  src8 = (char *) src16;

  /*
   * simple copy for a 8-bit display: 256 colors
   */
  switch (bytes_per_pixel)
    {
    case 1:
      while (i < filesize)
        {
          /* read offset for a offscreen of 512 pixels wide */
          offset = (Uint16) little_endian_to_short (src16++);
          i += 2;
          *(dest32++) = offset;
          src8 = (char *) src16;
          dest16 = (Uint16 *) dest32;
          /* number of consecutives 32-bit words */
          repeat = (Uint16) * (src8++);
          i++;
          *(dest16++) = repeat;
          /* number of consecutives bytes */
          repeat = (Uint16) * (src8++);
          i++;
          *(dest16++) = repeat;
          src16 = (Sint16 *) src8;
          dest32 = (Uint32 *) dest16;
        }
      filedata = (char *) src8;
      break;

      /*
       * transform the table for a 16-bit display: 65,356 colors
       */
    case 2:
      while (i < filesize)
        {
          /* read offset for a offscreen of 512 pixels wide */
          offset = (Uint16) little_endian_to_short (src16++);
          /* double the offset, width of the line is two times as big */
          offset = offset * 2;
          *(dest32++) = offset;
          src8 = (char *) src16;
          dest16 = (Uint16 *) dest32;
          /* number of consecutives 32-bit words */
          repeat1 = (Uint16) * (src8++);
          /* 32-bit words => bytes in 16-bit depth */
          repeat1 = repeat1 * 2 * 4;
          /* number of consecutives bytes */
          repeat2 = (Uint16) * (src8++);
          /* number of consecutives bytes in 16-bit depth */
          repeat2 = repeat2 * 2;
          /* number total of bytes */
          repeat1 = repeat1 + repeat2;
          repeat2 = repeat1;
          /* bytes => 32-bit words */
          repeat1 = repeat1 / 4;
          /* remaining number of bytes */
          repeat2 = repeat2 % 4;
          /* save number of consecutives 32-bit words */
          *(dest16++) = repeat1;
          /* save number of consecutives 16-bit words */
          *(dest16++) = repeat2 / 2;
          src16 = (Sint16 *) src8;
          dest32 = (Uint32 *) dest16;
          i += 4;
        }
      filedata = (char *) src8;
      break;

      /*
       * transform the table for a 24-bit display: 16,777,216 colors
       */
    case 3:
      while (i < filesize)
        {
          /* read offset for a offscreen of 512 pixels wide */
          offset = (Uint16) little_endian_to_short (src16++);
          /* triple the offset, width of the line is three times as big */
          offset = offset * 3;
          *(dest32++) = offset;
          src8 = (char *) src16;
          dest16 = (Uint16 *) dest32;
          /* number of consecutives 32-bit words */
          repeat1 = (Uint16) * (src8++);
          /* 32-bit words => bytes in 24-bit depth */
          repeat1 = repeat1 * 3 * 4;
          /* number of consecutives bytes */
          repeat2 = (Uint16) * (src8++);
          /* number of consecutives bytes in 24-bit depth */
          repeat2 = repeat2 * 3;
          /* number total of bytes */
          repeat1 = repeat1 + repeat2;
          repeat2 = repeat1;
          /* bytes => 32-bit words */
          repeat1 = repeat1 / 4;
          /* remaining number of bytes */
          repeat2 = repeat2 % 4;
          /* save number of consecutives 32-bit words */
          *(dest16++) = repeat1;
          /* save number of consecutives bytes */
          *(dest16++) = repeat2;
          src16 = (Sint16 *) src8;
          dest32 = (Uint32 *) dest16;
          i += 4;
        }
      filedata = (char *) src8;
      break;
      /*
       * transform the table for a 32-bit display: 16,777,216 colors 
       */
    case 4:
      {
        while (i < filesize)
          {
            /* read offset for a offscreen of 512 pixels wide */
            offset = (Uint16) little_endian_to_short (src16++);
            i += 2;
            /* quadruple the offset, width of the line is four times as big */
            offset = offset * 4;
            *(dest32++) = offset;
            src8 = (char *) src16;
            dest16 = (Uint16 *) dest32;
            /* number of consecutives 32-bit words */
            repeat = (Uint16) * (src8++);
            repeat = repeat * 4;
            i++;
            *(dest16++) = repeat;
            repeat = (Uint16) * (src8++);
            i++;
            *(dest16++) = repeat;
            src16 = (Sint16 *) src8;
            dest32 = (Uint32 *) dest16;
          }
        filedata = (char *) src8;
      }
      break;
    }
  return filedata;
}

#ifdef PNG_EXPORT_ENABLE
/**
 * Copy a image structure into a buffer
 * @param width Height of the sprite in pixels 
 * @param height Width of the sprite in pixels
 * @param source Buffer containing 256 color pixels of the sprite
 * @param repeats Buffer containing values of offset and the number of
 *                contiguous pixels to be copied.
 * @param size_counter Size of 'repeats' buffer
 * @param size_of_line The width of the offscreen of the game.
 *                     Always 512 pixels.
 * @return A pointer to the buffer
 */
static char *
image_to_buffer_32_bit (Uint32 width, Uint32 height, unsigned char *source,
                        char *repeats, Uint32 size_counter,
                        Uint32 size_of_line)
{
  _compress *t;
  register Uint32 offset;
#if __WORDSIZE == 64
  Uint64 real_size;
#else
  Uint32 real_size;
#endif
  unsigned char *buffer, *p, *pal, pixel;
  Uint32 buffer_size, modulo_line;
  Uint32 line_jumps;
  Uint16 count, i;
  if (width < 1 || height < 1 || width > size_of_line
      || height > size_of_line)
    {
      LOG_ERR ("(!) size of image: %ix%i", width, height);
      return NULL;
    }

  /* allocate the destination buffer that will contain the 32-bit bitmap */
  buffer_size = width * height * 4;
  buffer = (unsigned char *) memory_allocation (buffer_size);
  if (buffer == NULL)
    {
      return NULL;
    }
  size_counter = size_counter >> 2;
  p = (unsigned char *) buffer;
  count = 0;
  t = (_compress *) repeats;
  do
    {
      offset = t->offset;
      if (offset >= size_of_line)
        {
          line_jumps = offset / size_of_line;
          modulo_line = offset % size_of_line;
          offset =
            (offset - line_jumps * size_of_line) + (line_jumps * width);
          if (modulo_line > width)
            {
              offset = offset - size_of_line + width;
            }
        }
      else
        {
          if (offset >= width)
            {
              offset = offset - size_of_line + width;
            }
        }
      /* offset destination */
      p = p + offset * 4;
      /* num of 32 words to copy */
      count = t->r1;
      for (i = 0; i < count * 4; i++)
        {
          pixel = *(source++);
          pal = &palette_24[pixel * 3];
#if __WORDSIZE == 64
          real_size = ((Uint64) (p + 3) - (Uint64) buffer);
#else
          real_size = ((Uint32) (p + 3) - (Uint32) buffer);
#endif
          if (real_size > buffer_size)
            {
              LOG_ERR ("1) count = %i", count);
              p += 3;
              continue;
            }
          *(p++) = pal[0];
          *(p++) = pal[1];
          *(p++) = pal[2];
          if (pal[0] < 64 && pal[1] < 64 && pal[2] < 64)
            {
              *(p++) = 255;
            }
          else
            {
              *(p++) = 255;
            }
        }
      /* 8 bits data */
      count = t->r2;
      for (i = 0; i < count; i++)
        {
          pixel = *(source++);
          pal = &palette_24[pixel * 3];
#if __WORDSIZE == 64
          real_size = ((Uint64) (p + 3) - (Uint64) buffer);
#else
          real_size = ((Uint32) (p + 3) - (Uint32) buffer);
#endif
          if (real_size > buffer_size)
            {
              LOG_ERR ("2) count = %i", count);
              p += 3;
              continue;
            }
          *(p++) = pal[0];
          *(p++) = pal[1];
          *(p++) = pal[2];
          if (pal[0] < 64 && pal[1] < 64 && pal[2] < 64)
            {
              *(p++) = 255;
            }
          else
            {
              *(p++) = 255;
            }
        }
      t++;
      size_counter--;
    }
  while (size_counter > 0);
#if __WORDSIZE == 64
  real_size = ((Uint64) p - (Uint64) buffer);
#else
  real_size = ((Uint32) p - (Uint32) buffer);
#endif
  if (real_size > buffer_size)
    {
      LOG_ERR ("%i is greater than %i (%ix%i)",
               (Sint32) real_size, buffer_size, width, height);
    }
  LOG_DBG ("width: %i, height: %i, real_size: %i, buffer_size: %i", width,
           height, real_size, buffer_size);
  return (char *) buffer;
}

static png_text text[5] = {
#ifdef PNG_iTXt_SUPPORTED
  {
   PNG_TEXT_COMPRESSION_NONE, "Title", "Powermanga Graphics", 19, 0, NULL,
   NULL},
  {

   PNG_TEXT_COMPRESSION_NONE, "Author",
   "Jean-Michel Martin de Santero, David Igreja", 43, 0, NULL, NULL},
  {
   PNG_TEXT_COMPRESSION_NONE, "Description", "Image of Powermanga Game", 24,
   0, NULL, NULL},
  {
   PNG_TEXT_COMPRESSION_NONE, "Copyright", "TLK Games", 9, 0, NULL, NULL},
  {
   PNG_TEXT_COMPRESSION_NONE, "Software", "GNU Linux", 9, 0, NULL, NULL},
#else
  {
   PNG_TEXT_COMPRESSION_NONE, "Title", "Powermanga Graphics", 19},
  {

   PNG_TEXT_COMPRESSION_NONE, "Author",
   "Jean-Michel Martin de Santero, David Igreja", 43},
  {
   PNG_TEXT_COMPRESSION_NONE, "Description", "Image of Powermanga Game", 24},
  {
   PNG_TEXT_COMPRESSION_NONE, "Copyright", "TLK Games", 9},
  {
   PNG_TEXT_COMPRESSION_NONE, "Software", "GNU Linux", 9},
#endif
};



/**
 * This function tries to determine the width and height of a sprite.
 * Because a sprite structure "bitmap" have no size. The result is not reliable.
 * @param bmp Pointer to an 'bitmap' structure
 * @param size_of_line The width of the offscreen of the game.
 *                     Always 512 pixels.
 */
#ifdef __BITMAP_CHECK_SIZE__
static bool
bitmap_check_size (bitmap * bmp, Uint32 size_of_line)
{
  _compress *compress;
  register Uint32 offset;
  Uint16 pixel_counter;
  Uint32 modulo_line, line_jumps, estimated_width, estimated_height,
    min_xoffset, yoffset, i, xpos, total_size;
  char *repeats = bmp->compress;
  Uint32 size_counter = bmp->nbr_data_comp;
  size_counter = size_counter >> 2;
  compress = (_compress *) repeats;
  estimated_width = estimated_height = i = xpos = total_size = yoffset = 0;
  min_xoffset = size_of_line;
  do
    {
      offset = compress->offset;
      if (offset >= size_of_line)
        {
          line_jumps = offset / size_of_line;
          modulo_line = offset % size_of_line;
          estimated_height += line_jumps;
          xpos = modulo_line;
          total_size += modulo_line;
          if (xpos < min_xoffset)
            {
              min_xoffset = xpos;
            }
          if (i == 0)
            {
              yoffset = line_jumps;
            }
        }
      else
        {
          total_size += offset;
          if ((xpos + offset) > size_of_line)
            {
              xpos = offset - (size_of_line - xpos);
              estimated_height++;
              if (xpos < min_xoffset)
                {
                  min_xoffset = xpos;
                }
            }
          else
            {
              xpos += offset;
            }
        }
      /* number of pixels to be copied by 4 bytes */
      pixel_counter = compress->r1 * 4;
      /* number of pixels to copy a single byte at a time */
      pixel_counter += compress->r2;
      total_size += pixel_counter;
      xpos += pixel_counter;
      if (xpos > estimated_width)
        {
          estimated_width = xpos;
        }
      size_counter--;
      compress++;
      i++;
    }
  while (size_counter > 0);
  estimated_height++;
  estimated_height += yoffset;
  estimated_width += min_xoffset;
  LOG_INF
    ("estimated width: %i, estimated height: %i, total_size: %i, min_xoffset: %i, yoffset = %i \n",
     estimated_width, estimated_height, total_size, min_xoffset, yoffset);
  return TRUE;
}
#endif

/**
 * Write PNG image data
 * @param buffer Image data
 * @param filename A filename of the PNG image to save
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param depth The bit depth of the image.
 *              Valid values shall include 1, 2, 4, 8, 16. 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
png_create (char *buffer, const char *filename, Uint32 width, Uint32 height,
            Sint32 depth)
{
  Uint32 i;
  FILE *out;
  LOG_DBG ("Write %s", filename);
  if (buffer == NULL)
    {
      LOG_ERR ("image_to_buffer() failed! (filename = %s)", filename);
      return FALSE;
    }
  out = fopen_data (filename, "w");
  if (out == NULL)
    {
      LOG_ERR ("can't open \"%s\"", filename);
      return FALSE;
    }

  /* allocate and initialize png_ptr struct for writing */
  png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
                                                 /* error_ptr */
                                                 (png_voidp) NULL,
                                                 /* error_fn */
                                                 NULL,
                                                 /* warn_fn */
                                                 NULL);
  if (png_ptr == NULL)
    {
      LOG_ERR ("png_create_write_struct() failed");
      return FALSE;
    }

  /* allocate and initialize the info structure */
  png_infop info_ptr = png_create_info_struct (png_ptr);
  if (png_ptr == NULL)
    {
      LOG_ERR ("png_create_info_struct() failed");
      return FALSE;
    }

  /* initialize input/output for PNG file to the default functions */
  png_init_io (png_ptr, out);

  /* set library compression level */
  png_set_IHDR (png_ptr, info_ptr,
                /* width */
                width,
                /* height */
                height,
                /* bit depth */
                depth,
                PNG_COLOR_TYPE_RGB_ALPHA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_compression_level (png_ptr, Z_BEST_COMPRESSION);

  png_set_text (png_ptr, info_ptr, text, sizeof text / sizeof text[0]);
  png_color_16 background;
  background.red = 0;
  background.green = 0;
  background.blue = 0;
  png_set_bKGD (png_ptr, info_ptr, &background);
  png_write_info (png_ptr, info_ptr);
  for (i = 0; i < height; i++)
    {
      {
        png_write_row (png_ptr, (png_byte *) & buffer[i * width * 4]);
      }
    }
  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);
  fclose (out);
  return TRUE;
}

/**
 * Converts a 'image' structure in a PNG file.
 * @param img Pointer to an 'image' structure
 * @param filename A filename of the PNG image to save
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
image_to_png (image * img, const char *filename)
{
  bool res;
  char *buffer =
    image_to_buffer_32_bit (img->w, img->h, (unsigned char *) img->img,
                            img->compress, img->nbr_data_comp,
                            offscreen_pitch);
  res = png_create (buffer, filename, img->w, img->h, 8);
  free_memory (buffer);
  return res;
}

/**
 * Converts a 'bitmap' structure in a PNG file.
 * @param bmp Pointer to an 'bitmap' structure
 * @param filename A filename of the PNG image to save
 * @param width Height of the sprite in pixels 
 * @param height Width of the sprite in pixels
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
bitmap_to_png (bitmap * bmp, const char *filename, Uint32 width,
               Uint32 height, Uint32 size_of_line)
{
  bool res;
#ifdef __BITMAP_CHECK_SIZE__
  bitmap_check_size (bmp, size_of_line);
#endif
  char *buffer =
    image_to_buffer_32_bit (width, height, (unsigned char *) bmp->img,
                            bmp->compress, bmp->nbr_data_comp, size_of_line);
  res = png_create (buffer, filename, width, height, 8);
  free_memory (buffer);
  return res;
}
#endif
