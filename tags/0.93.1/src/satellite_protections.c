/** 
 * @file satellite_protections.c 
 * @brief handle orbital satellite protections gravitate around the 
 *        player's spaceship and protect it from hostiles shots
 *        and enemies
 * @created 2006-11-10 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: satellite_protections.c,v 1.29 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "enemies.h"
#include "bonus.h"
#include "config_file.h"
#include "display.h"
#include "electrical_shock.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "menu.h"
#include "options_panel.h"
#include "shots.h"
#include "sdl_mixer.h"
#include "satellite_protections.h"
#include "spaceship.h"

/** Max. num of protections satellite active at any time */
#define SATELLITES_MAXOF 5
/** Number of different protection satellites */
#define SATELLITES_NUMOF_TYPES 5
/** Maximum number of images peer satellite */
#define SATELLITES_NUMOF_IMAGES 16

/** 
 * Structure of an orbital satellite protection
 */
typedef struct satellite_struct
{
  /* TRUE = display satellite */
  bool is_visible;
  /* TRUE = display satellite's white mask */
  bool is_mask;
  /* index area on the circle table */
  Sint16 pos_in_circle;
  /* power of destruction (collision) */
  Sint16 pow_of_dest;
  /* if <= 0 then satellite is destroyed */
  Sint16 energy_level;
  /* number of images of the sprite */
  Sint16 numof_images;
  /* current image index */
  Sint16 current_image;
  /* delay before next image */
  Sint16 anim_speed;
  /* delay's counter */
  Sint16 anim_count;
  /* structures of images satellites */
  image *img[IMAGES_MAXOF];
  /* x and y coordinates */
  Sint32 xcoord;
  Sint32 ycoord;
  Sint32 fire_rate_count;
  Sint32 fire_rate;
  /** Previous element of the chained list */
  struct satellite_struct *previous;
  /** Next element of the chained list */
  struct satellite_struct *next;
  /** TRUE if the element is already chained */
  bool is_enabled;
}
satellite_struct;
static image
  satellites_images[SATELLITES_NUMOF_TYPES][SATELLITES_NUMOF_IMAGES];
static satellite_struct *satellites;
static satellite_struct *satellite_first = NULL;
static satellite_struct *satellite_last = NULL;
static Sint32 num_of_satellites = 0;
/* precalculated circle table */
static const Sint32 SATELLITE_NUMOF_POINTS_CIRCLE = 80;
static Sint16 *satellite_circle_x = NULL;
static Sint16 *satellite_circle_y = NULL;
static satellite_struct *satellite_get (void);

/**
 * Allocate buffers, precalcule the circle, and load sprites images 
 * @return TRUE if it completed successfully or FALSE otherwise 
 */
