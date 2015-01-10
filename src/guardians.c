/**
 * @file guardians.c 
 * @brief handle the guardians 
 * @created 1998-04-21
 * @date 2014-10-11 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: guardians.c,v 1.63 2012/08/25 19:18:32 gurumeditation Exp $
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
#include "config_file.h"
#include "congratulations.h"
#include "curve_phase.h"
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "bonus.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "grid_phase.h"
#include "guardians.h"
#include "images.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#include "meteors_phase.h"
#include "lonely_foes.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "starfield.h"
#include "texts.h"

/** Data structure for the sprites images of the guardian */
image gardi[GUARDIAN_MAX_OF_ANIMS][ENEMIES_SPECIAL_NUM_OF_IMAGES];
const Sint32 clip_gard10 = 16;
guardian_struct *guardian;

/**
 * Initialization guardian that is only run once
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
guardians_once_init (void)
{
  meteors_free ();
  if (guardian == NULL)
    {
      guardian =
        (guardian_struct *) memory_allocation (sizeof (guardian_struct));
      if (guardian == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'guardian_struct'!");
          return FALSE;
        }
    }
  return TRUE;
}

/**
 * Release memory used by the current guardian
 */
static void
guardian_images_free (void)
{
  Uint32 i, j;
  for (i = 0; i < GUARDIAN_MAX_OF_ANIMS; i++)
    {
      for (j = 0; j < ENEMIES_SPECIAL_NUM_OF_IMAGES; j++)
        {
          if (gardi[i][j].img != NULL)
            {
              free_memory (gardi[i][j].img);
              gardi[i][j].img = NULL;
            }
          if (gardi[i][j].compress != NULL)
            {
              free_memory (gardi[i][j].compress);
              gardi[i][j].compress = NULL;
            }
        }
    }
}

/**
 * Release memory used by the guardians
 */
void
guardians_free (void)
{
  guardian_images_free ();
  if (guardian != NULL)
    {
      free_memory ((char *) guardian);
      guardian = NULL;
    }
}

/**
 * Initialize a new guardian
 * @param type Guardian identifier
 * @param current_image Current image index
 * @param energy_level Energy level of guardian
 * @param anim_speed Time delay before next image 
 * @param fire_rate Counter of delay between two shots
 * @param speed Speed of the sprite
 */
static enemy *
guardian_init (Uint32 index, Uint32 type, Sint16 current_image,
               Uint32 energy_level, Sint16 anim_speed, Uint32 fire_rate,
               float speed)
{
  Sint32 i;
  enemy *guard;
  spaceship_struct *ship = spaceship_get ();
  guard = enemy_get ();
  if (guard == NULL)
    {
      return NULL;
    }
  guard->spr.pow_of_dest = (Sint16) ((ship->type << 1) + type);
  guard->spr.energy_level =
    (Sint16) ((ship->type << 2) + (guard->spr.pow_of_dest << 3) / 3 +
              energy_level);
  guard->spr.max_energy_level = guard->spr.energy_level;
  guard->spr.numof_images = 32;
  guard->spr.current_image = current_image;
  guard->spr.anim_count = 0;
  guard->spr.anim_speed = anim_speed;
  for (i = 0; i < guard->spr.numof_images; i++)
    {
      guard->spr.img[i] = (image *) & gardi[index][i];
    }
  guard->fire_rate = fire_rate;
  guard->fire_rate_count = guard->fire_rate;
  guard->displacement = DISPLACEMENT_GUARDIAN;
  guard->spr.speed = speed;
  guard->type = type;
  guard->dead = FALSE;
  guard->visible = TRUE;
  guardian->foe[index] = guard;
  return guard;
}

/**
 * Add a guardian displacement
 * @param direction Direction of this displacement
 * @param delay Time delay of this displacement
 * @param speed Speed of this displacement
 */
static void
guardian_add_move (Uint32 direction, Sint32 delay, Uint32 speed)
{
  guardian->move_direction[guardian->move_max] = direction;
  guardian->move_delay[guardian->move_max] = delay;
  guardian->move_speed[guardian->move_max++] = speed;
}

/**
 * Initialize a direction toward left 
 */
static void
guardian_set_direction_toward_left (void)
{
  guardian_add_move (GUARD_IMMOBILE, 100, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_LEFT, 400, 1);
}

/**
 * Initialize a direction toward right
 */
static void
guardian_set_direction_toward_right (void)
{
  guardian_add_move (GUARD_IMMOBILE, 100, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_RIGHT, 400, 1);
}

/**
 * Initialize a direction toward top 
 */
static void
guardian_set_direction_toward_top (void)
{
  guardian_add_move (GUARD_IMMOBILE, 100, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 400, 3);
}

/**
 * Initialize a direction toward bottom 
 */
static void
guardian_set_direction_toward_bottom (void)
{
  guardian_add_move (GUARD_IMMOBILE, 100, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_BOTTOM, 400, 3);
}

/**
 * Intialize guardian 01 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_01_init (void)
{
  enemy *guard = guardian_init (0, THANIKEE, 15, 30, 4, 85, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[17]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  return TRUE;
}

/**
 * Intialize guardian 02
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_02_init (void)
{
  enemy *guard = guardian_init (0, BARYBOOG, 15, 50, 4, 50, 1.0);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_bottom ();
  guardian_set_direction_toward_top ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_bottom ();
  guardian_set_direction_toward_top ();
  guardian_set_direction_toward_right ();
  guardian->x_min = (float) (offscreen_clipsize - 10);
  guardian->x_max
    = (float) (offscreen_clipsize + offscreen_width_visible + 40);
  guardian->y_min = (float) (offscreen_clipsize - 15);
  guardian->y_max
    = (float) (offscreen_clipsize + offscreen_height_visible + 40);
  return TRUE;
}

/**
 * Intialize guardian 03
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_03_init (void)
{
  enemy *guard = guardian_init (0, PIKKIOU, 15, 70, 4, 75, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->lonely_foe_delay = 16;
  return TRUE;
}

/**
 * Intialize guardian 04
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_04_init (void)
{
  enemy *guard = guardian_init (0, NEGDEIS, 15, 90, 4, 75, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_bottom ();
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 400, 3);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_bottom ();
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 400, 3);
  guardian_set_direction_toward_left ();
  /* clear time delay of launch of SAPOUCH */
  guardian->lonely_foe_delay = 0;
  guardian->devilians_counter = 0;
  guardian->devilians_enable = FALSE;
  guardian->devilians_delay = 0;
  /* generate Naggys to amuse player before the appearance of guardian */
  lonely_foe_add (NAGGYS);
  return TRUE;
}

