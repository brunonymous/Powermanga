/** 
 * @file spaceship.c
 * @brief Handle the player's spaceship 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: spaceship.c,v 1.36 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "enemies.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "satellite_protections.h"
#include "menu.h"
#include "menu_sections.h"
#include "log_recorder.h"
#include "options_panel.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "starfield.h"

/** Maximum number of images peer spaceship */
#define SPACESHIP_MAX_OF_IMAGES 5
/** Differents players (always one player) */
typedef enum
{
  PLAYER_1,
  /* never used */
  PLAYER_2,
    /** Maximum of players */
  MAX_OF_PLAYERS
} PLAYERS_ENUM;
/** The player has lost his spaceship */
bool spaceship_is_dead = FALSE;
 /** If TRUE spaceship disappearing */
bool spaceship_disappears = FALSE;
/** Time counter of the appearance of the player's spaceship */
Sint32 spaceship_appears_count = 0;
/** Counter delay of the the increase of the energy level */
static Sint32 energy_restore_delay = 0;
/** Data structures of the images's spaceship  */
static image
  spaceships_sprites[SPACESHIP_NUM_OF_TYPES][SPACESHIP_MAX_OF_IMAGES];
/** Data structures of the players spaceships */
static spaceship_struct spaceships[MAX_OF_PLAYERS];

/**
 * Load sprites images and initialize structure of the player's spaceship
 * @return TRUE if successful
 */
