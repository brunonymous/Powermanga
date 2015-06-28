/** 
 * @file extra_gun.c 
 * @brief Handle extra guns positioned on the sides of the spaceship
 * @created 2006-11-25 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: extra_gun.c,v 1.26 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "display.h"
#include "electrical_shock.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "log_recorder.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "images.h"
#include "menu.h"
#include "options_panel.h"
#include "spaceship.h"

/** Maximum number guns active at any time */
#define GUNS_MAXOF 2
/** Number of types of differents guns */
#define GUNS_NUMOF_TYPES 5
/** Extra gun sprite's number of images  */
#define GUNS_NUMOF_IMAGES 16

/**
 * Structure io the side extra gun
 */
typedef struct gun_struct
{
  /** Display extra gun */
  bool is_visible;
  /** Display extra gun's white mask */
  bool is_white_mask_displayed;
  /** Power of destruction (collision) */
  Sint16 pow_of_dest;
  /** Damage status  (<= 0 destroyed gun) */
  Sint16 energy_level;
  /** Number of images of the sprite */
  Sint16 numof_images;
  /** Current image index */
  Sint16 current_image;
  /** Delay before next image */
  Sint16 anim_speed;
  /** Delay's counter */
  Sint16 anim_count;
  /** Structures of images extra gun */
  image *img[IMAGES_MAXOF];
  /** X-coordinates */
  Sint32 xcoord;
  /** Y-coordinates */
  Sint32 ycoord;
  /** Shot delay counter */
  Sint32 fire_rate_count;
  Sint32 fire_rate;
  /** 0=right side / 1=left side */
  Sint32 position;
  /** Previous element of the chained list */
  struct gun_struct *previous;
  /** Next element of the chained list */
  struct gun_struct *next;
  /** TRUE if the element is already chained */
  bool is_enabled;
}
gun_struct;

/** Data structures of all extra guns */
static gun_struct *extra_guns = NULL;
static gun_struct *gun_first = NULL;
static gun_struct *gun_last = NULL;
/** Data structure of the sprites images's guns */
static image guns_images[GUNS_NUMOF_TYPES][GUNS_NUMOF_IMAGES];
static gun_struct *gun_get (void);
static void gun_del (gun_struct * egun);

