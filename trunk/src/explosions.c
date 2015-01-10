/** 
 * @file explosions.c
 * @brief Handle explosions and explosion fragments 
 * @date 2014-09-18
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: explosions.c,v 1.28 2012/06/03 17:06:15 gurumeditation Exp $
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
#include "log_recorder.h"
#include "enemies.h"
#include "explosions.h"
#include "shots.h"
#include "gfx_wrapper.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "starfield.h"

/** Maximum of enemies active at any time */
#define MAX_OF_EXPLOSIONS 200
#define EXPLOSIONS_NUMOF_TYPES 3
#define EXPLOSIONS_NUMOF_IMAGES 32
#define FRAGMENTS_NUMOF_TYPES 5
#define FRAGMENTS_NUMOF_IMAGES 32

typedef struct explosion_struct
{
  /** Current image index */
  Sint16 current_image;
  /** Value of delay between two images */
  Sint16 anim_speed;
  /** Counter of delay between two images */
  Sint16 anim_count;
  Sint16 img_angle;
  /** Structures of the explosions images */
  image *img[EXPLOSIONS_NUMOF_IMAGES];
  /**  X-coordinate */
  float xcoord;
  /** Y-coordinate */
  float ycoord;
  /** Speed of the displacement */
  float speed;
  struct explosion_struct *previous;
  struct explosion_struct *next;
  bool is_enabled;
  /** Delay before begin explosion animation */
  Sint32 countdown;
} explosion_struct;

static image eclat[FRAGMENTS_NUMOF_TYPES][FRAGMENTS_NUMOF_IMAGES];
static image explo[EXPLOSIONS_NUMOF_TYPES][EXPLOSIONS_NUMOF_IMAGES];
static explosion_struct *explosions = NULL;
static explosion_struct *explosion_first = NULL;
static explosion_struct *explosion_last = NULL;
static Sint32 num_of_explosions = 0;
static explosion_struct *explosion_get (void);
static void explosion_del (explosion_struct *);

