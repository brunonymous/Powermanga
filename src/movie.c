/**
 * @file movie.c
 * @brief play start and congratulations animations files
 *        ("movie_congratulation.gca" and "movie_introduction.gca")
 * @date 2012-08-26 
 * @author Etienne Sobole
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: movie.c,v 1.17 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "tools.h"
#include "config_file.h"
#include "display.h"
#include "log_recorder.h"
#include "movie.h"

/** Wich movie si played: 1=intro or 2=congratulation */
Uint32 movie_playing_switch = MOVIE_INTRODUCTION;
/** Pointer to the buffer for the current animation movie */
unsigned char *movie_buffer = NULL;
static unsigned char *image2 = NULL;
static unsigned char *icmpr = NULL;
static unsigned char *smage1 = NULL;
static unsigned char *smage2 = NULL;
static unsigned char *scmpr = NULL;
/** Pointer to the filedata of the movie in memory */
static unsigned char *movie_filedata = NULL;
/* pointer to the colormap in the file */
static Sint32 movie_counter = 0;
static Sint32 images = 0;

static bool movie_initialize (const char *filename);
static bool movie_load (const char *filename);
static bool movie_play (void);
static unsigned char *decompress (unsigned char *, unsigned char *,
                                  unsigned char *);

/**
 * Play movie animation compressed
 * @return TRUE if it completed successfully or 0 otherwise
 */
bool
movie_player (void)
{
  const char *filename;
  switch (movie_playing_switch)
    {
      /* introduction movie */
    case MOVIE_INTRODUCTION:
      filename = "graphics/movie_introduction.gca";
      if (movie_initialize (filename))
        {
          movie_playing_switch = MOVIE_PLAYED_CURRENTLY;
        }
      else
        {
          movie_playing_switch = MOVIE_NOT_PLAYED;
          LOG_ERR ("movie_initialize(\"%s\") failed", filename);
          return FALSE;
        }
      break;
      /* congratulation movie */
    case MOVIE_CONGRATULATIONS:
      filename = "graphics/movie_congratulation.gca";
      if (movie_initialize (filename))
        {
          movie_playing_switch = MOVIE_PLAYED_CURRENTLY;
        }
      else
        {
          movie_playing_switch = MOVIE_NOT_PLAYED;
          LOG_ERR ("movie_initialize(\"%s\") failed", filename);
          return FALSE;
        }
      break;
      /* play movie animation compressed */
    case MOVIE_PLAYED_CURRENTLY:
      {
        if (!movie_play () || key_code_down > 0 || fire_button_down
            || mouse_b > 1)
          {
            movie_free ();
            movie_playing_switch = MOVIE_NOT_PLAYED;
          }
      }
      break;
    }
  return TRUE;
}

/**
 * Initialize an animation file, allocate buffers and create offscreen
 * @param filename: the file which should be loaded.
 * @return boolean value on success or failure
 */
static bool
movie_initialize (const char *filename)
{
  Sint32 i = 0;
  smage1 = smage2 = scmpr = NULL;
  smage1 = ((unsigned char *) memory_allocation (64000));
  if (smage1 == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'smage1'");
      return FALSE;
    }
  smage2 = ((unsigned char *) memory_allocation (64000));
  if (smage1 == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'smage2'");
      return FALSE;
    }
  movie_filedata = ((unsigned char *) memory_allocation ((1024 * 1024 * 4)));
  if (movie_filedata == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'movie_filedata'");
      return FALSE;
    }
  for (i = 0; i < 64000; i++)
    {
      smage1[i] = 0;
      smage2[i] = 0;
    }
  if (!movie_load (filename))
    {
      LOG_ERR ("movie_load(%s) failed!", filename);
      return FALSE;
    }
  movie_buffer = smage1;
  image2 = smage2;
  icmpr = scmpr;
  movie_counter = 0;
  if (!create_movie_offscreen ())
    {
      LOG_ERR ("create_movie_buffer() failed!");
      return FALSE;
    }
  return TRUE;
}

/**
 * Deallocates the memory used by the movie player
 */
void
movie_free (void)
{
  if (smage1 != NULL)
    {
      free_memory ((char *) smage1);
      smage1 = NULL;
    }
  if (smage2 != NULL)
    {
      free_memory ((char *) smage2);
      smage2 = NULL;
    }
  if (movie_filedata != NULL)
    {
      free_memory ((char *) movie_filedata);
      movie_filedata = NULL;
    }
  if (pal16PlayAnim != NULL)
    {
      free_memory ((char *) pal16PlayAnim);
      pal16PlayAnim = NULL;
    }
  if (pal32PlayAnim != NULL)
    {
      free_memory ((char *) pal32PlayAnim);
      pal32PlayAnim = NULL;
    }
  destroy_movie_offscreen ();
}

/**
 * Load an animation file and convert colors palette
 * @param filename: the file which should be loaded.
 * @return boolean value on success or failure
 */
