/**
 * @file starfield.c
 * @brief Handle the starfields 
 * @created 1999-08-17
 * @date 2012-08-25
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: starfield.c,v 1.17 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "images.h"
#include "display.h"
#include "electrical_shock.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "starfield.h"

image star_field[TYPE_OF_STARS][STAR_NUMOF_IMAGES];
float starfield_speed = 2.0;

/** 
 * Stars data structure
 */
typedef struct star_structure
{
  /** Structures of the star images */
  image *img;
  /** X coordinate */
  float coor_x;
  /** Y coordinate */
  float coor_y;
  /** Speed */
  float speed;
  Sint32 type;
  /** Delay before next image */
  Sint32 next_image_pause;
  /** Delay's counter */
  Uint32 next_image_pause_cnt;
} star_structure;

#define NUMOF_STARS_BY_TYPE 24
/** Maximum number of stars in the starfield */
const Uint32 NUMOF_STARS = NUMOF_STARS_BY_TYPE * 3;
static star_structure *stars = NULL;
static bool starfield_enable = TRUE;

static void starfield_init (void);
static bool starfield_load (void);

/**
 * Load data allocate buffers, initialize structure of the starfield 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
starfield_once_init (void)
{
  if (!starfield_load ())
    {
      return FALSE;
    }

  /* allocate starfield data structure */
  if (stars == NULL)
    {
      stars =
        (star_structure *) memory_allocation (NUMOF_STARS *
                                              sizeof (star_structure));
      if (stars == NULL)
        {
          LOG_ERR ("'stars' out of memory");
          return FALSE;
        }
    }
  starfield_init ();
  return TRUE;
}

/**
 * Load all stars (4 types of stars of 8 images each)
 * STAR_BIG, STAR_MIDDLE and STAR_LITTLE: starfield
 * ETOILE: stars on the player's spaceship
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
starfield_load (void)
{
  Uint32 i;

  /* big stars */
  for (i = 0; i < STAR_NUMOF_IMAGES; i++)
    {
      if (!image_load_num
          ("graphics/sprites/stars/star_big_%1d.spr", i,
           &star_field[STAR_BIG][i], 1, 1))
        {
          return FALSE;
        }
    }
  /* middle stars */
  for (i = 0; i < STAR_NUMOF_IMAGES; i++)
    {
      if (!image_load_num
          ("graphics/sprites/stars/star_middle_%1d.spr", i,
           &star_field[STAR_MIDDLE][i], 1, 1))
        {
          return FALSE;
        }
    }
  /* little stars */
  for (i = 0; i < STAR_NUMOF_IMAGES; i++)
    {
      if (!image_load_num
          ("graphics/sprites/stars/star_little_%1d.spr", i,
           &star_field[STAR_LITTLE][i], 1, 1))
        {
          return FALSE;
        }
    }
  /* stars on the players spaceship */
  for (i = 0; i < STAR_NUMOF_IMAGES; i++)
    {
      if (!image_load_num
          ("graphics/sprites/stars/star_spaceship_%1d.spr", i,
           &star_field[STAR_SPACESHIP][i], 1, 1))
        {
          return FALSE;
        }
    }
  return TRUE;
}

/**
 * Convert stars from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
starfield_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/stars/stars-%01d/star-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/stars"))
    {
      return FALSE;
    }
  for (type = 0; type < TYPE_OF_STARS; type++)
    {
      sprintf (filename, EXPORT_DIR "/stars/stars-%01d", type + 1);
      if (!create_dir (filename))
        {
          return FALSE;
        }
      for (frame = 0; frame < STAR_NUMOF_IMAGES; frame++)
        {
          sprintf (filename, EXPORT_DIR "/stars/stars-%01d/star-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&star_field[type][frame], filename))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif

/**
 * Filling starfield data structure
 */