/**
 * Intialize guardian 05
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_05_init (void)
{
  enemy *guard = guardian_init (0, FLASHY, 15, 110, 4, 80, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_bottom ();
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 400, 3);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_bottom ();
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 400, 3);
  guardian_set_direction_toward_left ();
  guardian->soukee_delay = 0;
  return TRUE;
}

/**
 * Intialize guardian 06
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_06_init (void)
{
  enemy *guard = guardian_init (0, MEECKY, 15, 130, 4, 80, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guard = guardian_init (1, MEECKY, 15, 0, 4, 80, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord = guardian->foe[0]->spr.xcoord;
  guard->spr.ycoord = guardian->foe[0]->spr.ycoord + 63;
  guard->spr.energy_level = (guard->spr.pow_of_dest << 3) / 3;
  guard->spr.max_energy_level = guard->spr.energy_level;
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->missile_delay = 0;
  guardian->x_min = (float) offscreen_clipsize - 16;
  guardian->x_max =
    (float) (offscreen_clipsize + offscreen_width_visible + 48);
  guardian->y_max =
    (float) (offscreen_clipsize + offscreen_height_visible + 40);
  guardian->y_inc = 1.0;
  return TRUE;
}

/**
 * Intialize guardian 07 (wheel with double reverse rotation)
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_07_init (void)
{
  enemy *guard = guardian_init (0, TYPYBOON, 0, 150, 4, 55, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[0]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[0]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->quibouly_delay = 0;
  if (num_of_enemies < (MAX_OF_ENEMIES - 2))
    {
      lonely_foe_add (NAGGYS);
    }
  return TRUE;
}

/**
 * Intialize guardian 08
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_08_init (void)
{
  enemy *guard = guardian_init (0, MATHYDEE, 0, 170, 4, 65, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[0]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[0]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->tournadee_delay = 0;
  if (num_of_enemies < (MAX_OF_ENEMIES - 2))
    {
      lonely_foe_add (NAGGYS);
    }
  return TRUE;
}

/**
 * Intialize guardian 09
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_09_init (void)
{
  enemy *guard = guardian_init (0, OVYDOON, 15, 190, 4, 60, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guard = guardian_init (1, OVYDOON, 15, 0, 4, 75, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.energy_level = (guard->spr.pow_of_dest * 3) / 2;
  guard->spr.max_energy_level = guard->spr.energy_level;
  guard->spr.xcoord = guardian->foe[0]->spr.xcoord;
  guard->spr.ycoord = guardian->foe[0]->spr.ycoord;
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->lonely_foe_delay = 0;
  if (num_of_enemies < (MAX_OF_ENEMIES - 2))
    {
      lonely_foe_add (NAGGYS);
    }
  return TRUE;
}

/**
 * Intialize guardian 10
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_10_init (void)
{
  enemy *guard = guardian_init (0, GATLEENY, 15, 210, 1, 50, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guard = guardian_init (1, GATLEENY, 8, 210, 20, 65, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord = guardian->foe[0]->spr.xcoord;
  guard->spr.ycoord = guardian->foe[0]->spr.ycoord;
  guardian->move_current = GUARD_MOVEMENT_TOWARD_BOTTOM;
  guardian->lonely_foe_delay = 0;
  if (num_of_enemies < (MAX_OF_ENEMIES - 2))
    {
      lonely_foe_add (NAGGYS);
    }

  return TRUE;
}

/**
 * Intialize guardian 11
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_11_init (void)
{
  enemy *guard = guardian_init (0, NAUTEE, 15, 230, 4, 60, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_set_direction_toward_right ();
  guardian_set_direction_toward_left ();
  guardian->missile_delay = 0;
  guardian->sapouch_delay = 0;
  if (num_of_enemies < (MAX_OF_ENEMIES - 2))
    {
      lonely_foe_add (NAGGYS);
    }
  return TRUE;
}

/**
 * Intialize guardian 12
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_12_init (void)
{
  enemy *guard = guardian_init (0, KAMEAMEA, 15, 250, 4, 60, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_add_move (GUARD_IMMOBILE, 100, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_LEFT, 50, 2);
  guardian_add_move (GUARD_IMMOBILE, 50, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_RIGHT, 50, 2);
  guardian_add_move (GUARD_IMMOBILE, 400, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_BOTTOM, 500, 4);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_TOP, 500, 4);
  guardian_add_move (GUARD_IMMOBILE, 50, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_RIGHT, 50, 2);
  guardian_add_move (GUARD_IMMOBILE, 50, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_LEFT, 50, 2);
  guardian->y_min = (float) (offscreen_clipsize);
  guardian->y_max
    = (float) (offscreen_clipsize + offscreen_height_visible + 30);
  return TRUE;
}

/**
 * Initialize guardian 13
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_13_init (void)
{
  enemy *guard;
  guard = guardian_init (0, SUPRALIS, 8, 250, 4, 60, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian->y_inc = 2.0;
  guardian_add_move (GUARD_IMMOBILE, 10, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_RIGHT, 400, 3);
  guardian_add_move (GUARD_IMMOBILE, 10, 0);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_LEFT, 400, 3);
  guardian->x_min = (float) (offscreen_clipsize - 20);
  guardian->x_max
    = (float) (offscreen_clipsize + offscreen_width_visible + 20);
  guardian->y_min = (float) (offscreen_clipsize);
  guardian->y_max = (float) (offscreen_clipsize + offscreen_height_visible);
  return TRUE;
}

/**
 * Intialize guardian 14, guardian with long trunk
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
guardian_14_init (void)
{
  enemy *guard;
  guard = guardian_init (0, GHOTTEN, 31, 250, 4, 60, 0.5);
  if (guard == NULL)
    {
      return FALSE;
    }
  guard->spr.xcoord =
    (float) (offscreen_width_visible - guard->spr.img[15]->w / 2);
  guard->spr.ycoord = (float) (offscreen_starty - guard->spr.img[15]->h);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_RIGHT, 400, 2);
  guardian_add_move (GUARD_MOVEMENT_TOWARD_LEFT, 400, 2);
  guardian->x_min = (float) (offscreen_clipsize + 10);
  guardian->x_max
    = (float) (offscreen_clipsize + offscreen_width_visible - 10);
  guardian->perturbians_delay = 0;
  guardian->saakamin_delay = 0;
  return TRUE;
}

/** 
 *  Meteors storm finished: initialize a new guardian 
 *  @param guardian_num number of the guardian 1 to 14, 15=congratulations
 *  @return TRUE if it completed successfully or FALSE otherwise
 */