/**
 * Allocate buffers and initialize structure of the explosions 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
explosions_once_init (void)
{
  Uint32 i;
  explosion_struct *blast;
  explosions_free ();

  /* extract explosions sprites images (61,750 bytes) */
  if (!image_load
      ("graphics/sprites/explosions.spr", &explo[0][0],
       EXPLOSIONS_NUMOF_TYPES, EXPLOSIONS_NUMOF_IMAGES))
    {
      return FALSE;
    }
  /* extract  explosion fragments sprites images (34,089 bytes) */
  if (!image_load
      ("graphics/sprites/explosion_fragments.spr", &eclat[0][0],
       FRAGMENTS_NUMOF_TYPES, FRAGMENTS_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* allocate explosions data structure */
  if (explosions == NULL)
    {
      explosions =
        (explosion_struct *) memory_allocation (MAX_OF_EXPLOSIONS *
                                                sizeof (explosion_struct));
      if (explosions == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'explosions'");
          return FALSE;
        }
    }
  for (i = 0; i < MAX_OF_EXPLOSIONS; i++)
    {
      blast = &explosions[i];
      blast->is_enabled = FALSE;
    }
  return TRUE;
}

/**
 * Convert explosions from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
explosions_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/explosions/explosion/explosion-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/explosions"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < EXPLOSIONS_NUMOF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/explosions/explosion-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < EXPLOSIONS_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/explosions/explosion-%01d/explosion-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&explo[type][frame], filename))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  for (type = 0; type < FRAGMENTS_NUMOF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/explosions/fragment-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < FRAGMENTS_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/explosions/fragment-%01d/fragment-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&eclat[type][frame], filename))
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
 * Release memory used for the explosions 
 */
void
explosions_free (void)
{
  images_free (&explo[0][0], EXPLOSIONS_NUMOF_TYPES, EXPLOSIONS_NUMOF_IMAGES,
               EXPLOSIONS_NUMOF_IMAGES);
  images_free (&eclat[0][0], FRAGMENTS_NUMOF_TYPES, FRAGMENTS_NUMOF_IMAGES,
               FRAGMENTS_NUMOF_IMAGES);
  if (explosions != NULL)
    {
      free_memory ((char *) explosions);
      explosions = NULL;
    }
}

/** 
 * Handle big, medium, little and special explosions
 */
void
explosions_handle ()
{
  Sint32 i;
  explosion_struct *blast;
  blast = explosion_first;
  if (blast == NULL)
    {
      return;
    }
  for (i = 0; i < num_of_explosions; i++, blast = blast->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (blast == NULL && i < (num_of_explosions - 1))
        {
          LOG_ERR ("blast->next is null %i/%i", i, num_of_explosions);
          break;
        }
#endif

      /* change x and y coordinates */
      blast->xcoord += depix[(Sint16) blast->speed][blast->img_angle];
      blast->ycoord += depiy[(Sint16) blast->speed][blast->img_angle];

      /* delay before begin animation */
      if (blast->countdown > 0)
        {
          blast->countdown--;
        }

      /* play sprite animation */
      else
        {
          /* inc. counter of delay between two images */
          blast->anim_count++;
          if (blast->anim_count >= blast->anim_speed)
            {
              blast->anim_count = 0;
              /* flip to the next image */
              blast->current_image++;
            }

          /* last image has been reached? */
          if (
               /* last image of explosion has been reached? */
               (blast->anim_speed < 5
                && blast->current_image >= EXPLOSIONS_NUMOF_IMAGES) ||
               /* last image of star has been reached? */
               (blast->anim_speed > 3
                && blast->current_image >= STAR_NUMOF_IMAGES) ||
               /* explosion move to the bottom out of the screen? */
               (((Sint16) blast->ycoord +
                 blast->img[blast->current_image]->h) >=
                (offscreen_height - 1)) ||
               /* explosion move to the right out of the screen? */
               (((Sint16) blast->xcoord +
                 blast->img[blast->current_image]->w) >=
                (offscreen_width - 1)) ||
               (((Sint16) blast->xcoord +
                 blast->img[blast->current_image]->w - 1) < offscreen_startx)
               ||
               (((Sint16) blast->ycoord +
                 blast->img[blast->current_image]->h - 1) < offscreen_starty))
            {
              /* remove a explosion or star element from list */
              explosion_del (blast);
            }
          else
            {
              /* draw explosion or star sprite */
              draw_sprite (blast->img[blast->current_image],
                           (Uint32) blast->xcoord, (Uint32) blast->ycoord);
            }
        }
    }
}

/** 
 * Add a new explosion 
 * @param coordx X-coordinate 
 * @param coordy Y-coordinate
 * @param speed Movement speed
 * @param type Type 0=32*32 / 1=64*14 / 2=16*16 / 3=player's spaceship star
 * @param delay Delay before begin explosion animation 
 */
void
explosion_add (float coordx, float coordy, float speed, Sint32 type,
               Sint32 delay)
{
  Sint32 k;
  explosion_struct *blast;
  if (type < 0 || type > 3)
    {
      return;
    }
  /* get a new explosion element */
  blast = explosion_get ();
  if (blast == NULL)
    {
      return;
    }
  /* set current image */
  blast->current_image = 0;
  /* clear counter of delay between two images */
  blast->anim_count = 0;
  /* set speed of the displacement */
  blast->speed = speed;
  blast->img_angle = 8;
  /* delay before begin explosion animation */
  blast->countdown = delay;
  switch (type)
    {
      /* 
       * medium explosion of 32*32 pixels size 
       */
    case EXPLOSION_MEDIUM:
      blast->anim_speed = 1;
      /* set addresses of the images buffer */
      for (k = 0; k < EXPLOSIONS_NUMOF_IMAGES; k++)
        {
          blast->img[k] = (image *) & explo[1][k];
        }
      /* set x and y coordinates */
      blast->xcoord = coordx - 8;
      blast->ycoord = coordy - 8;
#ifdef USE_SDLMIXER
      sound_play (SOUND_MEDIUM_EXPLOSION_1 + (global_counter & 3));
#endif
      break;

      /* 
       * big explosion of 64*64 pixels size 
       */
    case EXPLOSION_BIG:
      blast->anim_speed = 1;
      for (k = 0; k < EXPLOSIONS_NUMOF_IMAGES; k++)
        {
          blast->img[k] = (image *) & explo[2][k];
        }
      /* set x and y coordinates */
      blast->xcoord = coordx - 16;
      blast->ycoord = coordy - 16;
#ifdef USE_SDLMIXER
      sound_play (SOUND_BIG_EXPLOSION_1 + (global_counter & 3));
#endif
      break;

      /* 
       * small explosion of 16*16 pixels size 
       */
    case EXPLOSION_SMALL:
      blast->anim_speed = 1;
      for (k = 0; k < EXPLOSIONS_NUMOF_IMAGES; k++)
        {
          blast->img[k] = (image *) & explo[0][k];
        }
      /* set x and y coordinates */
      blast->xcoord = coordx - 4;
      blast->ycoord = coordy - 4;
#ifdef USE_SDLMIXER
      sound_play (SOUND_SMALL_EXPLOSION_1 + (global_counter & 3));
#endif
      break;

      /* 
       * star on player's spaceship 
       */
    case 3:
      /* set value of delay between two images */
      blast->anim_speed = 6;
      for (k = 0; k < STAR_NUMOF_IMAGES; k++)
        {
          blast->img[k] = (image *) & star_field[STAR_SPACESHIP][k];
        }
      /* set x and y coordinates */
      blast->xcoord = coordx;
      blast->ycoord = coordy;
      break;
    }
}

/** 
 * Add a new explosion for the guardian only 
 * @param coordx x coordinate 
 * @param coordy y coordinate
 */
void
explosion_guardian_add (float coordx, float coordy)
{
  Sint32 k;
  explosion_struct *blast;
  blast = explosion_get ();
  if (blast == NULL)
    {
      return;
    }
  /* set first image */
  blast->current_image = 0;
  /* value of delay between two images */
  blast->anim_speed = 1;
  /* counter of delay between two images */
  blast->anim_count = 0;
  for (k = 0; k < EXPLOSIONS_NUMOF_IMAGES; k++)
    {
      blast->img[k] = (image *) & explo[0][k];
    }
  /* set x and y coordinates */
  blast->xcoord = coordx - 4;
  blast->ycoord = coordy - 4;
  /* set speed of the displacement */
  blast->speed = -.5;
  blast->img_angle = 8;
  /* delay before begin explosion animation */
  blast->countdown = 0;
#ifdef USE_SDLMIXER
  if (!(rand () % 8))
    {
      sound_play (SOUND_SMALL_EXPLOSION_1 + (global_counter & 3));
    }
#endif
}

/**
 * Add one or more explosion fragments 
 * @param coordx X-coordinate 
 * @param coordy Y-coordinate
 * @param speed Movement speed
 * @param numof Number of the explosion fragments 
 * @param delay Delay before begin explosion animation 
 * @param anim_speed Time delay between two images
 */
void
explosions_fragments_add (float coordx, float coordy, float speed,
                          Sint32 numof, Sint32 delay, Sint16 anim_speed)
{
  Sint32 i, j, num_eclat;
  explosion_struct *blast;
  for (i = 0; i < numof; i++)
    {
      blast = explosion_get ();
      if (blast == NULL)
        {
          return;
        }
      /* set first image */
      blast->current_image = 0;
      /* value of delay between two images */
      blast->anim_speed = anim_speed;
      /* counter of delay between two images */
      blast->anim_count = 0;
      /* select a explosion fragment image at random */
      num_eclat = rand () % FRAGMENTS_NUMOF_TYPES;
      for (j = 0; j < FRAGMENTS_NUMOF_IMAGES; j++)
        {
          blast->img[j] = (image *) & eclat[num_eclat][j];
        }
      /* set x and y coordinates */
      blast->xcoord = coordx;
      blast->ycoord = coordy;
      /* set speed of the displacement */
      blast->speed = speed;
      blast->img_angle = (Sint16) (rand () % 32);
      /* delay before begin explosion animation */
      blast->countdown = delay;
    }
}

/**
 * Check validty of explosions chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
explosion_check_chained_list (void)
{
  Uint32 i;
  explosion_struct *blast;
  Sint32 count = 0;
  for (i = 0; i < MAX_OF_EXPLOSIONS; i++)
    {
      blast = &explosions[i];
      if (blast->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_explosions)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, num_of_explosions=%i", count, num_of_explosions);
    }
  count = 0;
  blast = explosion_first;
  do
    {
      count++;
      blast = blast->next;
    }
  while (blast != NULL && count <= (MAX_OF_EXPLOSIONS + 1));
  if (count != num_of_explosions)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_explosions=%i", count, num_of_explosions);
    }
  count = 0;
  blast = explosion_last;
  do
    {
      count++;
      blast = blast->previous;
    }
  while (blast != NULL && count <= (MAX_OF_EXPLOSIONS + 1));
  if (count != num_of_explosions)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_explosions=%i", count, num_of_explosions);
    }
}
#endif

/** 
 * Return a free explosion element 
 * @return Pointer to a explosion structure 
 */
static explosion_struct *
explosion_get (void)
{
  Uint32 i;
  explosion_struct *blast;
  for (i = 0; i < MAX_OF_EXPLOSIONS; i++)
    {
      blast = &explosions[i];
      if (blast->is_enabled)
        {
          continue;
        }
      blast->is_enabled = TRUE;
      blast->next = NULL;
      if (num_of_explosions == 0)
        {
          explosion_first = blast;
          explosion_last = blast;
          explosion_last->previous = NULL;
        }
      else
        {
          explosion_last->next = blast;
          blast->previous = explosion_last;
          explosion_last = blast;
        }
      num_of_explosions++;
#ifdef UNDER_DEVELOPMENT
      explosion_check_chained_list ();
#endif
      return blast;
    }
#ifdef UNDER_DEVELOPMENT
  LOG_ERR ("no more element explosion is available");
#endif
  return NULL;
}

/** 
 * Remove a explosion element from list
 * @param blast Pointer to a explosion structure
 */
void
explosion_del (explosion_struct * blast)
{
  blast->is_enabled = FALSE;
  num_of_explosions--;
  if (explosion_first == blast)
    {
      explosion_first = blast->next;
    }
  if (explosion_last == blast)
    {
      explosion_last = blast->previous;
    }
  if (blast->previous != NULL)
    {
      blast->previous->next = blast->next;
    }
  if (blast->next != NULL)
    {
      blast->next->previous = blast->previous;
    }
}

/**
 * Add a serie of explosions
 * @param Pointer to enemy structure or NULL if player's spaceship
 */
void
explosions_add_serie (enemy * foe)
{
  Sint32 type_of_vessel;
  Uint32 coordx, coordy, width, height;
  Sint32 height_big, height_normal, height_small, width_big, width_normal,
    width_small;
  spaceship_struct *ship = spaceship_get ();

  /* 
   * enemy vessel 
   */
  if (foe != NULL)
    {
      type_of_vessel = 0;
      if (foe->spr.img[foe->spr.current_image]->w > 32)
        {
          type_of_vessel += 2;
        }
      if (foe->spr.img[foe->spr.current_image]->h > 32)
        {
          type_of_vessel += 4;
        }
      coordx = (Uint32) foe->spr.xcoord;
      coordy = (Uint32) foe->spr.ycoord;
      width = foe->spr.img[foe->spr.current_image]->w;
      height = foe->spr.img[foe->spr.current_image]->h;
    }
  else
    {
      type_of_vessel = -1;
      coordx = (Uint32) ship->spr.xcoord;
      coordy = (Uint32) ship->spr.ycoord;
      width = ship->spr.img[ship->spr.current_image]->w;
      height = ship->spr.img[ship->spr.current_image]->h;
    }

  height_big = height - 32;
  if (height_big < 1)
    {
      height_big = 1;
    }

  height_normal = height - 16;
  if (height_normal < 1)
    {
      height_normal = 1;
    }

  height_small = height - 8;
  if (height_small < 1)
    {
      height_small = 1;
    }

  width_big = width - 32;
  if (width_big < 1)
    {
      width_big = 1;
    }

  width_normal = width - 16;
  if (width_normal < 1)
    {
      width_normal = 1;
    }

  width_small = width - 8;
  if (width_small < 1)
    {
      width_small = 1;
    }

  switch (type_of_vessel)
    {
      /* 
       * player's spaceship 
       */
    case -1:
      explosion_add ((float) coordx, (float) coordy, 0.3f, EXPLOSION_BIG, 0);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)), 0.3f,
                     EXPLOSION_MEDIUM, 20);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 30);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 40);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 50);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 30);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 40);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 50);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 60);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 70);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 80);
      break;
      /* enemy is lower than 32 width and height pixels */
    case 0:
      explosion_add ((float) coordx, (float) coordy, 0.3f, EXPLOSION_BIG, 0);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 10);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 20);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 30);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 40);
      break;
      /* enemy is higher than 32 width pixels */
    case 2:
      explosion_add ((float) (coordx + (rand () % width_big)),
                     (float) coordy, 0.3f, EXPLOSION_BIG, 0);
      explosion_add ((float) coordx + (rand () % width_big),
                     (float) coordy, 0.3f, EXPLOSION_BIG, 10);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 20);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 30);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 40);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 50);
      break;
      /* enemy is higher than 32 height pixels */
    case 4:
      explosion_add ((float) coordx,
                     (float) (coordy + (rand () % height_big)),
                     0.3f, EXPLOSION_BIG, 0);
      explosion_add ((float) coordx,
                     (float) coordy + (rand () % height_big),
                     0.3f, EXPLOSION_BIG, 10);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 20);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 30);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 40);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 50);
      break;
      /* enemy is higher than 32 width and height pixels */
    case 6:
      explosion_add ((float) (coordx + (rand () % width_big)),
                     (float) (coordy + (rand () % height_big)),
                     0.3f, EXPLOSION_BIG, 0);
      explosion_add ((float) (coordx + (rand () % width_big)),
                     (float) (coordy + (rand () % height_big)),
                     0.3f, EXPLOSION_BIG, 10);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 20);
      explosion_add ((float) (coordx + (rand () % width_normal)),
                     (float) (coordy + (rand () % height_normal)),
                     0.3f, EXPLOSION_MEDIUM, 30);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 40);
      explosion_add ((float) (coordx + (rand () % width_small)),
                     (float) (coordy + (rand () % height_small)),
                     0.3f, EXPLOSION_SMALL, 50);
      break;
    }
}