static void
starfield_init (void)
{
  Uint32 j, i;

  /* big stars */
  j = 0;
  for (i = 0; i < NUMOF_STARS_BY_TYPE; i++)
    {
      stars[i + 48].coor_x =
        (float) (rand () % offscreen_width_visible + offscreen_clipsize);
      stars[i + 48].coor_y = (float) (rand () % 10 + i * 15);
      stars[i + 48].speed =
        (float) ((float) (rand () % 8 / (float) 100.0 + 0.8));
      stars[i + 48].img = (image *) & star_field[STAR_BIG][j];
      stars[i + 48].type = STAR_BIG;
      stars[i + 48].next_image_pause = 16;
      stars[i + 48].next_image_pause_cnt = (rand () * 8) / RAND_MAX;
      j++;
      if (j == STAR_NUMOF_IMAGES)
        {
          j = 0;
        }
    }

  /* middle stars */
  j = 0;
  for (i = 0; i < NUMOF_STARS_BY_TYPE; i++)
    {
      stars[i + 24].coor_x =
        (float) (rand () % offscreen_width_visible + offscreen_clipsize);
      stars[i + 24].coor_y = (float) (rand () % 10 + i * 15);
      stars[i + 24].speed =
        (float) ((float) (rand () % 4 / (float) 100.0 + 0.4));
      stars[i + 24].img = (image *) & star_field[STAR_MIDDLE][j];
      stars[i + 24].type = STAR_MIDDLE;
      stars[i + 24].next_image_pause = 16;
      stars[i + 24].next_image_pause_cnt = (rand () * 8) / RAND_MAX;
      j++;
      if (j == STAR_NUMOF_IMAGES)
        {
          j = 0;
        }
    }
  /* little stars */
  j = 0;
  for (i = 0; i < NUMOF_STARS_BY_TYPE; i++)
    {
      stars[i].coor_x =
        (float) (rand () % offscreen_width_visible + offscreen_clipsize);
      stars[i].coor_y = (float) (rand () % 10 + i * 15);
      stars[i].speed = (float) ((float) (rand () % 2 / (float) 100.0 + 0.2));
      stars[i].img = (image *) & star_field[STAR_LITTLE][j];
      stars[i].type = STAR_LITTLE;
      stars[i].next_image_pause = 8;
      stars[i].next_image_pause_cnt = (rand () * 8) / RAND_MAX;
      j++;
      if (j == STAR_NUMOF_IMAGES)
        {
          j = 0;
        }
    }
}

/** 
 * Release memory used for the starfield 
 */
void
starfield_free (void)
{
  images_free (&star_field[0][0], TYPE_OF_STARS, STAR_NUMOF_IMAGES,
               STAR_NUMOF_IMAGES);
  if (stars != NULL)
    {
      free_memory ((char *) stars);
      stars = NULL;
    }
}

/** 
 * Draw the starfield background 
 */
void
starfield_handle (void)
{
  Uint32 i;
  star_structure *star;

  if (!starfield_enable)
    {
      return;
    }
  if (starfield_speed >= 0.0)
    {
      for (i = 0; i < NUMOF_STARS; i++)
        {
          star = &stars[i];
          star->coor_y += star->speed * (starfield_speed);
          if ((star->coor_y) >= offscreen_height - offscreen_clipsize)
            {
              star->coor_x =
                (float) (rand () % offscreen_width_visible +
                         offscreen_clipsize);
              star->coor_y = (float) (offscreen_clipsize - star->img->h);

            }
          star->next_image_pause_cnt++;
          if (!(star->next_image_pause_cnt &= (star->next_image_pause - 1)))
            {
              star->img =
                (image *) & star_field[star->type][rand () %
                                                   STAR_NUMOF_IMAGES];
            }
          draw_sprite (star->img, (Sint32) star->coor_x,
                       (Sint32) star->coor_y);
        }
    }
  else
    {
      for (i = 0; i < NUMOF_STARS; i++)
        {
          star = &stars[i];
          star->coor_y += star->speed * (starfield_speed);
          if ((star->coor_y + star->img->h) <= offscreen_clipsize)
            {
              star->coor_x =
                (float) (rand () % offscreen_width_visible +
                         offscreen_clipsize);
              star->coor_y = (float) (offscreen_height - offscreen_clipsize);
            }
          star->next_image_pause_cnt++;
          if (!(star->next_image_pause_cnt &= (star->next_image_pause - 1)))
            {
              star->img =
                (image *) & star_field[star->type][rand () %
                                                   STAR_NUMOF_IMAGES];
            }
          draw_sprite (star->img, (Sint32) star->coor_x,
                       (Sint32) star->coor_y);
        }
    }
}