bool
guardian_new (Uint32 guardian_num)
{
  bool result = TRUE;
  guardian->number = guardian_num;
  guardian->is_appearing = TRUE;
  courbe.activity = FALSE;
  grid.is_enable = FALSE;
  meteor_activity = FALSE;
  energy_gauge_guard_is_update = TRUE;
  guardian->is_enabled = TRUE;
  guardian->current_images_set = 0;
  guardian->move_current = 0;
  guardian->move_max = 0;
  guardian->x_min = (float) offscreen_clipsize;
  guardian->x_max = (float) (offscreen_clipsize + offscreen_width_visible);
  guardian->y_min = (float) offscreen_clipsize;
  guardian->y_max = (float) (offscreen_clipsize + offscreen_height_visible);
  guardian->y_inc = 0;
  switch (guardian_num)
    {
    case 1:
      result = guardian_01_init ();
      break;
    case 2:
      result = guardian_02_init ();
      break;
    case 3:
      result = guardian_03_init ();
      break;
    case 4:
      result = guardian_04_init ();
      break;
    case 5:
      result = guardian_05_init ();
      break;
    case 6:
      result = guardian_06_init ();
      break;
    case 7:
      result = guardian_07_init ();
      break;
    case 8:
      result = guardian_08_init ();
      break;
    case 9:
      result = guardian_09_init ();
      break;
    case 10:
      result = guardian_10_init ();
      break;
    case 11:
      result = guardian_11_init ();
      break;
    case 12:
      result = guardian_12_init ();
      break;
    case 13:
      result = guardian_13_init ();
      break;
    case 14:
      result = guardian_14_init ();
      break;
    case 15:
      bonus_disable_all ();
      congratulations_initialize ();
      break;
    default:
      result = TRUE;
    }
  if (!result)
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * The guardian appears from the screen top
 * @param guard Pointer to a enemy structure
 */
static void
guardian_appears_from_top (enemy * guard)
{
  guard->spr.ycoord += guard->spr.speed;
  if (guard->spr.ycoord >= offscreen_clipsize)
    {
      guard->spr.ycoord = (float) offscreen_clipsize;
      guardian->is_appearing = FALSE;
      guardian->move_current = 0;
      guardian->move_time_delay =
        guardian->move_delay[guardian->move_current];
    }
}

/**
 * Add a new shot (type bullet) in guardian phase
 * @param guard Pointer to the enemy guardian
 * @param power Power of the destruction 
 * @param speed Speed of the displacement
 * @return Number of bullets fired
 */
static Uint32
guardian_fire (enemy * guard, Sint16 power, float speed)
{
  Sint32 cannon_num;
  Uint32 numof_bullets;
  if (player_pause || menu_status != MENU_OFF
      || menu_section != NO_SECTION_SELECTED)
    {
      return 0;
    }
  guard->fire_rate_count--;
  if (guard->fire_rate_count > 0 || num_of_shots >= (MAX_OF_SHOTS - 1))
    {
      return 0;
    }
  guard->fire_rate_count = guard->fire_rate;
  numof_bullets = 0;
  for (cannon_num = 0;
       cannon_num < guard->spr.img[guard->spr.current_image]->numof_cannons;
       cannon_num++)
    {
      shot_guardian_add (guard, cannon_num, power, speed);
      numof_bullets++;
    }
#ifdef USE_SDLMIXER
  sound_play (SOUND_GUARDIAN_FIRE_1);
#endif
  return numof_bullets;
}

/**
 * Add one missile
 * @param type Type of foe 
 * @param current_image Current image index 
 * @param img_angle Angle of displacement
 * @param angle_tir 
 * @return Pointer to the new enemy element
 */
static enemy *
guardian_add_missile (Uint32 type, Sint16 current_image, Sint16 img_angle,
                      float angle_tir)
{
  Sint32 i;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  foe = enemy_get ();
  if (foe == NULL)
    {
      return NULL;
    }
  foe->spr.pow_of_dest = (Sint16) ((ship->type << 1) + 5);
  foe->spr.energy_level = foe->spr.pow_of_dest >> 2;
  foe->spr.numof_images = 32;
  foe->spr.current_image = current_image;
  foe->spr.anim_count = 0;
  foe->spr.anim_speed = 2;
  for (i = 0; i < foe->spr.numof_images; i++)
    {
      foe->spr.img[i] = (image *) & fire[MISSx4][i];
    }
  foe->fire_rate = 70;
  foe->fire_rate_count = foe->fire_rate;
  foe->displacement = DISPLACEMENT_LONELY_FOE;
  foe->spr.speed = 2.0f;
  foe->type = type;
  foe->dead = FALSE;
  foe->visible = TRUE;
  foe->img_angle = img_angle;
  foe->angle_tir = angle_tir;
  foe->img_old_angle = foe->img_angle;
  foe->agilite = 0.028f;
  return foe;
}

/**
 * Add one or more missiles homing head
 * @param guard Pointer the main enemy structure of the guardian
 * @param inc Counter step
 * @param max_counter Delay before send the missiles 
 * @param numof_miss Number of missiles to add from 1 to n
 * @return TRUE if at least one missile was added or FALSE otherwise
 */
static void
guardian_add_missiles (enemy * guard, Uint32 inc, Sint32 max_counter,
                       Uint32 numof_miss)
{
  Sint16 img_angle;
  float angle_tir;
  Uint32 i;
  enemy *foe;
  guardian->missile_delay += inc;
  while (guardian->missile_delay >= max_counter && !spaceship_is_dead)
    {
      guardian->missile_delay -= max_counter;
      if (guardian->missile_delay < 0)
        {
          guardian->missile_delay = 0;
        }
      img_angle = 0;
      for (i = 0; i < numof_miss; i++)
        {
          if (img_angle == 16)
            {
              img_angle = 0;
              angle_tir = 0.0;
            }
          else
            {
              img_angle = 16;
              angle_tir = PI;
            }
          foe = guardian_add_missile (SOUKEE, 16, img_angle, angle_tir);
          if (foe == NULL)
            {
              return;
            }
          foe->spr.xcoord =
            guard->spr.xcoord +
            guard->spr.img[guard->spr.current_image]->
            cannons_coords[i][XCOORD] - foe->spr.img[foe->img_angle]->x_gc;
          foe->spr.ycoord =
            guard->spr.ycoord +
            guard->spr.img[guard->spr.current_image]->
            cannons_coords[i][YCOORD] - foe->spr.img[foe->img_angle]->y_gc;
#ifdef USE_SDLMIXER
          if (img_angle == 16)
            {
              sound_play (SOUND_GUARDIAN_FIRE_2);
            }
          else
            {
              sound_play (SOUND_GUARDIAN_FIRE_3);
            }
#endif
        }
    }
}

/**
 * Add an enemy who is specific in the guardian
 * @param type Type of foe 
 * @param pow_of_des Power of destruction
 * @param energy_level Energy level  (<= 0 destroyed foe)
 * @param current_image Current image index 
 * @param anim_speed Time delay before next image (animation speed)
 * @param fire_rate Counter of delay between two shots
 * @param speed Speed of the sprite
 * @return Pointer to the new enemy element
 */
static enemy *
guardian_add_foe (Uint32 type, Sint16 pow_of_des, Sint16 energy_level,
                  Sint16 current_image, Sint16 anim_speed, Uint32 fire_rate,
                  float speed)
{
  Sint32 i;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  foe = enemy_get ();
  if (foe == NULL)
    {
      return NULL;
    }
  foe->spr.pow_of_dest = (Sint16) ((ship->type << 1) + pow_of_des);
  foe->spr.energy_level = energy_level;
  foe->spr.numof_images = 32;
  foe->spr.current_image = current_image;
  foe->spr.anim_count = 0;
  foe->spr.anim_speed = anim_speed;
  for (i = 0; i < foe->spr.numof_images; i++)
    {
      foe->spr.img[i] = (image *) & enemi[type][i];
    }
  foe->fire_rate = fire_rate;
  foe->fire_rate_count = foe->fire_rate;
  foe->displacement = DISPLACEMENT_LONELY_FOE;
  foe->spr.speed = speed;
  foe->type = type;
  foe->dead = FALSE;
  foe->visible = TRUE;
  return foe;
}

/**
 * Add a lonely foe to the enemies list
 * @param max_counter delay before send a lonely foe
 * @param foe_num Foe number of -1 if foe is automatically selected  
 */
static void
guardian_add_lonely_foe (Sint32 max_counter, Sint32 foe_num)
{
  guardian->lonely_foe_delay++;
  if (guardian->lonely_foe_delay < max_counter)
    {
      return;
    }
  lonely_foe_add (foe_num);
#ifdef USE_SDLMIXER
  sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
  guardian->lonely_foe_delay = 0;
}

/**
 * Add a Soukee foe to the enemies list
 * @param guard Pointer the main enemy structure of the guardian
 * @param inc Counter step
 * @param max_counter Delay before send a Soukee 
 */
static void
guardian_add_soukee (enemy * guard, Sint32 inc, Sint32 max_counter)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->soukee_delay += inc;
  while (guardian->soukee_delay >= max_counter && !spaceship_is_dead)
    {
      guardian->soukee_delay -= max_counter;
      if (guardian->soukee_delay < 0)
        {
          guardian->soukee_delay = 0;
        }
      foe =
        guardian_add_foe (SOUKEE, 14, (ship->type << 1) + 14, 8, 2, 70, 2.0f);
      if (foe == NULL)
        {
          return;
        }
      foe->spr.xcoord =
        guard->spr.xcoord + guard->spr.img[guard->spr.current_image]->x_gc -
        foe->spr.img[0]->w / 2;
      foe->spr.ycoord =
        guard->spr.ycoord + guard->spr.img[guard->spr.current_image]->y_gc -
        foe->spr.img[0]->h / 2;
      foe->img_angle = 8;
      foe->angle_tir = HALF_PI;
      foe->img_old_angle = foe->img_angle;
      foe->agilite = 0.018f;
#ifdef USE_SDLMIXER
      sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
    }
}

/**
 * Add a Shuriky foe to the enemies list
 * @param guard Pointer the main enemy structure of the guardian
 * @param max_counter Delay before send a Shuriky 
 */
void
guardian_add_shuriky (enemy * guard, Sint32 max_counter)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->shuriky_delay++;
  if (guardian->shuriky_delay < max_counter)
    {
      return;
    }
  guardian->shuriky_delay = 0;
  foe =
    guardian_add_foe (SHURIKY, 10, (ship->type << 1) + 10, 0, 2, 70, 0.3f);
  if (foe == NULL)
    {
      return;
    }
  foe->spr.xcoord =
    guard->spr.xcoord + guard->spr.img[guard->spr.current_image]->x_gc -
    foe->spr.img[0]->w / 2;
  foe->spr.ycoord =
    guard->spr.ycoord + guard->spr.img[guard->spr.current_image]->y_gc -
    foe->spr.img[0]->h / 2;
#ifdef USE_SDLMIXER
  sound_play (SOUND_GUARDIAN_FIRE_3);
#endif
}

/**
 * Add a Devilians foe to the enemies list
 * @param max_counter Delay before send a Quibouly 
 * @param max_foes Number of foes launched before disable
 */
static void
guardian_add_devilians (Sint32 max_counter, Sint32 max_foes)
{
  if (!guardian->devilians_enable)
    {
      return;
    }
  guardian->devilians_delay++;
  if (guardian->devilians_delay < max_counter)
    {
      return;
    }
  lonely_foe_add (DEVILIANS);
  guardian->devilians_delay = 0;
  guardian->devilians_counter++;
  if (guardian->devilians_counter >= max_foes)
    {
      guardian->devilians_counter = 0;
      guardian->devilians_enable = FALSE;
    }
}

/**
 * Add a Quibouly foe to the enemies list
 * @param guard Pointer the main enemy structure of the guardian
 * @param max_counter Delay before send a Quibouly 
 */
void
guardian_add_quibouly (enemy * guard, Sint32 max_counter)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->quibouly_delay++;
  if (guardian->quibouly_delay < max_counter)
    {
      return;
    }
  guardian->quibouly_delay = 0;
  foe =
    guardian_add_foe (QUIBOULY, 10, QUIBOULY + (ship->type << 1) + 10, 0, 2,
                      50, 0.25f);
  if (foe == NULL)
    {
      return;
    }
#ifdef USE_SDLMIXER
  sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
  foe->spr.xcoord =
    guard->spr.xcoord + guard->spr.img[guard->spr.current_image]->x_gc -
    foe->spr.img[0]->w / 2;
  foe->spr.ycoord = guard->spr.ycoord + 96;
}

/**
 * Add a Shuriky foe to the enemies list
 * @param max_counter Delay before send a Shuriky 
 */
static void
guardian_add_tournadee (Sint32 max_counter)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->tournadee_delay++;
  if (guardian->tournadee_delay < max_counter)
    {
      return;
    }
  guardian->tournadee_delay = 0;
  foe =
    guardian_add_foe (TOURNADEE, 10, TOURNADEE + (ship->type << 1) + 10, 0, 3,
                      20, -0.2f);
  if (foe == NULL)
    {
      return;
    }
  if (guardian->is_tournadee_left_pos)
    {
      guardian->is_tournadee_left_pos = FALSE;
      foe->spr.xcoord = (float) (offscreen_startx - 2);
    }
  else
    {
      guardian->is_tournadee_left_pos = TRUE;
      foe->spr.xcoord =
        (float) (offscreen_startx + offscreen_width_visible -
                 foe->spr.img[0]->w + 2);
    }
  foe->spr.ycoord
    = (float) (offscreen_starty + 32 + offscreen_height_visible);
#ifdef USE_SDLMIXER
  sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
}