bool
spaceship_once_init (void)
{
  Uint32 type, frame;
  char *filename;
  const char *model =
    "graphics/sprites/spaceships/spaceship_%01d_frame_%1d.spr";
  spaceship_struct *ship = spaceship_get ();
  spaceship_free ();

  /* allocate a temporary string */
  filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }

  /* extract all sprites images of the  player's spaceship */
  for (type = 0; type < SPACESHIP_NUM_OF_TYPES; type++)
    {
      for (frame = 0; frame < SPACESHIP_MAX_OF_IMAGES; frame++)
        {
          sprintf (filename, model, type + 1, frame);
          if (!image_load_single (filename, &spaceships_sprites[type][frame]))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  free_memory (filename);
  spaceship_initialize ();
  ship->gems_count = 0;
  return TRUE;
}

/**
 * Convert from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
spaceship_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/ships/ship-%01d/ship-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/ships"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < SPACESHIP_NUM_OF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/ships/ship-%01d", type + 1);
      if (!create_dir (filename))
        {
          return FALSE;
        }
      for (frame = 0; frame < SPACESHIP_MAX_OF_IMAGES; frame++)
        {
          sprintf (filename, EXPORT_DIR "/ships/ship-%01d/ship-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&spaceships_sprites[type][frame], filename))
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
 * Release sprites images and of the player's spaceship
 * @return TRUE if successful
 */
void
spaceship_free (void)
{
  images_free (&spaceships_sprites[0][0], SPACESHIP_NUM_OF_TYPES,
               SPACESHIP_MAX_OF_IMAGES, SPACESHIP_MAX_OF_IMAGES);
}


/**
 * Return the current player's spaceship
 * @return Pointer to a spaceship structure
 */
spaceship_struct *
spaceship_get (void)
{
  return &spaceships[PLAYER_1];
}

/** 
 * Restores the energy level of the player's spaceship 
 */
void
spaceship_energy_restore (void)
{
  spaceship_struct *ship = spaceship_get ();
  if (gameover_enable || player_pause || menu_status != MENU_OFF)
    {
      return;
    }

  /* increase the energie level of spaceship?  */
  energy_restore_delay += (SPACESHIP_NUM_OF_TYPES - ship->type);
  if (energy_restore_delay < 150)
    {
      return;
    }

  /* clear counter delay */
  energy_restore_delay = 0;
  /* level reach maximum? */
  if (ship->spr.energy_level < ship->spr.pow_of_dest)
    {
      /* increase level */
      ship->spr.energy_level++;
      /* maximum energy level: close option box */
      if (ship->spr.energy_level >= ship->spr.pow_of_dest)
        {
          option_anim_init (OPTION_PANEL_REPAIR, TRUE);
        }
      /* refresh gauge of energy level  */
      energy_gauge_spaceship_is_update = TRUE;
    }
}

/** 
 * Handle the temporary invincibility of the player's spaceship
 */
void
spaceship_invincibility (void)
{
  Sint32 coordx, coordy;
  spaceship_struct *ship = spaceship_get ();

  /* spaceship is invincibile?  */
  if (ship->invincibility_delay <= 0)
    {
      /* no, spaceship is not invincibile */
      return;
    }
  ship->invincibility_count--;

  /* spaceship is visible? */
  if (ship->is_visible)
    {
      if (ship->invincibility_count <= 0)
        {
          /* player's spaceship  must become invisible */
          ship->is_visible = FALSE;
          ship->invincibility_count = ship->invincibility_delay;
        }
      return;
    }


  /*
   * player's spaceship is not visible 
   */
  if (ship->invincibility_count <= 0)
    {
      /* player's spaceship  must become visible */
      ship->is_visible = TRUE;
      /* invincibility duration is decreased by one */
      if (!player_pause && menu_status == MENU_OFF)
        ship->invincibility_delay--;
      /* spaceship always invincible? */
      if (ship->invincibility_delay <= 0)
        {
          ship->is_visible = TRUE;
          /* no change of the spaceship must take place */
          ship->has_just_upgraded = FALSE;
        }
      else
        {
          /* spaceship is always invincible  */
          ship->invincibility_count = ship->invincibility_delay;
        }
    }

  /* add a star in the explosions list */
  coordx =
    (Sint32) ship->spr.xcoord +
    (Sint32) (((Sint32) rand () %
               (ship->spr.img[ship->spr.current_image]->w + 16))) - 16;
  coordy =
    (Sint32) ship->spr.ycoord +
    (Sint32) (((Sint32) rand () %
               (ship->spr.img[ship->spr.current_image]->h + 16))) - 8;
  if (coordx >= offscreen_clipsize
      && coordx <= (offscreen_clipsize + offscreen_width_visible)
      && ship->has_just_upgraded && coordy >= offscreen_clipsize
      && coordy <= (offscreen_clipsize + offscreen_height_visible))
    explosion_add ((float) coordx, (float) coordy, 0.7f, STAR_SPACESHIP, 0);
}

/** 
 * Initialise the spaceship structure 
 */
void
spaceship_initialize (void)
{
  Uint32 i;
  spaceship_struct *ship = spaceship_get ();

  /* the number of bonus collected */
  ship->gems_count = 4;
  /* clear number of satellite protections */
  ship->num_of_satellites = 0;
  /* clear number of extra guns */
  ship->num_of_extraguns = 0;
  /* the first basic spaceship */
  ship->type = SPACESHIP_TYPE_1;

  /* test only */
  /* ship->type = SPACESHIP_TYPE_4; */

  /* rate of fire */
  ship->fire_rate = 50 - (ship->type * 5 + 5);
  /* speed of the displacement */
  ship->speed_booster = 2;
  /* enable front shot force 1 */
  ship->shot_front_basic = 1;
  /* disable left shot force 1 */
  ship->shot_left_basic = 0;
  /* disable left right force 1 */
  ship->shot_right_basic = 0;
  /* disable rear shot force 1 */
  ship->shot_rear_basic = 0;
  /* disable front shot force 2 */
  ship->shot_front_enhanced = 0;
  /* disable rear shot force 2 */
  ship->shot_rear_enhanced = 0;
  /* disable left right force 2 */
  ship->shot_right_enhanced = 0;
  /* disable left shot force 2 */
  ship->shot_left_enhanced = 0;

  /* 
   * initialize sprite data structute 
   */
  /* indicate that is friend sprite */
  ship->spr.type = FRIEND;
  /* trajectory of sprite is calculated */
  ship->spr.trajectory = 1;
  /* set power of the destruction */
  ship->spr.pow_of_dest = (Sint16) (ship->type * 20 + 20);
  /* set the spaceship's energie level */
  ship->spr.energy_level = ship->spr.pow_of_dest;
  /* number of images of the sprite */
  ship->spr.numof_images = SPACESHIP_MAX_OF_IMAGES;
  /* set current image index */
  ship->spr.current_image = 2;
  /* set value of delay between two images */
  ship->spr.anim_speed = 10;
  /* clear counter of delay between two images */
  ship->spr.anim_count = 0;
  /* no change of the spaceship must take place */
  ship->has_just_upgraded = FALSE;
  /* set addresses of the sprites images buffer */
  for (i = 0; i < SPACESHIP_MAX_OF_IMAGES; i++)
    {
      ship->spr.img[i] = (image *) & spaceships_sprites[ship->type][i];
    }
}

/** 
 * Start the appearance of the spaceship
 */
void
spaceship_show (void)
{
  spaceship_struct *ship = spaceship_get ();
  ship->spr.xcoord =
    (float) (offscreen_width_visible - ship->spr.img[2]->w / 2);
  ship->spr.ycoord = (float) (offscreen_starty - 32);
  /* vertical speed */
  ship->y_speed = 1.2f;
  /* horizontal speed */
  ship->x_speed = 0.0;
  spaceship_appears_count = 90;
  spaceship_disappears = FALSE;
}

/**
 * First basic spaceship was destroyed, enable the game over
 */
void
spaceship_gameover (void)
{
  float coordx, coordy;
  image *ship_img;
  spaceship_struct *ship = spaceship_get ();
  ship_img = ship->spr.img[ship->spr.current_image];
  /* sort the scores and prepares display */
  menu_section_set (SECTION_GAME_OVER);

  /* add a serie of explosions */
  explosions_add_serie (NULL);

  /* add many explosions fragments */
  coordx = ship->spr.xcoord + ship_img->x_gc - 8 * pixel_size;
  coordy = ship->spr.ycoord + ship_img->y_gc - 8 * pixel_size;
  explosions_fragments_add (coordx, coordy, 0.5, 5, 0, 3);
  explosions_fragments_add (coordx, coordy, 1.0, 5, 0, 2);
  explosions_fragments_add (coordx, coordy, 1.5, 5, 0, 1);

  /* display "GAME OVER" sprites text */
  gameover_enable = TRUE;
  /* player's spaceship make invisible */
  ship->is_visible = FALSE;
  /* close all options boxes on the right options panel */
  options_close_all ();
  /* clear the number of bonus collected */
  ship->gems_count = 0;
  /* clear keyboard flags */
  clear_keymap ();
#ifdef USE_SDLMIXER
  /* play introduction music */
  sound_music_play (MUSIC_INTRO);
#endif
}

/**
 * Handle the loss and the regression of the spaceship or cause the end
 * of the game
 */
void
spaceship_downgrading (void)
{
  Uint32 i;
  spaceship_struct *ship = spaceship_get ();
  if (!spaceship_is_dead || gameover_enable || menu_section != 0)
    {
      return;
    }
  /* go back to the ship of the previous level */
  if (ship->type != SPACESHIP_TYPE_1)
    {
      ship->type--;
#ifdef USE_SDLMIXER
      sound_play (SOUND_DOWNGRADE_SPACESHIP);
#endif
      /* change of spaceship */
      ship->has_just_upgraded = TRUE;
      /* set rate of fire */
      ship->fire_rate = 50 - (ship->type * 5 + 5);
      /* set power of the destruction */
      ship->spr.pow_of_dest = (Sint16) (ship->type * 20 + 20);
      /* set energy level of the spaceship */
      ship->spr.energy_level = ship->spr.pow_of_dest;
      /* clear the score multiplier */
      score_multiplier = 0;
      if (!option_boxes[OPTION_PANEL_REPAIR].close_option)
        {
          /* close "spaceship repair" option box */
          option_anim_init (OPTION_PANEL_REPAIR, TRUE);
        }
      if (option_boxes[OPTION_PANEL_NEW_SPACESHIP].close_option)
        {
          /* open "change of spaceship" option box */
          option_anim_init (OPTION_PANEL_NEW_SPACESHIP, FALSE);
        }
      ship->invincibility_delay = SPACESHIP_INVINCIBILITY_TIME;
      /* update spaceship's energy gauge */
      energy_gauge_spaceship_is_update = TRUE;
      spaceship_is_dead = FALSE;
      /* set addresses of the sprites images buffer */
      for (i = 0; i < SPACESHIP_MAX_OF_IMAGES; i++)
        {
          ship->spr.img[i] = (image *) & spaceships_sprites[ship->type][i];
        }
      /* remove protection satellites */
      satellites_init ();
      /* remove extra guns */
      guns_init ();
    }
  else
    {
      /* it was the first basic spaceship, the game is over */
      spaceship_gameover ();
    }
}

/**
 * Set invincibility counter of the player's spaceship
 * @param invincibility Invincibility counter time 
 */
void
spaceship_set_invincibility (Sint16 invincibility)
{
  spaceship_struct *ship = spaceship_get ();
  ship->invincibility_delay = invincibility;
}

/**
 * Control movements of the player's spaceship
 */
void
spaceship_control_movements (void)
{
  spaceship_struct *ship = spaceship_get ();
  if (spaceship_appears_count <= 0 && !spaceship_disappears)
    {
      if (ship->x_speed >= 0.5f)
        {
          ship->x_speed -= 0.2f;
        }
      if (ship->x_speed <= -0.5f)
        {
          ship->x_speed += 0.2f;
        }
      if (ship->y_speed >= 0.5f)
        {
          ship->y_speed -= 0.2f;
        }
      if (ship->y_speed <= -0.5f)
        {
          ship->y_speed += 0.2f;
        }
      /* player's spaceship is dead? */
      if (!spaceship_is_dead)
        {
          /* keyboard or joystick movement */
          if (keys_down[K_LEFT] || joy_left)
            {
              ship->x_speed -= 0.5f;
            }
          if (keys_down[K_RIGHT] || joy_right)
            {
              ship->x_speed += 0.5f;
            }
          if (keys_down[K_UP] || joy_top)
            {
              ship->y_speed -= 0.5f;
            }
          if (keys_down[K_DOWN] || joy_down)
            {
              ship->y_speed += 0.5f;
            }
          if (keys_down[K_CTRL] && keys_down[K_Q])
            {
              /* [Crtl]+[Q] force "game over" */
              spaceship_gameover ();
            }
        }
    }
  else
    {
      if (ship->x_speed >= 0.5f)
        {
          ship->x_speed -= 0.4f;
        }
      if (ship->x_speed <= -0.5f)
        {
          ship->x_speed += 0.4f;
        }
      if (ship->y_speed >= 0.5f)
        {
          ship->y_speed -= 0.01f;
        }
      if (ship->y_speed <= -0.5f)
        {
          ship->y_speed += 0.01f;
        }
    }
}


/**
 * Get the more powerfull spaceship
 */
void
spaceship_most_powerfull (void)
{
  spaceship_struct *ship = spaceship_get ();
  ship->shot_front_basic = 5;
  ship->shot_left_basic = 5;
  ship->shot_right_basic = 5;
  ship->shot_rear_basic = 5;
  ship->shot_front_enhanced = 5;
  ship->shot_rear_enhanced = 5;
  ship->shot_right_enhanced = 5;
  ship->shot_left_enhanced = 5;
  while (spaceship_upgrading ())
    {
    }
  energy_gauge_spaceship_is_update = TRUE;
  satellites_add ();
  electrical_shock_enable = TRUE;
  while (gun_add ())
    {
    }
  options_close_all ();
}

/**
 * Get a new spaceship more resistant and more powerful
 * @return TRUE if the spaceship were upgraded
 */
bool
spaceship_upgrading (void)
{
  Uint32 i;
  spaceship_struct *ship = spaceship_get ();
  if (ship->type >= (SPACESHIP_NUM_OF_TYPES - 1))
    {
      return FALSE;
    }

  /* get the ship of the next level */
  ship->type++;
  /* notice that spaceship change */
  ship->has_just_upgraded = TRUE;
#ifdef USE_SDLMIXER
  sound_play (SOUND_UPGRADE_SPACESHIP);
#endif
  ship->invincibility_delay = SPACESHIP_INVINCIBILITY_TIME;
  /* set shot time-frequency */
  ship->fire_rate = 50 - (ship->type * 5 + 5);
  /* set addresses of the images buffer */
  for (i = 0; i < SPACESHIP_MAX_OF_IMAGES; i++)
    {
      ship->spr.img[i] = (image *) & spaceships_sprites[ship->type][i];
    }
  /* remove protection satellites */
  satellites_init ();
  /* remove extra guns */
  guns_init ();
  if (ship->spr.energy_level < ship->spr.pow_of_dest)
    {
      /* maximum energy level: start anim of closing option box */
      option_anim_init (OPTION_PANEL_REPAIR, TRUE);
    }
  /* set power of the destruction */
  ship->spr.pow_of_dest = (Sint16) (ship->type * 20 + 20);
  /* set level of energy */
  ship->spr.energy_level = ship->spr.pow_of_dest;
  energy_gauge_spaceship_is_update = TRUE;
  /* clear the number of bonus collected */
  ship->gems_count = 0;
  option_change = TRUE;
  if (ship->type >= (SPACESHIP_NUM_OF_TYPES - 1))
    {
      /* start the closing anim of the spaceship change option box */
      option_anim_init (OPTION_PANEL_NEW_SPACESHIP, TRUE);
    }
  return TRUE;
}

/** 
 * Collisions between the spaceship and an enemy
 * @param foe pointer to the structure of an enemy
 * @param speed_x speed to add to spaceship if collision
 * @param speed_y speed to add to spaceship if collision
 * @return TRUE if enemy is destroyed
 */
bool
spaceship_enemy_collision (enemy * foe, float speed_x, float speed_y)
{
  Sint32 i, l, x1, y1, x2, y2;
  image *ship_img, *foe_img;
  spaceship_struct *ship = spaceship_get ();
  ship_img = ship->spr.img[ship->spr.current_image];
  foe_img = foe->spr.img[foe->spr.current_image];

  /* for each collision point of the spaceship */
  for (i = 0; i < ship_img->numof_collisions_points; i++)
    {
      /* coordinates of the collision point of the spaceship */
      x1 = (Sint32) ship->spr.xcoord + ship_img->collisions_points[i][XCOORD];
      y1 = (Sint32) ship->spr.ycoord + ship_img->collisions_points[i][YCOORD];

      /* for each collision zone of the enemy */
      for (l = 0; l < foe_img->numof_collisions_zones; l++)
        {
          /* coordinates of the collision zone of the enemy */
          x2 =
            (Sint32) foe->spr.xcoord + foe_img->collisions_coords[l][XCOORD];
          y2 =
            (Sint32) foe->spr.ycoord + foe_img->collisions_coords[l][YCOORD];

          /* check if spaceship collision point is into enemy collision zone */
          if (x1 < x2 ||
              y1 < y2 ||
              x1 >= x2 + foe_img->collisions_sizes[l][IMAGE_WIDTH] ||
              y1 >= y2 + foe_img->collisions_sizes[l][IMAGE_HEIGHT])
            {
              continue;
            }
          /* decrease energy level of enemy */
          foe->spr.energy_level -= (ship->spr.pow_of_dest << 1);
          if (foe->type >= THANIKEE)
            {
              energy_gauge_guard_is_update = TRUE;
            }
          /* spaceship is it invincible? */
          if (!ship->invincibility_delay)
            {
              if (ship->spr.energy_level == ship->spr.pow_of_dest)
                {
                  /* energy level was full, start anim of opening option box */
                  option_anim_init (OPTION_PANEL_REPAIR, FALSE);
                }
              /* decrease energy level of satellite */
              ship->spr.energy_level =
                (Sint16) (ship->spr.energy_level - foe->spr.pow_of_dest);
            }
          /* update spaceship's energy gauge */
          energy_gauge_spaceship_is_update = TRUE;
          if (ship->spr.energy_level <= 0)
            {
              /* spaceship is destroyed: game over */
              spaceship_is_dead = TRUE;
            }
          else
            {
              /* spaceship not destroyed, display white mask */
              ship->is_white_mask_displayed = TRUE;
            }
          /* add enemy speed of displacement to spaceship */
          ship->x_speed += speed_x;
          ship->y_speed += speed_y;

          /* check if enemy is destroyed */
          if (foe->spr.energy_level <= 0)
            {
              /* update player's score */
              player_score += foe->spr.pow_of_dest << 2 << score_multiplier;
              return TRUE;
            }
          else
            {
              /* draw the sprite mask with a white color */
              foe->is_white_mask_displayed = TRUE;
            }
          return FALSE;
        }
    }
  return FALSE;
}

/** 
 * Collisions between the spaceship and a shot 
 * @param x1 x coordinate of the collision point of the shot
 * @param y1 y coordinate of the collision point of the shot
 * @param bullet pointer to the structure of an shot 
 * @return TRUE if the shot touched the spaceship  
 */
bool
spaceship_shot_collision (Sint32 x1, Sint32 y1, shot_struct * bullet)
{
  Sint32 i, x2, y2;
  spaceship_struct *ship = spaceship_get ();
  image *ship_img = ship->spr.img[ship->spr.current_image];

  for (i = 0; i < ship_img->numof_collisions_zones; i++)
    {
      x2 = (Sint32) ship->spr.xcoord + ship_img->collisions_coords[i][XCOORD];
      y2 = (Sint32) ship->spr.ycoord + ship_img->collisions_coords[i][YCOORD];

      /* check if shot collision point is into spaceship collision zone */
      if (x1 < x2
          || x1 >= x2 + ship_img->collisions_sizes[i][IMAGE_WIDTH]
          || y1 < y2
          || y1 >= y2 + ship_img->collisions_sizes[i][IMAGE_HEIGHT])
        {
          continue;
        }
      /* spaceship is it invincible? */
      if (!ship->invincibility_delay)
        {
          if (ship->spr.energy_level == ship->spr.pow_of_dest)
            {
              /* energy level was full, start anim
               * of opening option box */
              option_anim_init (OPTION_PANEL_REPAIR, FALSE);
            }
          /* decrease energy level of spaceship */
          ship->spr.energy_level =
            (Sint16) (ship->spr.energy_level - bullet->spr.pow_of_dest);
        }
      /* update spaceship's energy gauge */
      energy_gauge_spaceship_is_update = TRUE;
      if (ship->spr.energy_level <= 0)
        {
          /* spaceship is destroyed: game over */
          spaceship_is_dead = TRUE;
        }
      else
        {
          /* spaceship not destroyed, display white mask */
          ship->is_white_mask_displayed = TRUE;
        }
      explosion_add (bullet->spr.xcoord, bullet->spr.ycoord, 0.35f,
                     EXPLOSION_SMALL, 0);
      /* remove the shot which has just touched the enemy */
      return TRUE;
    }
  return FALSE;
}

/**
 * Control the speed of the spaceship
 */
void
spaceship_speed_control (void)
{
  spaceship_struct *ship = spaceship_get ();

  /* limit maximum right speed of the spaceship */
  if (ship->x_speed >= (float) ship->speed_booster)
    {
      /* max. right speed */
      ship->x_speed = (float) ship->speed_booster;
      /* all on the right displacement image */
      ship->spr.current_image = 4;
    }
  else
    {
      if (ship->x_speed > 0)
        {
          /* right displacement image */
          ship->spr.current_image = 3;
        }
    }
  if (ship->x_speed <= (float) (-ship->speed_booster))
    {
      /* max. left speed */
      ship->x_speed = (float) (-ship->speed_booster);
      /* all on the left displacement image */
      ship->spr.current_image = 0;
    }
  else
    {
      if (ship->x_speed < 0)
        {
          /* left displacement image */
          ship->spr.current_image = 1;
        }
    }
  /* limit maximum right speed of the spaceship */
  if (spaceship_appears_count <= 0 && !spaceship_disappears)
    {
      if (ship->y_speed > ship->speed_booster)
        {
          ship->y_speed = (float) ship->speed_booster;
        }
      if (ship->y_speed < (-ship->speed_booster))
        {
          ship->y_speed = (float) (-ship->speed_booster);
        }
    }
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* change x and y coordinates */
      ship->spr.xcoord += ship->x_speed;
      ship->spr.ycoord += ship->y_speed;
      if (ship->spr.ycoord < (float) (offscreen_starty - 100))
        {
          ship->spr.ycoord = (float) (offscreen_starty - 100);
        }
    }

  /* limit minimum speed */
  if (fabs (ship->x_speed) < 0.5)
    {
      ship->x_speed = 0.0;
      /* center image of the spaceship */
      ship->spr.current_image = 2;
    }

  /* spacheship is affected by gravity, and fall to the bottom of the screen */
  if (spaceship_appears_count <= 0 && !spaceship_disappears)
    {
      if (!player_pause && menu_status == MENU_OFF
          && menu_section == NO_SECTION_SELECTED)
        {
          {
            if (!keys_down[K_DOWN] && !keys_down[K_UP] && !joy_top
                && !joy_down)
              if (ship->y_speed <= 0.2f)
                {
                  ship->y_speed += 0.025f;
                }
          }
        }

      /* clip position of the player's spacheship */
      if ((Sint16) ship->spr.xcoord < 128)
        {
          ship->spr.xcoord = 128.0;
          ship->x_speed = 0.0;
        }
      if (((Sint16) ship->spr.xcoord +
           (Sint16) ship->spr.img[ship->spr.current_image]->w) >
          128 + offscreen_width_visible)
        {
          ship->spr.xcoord =
            (float) (offscreen_width_visible + 128.0f -
                     ship->spr.img[ship->spr.current_image]->w);
          ship->x_speed = 0.0f;
        }
      if ((Sint16) ship->spr.ycoord < 128)
        {
          ship->spr.ycoord = 128.0;
          ship->y_speed = 0.0;
        }
      if (((Sint16) ship->spr.ycoord +
           (Sint16) ship->spr.img[ship->spr.current_image]->h) >
          128 + offscreen_height_visible)
        {
          ship->spr.ycoord =
            (float) (offscreen_height_visible + 128.0f -
                     ship->spr.img[ship->spr.current_image]->h);
          ship->y_speed = 0.0f;
        }
    }
}

/**
 * Draw the player spaceship
 */
void
spaceship_draw (void)
{
  spaceship_struct *ship = spaceship_get ();
  /* draw the player spaceship */
  if (gameover_enable || !ship->is_visible)
    {
      return;
    }

  if (((Sint16) ship->spr.ycoord +
       ship->spr.img[ship->spr.current_image]->h) >= offscreen_clipsize)
    {
      /* display white mask of the sprite */
      if (ship->is_white_mask_displayed)
        {
          draw_sprite_mask (coulor[WHITE],
                            ship->spr.img[ship->spr.current_image],
                            (Sint32) (ship->spr.xcoord),
                            (Sint32) (ship->spr.ycoord));
          ship->is_white_mask_displayed = FALSE;
        }
      /* display the sprite normally */
      else
        {
          draw_sprite (ship->spr.img[ship->spr.current_image],
                       (Uint32) ship->spr.xcoord, (Uint32) ship->spr.ycoord);
        }
    }
}