/**
 * Allocate buffers and initialize structure of the extras guns 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
guns_once_init (void)
{
  guns_free ();

  /* extract extra guns sprites images (28,450 bytes) */
  if (!image_load
      ("graphics/sprites/extra_guns.spr", &guns_images[0][0],
       GUNS_NUMOF_TYPES, GUNS_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* allocate extra guns data structure */
  if (extra_guns == NULL)
    {
      extra_guns =
        (gun_struct *) memory_allocation (GUNS_MAXOF * sizeof (gun_struct));
      if (extra_guns == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'gun_struct'");
          return FALSE;
        }
    }

  guns_init ();
  return TRUE;
}

/**
 * Convert extra guns from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
guns_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/extra-guns/extra-gun-x/extra-gun-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/extra-guns"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < GUNS_NUMOF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/extra-guns/extra-gun-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < GUNS_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/extra-guns/extra-gun-%01d/extra-gun-%02d.png",
                   type + 1, frame);
          if (!image_to_png (&guns_images[type][frame], filename))
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
 * Release memory of images and structures used for the etra guns
 */
void
guns_free (void)
{
  if (extra_guns != NULL)
    {
      free_memory ((char *) extra_guns);
      extra_guns = NULL;
    }
  images_free (&guns_images[0][0], GUNS_NUMOF_TYPES, GUNS_NUMOF_IMAGES,
               GUNS_NUMOF_IMAGES);
}

/**
 * Initialize structures of the extra guns 
 */
void
guns_init (void)
{
  Sint32 i;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();

  /* release all extra guns */
  for (i = 0; i < GUNS_MAXOF; i++)
    {
      egun = &extra_guns[i];
      egun->is_enabled = FALSE;
    }
  /* clear the number of extra guns */
  ship->num_of_extraguns = 0;
}

/** 
 * Handle the extra gun positioned on the sides of the player's spaceship 
 */
void
guns_handle (void)
{
  Sint32 i, k, l;
  gun_struct *egun;
  shot_struct *bullet;
  image *egun_img;
  spaceship_struct *ship = spaceship_get ();
  egun = gun_first;
  if (egun == NULL)
    {
      return;
    }
  for (i = 0; i < ship->num_of_extraguns; i++, egun = egun->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (egun == NULL && i < (ship->num_of_extraguns - 1))
        {
          LOG_ERR ("egun->next is null %i/%i", i, ship->num_of_extraguns);
          break;
        }
#endif
      egun_img = egun->img[egun->current_image];

      /* update vertical coordinate of the gun */
      egun->ycoord =
        (Sint16) (ship->spr.ycoord) +
        ship->spr.img[ship->spr.current_image]->h - egun_img->h;

      /* update horizontal coordinate of the right gun */
      if (egun->position == 0)
        {
          egun->xcoord =
            (Sint16) (ship->spr.xcoord) +
            ship->spr.img[ship->spr.current_image]->w;
        }

      /* update horizontal coordinate of the left gun */
      if (egun->position == 1)
        {
          egun->xcoord = (Sint16) (ship->spr.xcoord) - egun_img->w;
        }

      if (!player_pause && menu_status == MENU_OFF)
        {
          /* decrease counter of delay between two shots */
          egun->fire_rate_count--;
          /* check if the satellite fire or not a shot */
          if (egun->fire_rate_count <= 0)
            {
              /* reset counter of delay between two shots */
              egun->fire_rate_count = egun->fire_rate;
              /* process each origin of the shot (location of the cannon) */
              for (k = 0; k < egun_img->numof_cannons; k++)
                {
                  if (num_of_shots < (MAX_OF_SHOTS - 1))
                    {
                      /* get a new shot */
                      bullet = shot_get ();
                      if (bullet == NULL)
                        {
                          break;
                        }
                      /* animated sprite (flicker shot) */
                      bullet->is_blinking = TRUE;
                      /* indicate that is friend sprite */
                      bullet->spr.type = FRIEND;
                      /* fixed trajectory */
                      bullet->spr.trajectory = 0;
                      /* set number of images of the sprite */
                      bullet->spr.numof_images = 32;

                      /* initialize power of the shot */
                      switch (ship->type)
                        {
                        case SPACESHIP_TYPE_1:
                          bullet->spr.pow_of_dest = 2;
                          break;
                        case SPACESHIP_TYPE_2:
                          bullet->spr.pow_of_dest = 3;
                          break;
                        case SPACESHIP_TYPE_3:
                          bullet->spr.pow_of_dest = 4;
                          break;
                        case SPACESHIP_TYPE_4:
                          bullet->spr.pow_of_dest = 3;
                          break;
                        case SPACESHIP_TYPE_5:
                          bullet->spr.pow_of_dest = 4;
                          break;
                        }
                      /* set addresses of the sprites images buffer */
                      for (l = 0; l < bullet->spr.numof_images; l++)
                        {
                          /* shot 2 force 1 */
                          bullet->spr.img[l] = (image *) & fire[V1TN1][l];
                        }
                      /* set energy level of the sprite */
                      bullet->spr.energy_level = bullet->spr.pow_of_dest;
                      /* set current image index */
                      bullet->spr.current_image = 0;
                      /* set value of delay between two images */
                      bullet->spr.anim_speed = 4;
                      /* set delay counter between two images */
                      bullet->spr.anim_count = 0;
                      bullet->img_angle = egun_img->cannons_angles[k];
                      bullet->img_old_angle = bullet->img_angle;
                      /* set x and y coordinates */
                      bullet->spr.xcoord =
                        (float) (egun->xcoord +
                                 egun_img->cannons_coords[k][XCOORD] -
                                 bullet->spr.img[bullet->img_angle]->x_gc);
                      bullet->spr.ycoord =
                        (float) (egun->ycoord +
                                 egun_img->cannons_coords[k][YCOORD] -
                                 bullet->spr.img[bullet->img_angle]->y_gc);
                      bullet->timelife = 400;
                      /* set angle of the projectile */
                      bullet->angle = PI_BY_16 * bullet->img_angle;
                      /* set speed of the displacement */
                      bullet->spr.speed = 9.0;
                    }
                }
            }
        }

      /* check if the sprite is visible or not */
      if ((egun->xcoord + egun->img[0]->w) <
          offscreen_startx
          || (egun->ycoord + egun->img[0]->h) <
          offscreen_starty
          || egun->xcoord >
          (offscreen_startx + offscreen_width_visible - 1)
          || egun->ycoord > (offscreen_starty + offscreen_height_visible - 1))
        {
          /* extra gun not visible, don't perform the tests of collision */
          egun->is_visible = FALSE;
        }
      else
        {
          /* extra gun is visible, perform the tests of collision */
          egun->is_visible = TRUE;
          /* increase counter delay between two images */
          egun->anim_count++;
          /* value of delay between two images reached? */
          if (egun->anim_count >= egun->anim_speed)
            {
              /* clear counter delay between two images */
              egun->anim_count = 0;
              /* flip to the next image */
              egun->current_image++;
              /* check if last image has been reached  */
              if (egun->current_image >= egun->numof_images)
                {
                  /* resets the animation to the first image of the animation
                   * sequence */
                  egun->current_image = 0;
                }
            }

          /* display extra gun */
          if (egun->is_white_mask_displayed)
            {
              draw_sprite_mask (coulor[WHITE], egun->img[egun->current_image],
                                egun->xcoord, egun->ycoord);
              egun->is_white_mask_displayed = 0;
            }
          else
            {
              draw_sprite (egun->img[egun->current_image], egun->xcoord,
                           egun->ycoord);
            }
        }
    }
}

/**
 * Add an extra gun on the player's spaceship side 
 * @return TRUE if a extra gun was added
 */
bool
gun_add (void)
{
  Sint32 i;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();
  /* get an extra gun, cannot install more than 2 guns */
  if (ship->num_of_extraguns >= 2)
    {
      return FALSE;
    }
  /* get new extra gun index */
  egun = gun_get ();
  if (egun == NULL)
    {
      return FALSE;
    }
  /* set level of energy (zero correspond to destruction of the gun) */
  egun->energy_level = (Sint16) (100 + ship->type * 10);
  /* set power of the gun's shot */
  egun->pow_of_dest = (Sint16) (30 + ship->type * 5);
  /* set number of images of the sprite */
  egun->numof_images = GUNS_NUMOF_IMAGES;
  /* set current image */
  egun->current_image = 0;
  /* value of delay between two images */
  egun->anim_speed = 10;
  /* counter of delay between two images */
  egun->anim_count = 0;
  /* set shot time-frequency */
  egun->fire_rate = 25 - (ship->type << 1);
  egun->fire_rate_count = egun->fire_rate;
  /* set addresses of the images buffer */
  for (i = 0; i < egun->numof_images; i++)
    {
      egun->img[i] = (image *) & guns_images[ship->type][i];
    }
  /* position the gun on the side of the player's spaceship */
  switch (ship->num_of_extraguns)
    {
    case 1:
      /* gun's positioned on the right */
      egun->position = 0;
      break;
    case 2:
      if (gun_first->position == 0)
        {
          egun->position = 1;
        }
      else
        {
          egun->position = 0;
        }
      break;
    }
  return TRUE;
}

/**
 * Check validty of extra_gun chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
gun_check_chained_list (void)
{
  Uint32 i;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();
  Sint32 count = 0;
  for (i = 0; i < GUNS_MAXOF; i++)
    {
      egun = &extra_guns[i];
      if (egun->is_enabled)
        {
          count++;
        }
    }
  if (count != ship->num_of_extraguns)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, ship->num_of_extraguns=%i",
               count, ship->num_of_extraguns);
    }
  count = 0;
  egun = gun_first;
  do
    {
      count++;
      egun = egun->next;
    }
  while (egun != NULL && count <= (GUNS_MAXOF + 1));
  if (count != ship->num_of_extraguns)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, ship->num_of_extraguns=%i", count,
               ship->num_of_extraguns);
    }
  count = 0;
  egun = gun_last;
  do
    {
      count++;
      egun = egun->previous;
    }
  while (egun != NULL && count <= (GUNS_MAXOF + 1));
  if (count != ship->num_of_extraguns)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, ship->num_of_extraguns=%i", count,
               ship->num_of_extraguns);
    }
}
#endif

/** 
 * Return a free extra gun element 
 * @return Pointer to a gun structure, NULL if not gun available 
 */
static gun_struct *
gun_get (void)
{
  Uint32 i;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();
  for (i = 0; i < GUNS_MAXOF; i++)
    {
      egun = &extra_guns[i];
      if (egun->is_enabled)
        {
          continue;
        }
      egun->is_enabled = TRUE;
      egun->next = NULL;
      if (ship->num_of_extraguns == 0)
        {
          gun_first = egun;
          gun_last = egun;
          gun_last->previous = NULL;
        }
      else
        {
          gun_last->next = egun;
          egun->previous = gun_last;
          gun_last = egun;
        }
      ship->num_of_extraguns++;
#ifdef UNDER_DEVELOPMENT
      gun_check_chained_list ();
#endif
      return egun;
    }
  LOG_ERR ("no more element gun is available");
  return NULL;
}

/** 
 * Remove a extra gun element from list
 * @param Pointer to a extra gun structure 
 */
static void
gun_del (gun_struct * egun)
{
  spaceship_struct *ship = spaceship_get ();
  egun->is_enabled = FALSE;
  ship->num_of_extraguns--;
  if (gun_first == egun)
    {
      gun_first = egun->next;
    }
  if (gun_last == egun)
    {
      gun_last = egun->previous;
    }
  if (egun->previous != NULL)
    {
      egun->previous->next = egun->next;
    }
  if (egun->next != NULL)
    {
      egun->next->previous = egun->previous;
    }
}

/** 
 * Collisions between extra guns and an enemy
 * @param foe pointer to the structure of an enemy
 * @param num_of_fragments number of fragments to add if enemy is destroyed
 * @return TRUE if enemy is destroyed
 */
bool
guns_enemy_collisions (enemy * foe, Sint32 num_of_fragments)
{
  Sint32 i, l, m, x1, y1, x2, y2;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();
  egun = gun_first;
  if (egun == NULL)
    {
      return FALSE;
    }
  /* process each extra gun */
  for (i = 0; i < ship->num_of_extraguns; i++, egun = egun->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (egun == NULL && i < (ship->num_of_extraguns - 1))
        {
          LOG_ERR ("egun->next is null %i/%i", i, ship->num_of_extraguns);
          break;
        }
#endif
      /* if extra gun is invisible, don't perform the tests of collision */
      if (!egun->is_visible)
        {
          continue;
        }
      for (l = 0; l < egun->img[egun->current_image]->numof_collisions_points;
           l++)
        {
          /* coordinates of the collision point of the gun */
          x1 =
            egun->xcoord +
            egun->img[egun->current_image]->collisions_points[l][XCOORD];
          y1 =
            egun->ycoord +
            egun->img[egun->current_image]->collisions_points[l][YCOORD];
          /* for each collision zone of the enemy */
          for (m = 0;
               m <
               foe->spr.img[foe->spr.current_image]->numof_collisions_zones;
               m++)
            {
              /* coordinates of the collision zone of the enemy */
              x2 =
                (Sint32) foe->spr.xcoord +
                foe->spr.img[foe->spr.current_image]->
                collisions_coords[m][XCOORD];
              y2 =
                (Sint32) foe->spr.ycoord +
                foe->spr.img[foe->spr.current_image]->
                collisions_coords[m][YCOORD];
              /* check if gun collision point is into enemy collision zone */
              if (x1 >= x2 && y1 >= y2
                  && x1 <
                  (x2 +
                   foe->spr.img[foe->spr.current_image]->collisions_sizes[m]
                   [IMAGE_WIDTH])
                  && y1 <
                  (y2 +
                   foe->spr.img[foe->spr.
                                current_image]->collisions_sizes[m]
                   [IMAGE_HEIGHT]))
                {
                  /* decrease energy level of enemy */
                  foe->spr.energy_level =
                    (Sint16) (foe->spr.energy_level - egun->pow_of_dest);
                  if (foe->type >= THANIKEE)
                    {
                      energy_gauge_guard_is_update = TRUE;
                    }
                  /* decrease energy level of gun */
                  egun->energy_level =
                    (Sint16) (egun->energy_level - foe->spr.pow_of_dest);
                  /* check if gun is destroyed */
                  if (egun->energy_level <= 0)
                    {
                      /* remove gun from the list */
                      gun_del (egun);
                      break;
                    }
                  else
                    {
                      /* gun not destroyed, display white mask */
                      egun->is_white_mask_displayed = TRUE;
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
                                                        foe->spr.img[foe->spr.
                                                                     current_image]->x_gc
                                                        - 8,
                                                        foe->spr.ycoord +
                                                        foe->spr.img[foe->spr.
                                                                     current_image]->y_gc
                                                        - 8, 1.0,
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
                      /* enemy destroyed! */
                      return TRUE;
                    }
                  else
                    {
                      /* enemy not destroyed, display white mask */
                      foe->is_white_mask_displayed = TRUE;
                    }
                  explosion_add ((float) x1, (float) y1, 0.3f,
                                 EXPLOSION_SMALL, 0);
                  break;
                }
            }
        }
    }
  return FALSE;
}

/** 
 * Collisions between extra guns and a shot 
 * @param x1 x coordinate of the collision point of the shot
 * @param y1 y coordinate of the collision point of the shot
 * @param bullet pointer to the structure of an shot 
 * @return TRUE if the shot touched a extra gun 
 */
bool
guns_shot_collisions (Sint32 x1, Sint32 y1, shot_struct * bullet)
{
  Sint32 i, j, x2, y2;
  gun_struct *egun;
  spaceship_struct *ship = spaceship_get ();
  egun = gun_first;
  if (egun == NULL)
    {
      return FALSE;
    }

  /* process each extra gun */
  for (i = 0; i < ship->num_of_extraguns; i++, egun = egun->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (egun == NULL && i < (ship->num_of_extraguns - 1))
        {
          LOG_ERR ("egun->next is null %i/%i", i, ship->num_of_extraguns);
          break;
        }
#endif
      /* if extra gun is invisible, don't perform the tests of collision */
      if (!egun->is_visible)
        {
          continue;
        }
      /* for each collision zone of the gun */
      for (j = 0; j < egun->img[egun->current_image]->numof_collisions_zones;
           j++)
        {
          /* coordinates of the collision zone of the gun */
          x2 =
            egun->xcoord +
            egun->img[egun->current_image]->collisions_coords[j][XCOORD];
          y2 =
            egun->ycoord +
            egun->img[egun->current_image]->collisions_coords[j][YCOORD];
          /* check if shot collision point is into gun collision zone */
          if (x1 >= x2 &&
              y1 >= y2 &&
              x1 <
              (x2 +
               egun->img[egun->current_image]->
               collisions_sizes[j][IMAGE_WIDTH])
              && y1 <
              (y2 +
               egun->img[egun->current_image]->
               collisions_sizes[j][IMAGE_HEIGHT]))
            {
              /* decrease energy level of gun */
              egun->energy_level =
                (Sint16) (egun->energy_level - bullet->spr.pow_of_dest);
              /* check if gun is destroyed */
              if (egun->energy_level <= 0)
                {
                  gun_del (egun);
                }
              else
                {
                  /* gun not destroyed, display white mask */
                  egun->is_white_mask_displayed = TRUE;
                }
              explosion_add (bullet->spr.xcoord, bullet->spr.ycoord, 0.35f,
                             EXPLOSION_SMALL, 0);
              return TRUE;
            }
        }
    }
  return FALSE;
}