/**
 * Add a Sapouch foe to the enemies list
 * @param numof Number of Sapouch launched 1 or 2
 * @param max_counter Delay before send a Sapouch 
 * @param is_left TRUE if first Sapouch will be launched on left side
 */
static void
guardian_add_sapouch (Uint32 numof, Sint32 max_counter, bool is_left)
{
  float ycoord, speed;
  Uint32 i;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->sapouch_delay++;
  if (guardian->sapouch_delay < max_counter)
    {
      return;
    }
  guardian->sapouch_delay = 0;
  ycoord = (float) (offscreen_starty - 64);
  speed = 2.5f + (float) (((long) rand () % (100))) / 100.0f;
  for (i = 0; i < numof; i++)
    {
      foe =
        guardian_add_foe (SAPOUCH, 10, SAPOUCH + (ship->type << 1) + 10, 0, 4,
                          50 + (Sint32) ((long) rand () % (50)), speed);
      if (foe == NULL)
        {
          return;
        }
      if (is_left)
        {
          foe->spr.xcoord = (float) (offscreen_startx);
        }
      else
        {
          foe->spr.xcoord =
            (float) (offscreen_startx +
                     offscreen_width_visible - foe->spr.img[0]->w);
        }
      foe->spr.ycoord = ycoord - foe->spr.img[0]->h;
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
#ifdef USE_SDLMIXER
      sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
      is_left = is_left ? FALSE : TRUE;
    }
}

/**
 * Add a Perturbians foe to the enemies list
 * @param guard Pointer the main enemy structure of the guardian
 * @param numof Number of Perturbians launched 1 or 2
 * @param max_counter Delay before send a Perturbians 
 * @param is_left TRUE if first Perturbians will be launched on left side
 */
static void
guardian_add_perturbians (Uint32 numof, Sint32 max_counter, bool is_left)
{
  Uint32 i, fire_rate, power;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->perturbians_delay++;
  if (guardian->perturbians_delay < max_counter)
    {
      return;
    }
  guardian->perturbians_delay = 0;
  fire_rate = 200 + (Sint32) ((long) rand () % (50));
  power = (Sint32) ((ship->type << 1) + PERTURBIANS - 40);
  for (i = 0; i < numof; i++)
    {
      foe =
        guardian_add_foe (PERTURBIANS,
                          (Sint16) ((ship->type << 1) + PERTURBIANS - 40),
                          (Sint16) ((ship->type << 2) + (power << 3) / 3 +
                                    10), 0, 6, fire_rate, 0.2f);
      if (foe == NULL)
        {
          return;
        }
      if (is_left)
        {
          foe->spr.xcoord = (float) (offscreen_startx);
        }
      else
        {
          foe->spr.xcoord =
            (float) (offscreen_startx + offscreen_width_visible
                     - foe->spr.img[0]->w);
        }
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
#ifdef USE_SDLMIXER
      sound_play (SOUND_GUARDIAN_FIRE_2);
#endif
      is_left = is_left ? FALSE : TRUE;
    }
}

/**
 * Add a Perturbians foe to the enemies list
 * @param guard Pointer the main enemy structure of the guardian
 * @param max_counter Delay before send a Perturbians 
 */
static void
guardian_add_saakamin (enemy * guard, Sint32 max_counter, Uint32 numof)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  guardian->saakamin_delay++;
  if (guardian->saakamin_delay < max_counter)
    {
      return;
    }
  guardian->saakamin_delay = 0;
  while (--numof > 0)
    {
      foe =
        guardian_add_foe (SAAKAMIN,
                          14, (ship->type << 1) + 14, 8, 2, 70, 2.0f);
      if (foe == NULL)
        {
          return;
        }
      foe->num_courbe = (Sint16) (rand () % 121);
      foe->pos_vaiss[POS_CURVE] = 0;
      foe->spr.xcoord =
        (guard->spr.xcoord +
         guard->spr.img[guard->spr.current_image]->
         cannons_coords[0][XCOORD]) - foe->spr.img[0]->w / 2;
      foe->spr.ycoord =
        (guard->spr.ycoord +
         guard->spr.img[guard->spr.current_image]->
         cannons_coords[0][YCOORD]) - foe->spr.img[0]->h / 2;
    }
}

/**
 * Guadian go to next trajectory
 */
static void
guardian_next_trajectory (void)
{
  /* go to next trajectory */
  guardian->move_current++;
  /* maximum of displacements reached? */
  if (guardian->move_current >= guardian->move_max)
    {
      guardian->move_current = 0;
    }
  guardian->has_changed_direction = TRUE;
  guardian->move_time_delay = guardian->move_delay[guardian->move_current];
}

/**
 * Guadian get next trajectory identifier
 */
static Uint32
guardian_get_next_trajectory (void)
{
  Uint32 move = guardian->move_current + 1;
  if (move >= guardian->move_max)
    {
      move = 0;
    }
  return guardian->move_direction[move];
}

/**
 * Flip to next image. The animation is played once
 * @param guard Pointer the main enemy structure of the guardian
 * @param delay Time delay before next image
 * @param reach Image number to reach
 */
static void
guardian_flip_image (enemy * guard, Uint32 delay, Sint32 reach)
{
  guardian->anim_delay_count++;
  if (guardian->anim_delay_count < delay)
    {
      return;
    }
  guardian->anim_delay_count = 0;
  if (guard->spr.current_image == reach)
    {
      return;
    }
  if (guard->spr.current_image < reach)
    {
      guard->spr.current_image++;
    }
  else
    {
      guard->spr.current_image--;
    }
}

/**
 * Flip to next image. The animation is played in loop-mode 
 * @param guard Pointer the main enemy structure of the guardian
 * @param delay Time delay before next image
 * @param inc Step value 1 or -1
 * @param limit Image number of the last image
 * @param first Image number of the first image 
 */
static void
guardian_loop_flip (enemy * guard, Uint32 delay, Sint16 inc, Sint32 limit,
                    Sint16 first)
{
  guardian->anim_delay_count++;
  if (guardian->anim_delay_count < delay)
    {
      return;
    }
  guardian->anim_delay_count = 0;
  guard->spr.current_image = (Sint16) (guard->spr.current_image + inc);
  if ((inc > 0 && guard->spr.current_image > limit)
      || (inc < 0 && guard->spr.current_image < limit))
    {
      guard->spr.current_image = first;
    }
}

/**
 * Change images set of a guadian 
 * @param guard Pointer the main enemy structure of the guardian
 * @param images_num Images set number
 */
static void
guardian_change_images_set (enemy * guard, Uint32 images_num)
{
  Sint32 i;
  for (i = 0; i < guard->spr.numof_images; i++)
    {
      guard->spr.img[i] = (image *) & gardi[images_num][i];
    }
  guardian->current_images_set = images_num;
}

/**
 * Change images set of a guadian and the current image
 * @param guard Pointer the main enemy structure of the guardian
 * @param images_num Images set number
 * @param current Current image number
 */
static void
guardian_change_images (enemy * guard, Uint32 images_num, Sint16 current)
{
  guard->spr.current_image = current;
  guardian_change_images_set (guard, images_num);
}

/**
 * Move the guardian follow a straightline
 * @param guard Pointer to a enemy structure
 */
static void
guardian_line_moving (enemy * guard)
{
  Uint32 move, speed, dir;
  if (player_pause || menu_status != MENU_OFF
      || menu_section != NO_SECTION_SELECTED)
    {
      return;
    }
  move = guardian->move_current;
  speed = guardian->move_speed[move];
  dir = guardian->move_direction[move];
  /* update coordinates */
  guard->spr.xcoord += depix[speed][dir];
  guard->spr.ycoord += depiy[speed][dir];

  if (guard->spr.xcoord < guardian->x_min - EPS)
    {
      guard->spr.xcoord = guardian->x_min;
      guardian->move_time_delay = 0;
      guard->spr.ycoord += guardian->y_inc;
    }
  if (guard->spr.xcoord + guard->spr.img[guard->spr.current_image]->w >
      guardian->x_max + EPS)
    {
      guard->spr.xcoord =
        guardian->x_max - guard->spr.img[guard->spr.current_image]->w;
      guardian->move_time_delay = 0;
      guard->spr.ycoord += guardian->y_inc;
    }
  if (guard->spr.ycoord < guardian->y_min - EPS)
    {
      guard->spr.ycoord = guardian->y_min;
      guardian->move_time_delay = 0;
    }
  if (guard->spr.ycoord + guard->spr.img[guard->spr.current_image]->h >
      guardian->y_max + EPS)
    {
      guard->spr.ycoord =
        guardian->y_max - guard->spr.img[guard->spr.current_image]->h;
      guardian->move_time_delay = 0;
    }
}

/**
 * Move the guardian follow sinus curve 
 */
