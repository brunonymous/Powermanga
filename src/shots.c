/** 
 * @file shots.c
 * @brief Handle all shots elements 
 * @date 2012-10-22
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: shots.c,v 1.34 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "curve_phase.h"
#include "display.h"
#include "electrical_shock.h"
#include "shots.h"
#include "extra_gun.h"
#include "enemies.h"
#include "explosions.h"
#include "bonus.h"
#include "energy_gauge.h"
#include "gfx_wrapper.h"
#include "guardians.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "sdl_mixer.h"
#include "spaceship.h"

/** The number of currently active shots */
Sint32 num_of_shots = 0;
image fire[SHOT_MAX_OF_TYPE][SHOT_NUMOF_IMAGES];
/** Data structures of all shots */
static shot_struct *shots;
static shot_struct *shot_first = NULL;
static shot_struct *shot_last = NULL;
/** List of the shots indexes on the shots data structures */
static bool shot_moving (shot_struct * bullet);
static bool shot_display (shot_struct * bullet);
static void shot_delete (shot_struct * bullet);
static bool shot_enemies_collisions (shot_struct * bullet);

/**
 * Allocate buffers and initialize structure of the extras guns 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
shots_once_init (void)
{
  shots_free ();

  /* extract shots sprites images (99,6131 bytes) */
  if (!image_load
      ("graphics/sprites/all_shots.spr", &fire[0][0], SHOT_MAX_OF_TYPE,
       SHOT_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* allocate extra guns data structure */
  if (shots == NULL)
    {
      shots =
        (shot_struct *) memory_allocation (MAX_OF_SHOTS *
                                           sizeof (shot_struct));
      if (shots == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'shots'");
          return FALSE;
        }
    }

  shots_init ();
  return TRUE;
}

/**
 * Convert shots from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
shots_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/shots/shot-%03d/shot-%02d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/shots"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < SHOT_MAX_OF_TYPE; type++)
    {
      sprintf (filename, EXPORT_DIR "/shots/shot-%03d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < SHOT_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/shots/shot-%03d/shot-%02d.png",
                   type + 1, frame);
          if (!image_to_png (&fire[type][frame], filename))
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
 * Release memory used for the shots 
 */
void
shots_free (void)
{
  if (shots != NULL)
    {
      free_memory ((char *) shots);
      shots = NULL;
    }
  images_free (&fire[0][0], SHOT_MAX_OF_TYPE, SHOT_NUMOF_IMAGES,
               SHOT_NUMOF_IMAGES);
}

/** 
 * Initialize shots data structure 
 */
void
shots_init (void)
{
  Sint32 i;
  shot_struct *bullet;

  /* initialize shots data structure */
  for (i = 0; i < MAX_OF_SHOTS; i++)
    {
      bullet = &shots[i];
      bullet->is_enabled = FALSE;
    }
  num_of_shots = 0;
}

/** 
 * Handle all shots fired by enemies and player's starship
 */
void
shots_handle (void)
{
  Sint32 i;
  shot_struct *bullet = shot_first;
  if (bullet == NULL)
    {
      return;
    }
  for (i = 0; i < num_of_shots; i++, bullet = bullet->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (bullet == NULL && i < (num_of_shots - 1))
        {
          LOG_ERR ("shot->next is null %i/%i", i, num_of_shots);
          break;
        }
#endif

      /* shot disable */
      if (bullet->timelife == 0)
        {
          shot_delete (bullet);
          continue;
        }

      /* moving a shot */
      if (!player_pause && menu_status == MENU_OFF && menu_section == 0)
        {
          if (!shot_moving (bullet))
            {
              shot_delete (bullet);
              continue;
            }
        }

      /* clip a shot sprite */
      if ((Sint16)
          (bullet->spr.xcoord + bullet->spr.img[0]->h -
           1) < offscreen_startx
          || (Sint16) (bullet->spr.ycoord +
                       bullet->spr.img[0]->w - 1) <
          offscreen_starty
          || (Sint16) bullet->spr.ycoord >
          offscreen_starty + offscreen_height_visible
          || (Sint16) bullet->spr.xcoord >
          offscreen_startx + offscreen_width_visible)
        {

          /* linear-trajectory: disable shot */
          if (bullet->spr.trajectory == 0)
            {
              shot_delete (bullet);
            }
          else
            {
              if ((Sint16) (bullet->spr.xcoord) <
                  (offscreen_clipsize - 32)
                  || (Sint16) (bullet->spr.ycoord) <
                  (offscreen_clipsize - 32)
                  || (Sint16) bullet->spr.ycoord >
                  (offscreen_clipsize + offscreen_height_visible + 32)
                  || (Sint16) bullet->spr.xcoord >
                  (offscreen_clipsize + offscreen_width_visible + 32))
                {
                  shot_delete (bullet);
                  i--;
                }
              /* shot not visible */
              else
                {
                  /* trajectory calculated (missile homing head) */
                  if (bullet->spr.trajectory == 1)
                    {
                      /* change x and y coordinates */
                      bullet->spr.xcoord +=
                        bullet->spr.img[bullet->img_old_angle]->x_gc;
                      bullet->spr.ycoord +=
                        bullet->spr.img[bullet->img_old_angle]->y_gc;
                      /* decrement lifetime */
                      bullet->timelife--;
                    }
                }
            }
          continue;
        }
      else
        {
          if (!shot_display (bullet))
            {
              shot_delete (bullet);
              continue;
            }
        }
    }
}

/** 
 * Moving all shots, from enemies and player's starship 
 * @return if FALSE then disable the shot
 */
static bool
shot_moving (shot_struct * bullet)
{
  Uint32 i;
  float a;
  enemy *foe;
  sprite *spr = &bullet->spr;

  switch (spr->trajectory)
    {
      /* linear-trajectory */
    case 0:
      {
        spr->xcoord += depix[(Sint16) spr->speed][bullet->img_angle];
        spr->ycoord += depiy[(Sint16) spr->speed][bullet->img_angle];
      }
      break;

      /* trajectory calculated (missile homing head) */
    case 1:
      {
        foe = enemy_get_first ();
        if (foe != NULL)
          {
            a = calc_target_angle ((Sint16)
                                   (spr->xcoord +
                                    spr->img[bullet->img_old_angle]->x_gc),
                                   (Sint16) (spr->ycoord +
                                             spr->img[bullet->
                                                      img_old_angle]->y_gc),
                                   (Sint16) (foe->spr.xcoord +
                                             foe->spr.img[foe->
                                                          spr.current_image]->x_gc),
                                   (Sint16) (foe->spr.ycoord +
                                             foe->spr.img[foe->
                                                          spr.current_image]->y_gc));
            bullet->angle =
              get_new_angle (bullet->angle, a, bullet->velocity);
          }
        else
          {
            a = calc_target_angle ((Sint16)
                                   (spr->xcoord +
                                    spr->img[bullet->img_old_angle]->x_gc),
                                   (Sint16) (spr->ycoord +
                                             spr->img[bullet->
                                                      img_old_angle]->y_gc),
                                   256, 0);
            bullet->angle =
              get_new_angle (bullet->angle, a, bullet->velocity);
          }
        /* change x coordinate */
        spr->xcoord =
          shot_x_move (bullet->angle,
                       spr->speed,
                       spr->xcoord - spr->img[bullet->img_old_angle]->x_gc);
        /* change y coordinate */
        spr->ycoord =
          shot_y_move (bullet->angle,
                       spr->speed,
                       spr->ycoord - spr->img[bullet->img_old_angle]->y_gc);
      }
      break;

      /* shot trajectory follow a curve */
    case 2:
      {
        /* read two values to accelerate the shot's speed */
        for (i = 0; i < 2; i++)
          {
            bullet->curve_index++;
            /* disable the shot if out the curve */
            if (bullet->curve_index >=
                initial_curve[bullet->curve_num].nbr_pnt_curve)
              {
                /* disable shot! */
                return FALSE;
              }
            /* change x and y coordinates */
            spr->xcoord +=
              (float) initial_curve[bullet->curve_num].delta_x[bullet->
                                                               curve_index];
            spr->ycoord +=
              (float) initial_curve[bullet->curve_num].delta_y[bullet->
                                                               curve_index];
          }
      }
      break;
    }
  return TRUE;
}

/**
 * Draw the sprites of the shots and perform the tests of collisions
 * @return if FALSE then disable the shot
 */
static bool
shot_display (shot_struct * bullet)
{
  Sint32 k, tmp_tsts_x, tmp_tsts_y;

  switch (bullet->spr.trajectory)
    {

      /* 
       * fixed trajectory: linear or follow a curve 
       */
    case 0:
    case 2:
      {
        /* animated sprite (flicker shot) */
        if (bullet->is_blinking)
          {
            /* increase animation delay counter */
            bullet->spr.anim_count++;
            if (bullet->spr.anim_count >= bullet->spr.anim_speed)
              {
                /* clear counter */
                bullet->spr.anim_count = 0;
                /* next image */
                bullet->spr.current_image++;
                if (bullet->spr.current_image >= bullet->spr.numof_images)
                  {
                    /* first image */
                    bullet->spr.current_image = 0;
                  }
              }
            /* display shot sprite */
            draw_sprite (bullet->spr.img[bullet->spr.current_image],
                         (Uint32) bullet->spr.xcoord,
                         (Uint32) bullet->spr.ycoord);
          }
        else
          {
            /* the sprite is not animated */
            draw_sprite (bullet->spr.img[bullet->img_angle],
                         (Uint32) bullet->spr.xcoord,
                         (Uint32) bullet->spr.ycoord);
          }

        /* fixed trajectory: collisions spaceship shots and enemies */
        if (bullet->spr.type == FRIEND)
          {
            if (!shot_enemies_collisions (bullet))
              {
                return FALSE;
              }
          }

        /*
         * enemy's shot
         */
        else
          {
            if (!gameover_enable)
              {
                /* for each collision point of the shot */
                for (k = 0;
                     k <
                     bullet->spr.img[bullet->
                                     img_angle]->numof_collisions_points; k++)
                  {
                    /* coordinates of the collision point of the shot */
                    tmp_tsts_x =
                      (Sint16) bullet->spr.xcoord +
                      bullet->spr.img[bullet->
                                      img_angle]->collisions_points[k]
                      [XCOORD];
                    tmp_tsts_y =
                      (Sint16) bullet->spr.ycoord +
                      bullet->spr.img[bullet->
                                      img_angle]->collisions_points[k]
                      [YCOORD];
                    /* for each collision zone of the spaceship */
                    if (spaceship_shot_collision
                        (tmp_tsts_x, tmp_tsts_y, bullet))
                      {
                        return FALSE;
                      }

                    if (satellites_shot_collisions
                        (tmp_tsts_x, tmp_tsts_y, bullet))
                      {
                        /* shoot which has just touched satellite is removed */
                        return FALSE;
                      }

                    if (guns_shot_collisions (tmp_tsts_x, tmp_tsts_y, bullet))
                      {
                        /* shoot which has just touched extra gun is removed */
                        return FALSE;
                      }
                  }
              }
          }
      }
      break;

      /*
       * trajectory calculated (missile homing head)
       */
    case 1:
      {
        /* specify sprite image functions of angle */
        if (sign (bullet->angle) < 0)
          {
            bullet->img_angle =
              (Sint16) ((bullet->angle + TWO_PI) / PI_BY_16);
          }
        else
          {
            bullet->img_angle = (Sint16) (bullet->angle / PI_BY_16);
          }
        /* avoid negative indexes */
        bullet->img_angle = (Sint16) abs (bullet->img_angle);
        /* avoid a shot angle higher than the number of images */
        if (bullet->img_angle >= bullet->spr.numof_images)
          {
            bullet->img_angle = (Sint16) (bullet->spr.numof_images - 1);
          }
        /* save current angle for the calculation of the next angle */
        bullet->img_old_angle = bullet->img_angle;
        /* draw the shot sprite */
        draw_sprite (bullet->spr.img[bullet->img_angle],
                     (Uint32) bullet->spr.xcoord,
                     (Uint32) bullet->spr.ycoord);

        /* trajectory calculated: collisions spaceship shots and enemies */
        if (bullet->spr.type == FRIEND)
          {
            if (!shot_enemies_collisions (bullet))
              {
                return FALSE;
              }
          }
        if (!player_pause && menu_status == MENU_OFF)
          {
            /* update x and y coordinates */
            bullet->spr.xcoord +=
              bullet->spr.img[bullet->img_old_angle]->x_gc;
            bullet->spr.ycoord +=
              bullet->spr.img[bullet->img_old_angle]->y_gc;
          }
      }
      break;
    }
  /* decrease the lifetime of the shot */
  if (!player_pause && menu_status == MENU_OFF)
    {
      bullet->timelife--;
    }
  return TRUE;
}


/**
 * Add a new shot (type bullet) in guardian phase
 * @param guard Pointer to enemy guardian
 * @param cannon Cannon number from 0 to n 
 * @param power Power of the destruction 
 * @param speed Speed of the displacement
 */
shot_struct *
shot_guardian_add (const enemy * const guard, Uint32 cannon, Sint16 power,
                   float speed)
{
  Sint32 i;
  shot_struct *bullet;
  /* verify if it is possible to add a new shot to the list */
  if (num_of_shots > (MAX_OF_SHOTS - 2))
    {
      return NULL;
    }

  /* get a new shot element */
  bullet = shot_get ();
  if (bullet == NULL)
    {
      return NULL;
    }
  bullet->is_blinking = TRUE;
  bullet->spr.type = ENEMY;
  bullet->spr.trajectory = FALSE;
  bullet->spr.numof_images = 32;
  /* set power of the destruction */
  bullet->spr.pow_of_dest = power;
  for (i = 0; i < bullet->spr.numof_images; i++)
    {
      bullet->spr.img[i] = (image *) & fire[TIR1P3E][i];
    }
  bullet->spr.energy_level = bullet->spr.pow_of_dest;
  bullet->spr.current_image = 0;
  bullet->spr.anim_speed = 1;
  bullet->spr.anim_count = 0;
  bullet->img_angle =
    guard->spr.img[guard->spr.current_image]->cannons_angles[cannon];
  bullet->img_old_angle = bullet->img_angle;
  bullet->spr.xcoord =
    guard->spr.xcoord +
    guard->spr.img[guard->spr.current_image]->cannons_coords[cannon][XCOORD] -
    bullet->spr.img[bullet->img_angle]->x_gc;
  bullet->spr.ycoord =
    guard->spr.ycoord +
    guard->spr.img[guard->spr.current_image]->cannons_coords[cannon][YCOORD] -
    bullet->spr.img[bullet->img_angle]->y_gc;
  bullet->timelife = 400;
  bullet->angle = (float) (PI_BY_16 * bullet->img_angle);
  /* set speed of the displacement */
  bullet->spr.speed = speed;
  return bullet;
}

/**
 * Add a new shot (type bullet)
 * @param foe pointer to enemy vessel
 * @param k
 */
void
shot_enemy_add (const enemy * const foe, Sint32 k)
{
  Sint32 j;
  shot_struct *bullet;

  /* verify if it is possible to add a new shot to the list */
  if (num_of_shots > (MAX_OF_SHOTS - 2))
    {
      return;
    }

  /* get a new shot element */
  bullet = shot_get ();
  if (bullet == NULL)
    {
      return;
    }
#ifdef USE_SDLMIXER
  sound_play (SOUND_ENEMY_FIRE_1 + (global_counter & 1));
#endif
  /* animated sprite (flicker shot) */
  bullet->is_blinking = TRUE;
  /* indicate that is ennemy sprite */
  bullet->spr.type = ENEMY;
  /* fixed trajectory */
  bullet->spr.trajectory = FALSE;
  /* set number of images of the sprite */
  bullet->spr.numof_images = 32;
  /* check type of enemy to set the power of the destruction */
  switch (foe->type)
    {
      /* size of the enemy sprite as 16x16 pixels */
    case 0:
      /* quibouly is used by guardian 7 */
    case QUIBOULY:
      /* set power of the destruction */
      bullet->spr.pow_of_dest = 2;
      /* set addresses of the sprites images buffer */
      for (j = 0; j < bullet->spr.numof_images; j++)
        {
          /* shot 2 force 1 */
          bullet->spr.img[j] = (image *) & fire[TIR1P1E][j];
        }
      break;
      /* size of the enemy sprite as 32x32 pixels or lonely foe */
    default:
      /* set power of the destruction */
      bullet->spr.pow_of_dest = 4;
      /* set addresses of the sprites images buffer */
      for (j = 0; j < bullet->spr.numof_images; j++)
        {
          /* shot 2 force 2 */
          bullet->spr.img[j] = (image *) & fire[TIR1P2E][j];
        }
      break;
    }
  bullet->spr.energy_level = bullet->spr.pow_of_dest;
  /* set current image */
  bullet->spr.current_image = 0;
  /* value of delay between two images */
  bullet->spr.anim_speed = 1;
  /* counter of delay between two images */
  bullet->spr.anim_count = 0;
  bullet->img_angle = foe->spr.img[foe->spr.current_image]->cannons_angles[k];
  bullet->img_old_angle = bullet->img_angle;
  /* set x and y coordinates */
  bullet->spr.xcoord =
    foe->spr.xcoord +
    foe->spr.img[foe->spr.current_image]->cannons_coords[k][XCOORD] -
    bullet->spr.img[bullet->img_angle]->x_gc;
  bullet->spr.ycoord =
    foe->spr.ycoord +
    foe->spr.img[foe->spr.current_image]->cannons_coords[k][YCOORD] -
    bullet->spr.img[bullet->img_angle]->y_gc;
  bullet->timelife = 400;
  /* set angle of the projectile */
  bullet->angle = PI_BY_16 * bullet->img_angle;
  /* set speed of the displacement */
  bullet->spr.speed = 1.0f + (float) num_level / 20.0f;
}

/**
 * Check validty of shots chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
shot_check_chained_list (void)
{
  Uint32 i;
  shot_struct *bullet;
  Sint32 count = 0;
  for (i = 0; i < MAX_OF_SHOTS; i++)
    {
      bullet = &shots[i];
      if (bullet->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_shots)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, num_of_shots=%i", count, num_of_shots);
    }
  count = 0;
  bullet = shot_first;
  do
    {
      count++;
      bullet = bullet->next;
    }
  while (bullet != NULL && count <= (MAX_OF_SHOTS + 1));
  if (count != num_of_shots)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_shots=%i", count, num_of_shots);
    }
  count = 0;
  bullet = shot_last;
  do
    {
      count++;
      bullet = bullet->previous;
    }
  while (bullet != NULL && count <= (MAX_OF_SHOTS + 1));
  if (count != num_of_shots)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_shots=%i", count, num_of_shots);
    }
}
#endif

/** 
 * Return a free shot element 
 * @return Pointer to a shot structure 
 */
shot_struct *
shot_get (void)
{
  Uint32 i;
  shot_struct *bullet;
  for (i = 0; i < MAX_OF_SHOTS; i++)
    {
      bullet = &shots[i];
      if (bullet->is_enabled)
        {
          continue;
        }
      bullet->is_enabled = TRUE;
      bullet->next = NULL;
      if (num_of_shots == 0)
        {
          shot_first = bullet;
          shot_last = bullet;
          shot_last->previous = NULL;
        }
      else
        {
          shot_last->next = bullet;
          bullet->previous = shot_last;
          shot_last = bullet;
        }
      num_of_shots++;
#ifdef UNDER_DEVELOPMENT
      shot_check_chained_list ();
#endif
      return bullet;
    }
  LOG_ERR ("no more element shot is available");
  return NULL;
}

/** 
 * Remove one shot of the list of shots
 * @param Pointer to a shot structure 
 */
static void
shot_delete (shot_struct * bullet)
{
  bullet->is_enabled = FALSE;
  num_of_shots--;
  if (shot_first == bullet)
    {
      shot_first = bullet->next;
    }
  if (shot_last == bullet)
    {
      shot_last = bullet->previous;
    }
  if (bullet->previous != NULL)
    {
      bullet->previous->next = bullet->next;
    }
  if (bullet->next != NULL)
    {
      bullet->next->previous = bullet->previous;
    }
}

/**
 * Add a shot from a satellite from player's spaceship
 * this part is used normally never) 
 * @param Cannon_pos Cannon's position from 0 to 12
 * @param sat Pointer to a satellite_struct
 */
void
shot_satellite_add (Sint32 xcoord, Sint32 ycoord, Sint16 img_angle)
{
  Sint32 i;
  shot_struct *bullet;
  bullet = shot_get ();
  if (bullet == NULL)
    {
      return;
    }
  /* animated sprite (flicker shot) */
  bullet->is_blinking = TRUE;
  /* indicate that is friend sprite */
  bullet->spr.type = FRIEND;
  /* fixed trajectory */
  bullet->spr.trajectory = 0;
  /* set number of images of the sprite */
  bullet->spr.numof_images = 32;
  /* set power of the destruction */
  bullet->spr.pow_of_dest = 1;
  /* set addresses of the images buffer */
  for (i = 0; i < bullet->spr.numof_images; i++)
    {
      /* shot 1 force 2 */
      bullet->spr.img[i] = (image *) & fire[V1TN1][i];
    }
  /* set energy level of the sprite */
  bullet->spr.energy_level = bullet->spr.pow_of_dest;
  /* set current image */
  bullet->spr.current_image = 0;
  /* value of delay between two images */
  bullet->spr.anim_speed = 4;
  /* counter of delay between two images */
  bullet->spr.anim_count = 0;
  bullet->img_angle = img_angle;
  bullet->img_old_angle = bullet->img_angle;
  /* set x and y coordinates */
  bullet->spr.xcoord =
    (float) (xcoord - bullet->spr.img[bullet->img_angle]->x_gc);
  bullet->spr.ycoord =
    (float) (ycoord - bullet->spr.img[bullet->img_angle]->y_gc);
  bullet->timelife = 400;
  /* set angle of the projectile */
  bullet->angle = PI_BY_16 * bullet->img_angle;
  /* set speed of the displacement */
  bullet->spr.speed = 9.0;
}

/**
 * Add a shot from the player's spaceship
 * @param damage Damage caused by the shot 1 to 18
 * @param anim_speed Delay before next image
 * @param image_num Bitmap image used
 * @param angle Angle 0 to 31
 * @param Cannon_pos Cannon's position from 0 to 12
 */
static shot_struct *
shot_spaceship_add (Sint16 damage, Sint16 anim_speed,
                    Sint32 image_num, Sint16 angle, Sint32 cannon_pos)
{
  Sint32 j;
  sprite *spr;
  shot_struct *bullet;
  spaceship_struct *ship = spaceship_get ();
  bullet = shot_get ();
  if (bullet == NULL)
    {
      return NULL;
    }
  spr = &bullet->spr;
  /* animated sprite (flicker shot) */
  bullet->is_blinking = TRUE;
  /* indicate that is friend sprite */
  spr->type = FRIEND;
  /* damage done by the shot */
  spr->pow_of_dest = damage;
  /* life energy level */
  spr->energy_level = spr->pow_of_dest;
  /* set number of images of the sprite */
  spr->numof_images = 32;
  /* delay before next image */
  spr->anim_speed = anim_speed;
  /* counter delay before next image */
  spr->anim_count = 0;
  /* set addresses of the sprites images buffer */
  for (j = 0; j < spr->numof_images; j++)
    {
      spr->img[j] = (image *) & fire[image_num][j];
    }
  bullet->img_angle = angle;
  bullet->img_old_angle = bullet->img_angle;
  /* set x and y coordinates */
  spr->xcoord =
    ship->spr.xcoord +
    ship->spr.img[ship->spr.
                  current_image]->cannons_coords[cannon_pos][XCOORD] -
    spr->img[bullet->img_angle]->x_gc;
  spr->ycoord =
    ship->spr.ycoord +
    ship->spr.img[ship->spr.
                  current_image]->cannons_coords[cannon_pos][YCOORD] -
    spr->img[bullet->img_angle]->y_gc;
  bullet->timelife = 500;
  return bullet;
}

/*
 * Add a linear shot from the player's spaceship
 * @param damage damage caused by the shot 1 to 18
 * @param anim_speed delay before next image 
 * @param image_num bitmap image number used
 * @param angle 0 to 31
 * @param cannon_pos 0 to 12
 * @param speed speed of the displacement  
 */
void
shot_linear_spaceship_add (Sint16 damage, Sint16 anim_speed, Sint32 image_num,
                           Sint16 angle, Sint32 cannon_pos, float speed)
{
  shot_struct *bullet;
  bullet =
    shot_spaceship_add (damage, anim_speed, image_num, angle, cannon_pos);
  if (bullet == NULL)
    {
      return;
    }
  /* linear trajectory */
  bullet->spr.trajectory = 0;
  bullet->spr.speed = speed;
}

/**
 * Add a calculates shot from the player's spaceship
 * @param damage damage caused by the shot 1 to 18
 * @param image_num bitmap image number used
 * @param angle 0 to 31
 * @param cannon_pos 0 to 12
 * @param shot_angle
 */
void
shot_calulated_spaceship_add (Sint16 damage, Sint32 image_num, Sint16 angle,
                              Sint32 cannon_pos, float shot_angle)
{
  shot_struct *bullet;
  bullet = shot_spaceship_add (damage, 0, image_num, angle, cannon_pos);
  if (bullet == NULL)
    {
      return;
    }
  /* trajectory calculated */
  bullet->spr.trajectory = 1;
  bullet->spr.speed = 4.0;
  bullet->angle = shot_angle;
  bullet->velocity = 0.04f;
}


/**
 * Add a shot follow a curve from the player's spaceship
 * @param damage damage caused by the shot 1 to 18
 * @param anim_speed delay before next image 
 * @pamra image_num bitmap image number used
 * @param angle 0 to 31
 * @param cannon_pos 0 to 12
 * @param curve_num curve number follow by the shot 
 */
void
shot_curve_spaceship_add (Sint16 damage, Sint16 anim_speed, Sint32 image_num,
                          Sint16 angle, Sint32 cannon_pos, Sint16 curve_num)
{
  shot_struct *bullet;
  bullet =
    shot_spaceship_add (damage, anim_speed, image_num, angle, cannon_pos);
  if (bullet == NULL)
    {
      return;
    }
  /* shot trajectory follow a curve */
  bullet->spr.trajectory = 2;
  /* set curve number used */
  bullet->curve_num = curve_num;
  /* clear index on the precalculated curve */
  bullet->curve_index = 0;
}

/**
 * Collisions between enemies and a spaceship's shot
 * @param bullet pointer to the structure of an shot
 * @return FALSE if the shot touched a enemy
 */
static bool
shot_enemies_collisions (shot_struct * bullet)
{
  Sint32 i, j, k, x1, y1, x2, y2;
  image *foe_img, *bullet_img;
  enemy *foe;
  bullet_img = bullet->spr.img[bullet->img_angle];

  foe = enemy_get_first ();
  if (foe == NULL)
    {
      return TRUE;
    }

  /* for each enemy */
  for (i = 0; i < num_of_enemies; i++, foe = foe->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (foe == NULL && i < (num_of_enemies - 1))
        {
          LOG_ERR ("foe->next is null %i/%i", i, num_of_enemies);
          break;
        }
#endif

      /* ignore collision, if enemy's not visible, dead, or guardian appearing */
      if (!foe->visible || foe->dead ||
          (guardian->is_appearing
           && foe->displacement == DISPLACEMENT_GUARDIAN))
        {
          continue;
        }
      foe_img = foe->spr.img[foe->spr.current_image];

      /* for each collision point of the shot */
      for (j = 0; j < bullet_img->numof_collisions_points; j++)
        {
          x1 =
            (Sint32) bullet->spr.xcoord +
            bullet_img->collisions_points[j][XCOORD];
          y1 =
            (Sint32) bullet->spr.ycoord +
            bullet_img->collisions_points[j][YCOORD];

          /* for each collision zone of the enemy */
          for (k = 0; k < foe_img->numof_collisions_zones; k++)
            {
              x2 =
                (Sint32) foe->spr.xcoord +
                foe_img->collisions_coords[k][XCOORD];
              y2 =
                (Sint32) foe->spr.ycoord +
                foe_img->collisions_coords[k][YCOORD];

              /* test if the point of collision of the shot
               * is inside the zone of collision of the enemy? */
              if (x1 < x2
                  || x1 >= (x2 + foe_img->collisions_sizes[k][IMAGE_WIDTH])
                  || y1 < y2
                  || y1 >= y2 + foe_img->collisions_sizes[k][IMAGE_HEIGHT])
                {
                  continue;
                }

              /* decrease in the level of energy  */
              foe->spr.energy_level =
                (Sint16) (foe->spr.energy_level - bullet->spr.pow_of_dest);

              /* enemy destroyed */
              if (foe->spr.energy_level <= 0)
                {
                  /* add bonus gem or penalty */
                  if (foe->type >= BIGMETEOR)
                    {
                      bonus_meteor_add (foe);
                    }
                  else
                    {
                      bonus_add (foe);
                    }

                  /*  lonely foes or guardian? */
                  if (foe->displacement == DISPLACEMENT_LONELY_FOE
                      || foe->displacement == DISPLACEMENT_GUARDIAN)
                    {
                      explosions_add_serie (foe);
                      explosions_fragments_add (foe->spr.xcoord +
                                                foe_img->x_gc - 8,
                                                foe->spr.ycoord +
                                                foe_img->y_gc - 8, 1.0, 5, 0,
                                                2);
                    }
                  else
                    {
                      explosion_add (foe->spr.xcoord,
                                     foe->spr.ycoord, 0.25, foe->type, 0);
                      explosions_fragments_add (foe->spr.xcoord +
                                                foe_img->x_gc - 8,
                                                foe->spr.ycoord +
                                                foe_img->y_gc - 8, 1.0, 3, 0,
                                                1);
                    }
                  /* update player's score */
                  player_score +=
                    foe->spr.pow_of_dest << 2 << score_multiplier;
                  enemy_set_fadeout (foe);
                }
              else
                {
                  /* enemy not destroyed, display white mask */
                  foe->is_white_mask_displayed = TRUE;
                  /* if guardian, updates guardian's energy gauge */
                  if (foe->displacement == DISPLACEMENT_GUARDIAN)
                    {
                      energy_gauge_guard_is_update = TRUE;
                    }
                  if (foe->type != NAGGYS)
                    {
                      /* enemy is riled up, accelerate it rate of fire */
                      foe->fire_rate_count = foe->fire_rate_count >> 1;
                    }
                }
              explosion_add (bullet->spr.xcoord,
                             bullet->spr.ycoord, 0.35f, EXPLOSION_SMALL, 0);
              /* remove the shot which has just touched the enemy */
              return FALSE;
            }
        }
    }
  return TRUE;
}

/**
 * Move a shoot horizontally
 * @param angle
 * @param speed
 * @param xcoord
 * @return New x-coordinate
 */
float
shot_x_move (float angle, float speed, float xcoord)
{
  xcoord += (float) (cos (angle)) * speed;
  return xcoord;
}

/*
 * Move a shoot vertically
 * @param angle
 * @param speed
 * @param ycoord
 * @return New y-coordinate
 */
float
shot_y_move (float angle, float speed, float ycoord)
{
  ycoord += (float) (sin (angle)) * speed;
  return ycoord;
}