static bool
movie_load (const char *filename)
{
  Sint32 i, *ptr32;
  unsigned char *_p8, *_p, *_pPal, *pcxpal;
  if (!loadfile_into_buffer (filename, (char *) movie_filedata))
    {
      return FALSE;
    }
  ptr32 = (Sint32 *) movie_filedata;
  images = little_endian_to_int (ptr32++);
  _p8 = (unsigned char *) ptr32;
  pcxpal = _p8;
  scmpr = _p8 + 768;
  if (bytes_per_pixel == 2)
    {
      if (pal16PlayAnim == NULL)
        {
          pal16PlayAnim = (Uint16 *) memory_allocation (256 * 2);
          if (pal16PlayAnim == NULL)
            {
              LOG_ERR ("not enough memory to allocate 'pal16PlayAnim'");
              return FALSE;
            }
        }
      convert_palette_24_to_16 (pcxpal, pal16PlayAnim);
    }
  if (bytes_per_pixel > 2)
    {
      if (pal32PlayAnim == NULL)
        {
          pal32PlayAnim = (Uint32 *) memory_allocation (256 * 4);
          if (pal32PlayAnim == NULL)
            {
              LOG_ERR ("not enough memory to allocate 'pal32PlayAnim'");
              return FALSE;
            }
        }
      _p = (unsigned char *) pal32PlayAnim;
      _pPal = pcxpal;
      for (i = 0; i < 256; i++)
        {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
          _p[3] = _pPal[2];
          _p[2] = _pPal[1];
          _p[1] = _pPal[0];
          _p[0] = 0;
#else
          if (power_conf->scale_x >= 2)
            {
              _p[0] = _pPal[2];
              _p[1] = _pPal[1];
              _p[2] = _pPal[0];
              _p[3] = 0;
            }
          else
            {
              _p[2] = _pPal[2];
              _p[1] = _pPal[1];
              _p[0] = _pPal[0];
              _p[3] = 0;
            }
#endif
          _p += 4;
          _pPal += 3;
        }
    }
  return TRUE;
}

/*
 * Play a movie
 * @return TRUE if the movie is finished
 */
static bool
movie_play (void)
{
  /* images = 64 + 16 + 2 + 1 + 1 */
  unsigned char *tmp;
  movie_counter++;
  if (movie_counter == images - 1)
    {
      return FALSE;
    }
  icmpr = decompress (icmpr, movie_buffer, image2);
  tmp = movie_buffer;
  movie_buffer = image2;
  image2 = tmp;
  return TRUE;
}

/*
 *
 */
static unsigned char *
decompress (unsigned char *wsc, unsigned char *im1, unsigned char *im2)
{
  Sint32 i, j, mode;
  Uint32 wr;
  unsigned char *idec;
  unsigned char *wdec;
  unsigned char c;
  unsigned char color;
  Sint32 length = 0;
  Sint32 retour = 0;
  Sint32 position = 0;
  i = 0;
  idec = im2;
  while (i < 64000)
    {
      mode = 0;
      /* read a byte */
      c = *wsc++;
      if (c == 255)
        {
          color = *wsc++;
          *idec++ = color;
          i++;
          mode = 0;
        }
      else if ((c & 0xc0) == 0)
        {
          wr = 0;
          wr = c << 16;
          wr += (*wsc++) << 8;
          wr += *wsc++;
          length = wr & 63;
          position = (wr >> 6) & 65535;
          mode = 1;
        }
      else if ((c & 0xc0) == 0x40)
        {
          wr = 0;
          wr = c << 24;
          wr += (*wsc++) << 16;
          wr += (*wsc++) << 8;
          wr += *wsc++;
          length = wr & 16383;
          position = (wr >> 14) & 65535;
          mode = 1;
        }
      else if ((c & 0xe0) == 0x80)
        {
          wr = 0;
          wr = c << 8;
          wr += *wsc++;
          length = wr & 63;
          retour = (wr >> 6) & 255;
          mode = 2;
        }
      else if ((c & 0xe0) == 0xa0)
        {
          wr = 0;
          wr = c << 16;
          wr += (*wsc++) << 8;
          wr += *wsc++;
          length = wr & 255;
          retour = (wr >> 8) & 8191;
          mode = 2;
        }
      else if ((c & 0xe0) == 0xc0)
        {
          wr = 0;
          wr = c << 24;
          wr += (*wsc++) << 16;
          wr += (*wsc++) << 8;
          wr += *wsc++;
          length = wr & 8191;
          position = (wr >> 13) & 65535;
          mode = 3;
        }
      if (i + length > 64000)
        {
          length = 64000 - i;
        }
      if (mode == 1)
        {
          for (j = 0; j < length; j++)
            {
              *idec++ = im1[position + j];
            }
          i += length;
        }
      else if (mode == 2)
        {
          wdec = idec - retour;
          for (j = 0; j < length; j++)
            {
              *idec++ = *wdec++;
            }
          i += length;
        }
      else if (mode == 3)
        {
          for (j = 0; j < length; j++)
            {
              *idec++ = im2[position + j];
            }
          i += length;
        }
    }
  return wsc;
}