static void
guardian_move_sinus (enemy * guard)
{
  if (player_pause || menu_status != MENU_OFF
      || menu_section != NO_SECTION_SELECTED)
    {
      return;
    }

  /* move horizontally the guardian follow a straightline */
  guard->spr.xcoord +=
    depix[guardian->move_speed[guardian->move_current]][guardian->
                                                        move_direction
                                                        [guardian->
                                                         move_current]];
  guard->spr.ycoord += precalc_sin[guardian->move_time_delay & 0x001f] * 4;
  if (guard->spr.ycoord > (float) (offscreen_clipsize + 40 + EPS))
    {
      guard->spr.ycoord = (float) (offscreen_clipsize + 40);
    }
  if (guard->spr.xcoord < (float) offscreen_clipsize - EPS)
    {
      guard->spr.xcoord = (float) offscreen_clipsize;
      guardian->move_time_delay = 0;
    }
  if ((guard->spr.xcoord + guard->spr.img[guard->spr.current_image]->w) >
      (float) (offscreen_clipsize + offscreen_width_visible + EPS))
    {
      guard->spr.xcoord =
        (float) (offscreen_clipsize + offscreen_width_visible) -
        guard->spr.img[guard->spr.current_image]->w;
      guardian->move_time_delay = 0;
    }
}

/**
 * Handle the guardian 1 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_01 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] <
              GUARD_IMMOBILE)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
              guardian_flip_image (guard, 3, 31);
              break;

            case GUARD_MOVEMENT_TOWARD_LEFT:
              guardian_flip_image (guard, 3, 0);
              break;

            case GUARD_IMMOBILE:
              guardian_flip_image (guard, 3, 15);
              break;
            }
        }
      else
        {
          guardian_next_trajectory ();
        }
    }
  guardian_fire (guard, 8, 2.0);
}

/**
 * Handle the guardian 2 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_02 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guard->spr.ycoord += guard->spr.speed;
      if (guard->spr.ycoord >= (float) (offscreen_clipsize - 15))
        {
          guard->spr.ycoord = (float) (offscreen_clipsize - 15);
          guardian->is_appearing = FALSE;
          guardian->move_current = 0;
          guardian->move_time_delay =
            guardian->move_delay[guardian->move_current];
        }
    }
  else
    {
      /* next trajectory? */
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] <
              GUARD_IMMOBILE && !player_pause && menu_status == MENU_OFF
              && menu_section == NO_SECTION_SELECTED)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
              if (guardian->has_changed_direction)
                {
                  guardian_change_images (guard, 0, 15);
                  guardian->has_changed_direction = FALSE;
                }
              guardian_flip_image (guard, 4, 31);
              break;

            case GUARD_MOVEMENT_TOWARD_LEFT:
              if (guardian->has_changed_direction)
                {
                  guardian_change_images (guard, 0, 15);
                  guardian->has_changed_direction = FALSE;
                }
              /* flip to the previous image */
              guardian_flip_image (guard, 4, 0);
              break;

            case GUARD_MOVEMENT_TOWARD_TOP:
            case GUARD_MOVEMENT_TOWARD_BOTTOM:
              if (guardian->has_changed_direction)
                {
                  guardian_change_images (guard, 1, 0);
                  guardian->has_changed_direction = FALSE;
                }
              guardian_flip_image (guard, 2, 31);
              break;

            case GUARD_IMMOBILE:
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                  guardian_change_images_set (guard, 0);
                  if (guardian->is_vertical_trajectory)
                    {
                      guard->spr.current_image = 15;
                      guardian->is_vertical_trajectory = FALSE;
                    }
                }
              /* set image's guardian of central position */
              guardian_flip_image (guard, 3, 15);
              break;
            }
        }
      else
        {
          if (guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_BOTTOM
              || guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_TOP)
            {
              guardian->is_vertical_trajectory = TRUE;
            }
          guardian_next_trajectory ();
        }
    }
  if (guardian_fire (guard, 10, 3.0) > 0)
    {
      guardian_add_shuriky (guard, 4);
    }
}

/**
 * Handle the guardian 3 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_03 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] <
              GUARD_IMMOBILE)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
            case GUARD_MOVEMENT_TOWARD_LEFT:
            case GUARD_IMMOBILE:
              guardian_loop_flip (guard, 3, 1, 31, 0);
              break;
            }
        }
      else
        {
          guardian_next_trajectory ();
        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (16, NAGGYS);
    }
}

/**
 * Handle the guardian 4 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_04 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      guardian_add_devilians (141, 3);
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_RIGHT
              || guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_LEFT)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                  guardian_change_images (guard, 0, 15);
                }
              guardian_flip_image (guard, 4, 31);
              break;

            case GUARD_MOVEMENT_TOWARD_LEFT:
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                  guardian_change_images (guard, 0, 15);
                }
              guardian_flip_image (guard, 4, 0);
              break;

            case GUARD_MOVEMENT_TOWARD_BOTTOM:
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                  guardian_change_images (guard, 1, 0);
                  guardian->devilians_enable = TRUE;
                  guardian->devilians_counter = 0;
                }
              guardian_flip_image (guard, 2, 31);
              break;

            case GUARD_MOVEMENT_TOWARD_TOP:
              guardian_flip_image (guard, 2, 0);
              break;

            case GUARD_IMMOBILE:
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                  guardian_change_images_set (guard, 0);
                  if (guardian->is_vertical_trajectory)
                    {
                      guard->spr.current_image = 15;
                      guardian->is_vertical_trajectory = FALSE;
                    }
                }
              guardian_flip_image (guard, 3, 15);
              break;
            }
        }
      else
        {
          if (guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_BOTTOM
              || guardian->move_direction[guardian->move_current] ==
              GUARD_MOVEMENT_TOWARD_TOP)
            {
              guardian->is_vertical_trajectory = TRUE;
            }
          guardian_next_trajectory ();
        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (8, SAPOUCH);
    }
}

/**
 * Handle the guardian 5
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_05 (enemy * guard)
{
  Uint32 numof_bullets;
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              if (guardian->move_direction[guardian->move_current] ==
                  GUARD_MOVEMENT_TOWARD_RIGHT
                  || guardian->move_direction[guardian->move_current] ==
                  GUARD_MOVEMENT_TOWARD_LEFT)
                {
                  guardian_move_sinus (guard);
                }
              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 31);
                  break;

                case GUARD_MOVEMENT_TOWARD_LEFT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 0);
                  break;

                case GUARD_MOVEMENT_TOWARD_BOTTOM:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 1, 0);
                    }
                  guardian_loop_flip (guard, 2, 1, 31, 0);
                  break;

                case GUARD_IMMOBILE:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images_set (guard, 0);
                      if (guardian->is_vertical_trajectory)
                        {
                          guard->spr.current_image = 15;
                          guardian->is_vertical_trajectory = FALSE;
                        }
                    }
                  guardian_flip_image (guard, 3, 15);
                  break;
                }
            }
          else
            {
              if (guardian->move_direction[guardian->move_current] ==
                  GUARD_MOVEMENT_TOWARD_BOTTOM
                  || guardian->move_direction[guardian->move_current] ==
                  GUARD_MOVEMENT_TOWARD_TOP)
                {
                  guardian->is_vertical_trajectory = TRUE;
                }
              guardian_next_trajectory ();
            }
        }
    }

  numof_bullets = guardian_fire (guard, 8, 2.0);
  guardian_add_soukee (guard, numof_bullets, 16);
}

/**
 * Handle the guardian 6
 * Guardian with a articulated arm (2 sprites used)
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_06 (enemy * guard)
{
  Uint32 numof_bullets;
  if (guardian->is_appearing)
    {
      if (guard == guardian->foe[0])
        {
          guardian_appears_from_top (guard);
          guardian->foe[1]->spr.xcoord = guard->spr.xcoord;
          guardian->foe[1]->spr.ycoord = guard->spr.ycoord + 63;
        }
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && guard == guardian->foe[0] && menu_section == NO_SECTION_SELECTED)
        {
          guardian->move_time_delay--;
          if (guardian->move_time_delay > 0)
            {
              if (guardian->move_direction[guardian->move_current] <
                  GUARD_IMMOBILE)
                {
                  guardian_line_moving (guard);
                }
              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 31);
                  break;

                case GUARD_MOVEMENT_TOWARD_LEFT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 0);
                  break;

                case GUARD_IMMOBILE:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images_set (guard, 0);
                      if (guardian->is_vertical_trajectory)
                        {
                          guard->spr.current_image = 15;
                          guardian->is_vertical_trajectory = FALSE;
                        }
                    }
                  guardian_flip_image (guard, 3, 15);
                  break;
                }
            }
          else
            {
              guardian_next_trajectory ();
            }
        }
      /* articulated arm enable? */
      if (guard == guardian->foe[1]
          && guard->displacement == DISPLACEMENT_GUARDIAN
          && guard->is_enabled)
        {
          guard->spr.xcoord = guardian->foe[0]->spr.xcoord;
          guard->spr.ycoord = guardian->foe[0]->spr.ycoord + 63;
          guard->spr.anim_count++;
          if (!(guard->spr.anim_count &= (guard->spr.anim_speed - 1)))
            {
              guard->spr.current_image++;
              guard->spr.current_image &= 31;
            }
        }
    }
  numof_bullets = guardian_fire (guard, 8, 2.0);
  guardian_add_missiles (guard, numof_bullets, 8, 2);
}