bool
satellites_once_init (void)
{
  Sint32 i, r;
  Sint16 x, y;
  double a, step, pi;
  satellites_free ();

  /* extract satellites sprites images (23,191 bytes) */
  if (!image_load
      ("graphics/sprites/satellite_protections.spr", &satellites_images[0][0],
       SATELLITES_NUMOF_TYPES, SATELLITES_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* allocate satellites data structure */
  if (satellites == NULL)
    {
      satellites =
        (satellite_struct *) memory_allocation (SATELLITES_MAXOF *
                                                sizeof (satellite_struct));
      if (satellites == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'satellites'");
          return FALSE;
        }
    }

  if (satellite_circle_x == NULL)
    {
      satellite_circle_x =
        (Sint16 *) memory_allocation (SATELLITE_NUMOF_POINTS_CIRCLE * 2 *
                                      sizeof (Sint16));
      if (satellite_circle_x == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'satellite_circle_x'");
          return FALSE;
        }
      satellite_circle_y = satellite_circle_x + SATELLITE_NUMOF_POINTS_CIRCLE;
    }

  /* 
   * precalculate the circle 
   */
  pi = 4 * atan (1.0);
  a = 0.0;
  r = 30 * pixel_size;
  step = (pi * 2) / SATELLITE_NUMOF_POINTS_CIRCLE;
  for (i = 0; i < SATELLITE_NUMOF_POINTS_CIRCLE; i++)
    {
      x = (Sint16) (cos (a) * r);
      y = (Sint16) (sin (a) * r);
      a = a + step;
      satellite_circle_x[i] = x;
      satellite_circle_y[i] = y;
    }

  satellites_init ();
  return TRUE;
}

/**
 * Convert satellite from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
satellite_extract (void)
{
  Uint32 type, frame;
  const char *model =
    EXPORT_DIR "/satellites/satellite-%01d/satellite-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/satellites"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < SATELLITES_NUMOF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/satellites/satellite-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < SATELLITES_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/satellites/satellite-%01d/satellite-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&satellites_images[type][frame], filename))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  return TRUE;
}
#endif

/** 
 * Release memory used for the satellites 
 */
void
satellites_free (void)
{
  LOG_DBG ("deallocates the memory used by the bitmap and structure");
  images_free (&satellites_images[0][0], SATELLITES_NUMOF_TYPES,
               SATELLITES_NUMOF_IMAGES, SATELLITES_NUMOF_IMAGES);
  if (satellites != NULL)
    {
      free_memory ((char *) satellites);
      satellites = NULL;
    }
  if (satellite_circle_x != NULL)
    {
      free_memory ((char *) satellite_circle_x);
      satellite_circle_x = NULL;
      satellite_circle_y = NULL;
    }
}

/** 
 * Initialize satellite protections data structure and index list
 */
void
satellites_init (void)
{
  Sint32 i;
  satellite_struct *sat;
  electrical_shock_enable = FALSE;

  /* release all satellites */
  for (i = 0; i < SATELLITES_MAXOF; i++)
    {
      sat = &satellites[i];
      sat->is_enabled = FALSE;
    }
  num_of_satellites = 0;
  satellite_first = NULL;
  satellite_last = NULL;
}

/** 
 * Move and draw the sprites of the protections satellite 
 */
void
satellites_handle (void)
{
  Sint32 i, k;
  satellite_struct *sat;
  spaceship_struct *ship = spaceship_get ();

  sat = satellite_first;
  if (sat == NULL)
    {
      return;
    }

  /* process each satellite sprite */
  for (i = 0; i < num_of_satellites; i++, sat = sat->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (sat == NULL && i < (num_of_satellites - 1))
        {
          LOG_ERR ("sat->next is null %i/%i", i, num_of_satellites);
          break;
        }
#endif
      if (!player_pause && menu_status == MENU_OFF)
        {
          /* increment index to next position on the precalculated circle table */
          sat->pos_in_circle++;
          if (sat->pos_in_circle >= SATELLITE_NUMOF_POINTS_CIRCLE)
            {
              sat->pos_in_circle = 0;
            }
          /* update x and y coordinates */
          sat->xcoord =
            (Sint16) (ship->spr.xcoord) +
            ship->spr.img[ship->spr.current_image]->x_gc +
            satellite_circle_x[sat->pos_in_circle] -
            sat->img[sat->current_image]->x_gc;
          sat->ycoord =
            (Sint16) (ship->spr.ycoord) +
            ship->spr.img[ship->spr.current_image]->y_gc +
            satellite_circle_y[sat->pos_in_circle] -
            sat->img[sat->current_image]->y_gc;
          /* decrease delay before next shot */
          sat->fire_rate_count--;

          /* check if the satellite fire or not a shot 
           * (this part is used normally never) */
          if (sat->fire_rate_count <= 0)
            {
              /* reset shot time-frequency */
              sat->fire_rate_count = sat->fire_rate;
              /* process each origin of the shot (location of the cannon) */
              for (k = 0; k < sat->img[sat->current_image]->numof_cannons;
                   k++)
                {
                  shot_satellite_add (sat->xcoord +
                                      sat->img[sat->current_image]->
                                      cannons_coords[k][XCOORD],
                                      sat->ycoord +
                                      sat->img[sat->current_image]->
                                      cannons_coords[k][YCOORD],
                                      sat->img[sat->current_image]->
                                      cannons_angles[k]);
                }
            }
        }
      /* check if the sprite is visible or not */
      if ((sat->xcoord + sat->img[0]->w) < offscreen_startx
          || (sat->ycoord + sat->img[0]->h) < offscreen_starty
          || sat->xcoord > (offscreen_startx + offscreen_width_visible - 1)
          || sat->ycoord > (offscreen_starty + offscreen_height_visible - 1))
        {
          /* satellite is not visible, don't perform the tests of collision */
          sat->is_visible = FALSE;
        }
      else
        {
          /* satellite is visible, perform the tests of collision */
          sat->is_visible = TRUE;
          /* increase counter delay between two images */
          sat->anim_count++;
          /* value of delay between two images reached? */
          if (sat->anim_count >= sat->anim_speed)
            {
              /* clear counter delay between two images */
              sat->anim_count = 0;
              /* flip to the next image */
              sat->current_image++;
              /* check if last image has been reached  */
              if (sat->current_image >= sat->numof_images)
                {
                  /* resets the animation to the first image of the animation
                   * sequence */
                  sat->current_image = 0;
                }
            }

          /* draw the satellite sprite image */
          if (sat->is_mask)
            {
              /* draw white mask of the sprite image */
              draw_sprite_mask (coulor[WHITE],
                                sat->img[sat->current_image],
                                sat->xcoord, sat->ycoord);
              sat->is_mask = FALSE;
            }
          else
            {
              draw_sprite (sat->img[sat->current_image], sat->xcoord,
                           sat->ycoord);
            }
        }
    }
}

/**
 * Add all satellite protections
 */
void
satellites_add (void)
{
  while (num_of_satellites < 5)
    {
      satellite_add ();
    }
}

/** 
 * Add a satellite protection or enable electrical shocks
 */
void
satellite_add (void)
{
  Sint32 i;
  satellite_struct *sat;
  spaceship_struct *ship = spaceship_get ();

  /* restore the energy of all satellites */
  for (i = 0; i < SATELLITES_MAXOF; i++)
    {
      /* set level of energy */
      satellites[i].energy_level = (Sint16) (50 + ship->type * 10);
      /* set power of destruction */
      satellites[i].pow_of_dest = (Sint16) (15 + ship->type * 5);
    }

  if (num_of_satellites < 5)
    {
      /* get a free satellite protection */
      sat = satellite_get ();
      if (sat == NULL)
        {
          return;
        }
      /* set level of energy (zero correspond to destruction of the satellite) */
      sat->energy_level = (Sint16) (50 + ship->type * 10);
      /* set power of the satellite's shot */
      sat->pow_of_dest = (Sint16) (15 + ship->type * 5);
      /* set number of images of the sprite */
      sat->numof_images = SATELLITES_NUMOF_IMAGES;
      /* set current image */
      sat->current_image = 0;
      /* value of delay between two images */
      sat->anim_speed = 8;
      /* counter of delay between two images */
      sat->anim_count = 0;
      /* set shot time-frequency */
      sat->fire_rate = 25;
      sat->fire_rate_count = sat->fire_rate;
      /* set addresses of the images buffer */
      for (i = 0; i < sat->numof_images; i++)
        {
          sat->img[i] = (image *) & satellites_images[ship->type][i];
        }
      satellites_setup ();
    }
  else
    {
      /* maximum number of satellites reached, enable electrical shocks */
      electrical_shock_enable = TRUE;
    }
}

/**
 * Positioning the satellite protections around the player's spaceship 
 */
void
satellites_setup (void)
{
  Sint32 i;
  Sint16 step;
  Sint16 pos;
  satellite_struct *sat;
  if (num_of_satellites < 1)
    {
      return;
    }
  sat = satellite_first;
  if (sat == NULL)
    {
      return;
    }
  step = (Sint16) (SATELLITE_NUMOF_POINTS_CIRCLE / num_of_satellites);
  pos = 0;
  for (i = 0; i < num_of_satellites;
       i++, sat = sat->next, pos = (Sint16) (pos + step))
    {
#ifdef UNDER_DEVELOPMENT
      if (sat == NULL && i < (num_of_satellites - 1))
        {
          LOG_ERR ("sat->next is null %i/%i", i, num_of_satellites);
          break;
        }
#endif
      /* set index on the table on points */
      sat->pos_in_circle = pos;
    }
}

/**
 * Check validty of satellites chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
satellite_check_chained_list (void)
{
  Uint32 i;
  satellite_struct *sat;
  Sint32 count = 0;
  for (i = 0; i < SATELLITES_MAXOF; i++)
    {
      sat = &satellites[i];
      if (sat->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_satellites)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, =%i", count, num_of_satellites);
    }
  count = 0;
  sat = satellite_first;
  do
    {
      count++;
      sat = sat->next;
    }
  while (sat != NULL && count <= (SATELLITES_MAXOF + 1));
  if (count != num_of_satellites)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_satellites=%i", count, num_of_satellites);
    }
  count = 0;
  sat = satellite_last;
  do
    {
      count++;
      sat = sat->previous;
    }
  while (sat != NULL && count <= (SATELLITES_MAXOF + 1));
  if (count != num_of_satellites)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_satellites=%i", count, num_of_satellites);
    }
}
#endif

/** 
 * Return a free satellite element 
 * @return Pointer to a satellite structure, NULL if not satellite available 
 */
static satellite_struct *
satellite_get (void)
{
  Uint32 i;
  satellite_struct *sat;
  for (i = 0; i < SATELLITES_MAXOF; i++)
    {
      sat = &satellites[i];
      if (sat->is_enabled)
        {
          continue;
        }
      sat->is_enabled = TRUE;
      sat->next = NULL;
      if (num_of_satellites == 0)
        {
          satellite_first = sat;
          satellite_last = sat;
          satellite_last->previous = NULL;
        }
      else
        {
          satellite_last->next = sat;
          sat->previous = satellite_last;
          satellite_last = sat;
        }
      num_of_satellites++;
#ifdef UNDER_DEVELOPMENT
      satellite_check_chained_list ();
#endif
      return sat;
    }
  LOG_ERR ("no more element char is available");
  return NULL;
}

/** 
 * Remove a satellite element from list
 * @param Pointer to a satellite structure 
 */
static void
satellite_del (satellite_struct * sat)
{
  /* disable electrical shocks */
  electrical_shock_enable = FALSE;
  sat->is_enabled = FALSE;
  num_of_satellites--;
  if (satellite_first == sat)
    {
      satellite_first = sat->next;
    }
  if (satellite_last == sat)
    {
      satellite_last = sat->previous;
    }
  if (sat->previous != NULL)
    {
      sat->previous->next = sat->next;
    }
  if (sat->next != NULL)
    {
      sat->next->previous = sat->previous;
    }
}

/** 
 * Collisions between satellite protections and an enemy
 * @param foe pointer to the structure of an enemy
 * @param num_of_fragments number of fragments to add if enemy is destroyed
 * @return TRUE if enemy is destroyed
 */
bool
satellites_enemy_collisions (enemy * foe, Sint32 num_of_fragments)
{
  Sint32 i, l, m, x1, y1, x2, y2;
  satellite_struct *sat;

  sat = satellite_first;
  if (sat == NULL)
    {
      return FALSE;
    }

  /* process each protection satellite */
  for (i = 0; i < num_of_satellites; i++, sat = sat->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (sat == NULL && i < (num_of_satellites - 1))
        {
          LOG_ERR ("sat->next is null %i/%i", i, num_of_satellites);
          break;
        }
#endif
      /* if satellite is invisible, don't perform the tests of collision */
      if (!sat->is_visible)
        {
          continue;
        }

      /* for each collision point of the satellite */
      for (l = 0; l < sat->img[sat->current_image]->numof_collisions_points;
           l++)
        {
          /* coordinates of the collision point of the satellite */
          x1 =
            sat->xcoord +
            sat->img[sat->current_image]->collisions_points[l][XCOORD];
          y1 =
            sat->ycoord +
            sat->img[sat->current_image]->collisions_points[l][YCOORD];

          /* for each collision zone of the enemy */
          for (m = 0;
               m <
               foe->spr.img[foe->spr.current_image]->numof_collisions_zones;
               m++)
            {
              /* coordinates of the collision zone of the enemy */
              x2 =
                (Sint32) foe->spr.xcoord +
                foe->spr.img[foe->spr.
                             current_image]->collisions_coords[m][XCOORD];
              y2 =
                (Sint32) foe->spr.ycoord +
                foe->spr.img[foe->spr.
                             current_image]->collisions_coords[m][YCOORD];

              /* check if satellite collision point is into enemy collision zone */
              if (x1 >= x2 &&
                  y1 >= y2 &&
                  x1 <
                  (x2 +
                   foe->spr.img[foe->spr.
                                current_image]->collisions_sizes[m]
                   [IMAGE_WIDTH])
                  && y1 <
                  (y2 +
                   foe->spr.img[foe->spr.current_image]->
                   collisions_sizes[m][IMAGE_HEIGHT]))
                {
                  /* decrease energy level of enemy */
                  foe->spr.energy_level =
                    (Sint16) (foe->spr.energy_level - sat->pow_of_dest);
                  if (foe->type >= THANIKEE)
                    {
                      energy_gauge_guard_is_update = TRUE;
                    }
                  /* decrease energy level of satellite */
                  sat->energy_level =
                    (Sint16) (sat->energy_level - foe->spr.pow_of_dest);
                  /* check if satellite is destroyed */
                  if (sat->energy_level <= 0)
                    {
                      /* remove satellite from the list */
                      satellite_del (sat);
                      /* positioning the satellites around the spaceship */
                      satellites_setup ();
                      goto next_satellite;
                    }
                  else
                    {
                      /* satellite not destroyed, display white mask */
                      sat->is_mask = TRUE;
                    }
                  /* check if enemy is destroyed */
                  if (foe->spr.energy_level <= 0)
                    {
                      /* check if the enemy is a meteor */
                      if ((foe->type >= BIGMETEOR && foe->type <= SMALLMETEOR)
                          || foe->type >= THANIKEE)
                        {
                          /* add a bonus gem or a lonely foe */
                          bonus_meteor_add (foe);
                          if (num_of_fragments > 0)
                            {
                              explosions_fragments_add (foe->spr.xcoord +
                                                        foe->spr.img[foe->
                                                                     spr.current_image]->
                                                        x_gc - 8,
                                                        foe->spr.ycoord +
                                                        foe->spr.img[foe->
                                                                     spr.current_image]->
                                                        y_gc - 8, 1.0,
                                                        num_of_fragments, 0,
                                                        2);
                            }
                        }
                      else
                        {
                          /* add a bonus gem or a lonely foe */
                          bonus_add (foe);
                        }
                      player_score +=
                        foe->spr.pow_of_dest << 2 << score_multiplier;
                      return TRUE;
                    }
                  else
                    {
                      /* enemy not destroyed, display white mask */
                      foe->is_white_mask_displayed = TRUE;
                    }
                  explosion_add ((float) x1, (float) y1, 0.3f,
                                 EXPLOSION_SMALL, 0);
                  goto next_satellite;
                }
            }
        }
    next_satellite:;
    }
  return FALSE;
}

/** 
 * Collisions between satellite protections and a shot 
 * @param x1 x coordinate of the collision point of the shot
 * @param y1 y coordinate of the collision point of the shot
 * @param projectile pointer to the structure of an shot 
 * @return TRUE if the shot touched a satellite  
 */
bool
satellites_shot_collisions (Sint32 x1, Sint32 y1, shot_struct * projectile)
{
  Sint32 i, m, x2, y2;
  satellite_struct *sat;

  sat = satellite_first;
  if (sat == NULL)
    {
      return FALSE;
    }

  /* process each protection satellite */
  for (i = 0; i < num_of_satellites; i++, sat = sat->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (sat == NULL && i < (num_of_satellites - 1))
        {
          LOG_ERR ("sat->next is null %i/%i", i, num_of_satellites);
          break;
        }
#endif

      /* if satellite is invisible, don't perform the tests of collision */
      if (!sat->is_visible)
        {
          continue;
        }
      /* for each collision point of the satellite */
      for (m = 0; m < sat->img[sat->current_image]->numof_collisions_zones;
           m++)
        {
          /* coordinates of the collision zone of the satellite */
          x2 =
            sat->xcoord +
            sat->img[sat->current_image]->collisions_coords[m][XCOORD];
          y2 =
            sat->ycoord +
            sat->img[sat->current_image]->collisions_coords[m][YCOORD];

          /* check if satellite collision point is into enemy collision zone */
          if (x1 >= x2 &&
              y1 >= y2 &&
              x1 <
              (x2 +
               sat->img[sat->current_image]->collisions_sizes[m][IMAGE_WIDTH])
              && y1 <
              (y2 +
               sat->img[sat->
                        current_image]->collisions_sizes[m][IMAGE_HEIGHT]))
            {
              /* decrease energy level of satellite */
              sat->energy_level =
                (Sint16) (sat->energy_level - projectile->spr.pow_of_dest);

              /* check if satellite is destroyed */
              if (sat->energy_level <= 0)
                {
                  /* remove satellite from the list */
                  satellite_del (sat);
                  /* positioning the satellites around the spaceship */
                  satellites_setup ();
                }
              else
                {
                  /* satellite not destroyed, display white mask */
                  sat->is_mask = TRUE;
                }
              /* add a little explosion */
              explosion_add ((float) projectile->spr.xcoord,
                             (float) projectile->spr.ycoord, 0.35f,
                             EXPLOSION_SMALL, 0);
              return TRUE;
            }
        }
    }
  return FALSE;
}