/**
 * Handle the guardian 7
 * wheel with double reverse rotation
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_07 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] <
              GUARD_IMMOBILE)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
            case GUARD_MOVEMENT_TOWARD_LEFT:
            case GUARD_IMMOBILE:
              guardian_loop_flip (guard, 3, 1, 31, 0);
              break;
            }
        }
      else
        {
          guardian_next_trajectory ();
        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (50, -1);
      guardian_add_quibouly (guard, 6);
    }
}

/**
 * Handle the guardian 8
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_08 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (--guardian->move_time_delay > 0)
        {
          if (guardian->move_direction[guardian->move_current] <
              GUARD_IMMOBILE)
            {
              guardian_line_moving (guard);
            }
          switch (guardian->move_direction[guardian->move_current])
            {
            case GUARD_MOVEMENT_TOWARD_RIGHT:
            case GUARD_MOVEMENT_TOWARD_LEFT:
            case GUARD_IMMOBILE:
              guardian_loop_flip (guard, 3, 1, 31, 0);
              break;
            }
        }
      else
        {
          guardian_next_trajectory ();
        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (40, -1);
      guardian_add_tournadee (12);
    }
}

/**
 * Handle the guardian 9
 * Aircraft with rotary nose (2 sprites)
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_09 (enemy * guard)
{
  if (guardian->is_appearing)
    {
      if (guard == guardian->foe[0])
        {
          guardian_appears_from_top (guard);
          guardian->foe[1]->spr.xcoord = guard->spr.xcoord;
          guardian->foe[1]->spr.ycoord = guard->spr.ycoord;
        }
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && guard == guardian->foe[0] && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              if (guardian->move_direction[guardian->move_current] <
                  GUARD_IMMOBILE)
                {
                  guardian_line_moving (guard);
                }
              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 31);
                  break;

                case GUARD_MOVEMENT_TOWARD_LEFT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 0);
                  break;

                case GUARD_IMMOBILE:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images_set (guard, 0);
                      /* check if one were previously on a vertical trajectory 
                       * so set the fair image */
                      if (guardian->is_vertical_trajectory)
                        {
                          guard->spr.current_image = 15;
                          guardian->is_vertical_trajectory = FALSE;
                        }
                    }
                  guardian_flip_image (guard, 3, 15);
                  break;
                }
            }
          else
            {
              guardian_next_trajectory ();
            }
        }

      if (guard == guardian->foe[1]
          && guard->displacement == DISPLACEMENT_GUARDIAN
          && guard->is_enabled)
        {
          guard->spr.xcoord = guardian->foe[0]->spr.xcoord;
          guard->spr.ycoord = guardian->foe[0]->spr.ycoord;
          guard->spr.anim_count++;
          if (!(guard->spr.anim_count &= (guard->spr.anim_speed - 1)))
            {
              guard->spr.current_image++;
            }
          guard->spr.current_image &= 31;
        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (40, -1);
    }
}

/**
 * Handle the guardian 10 
 * Disk with a big adjustable canon 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_10 (enemy * guard)
{
  float angle;
  Uint32 i;
  Sint32 anim_count, length1, length2;
  enemy *guard2 = guardian->foe[1];
  spaceship_struct *ship = spaceship_get ();
  if (guardian->is_appearing)
    {
      if (guard == guardian->foe[0])
        {
          guardian_appears_from_top (guard);
          guardian->foe[1]->spr.xcoord = guard->spr.xcoord;
          guardian->foe[1]->spr.ycoord = guard->spr.ycoord;
        }
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && guard == guardian->foe[0] && menu_section == NO_SECTION_SELECTED)
        {
          if (guardian->move_current == GUARD_MOVEMENT_TOWARD_RIGHT)
            {
              guard->spr.xcoord++;
            }
          if (guardian->move_current == GUARD_MOVEMENT_TOWARD_BOTTOM)
            {
              guard->spr.ycoord++;
            }
          if (guardian->move_current == GUARD_MOVEMENT_TOWARD_LEFT)
            {
              guard->spr.xcoord--;
            }
          if (guardian->move_current == GUARD_MOVEMENT_TOWARD_TOP)
            {
              guard->spr.ycoord--;
            }
          if (guard->spr.xcoord < (float) offscreen_clipsize - clip_gard10)
            {
              guard->spr.xcoord = (float) (offscreen_clipsize - clip_gard10);
              guardian->move_current = GUARD_MOVEMENT_TOWARD_TOP;
            }
          if ((guard->spr.xcoord +
               guard->spr.img[guard->spr.current_image]->w) >
              (float) (offscreen_clipsize + offscreen_width_visible +
                       clip_gard10))
            {
              guard->spr.xcoord =
                (float) (offscreen_clipsize + offscreen_width_visible +
                         clip_gard10) -
                guard->spr.img[guard->spr.current_image]->w;
              guardian->move_current = GUARD_MOVEMENT_TOWARD_BOTTOM;
            }
          if (guard->spr.ycoord < (float) offscreen_clipsize - clip_gard10)
            {
              guard->spr.ycoord = (float) (offscreen_clipsize - clip_gard10);
              guardian->move_current = GUARD_MOVEMENT_TOWARD_RIGHT;
            }
          if ((guard->spr.ycoord +
               guard->spr.img[guard->spr.current_image]->h) >
              (float) (offscreen_clipsize + offscreen_height_visible +
                       clip_gard10))
            {
              guard->spr.ycoord =
                (float) (offscreen_clipsize + offscreen_height_visible +
                         clip_gard10) -
                guard->spr.img[guard->spr.current_image]->h;
              guardian->move_current = GUARD_MOVEMENT_TOWARD_LEFT;
            }
        }

      if (guard == guardian->foe[0])
        {
          guard->spr.anim_count++;
          if (guard->spr.anim_count >= guard->spr.anim_speed)
            {
              guard->spr.anim_count = 0;
              guard->spr.current_image++;
              if (guard->spr.current_image > 31)
                {
                  guard->spr.current_image = 0;
                }
            }
        }

      /* check if second part of the guardian existing */
      if (guard2->displacement == DISPLACEMENT_GUARDIAN)
        {
          guard2->spr.xcoord = guardian->foe[0]->spr.xcoord;
          guard2->spr.ycoord = guardian->foe[0]->spr.ycoord;
          /* cannon rotation to follow the player's spaceship */
          if (guard2->img_angle != guard2->spr.current_image)
            {
              if (guard2->sens_anim)
                {
                  guard2->spr.anim_count++;
                  if (guard2->spr.anim_count >= guard2->spr.anim_speed)
                    {
                      guard2->spr.anim_count = 0;
                      guard2->spr.current_image--;
                      if (guard2->spr.current_image < 0)
                        {
                          guard2->spr.current_image =
                            (Sint16) (guard2->spr.numof_images - 1);
                        }
                    }
                }
              else
                {
                  guard2->spr.anim_count++;
                  if (guard2->spr.anim_count >= guard2->spr.anim_speed)
                    {
                      guard2->spr.anim_count = 0;
                      guard2->spr.current_image++;
                      if (guard2->spr.current_image >=
                          guard2->spr.numof_images)
                        {
                          guard2->spr.current_image = 0;
                        }
                    }
                }
              /* count length in the oposite trigonometrical direction */
              anim_count = guard2->spr.current_image;
              length1 = 0;
              for (i = 0; i < 32; i++)
                {
                  length1++;
                  anim_count++;
                  if (anim_count > 31)
                    {
                      anim_count = 0;
                    }
                  if (guard2->img_angle == anim_count)
                    {
                      i = 32;
                    }
                }
              /* count length in the trigonometrical direction */
              anim_count = guard2->spr.current_image;
              length2 = 0;
              for (i = 0; i < 32; i++)
                {
                  length2++;
                  anim_count--;
                  if (anim_count < 0)
                    {
                      anim_count = 31;
                    }
                  if (guard2->img_angle == anim_count)
                    {
                      i = 32;
                    }
                }
              if (length1 < length2)
                {
                  guard2->sens_anim = 0;
                }
              else
                {
                  guard2->sens_anim = 1;
                }
            }
          /* search cannon position compared to the direction of spaceship  */
          angle =
            calc_target_angle ((Sint16)
                               (guard2->spr.xcoord +
                                guard2->spr.img[guard2->spr.current_image]->
                                x_gc),
                               (Sint16) (guard2->spr.ycoord +
                                         guard2->spr.img[guard2->spr.
                                                         current_image]->
                                         y_gc),
                               (Sint16) (ship->spr.xcoord +
                                         ship->spr.img[ship->spr.
                                                       current_image]->x_gc),
                               (Sint16) (ship->spr.ycoord +
                                         ship->spr.img[ship->spr.
                                                       current_image]->y_gc));
          /* search image to draw determined upon angle */
          if (sign (angle) < 0)
            {
              guard2->img_angle = (Sint16) ((angle + TWO_PI) / PI_BY_16);
            }
          else
            {
              guard2->img_angle = (Sint16) (angle / PI_BY_16);
            }
          /* avoid a negative table index */
          guard2->img_angle = (Sint16) abs (guard2->img_angle);
          /* avoid shot angle higher than the number of images of the sprite */
          if (guard2->img_angle >= guard2->spr.numof_images)
            {
              guard2->img_angle = (Sint16) (guard2->spr.numof_images - 1);
            }

        }
    }
  if (guardian_fire (guard, 8, 2.0) > 0)
    {
      guardian_add_lonely_foe (40, -1);
    }
}

/**
 * Handle the guardian 11 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_11 (enemy * guard)
{
  Uint32 numof_bullets;
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              if (guardian->move_direction[guardian->move_current] <
                  GUARD_IMMOBILE)
                {
                  guardian_line_moving (guard);
                }
              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  if (guardian->has_changed_direction)
                    {
                      guardian_change_images (guard, 0, 15);
                      guardian->has_changed_direction = FALSE;
                    }
                  guardian_flip_image (guard, 4, 31);
                  break;

                case GUARD_MOVEMENT_TOWARD_LEFT:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images (guard, 0, 15);
                    }
                  guardian_flip_image (guard, 4, 0);
                  break;

                case GUARD_IMMOBILE:
                  if (guardian->has_changed_direction)
                    {
                      guardian->has_changed_direction = FALSE;
                      guardian_change_images_set (guard, 0);
                      if (guardian->is_vertical_trajectory)
                        {
                          guard->spr.current_image = 15;
                          guardian->is_vertical_trajectory = FALSE;
                        }
                    }
                  guardian_flip_image (guard, 3, 15);
                  break;
                }
            }
          else
            {
              guardian_next_trajectory ();
            }
        }
    }
  numof_bullets = guardian_fire (guard, 8, 2.0);
  guardian_add_missiles (guard, numof_bullets, 8, 2);
}

/**
 * Handle the guardian 12 
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_12 (enemy * guard)
{
  Uint32 numof_bullets;
  bool add_shuriky = FALSE;
  bool add_missiles = FALSE;
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  guardian_line_moving (guard);
                  guardian_flip_image (guard, 4, 31);
                  add_shuriky = TRUE;
                  break;

                case GUARD_MOVEMENT_TOWARD_LEFT:
                  guardian_line_moving (guard);
                  guardian_flip_image (guard, 4, 0);
                  add_shuriky = TRUE;
                  break;

                case GUARD_MOVEMENT_TOWARD_BOTTOM:
                  guardian->is_vertical_trajectory = TRUE;
                  /* from  8 to 14: bottom to up
                   * from 15 to 31: up to bottom */
                  guardian_flip_image (guard, 3, 31);
                  if (guard->spr.current_image >= 15)
                    {
                      guardian_line_moving (guard);
                    }
                  break;

                case GUARD_MOVEMENT_TOWARD_TOP:
                  guardian_line_moving (guard);
                  guardian_flip_image (guard, 2, 31);
                  break;

                case GUARD_IMMOBILE:
                  if (guardian->current_images_set == 0)
                    {
                      add_shuriky = TRUE;
                      if (guard->spr.current_image != 15)
                        {
                          guardian_flip_image (guard, 3, 15);
                        }
                      else if (guardian_get_next_trajectory () ==
                               GUARD_MOVEMENT_TOWARD_BOTTOM)
                        {
                          guardian_change_images (guard, 1, 0);
                        }
                      else if (guardian_get_next_trajectory () ==
                               GUARD_MOVEMENT_TOWARD_LEFT)
                        {
                          guardian_add_sapouch (2, 80, TRUE);
                        }
                    }
                  else
                    {
                      if (guardian->is_vertical_trajectory)
                        {
                          if (guard->spr.current_image < 31)
                            {
                              guardian_flip_image (guard, 3, 31);
                            }
                          else
                            {
                              guardian_change_images (guard, 0, 15);
                              guardian->is_vertical_trajectory = FALSE;
                            }
                        }
                      else
                        {
                          /* lamps of the guardian wink */
                          /*  image from O to 7 */
                          guardian_loop_flip (guard, 1, 1, 7, 0);
                          add_missiles = TRUE;
                        }
                    }
                }
              if (guardian->has_changed_direction)
                {
                  guardian->has_changed_direction = FALSE;
                }
            }
          else
            {
              guardian_next_trajectory ();
            }

          numof_bullets = guardian_fire (guard, 8, 2.0);
          if (numof_bullets > 0)
            {
              if (add_shuriky)
                {
                  guardian_add_shuriky (guard, 6);
                }
              if (add_missiles)
                {
                  guardian_add_missiles (guard, numof_bullets, 24, 4);
                }
            }
        }
    }
}

/**
 * Handle the guardian 13
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_13 (enemy * guard)
{
  bool add_missiles = FALSE;
  Uint32 numof_bullets;
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              guardian_add_devilians (121, 3);
              if (guardian->move_direction[guardian->move_current] <
                  GUARD_IMMOBILE)
                {
                  guardian_line_moving (guard);
                }

              if (guardian->current_images_set == 0)
                {
                  /* change sprite if level of the life of the guardian is has its half */
                  if (guard->spr.energy_level <=
                      guard->spr.max_energy_level / 2)
                    {
                      guardian_change_images (guard, 1, 16);
                      guardian->y_inc = 3.0;
                      guardian->devilians_enable = TRUE;
                    }
                  else
                    {
                      guardian_loop_flip (guard, 2, 1, 31, 0);
                    }
                }
              else
                /* guardian weakened, low part destroyed: animation 2 */
                {
                  add_missiles = TRUE;
                  switch (guardian->move_direction[guardian->move_current])
                    {
                    case GUARD_MOVEMENT_TOWARD_RIGHT:
                      guardian_flip_image (guard, 4, 31);
                      break;

                    case GUARD_MOVEMENT_TOWARD_LEFT:
                      guardian_flip_image (guard, 0, 0);
                      break;

                    case GUARD_IMMOBILE:
                      guardian_flip_image (guard, 0, 15);
                      guardian->devilians_enable = TRUE;
                      break;
                    }
                }
            }
          else
            {
              guardian_next_trajectory ();
            }
        }
    }
  numof_bullets = guardian_fire (guard, 8, 2.0);
  if (add_missiles)
    {
      guardian_add_missiles (guard, numof_bullets, 6, 2);
    }
}

/**
 * Handle the guardian 14
 * @param guard Pointer to the enemy structure of the guardian
 */
static void
guardian_14 (enemy * guard)
{
  Uint32 numof_bullets;
  if (guardian->is_appearing)
    {
      guardian_appears_from_top (guard);
    }
  else
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          if (--guardian->move_time_delay > 0)
            {
              if (guardian->move_direction[guardian->move_current] <
                  GUARD_IMMOBILE)
                {
                  guardian_line_moving (guard);
                }

              if (guardian->anim_delay_count >= 1
                  && guard->spr.current_image == 31)
                {
                  guardian->anim_delay_count = 0;
                  if (guardian->current_images_set == 0)
                    {
                      guardian_change_images (guard, 1, 0);
                    }
                  else
                    {
                      guardian_change_images (guard, 0, 0);
                    }
                }
              else
                {
                  guardian_flip_image (guard, 2, 31);
                }

              switch (guardian->move_direction[guardian->move_current])
                {
                case GUARD_MOVEMENT_TOWARD_RIGHT:
                  break;
                case GUARD_MOVEMENT_TOWARD_LEFT:
                  break;
                }
            }
          else
            {
              guardian_next_trajectory ();
            }
        }
      guardian_add_perturbians (2, 210, TRUE);
    }
  numof_bullets = guardian_fire (guard, 4, 2.0);
  if (numof_bullets > 0)
    {
      guardian_add_saakamin (guard, 6, 7);
    }
}

/**
 * Draw a guardian vessel enemy
 * @param guard Pointer to enemy structure 
 */
static void
guardian_draw (enemy * guard)
{
  Sint16 zon_col;
  float x_expl, y_expl;
  sprite ve_spr = guard->spr;

  /* display white mask */
  if (guard->is_white_mask_displayed)
    {
      draw_sprite_mask (coulor[WHITE],
                        guard->spr.img[guard->spr.current_image],
                        (Sint32) (guard->spr.xcoord),
                        (Sint32) (guard->spr.ycoord));
      guard->is_white_mask_displayed = FALSE;
    }
  else
    {
      draw_sprite (guard->spr.img[guard->spr.current_image],
                   (Uint32) guard->spr.xcoord, (Uint32) guard->spr.ycoord);
      if (rand () % 2
          && rand () % (ve_spr.max_energy_level + 1) >
          ve_spr.energy_level + (ve_spr.max_energy_level >> 3))
        {
          zon_col =
            (Sint16) (rand () %
                      ((Sint32) ve_spr.img[ve_spr.current_image]->
                       numof_collisions_zones));
          x_expl =
            (float) (ve_spr.xcoord +
                     ve_spr.img[ve_spr.current_image]->
                     collisions_coords[zon_col][XCOORD] +
                     rand () %
                     ((Sint32) ve_spr.img[ve_spr.current_image]->
                      collisions_sizes[zon_col][XCOORD] + 1));
          y_expl =
            (float) (ve_spr.ycoord +
                     ve_spr.img[ve_spr.current_image]->
                     collisions_coords[zon_col][YCOORD] +
                     rand () %
                     ((Sint32) ve_spr.img[ve_spr.current_image]->
                      collisions_sizes[zon_col][YCOORD] + 1));
          explosion_guardian_add (x_expl, y_expl);
        }
    }
}

/**
 * Run all guardians + congratulations
 * @param guard Pointer to enemy structure
 */
void
guardian_handle (enemy * guard)
{
  spaceship_struct *ship = spaceship_get ();
  switch (guardian->number)
    {
    case 1:
      guardian_01 (guard);
      break;
    case 2:
      guardian_02 (guard);
      break;
    case 3:
      guardian_03 (guard);
      break;
    case 4:
      guardian_04 (guard);
      break;
    case 5:
      guardian_05 (guard);
      break;
    case 6:
      guardian_06 (guard);
      break;
    case 7:
      guardian_07 (guard);
      break;
    case 8:
      guardian_08 (guard);
      break;
    case 9:
      guardian_09 (guard);
      break;
    case 10:
      guardian_10 (guard);
      break;
    case 11:
      guardian_11 (guard);
      break;

      /* guardians 12, 13, 14 and congratulations */
    case 12:
      guardian_12 (guard);
      break;
    case 13:
      guardian_13 (guard);
      break;
    case 14:
      guardian_14 (guard);
      break;
    case 15:
      congratulations ();
      break;
    }

  /* common part */
  if (guardian->is_appearing)
    {
      if (guard == guardian->foe[0])
        {
          /* guardian blink: display once on two */
          if (guardian->is_blinking)
            {
              /* the guardian consists of two sprites */
              if (guardian->number == 6 || guardian->number == 9
                  || guardian->number == 10)
                {
                  guardian_draw (guardian->foe[0]);
                  guardian_draw (guardian->foe[1]);
                }
              else
                {
                  guardian_draw (guard);
                }
              guardian->is_blinking = FALSE;
            }
          else
            {
              guardian->is_blinking = TRUE;
            }
        }
    }
  else
    {
      /* collisions with protections satellite */
      enemy_satellites_collisions (guard);
      /* collisions with extra guns */
      enemy_guns_collisions (guard);
      /* collision with player's spaceship */
      if (!ship->invincibility_delay)
        {
          enemy_spaceship_collision (guard);
        }
      /* display the guardian */
      guardian_draw (guard);
    }
}

/**
 * Loading guardian's sprites images in memory 
 * @param guardian_num number of the guardian 1 to 14
 * @return TRUE if successful
 */
bool
guardian_load (Sint32 guardian_num)
{
  Uint32 num_of_sprites;
  LOG_INF ("Load guardian %i", guardian_num);
  guardian_images_free ();

  switch (guardian_num)
    {
    case 2:
    case 4:
    case 5:
    case 6:
    case 9:
    case 10:
    case 12:
    case 13:
    case 14:
      num_of_sprites = 2;
      break;
    default:
      num_of_sprites = 1;
      break;
    }
  if (!image_load_num
      ("graphics/sprites/guardians/guardian_%02d.spr",
       guardian_num - 1, &gardi[0][0], num_of_sprites,
       ENEMIES_SPECIAL_NUM_OF_IMAGES))
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * Convert guardians from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
guardians_extract (void)
{
  Uint32 num_of_sprites, guardian_num, sprite_num, frame;
  const char *model = EXPORT_DIR "/guardians/guardian-xx/guardian-x-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/guardians"))
    {
      free_memory (filename);
      return FALSE;
    }

  for (guardian_num = 1; guardian_num <= 14; guardian_num++)
    {
      if (!guardian_load (guardian_num))
        {
          free_memory (filename);
          return FALSE;
        }
      switch (guardian_num)
        {
        case 2:
        case 4:
        case 5:
        case 6:
        case 9:
        case 10:
        case 12:
        case 13:
        case 14:
          num_of_sprites = 2;
          break;
        default:
          num_of_sprites = 1;
          break;
        }
      sprintf (filename, EXPORT_DIR "/guardians/guardian-%02d", guardian_num);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (sprite_num = 0; sprite_num < num_of_sprites; sprite_num++)
        {
          for (frame = 0; frame < ENEMIES_SPECIAL_NUM_OF_IMAGES; frame++)
            {
              sprintf (filename,
                       EXPORT_DIR
                       "/guardians/guardian-%02d/guardian-%01d-%02d.png",
                       guardian_num, sprite_num + 1, frame);
              if (!image_to_png (&gardi[sprite_num][frame], filename))
                {
                  free_memory (filename);
                  return FALSE;
                }
            }
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif

/** 
 * Prepare next level if end of guardian
 */
bool
guardian_finished (void)
{
  Uint32 guard_num = 0;
  bool is_finished;
  if (guardian->number == 0)
    {
      return TRUE;
    }
  /* guardian 11, 12, 13 and 14 requiring special handling */
  if (num_level == MAX_NUM_OF_LEVELS && guardian->number >= 11
      && guardian->number < 15)
    {
      switch (guardian->number)
        {
        case 11:
          if (!guardian_load (12))
            {
              return FALSE;
            }
          guardian_new (12);
          break;
        case 12:
          if (!guardian_load (13))
            {
              return FALSE;
            }
          guardian_new (13);
          break;
        case 13:
          if (!guardian_load (14))
            {
              return FALSE;
            }
          guardian_new (14);
          break;
        case 14:
          spaceship_disappears = TRUE;
          /* congratulations */
          if (starfield_speed == 0.0)
            {
              guardian_new (15);
            }
          break;
        }
    }
  else
    {
      /* player's spaceship disappear from the screen */
      spaceship_disappears = TRUE;
      text_level_move (num_level);
      is_finished = text_level_move (num_level);
      if (starfield_speed == 0.0 && is_finished)
        {
          /* next level */
          num_level++;
          if (num_level > MAX_NUM_OF_LEVELS)
            {
              num_level = 0;
            }

          /* load guardian files in advance */
          switch (num_level)
            {
            case 4:
              guard_num = 2;
              break;
            case 8:
              guard_num = 3;
              break;
            case 12:
              guard_num = 4;
              break;
            case 16:
              guard_num = 5;
              break;
            case 20:
              guard_num = 6;
              break;
            case 24:
              guard_num = 7;
              break;
            case 28:
              guard_num = 8;
              break;
            case 32:
              guard_num = 9;
              break;
            case 36:
              guard_num = 10;
              break;
            case 40:
              guard_num = 11;
              break;
            }
          if (guard_num > 0)
            {
              if (!guardian_load (guard_num))
                {
                  return FALSE;
                }
            }

          /* load grid level */
          if (!grid_load (num_level))
            {
              return FALSE;
            }
          /* load curve phase level file (little skirmish) */
          if (!curve_load_level (num_level))
            {
              return FALSE;
            }
          if (!meteors_load (num_level))
            {
              return FALSE;
            }
          /* enable the curve level file loaded previously */
          curve_enable_level ();
          courbe.activity = TRUE;
          grid.is_enable = FALSE;
          meteor_activity = FALSE;
          guardian->number = 0;
          spaceship_show ();
        }
    }
  return TRUE;
}

/*
gardien 1 : gauche a droite
gardien 2 : gauche a droite + animation descente
gardien 3 : gauche a droite
gardien 4 : gauche a droite + animation quand il pause
gardien 5 : gauche a droite (sinus vertical) + missile
gardien 6 : compose de 2 sprites : gauche a droite  (tube vertical)
gardien 7 : gauche a droite + mine (tir & anim si mort) + autres vaisseaux
gardien 8 : gauche a droite + autres vaisseaux
gardien 9 : compose de 2 sprites : gauche a droite (nez du vaisseau) + autres vaisseaux
gardien 10 : tourne en rectangle + autres vaisseaux
gardien 11 : gauche a droite + autres vaisseaux

lonely foe sympas :
-  9 LONELY_HOCKYS:        boule canon va de haut en bas
- 20 LONELY_DEMONIANS:     vaisseau va de haut en bas
- 22 LONELY_FIDGETINIANS:  vaisseau dentee va de haut en bas
- 28 LONELY_DIVERTIZERS    vaisseau tourne va de haut en bas
- 31 LONELY_CARRYONIANS    vaisseau bas ailes va de bas en haut
- 39 LONELY_MADIRIANS      vaisseau canon gauche a droite
*/
