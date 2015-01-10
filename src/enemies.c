/** 
 * @file enemies.c
 * @brief handle all enemies 
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: enemies.c,v 1.48 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "enemies.h"
#include "bonus.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "grid_phase.h"
#include "guardians.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#include "meteors_phase.h"
#include "lonely_foes.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "shockwave.h"
#include "spaceship.h"

/** The number of currently active enemies */
Sint32 num_of_enemies = 0;
/* data structure of the images's enemies */
image enemi[ENEMIES_MAX_SMALL_TYPES + ENEMIES_MAX_BLOODSUCKER_TYPES +
            LONELY_FOES_MAX_OF + ENEMIES_MAX_SPECIAL_TYPES][IMAGES_MAXOF];
/* data structure of the enemies vessels */
enemy *enemies = NULL;
static enemy *enemy_first = NULL;
static enemy *enemy_last = NULL;
/** Num of colors used in the fade-out effect */
#define NUMOF_DEAD_COLORS 11
/** Colors used in the fade-out effect (gradual disappearance of an enemy) */
static unsigned char enemy_dead_colors[NUMOF_DEAD_COLORS + 1];

static bool enemies_load (void);
static bool enemy_curve (enemy * foe);
static bool enemy_grid (enemy * foe);
static bool enemy_lonely (enemy * foe);
static bool enemy_dead_fadeout (enemy * foe);
static void enemy_delete (enemy * foe);

/**
 * Allocate buffers and initialize structure of the enemies vessels 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
enemies_once_init (void)
{
  enemies_free ();

  if (!enemies_load ())
    {
      return FALSE;
    }

  /* allocate enemies data structure */
  if (enemies == NULL)
    {
      enemies = (enemy *) memory_allocation (MAX_OF_ENEMIES * sizeof (enemy));
      if (enemies == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'enemies'");
          return FALSE;
        }
    }

  /* colors if a ship's dead */
  enemy_dead_colors[0] = search_color (250, 250, 0);
  enemy_dead_colors[1] = search_color (250, 200, 0);
  enemy_dead_colors[2] = search_color (200, 200, 0);
  enemy_dead_colors[3] = search_color (200, 150, 0);
  enemy_dead_colors[4] = search_color (200, 150, 0);
  enemy_dead_colors[5] = search_color (150, 150, 0);
  enemy_dead_colors[6] = search_color (150, 100, 0);
  enemy_dead_colors[7] = search_color (100, 100, 0);
  enemy_dead_colors[8] = search_color (100, 50, 0);
  enemy_dead_colors[9] = search_color (50, 50, 0);
  enemy_dead_colors[10] = search_color (50, 0, 0);

  enemies_init ();
  return TRUE;
}

/**
 * Load all enemies
 *  21 small enemies for grid phase (assault by waves)
 *  - 40 animations per sprite (32 for rotation and 8 for still position)
 *  21 bloodsuckers for grid phase (assault by waves)
 *  - 8 animations per sprite
 *  40 lonely foes
 *  - 32 animations per sprite
 *  8 special objects used by the guardians
 *  - 32 animations per sprite
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
enemies_load (void)
{
  char *addr;
  image *img;
  char *filedata = load_file ("graphics/sprites/all_enemies.spr");
  if (filedata == NULL)
    {
      return FALSE;
    }
  addr = filedata;
  /* load 21 small enemies of 40 images each (grid phase) */
  img = &enemi[0][0];
  addr =
    images_read (img, ENEMIES_MAX_SMALL_TYPES, IMAGES_MAXOF, addr,
                 IMAGES_MAXOF);
  if (addr == NULL)
    {
      return FALSE;
    }
  /* load 21 bloodsuckers of 8 images each (grid phase) */
  img = img + (ENEMIES_MAX_SMALL_TYPES * IMAGES_MAXOF);
  addr =
    images_read (img, ENEMIES_MAX_BLOODSUCKER_TYPES,
                 BLOODSUCKER_NUM_OF_IMAGES, addr, IMAGES_MAXOF);
  if (addr == NULL)
    {
      return FALSE;
    }
  /* load 40 lonely foes + 8 special objects of 32 images each */
  img = img + (ENEMIES_MAX_BLOODSUCKER_TYPES * IMAGES_MAXOF);
  addr =
    images_read (img, LONELY_FOES_MAX_OF + ENEMIES_MAX_SPECIAL_TYPES,
                 ENEMIES_SPECIAL_NUM_OF_IMAGES, addr, IMAGES_MAXOF);
  if (addr == NULL)
    {
      return FALSE;
    }
  free_memory (filedata);
  return TRUE;
}

/**
 * Convert enemies from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
enemies_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/enemies/bloodsuker-%02d/enemy-%02d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes",
               (Sint32) strlen (model) + 1);
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/enemies"))
    {
      return FALSE;
    }
  for (type = 0; type < ENEMIES_MAX_SMALL_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/enemies/enemies-%02d", type + 1);
      create_dir (filename);
      for (frame = 0; frame < IMAGES_MAXOF; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/enemies/enemies-%02d/enemy-%01d.png",
                   type + 1, frame);
          if (!image_to_png (&enemi[type][frame], filename))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  for (type = 0; type < ENEMIES_MAX_BLOODSUCKER_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/enemies/bloodsuker-%02d", type + 1);
      if (!create_dir (filename))
        {
          return FALSE;
        }
      for (frame = 0; frame < BLOODSUCKER_NUM_OF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/enemies/bloodsuker-%02d/enemy-%02d.png",
                   type + 1, frame);
          if (!image_to_png
              (&enemi[ENEMIES_MAX_SMALL_TYPES + type][frame], filename))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }

  for (type = 0; type < LONELY_FOES_MAX_OF + ENEMIES_MAX_SPECIAL_TYPES;
       type++)
    {
      sprintf (filename, EXPORT_DIR "/enemies/special-%02d", type + 1);
      if (!create_dir (filename))
        {
          return FALSE;
        }
      for (frame = 0; frame < ENEMIES_SPECIAL_NUM_OF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/enemies/special-%02d/enemy-%02d.png",
                   type + 1, frame);
          if (!image_to_png
              (&enemi
               [ENEMIES_MAX_SMALL_TYPES + ENEMIES_MAX_BLOODSUCKER_TYPES +
                type][frame], filename))
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
 * Release memory used for the enemies 
 */
void
enemies_free (void)
{
  image *img;
  /* free 21 small enemies */
  img = &enemi[0][0];
  images_free (img, ENEMIES_MAX_SMALL_TYPES, IMAGES_MAXOF, IMAGES_MAXOF);
  /* free 21 bloodsuckers */
  img = img + (ENEMIES_MAX_SMALL_TYPES * IMAGES_MAXOF);
  images_free (img, ENEMIES_MAX_BLOODSUCKER_TYPES, BLOODSUCKER_NUM_OF_IMAGES,
               IMAGES_MAXOF);
  /* free 40 lonely foes + 8 special objects  */
  img = img + (ENEMIES_MAX_BLOODSUCKER_TYPES * IMAGES_MAXOF);
  images_free (img, LONELY_FOES_MAX_OF + ENEMIES_MAX_SPECIAL_TYPES,
               ENEMIES_SPECIAL_NUM_OF_IMAGES, IMAGES_MAXOF);

  if (enemies != NULL)
    {
      free_memory ((char *) enemies);
      enemies = NULL;
    }
}

/**
 * Initialize structure of the enemies vessels
 */
void
enemies_init (void)
{
  Sint32 i;
  enemy *foe;

  /* release all enemies */
  for (i = 0; i < MAX_OF_ENEMIES; i++)
    {
      foe = &enemies[i];
      foe->next = NULL;
      foe->is_enabled = FALSE;
      foe->dead = FALSE;
      foe->visible_dead = FALSE;
      /* clear index of color table */
      foe->dead_color_index = 0;
      foe->invincible = 0;
      foe->retournement = 0;
      foe->change_dir = 0;
      foe->id = i;
    }
  enemy_first = NULL;
  enemy_last = NULL;
  num_of_enemies = 0;
}

/**
 * Kill all enemies 
 */
void
enemies_kill (void)
{
  enemy *foe;
  Sint32 i;
  foe = enemy_get_first ();
  if (foe == NULL)
    {
      return;
    }
  for (i = 0; i < num_of_enemies; i++, foe = foe->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (foe == NULL && i < (num_of_enemies - 1))
        {
          LOG_ERR ("foe->next is null %i/%i", i, num_of_enemies);
          break;
        }
#endif
      if (!foe->dead)
        {
          enemy_set_fadeout (foe);
          explosions_add_serie (foe);
        }
    }
}

/**
 * Handling of all the possible types of enemies 
 */
void
enemies_handle (void)
{
  Sint32 i;
  enemy *foe;
  sprite *spr;
  foe = enemy_get_first ();
  if (foe == NULL)
    {
      return;
    }

  /* process each enemy */
  for (i = 0; i < num_of_enemies; i++, foe = foe->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (foe == NULL && i < (num_of_enemies - 1))
        {
          LOG_ERR ("foe->next is null %i/%i", i, num_of_enemies);
          break;
        }
#endif
      spr = &foe->spr;
      switch (foe->displacement)
        {
          /* 
           * phase 1: curves (little skirmish) 
           */
        case DISPLACEMENT_CURVE:
          if (!enemy_curve (foe))
            {
              enemy_delete (foe);
            }
          break;

          /*
           * phase 2: grids (enemy wave like Space Invaders)
           */
        case DISPLACEMENT_GRID:
          if (!enemy_grid (foe))
            {
              enemy_delete (foe);
            }
          break;

          /*
           * all phases: lonely foe
           */
        case DISPLACEMENT_LONELY_FOE:
          if (!enemy_lonely (foe))
            {
              enemy_delete (foe);
            }
          break;

          /*
           * guardians phase
           */
        case DISPLACEMENT_GUARDIAN:
          {
            if (guardian->number == 0)
              {
                break;
              }
            /* guardian is dead? */
            if (foe->dead)
              {
                /* fade-out effect in progress? */
                if (enemy_dead_fadeout (foe))
                  {
                    /* no, guardian is really dead */
                    foe->dead = FALSE;
                    enemy_delete (foe);
                    energy_gauge_guard_is_update = TRUE;
                    break;
                  }
                enemies_kill ();
              }
            /*  guardian is always alive */
            else
              {
                /* collision between shockwave and guardian? */
                if (shockwave_collision (foe))
                  {
                    enemy_set_fadeout (foe);
                    /* add explosions */
                    explosions_add_serie (foe);
                    explosions_fragments_add (spr->xcoord +
                                              spr->img[foe->spr.
                                                       current_image]->x_gc -
                                              8,
                                              spr->ycoord +
                                              spr->img[spr->current_image]->
                                              y_gc - 8, 1.0, 4, 0, 1);
                  }
                guardian_handle (foe);
              }
          }
          break;
        }
    }
}

/** 
 * Phase 1: curves (little skirmish) 
 * @param foe Pointer to a enemy structure
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_curve (enemy * foe)
{
  Sint32 k;
  sprite *spr = &foe->spr;

  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* increment index to next position on
       * the precalculated curve table */
      foe->pos_vaiss[POS_CURVE]++;
      if (foe->pos_vaiss[POS_CURVE] >=
          initial_curve[foe->num_courbe].nbr_pnt_curve)
        {
          /* remove enemy from the list */
          return FALSE;
        }
      /* update x and y coordinates */
      spr->xcoord +=
        (float) initial_curve[foe->num_courbe].
        delta_x[foe->pos_vaiss[POS_CURVE]];
      spr->ycoord +=
        (float) initial_curve[foe->num_courbe].
        delta_y[foe->pos_vaiss[POS_CURVE]];
    }
  /* set current image of the enemy sprite */
  spr->current_image =
    initial_curve[foe->num_courbe].angle[foe->pos_vaiss[POS_CURVE]];
  /* 
   * check if the sprite is visible or not 
   */
  if (((Sint16) spr->xcoord +
       spr->img[spr->current_image]->w) <
      offscreen_startx
      || ((Sint16) spr->ycoord +
          spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >=
      offscreen_starty + offscreen_height_visible
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      /* enemy is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      return TRUE;
    }

  /* 
   * enemy is dead? 
   */
  if (foe->dead)
    {
      /* gradual disappearance of the enemy? */
      if (enemy_dead_fadeout (foe))
        {
          /* enemy's really dead */
          foe->dead = FALSE;
          /* remove enemy from the list */
          return FALSE;
        }
      else
        {
          return TRUE;
        }
    }

  /* enemy is visible, perform the tests of collision */
  foe->visible = TRUE;
  /* draw the enemy sprite */
  enemy_draw (foe);

  /* collision between shockwave and current enemy? */
  if (shockwave_collision (foe))
    {
      /* add an explosion to the list */
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      /* add an explosion fragments to the list  */
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                4, 0, 1);
      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      return TRUE;
    }

  /* 
   * control enemy shots  
   */
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* decrease delay before next shot */
      foe->fire_rate_count--;

      /* check if the enemy fire a not shot */
      if (foe->fire_rate_count <= 0
          && num_of_shots <
          ((MAX_OF_SHOTS - 1) - spr->img[spr->current_image]->numof_cannons))
        {
          /* reset shot time-frequency */
          foe->fire_rate_count = foe->fire_rate;
          for (k = 0;
               (k <
                spr->img[spr->current_image]->numof_cannons
                && num_of_shots <
                ((MAX_OF_SHOTS - 1) -
                 spr->img[spr->current_image]->numof_cannons)); k++)
            {
              shot_enemy_add (foe, k);
            }
        }
    }

  if (gameover_enable)
    {
      return TRUE;
    }

  if (satellites_enemy_collisions (foe, 4))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                3, 0, 1);

      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      return TRUE;
    }

  /* collisions between extra guns and enemy */
  if (guns_enemy_collisions (foe, 4))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      /* add an explosion fragments to the list  */
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                3, 0, 1);
      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      return TRUE;
    }

  if (spaceship_enemy_collision
      (foe,
       (float) initial_curve[foe->num_courbe].delta_x[foe->
                                                      pos_vaiss[POS_CURVE]],
       (float) initial_curve[foe->num_courbe].delta_y[foe->
                                                      pos_vaiss[POS_CURVE]]))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                4, 0, 2);
      enemy_set_fadeout (foe);
      return TRUE;
    }
  return TRUE;
}

/**
 * Phase 2: grids (enemy wave like Space Invaders)
 * @param foe Pointer to a enemy structure
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_grid (enemy * foe)
{
  Sint32 k;
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update x and y coordinates */
      spr->xcoord = grid.coor_x + (foe->pos_vaiss[XCOORD] << 4);
      spr->ycoord = grid.coor_y + (foe->pos_vaiss[YCOORD] << 4);
    }

  /* 
   * check if the sprite is visible or not 
   */
  if (((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || ((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      foe->visible = FALSE;
      if ((Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
        {
          /* remove enemy from the list
           * if enemy disappear at the bottom on the screen */
          return FALSE;
        }
      else
        {
          /* enemy is not visible, don't perform the tests of collision */
          return TRUE;
        }
    }
  /* 
   * enemy is dead? 
   */
  if (foe->dead)
    {
      /* gradual disappearance of the enemy? */
      if (enemy_dead_fadeout (foe))
        {
          /* enemy's really dead */
          foe->dead = FALSE;
          /* remove enemy from the list */
          return FALSE;
        }
      else
        {
          return TRUE;
        }
    }
  /* enemy is visible, perform the tests of collision */
  foe->visible = TRUE;
  /* inc. counter delay between two images */
  spr->anim_count++;
  /* value of delay between two images reached? */
  if (spr->anim_count >= spr->anim_speed)
    {
      /* clear counter delay between two images */
      spr->anim_count = 0;
      /* flip to the next image */
      spr->current_image++;
      /* check if last image has been reached  */
      if (spr->current_image >= spr->numof_images)
        {
          /* resets the animation to the first image of the animation sequence */
          spr->current_image = 0;
        }
    }
  /* draw the enemy sprite */
  enemy_draw (foe);

  /* collision between shockwave(s) and current enemy? */
  if (shockwave_collision (foe))
    {
      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      /* add an explosion to the list */
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      /* add an explosion fragments to the list  */
      explosions_fragments_add (spr->xcoord +
                                spr->img[foe->spr.current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[foe->spr.current_image]->y_gc - 8,
                                1.5, 5, 0, 1);
      return TRUE;
    }

  /* 
   * control enemy shots  
   */
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* decrease delay before next shot */
      foe->fire_rate_count--;
      /* check if the enemy fire a not shot */
      if (foe->fire_rate_count <= 0 && num_of_shots < (MAX_OF_SHOTS - 1))
        {
          /* reset shot time-frequency */
          foe->fire_rate_count =
            grid.shoot_speed[foe->pos_vaiss[XCOORD]][foe->pos_vaiss[YCOORD]];
          /* process each origin of the shot (location of the cannon) */
          for (k = 0; k < spr->img[spr->current_image]->numof_cannons; k++)
            {
              shot_enemy_add (foe, k);
            }
        }
    }

  if (gameover_enable)
    {
      return TRUE;
    }
  if (satellites_enemy_collisions (foe, 2))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                3, 0, 1);

      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      return TRUE;
    }

  /* collisions between extra guns and enemy */
  if (guns_enemy_collisions (foe, 2))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                3, 0, 1);
      /* enable the gradual disappearance of the enemy */
      enemy_set_fadeout (foe);
      return TRUE;
    }
  if (spaceship_enemy_collision (foe, grid.speed_x, grid.vit_dep_y))
    {
      explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
      explosions_fragments_add (spr->xcoord +
                                spr->img[spr->current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[spr->current_image]->y_gc - 8, 1.0,
                                3, 0, 1);
      enemy_set_fadeout (foe);
      return TRUE;
    }
  return TRUE;
}

/**
 * Handle Souke foe: can correspond to two types of missiles homing head
 * One type fired by the guardian 5, and another type fired by the 
 * guardians 6 and 11
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_soukee (enemy * foe)
{
  float a;
  spaceship_struct *ship = spaceship_get ();
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      a =
        calc_target_angle ((Sint16)
                           (spr->xcoord +
                            spr->img[foe->spr.current_image]->x_gc),
                           (Sint16) (foe->spr.ycoord +
                                     foe->spr.img[foe->spr.current_image]->
                                     y_gc),
                           (Sint16) (ship->spr.xcoord +
                                     ship->spr.img[ship->spr.current_image]->
                                     x_gc),
                           (Sint16) (ship->spr.ycoord +
                                     ship->spr.img[ship->spr.current_image]->
                                     y_gc));
      foe->angle_tir = get_new_angle (foe->angle_tir, a, foe->agilite);

      /* update x and y coordinates of the missile */
      spr->xcoord =
        shot_x_move (foe->angle_tir,
                     spr->speed,
                     spr->xcoord - spr->img[foe->img_old_angle]->x_gc);
      spr->ycoord =
        shot_y_move (foe->angle_tir,
                     spr->speed,
                     spr->ycoord - spr->img[foe->img_old_angle]->y_gc);
      /* search image to draw  corresponding in the angle */
      if (sign (foe->angle_tir) < 0)
        {
          foe->img_angle = (Sint16) ((foe->angle_tir + TWO_PI) / PI_BY_16);
        }
      else
        {
          foe->img_angle = (Sint16) (foe->angle_tir / PI_BY_16);
        }
      /* avoid an negative index of table */
      foe->img_angle = (Sint16) abs (foe->img_angle);
      /* avoid a superior shoting angle among images of the sprite */
      if (foe->img_angle >= spr->numof_images)
        {
          foe->img_angle = (Sint16) (spr->numof_images - 1);
        }
      foe->img_old_angle = foe->img_angle;
      /* new coordinates of the missile */
      spr->xcoord += spr->img[foe->img_old_angle]->x_gc;
      spr->ycoord += spr->img[foe->img_old_angle]->y_gc;
      /* set angle of image to draw */
      spr->current_image = foe->img_angle;
    }

  /* check if the sprite is visible or not  */
  if (((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || ((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* check if the sprite is out of clip area */
      if ((Sint16) spr->xcoord < (offscreen_clipsize - 64)
          || (Sint16) spr->ycoord < (offscreen_clipsize - 64)
          || (Sint16) spr->ycoord >
          (offscreen_clipsize + offscreen_height_visible + 64)
          || (Sint16) spr->xcoord >
          (offscreen_clipsize + offscreen_width_visible + 64))
        {
          /* remove foe from the list if foe is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* foe is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe moving towards bottom of screen and goe back to the top of the screen
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_down_and_back (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* foe turn around? */
      if (foe->retournement)
        {
          spr->anim_count++;
          if (spr->anim_count >= spr->anim_speed)
            {
              /* clear counter delay between two images */
              spr->anim_count = 0;
              /* flip to the next image */
              spr->current_image++;
              /* check if last image has been reached  */
              if (spr->current_image >= (spr->numof_images - 1))
                {
                  spr->current_image = (Sint16) (spr->numof_images - 1);
                  /* indicate that the turn around is finished */
                  foe->retournement = FALSE;
                  /* indicate taht foe has just changed direction */
                  foe->change_dir = TRUE;
                }
            }
        }
      /* otherwise the enemy vessel is not in its phase of turn around */
      else
        {
          /* check if the foe has not changed a direction yet */
          if (!foe->change_dir)
            {
              /* check speed of displacement is null */
              if (spr->speed >= -0.1 && spr->speed <= 0.1)
                {
                  /* indicate that the foe has to turn around */
                  foe->retournement = TRUE;
                }
            }
        }
      /* decrease speed of movement of the foe */
      spr->speed -= 0.02f;
      spr->ycoord += spr->speed;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* check if the sprite is out of clip area */
      if ((Sint16) spr->ycoord < offscreen_starty && foe->change_dir)
        {
          /* remove foe from the list if it is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* enemy is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe moving from top of screen and towards the bottom of the screen
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_towards_bottom (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update y-coordinate of the foe */
      spr->ycoord += spr->speed;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      if ((Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
        {
          /* remove foe from the list if it is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* enemy is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe moving from bottom of screen and towards the top of the screen
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_towards_top (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update y-coordinate of the foe */
      spr->ycoord += spr->speed;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* check if the sprite is out of clip area */
      if ((Sint16) spr->ycoord < offscreen_starty)
        {
          /* remove foe from the list if it is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* foe is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe moving from right of screen and towards the left of the screen
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_towards_left (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update x-coordinate of the foe */
      spr->xcoord += spr->speed;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* check if the sprite is out of clip area */
      if (((Sint16) spr->xcoord +
           spr->img[spr->current_image]->w) < offscreen_startx)
        {
          /* remove foe from the list if it is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* foe is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe moving from left of screen and towards the right of the screen
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_towards_right (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update x-coordinate of the foe */
      spr->xcoord += spr->speed;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* check if the sprite is out of clip area */
      if ((Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
        {
          /* remove foe from the list if it is out of clip area */
          return FALSE;
        }
    }
  else
    {
      /* foe is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * Foe follow a curve 
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_follow_curve (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* increment index to next position on the precalculated curve table */
      foe->pos_vaiss[POS_CURVE]++;
      /* disable the foe if out the curve */
      if (foe->pos_vaiss[POS_CURVE] >=
          initial_curve[foe->num_courbe].nbr_pnt_curve)
        {
          return FALSE;
        }
      /* update y and x coordinates of the foe */
      spr->xcoord +=
        (float) initial_curve[foe->num_courbe].delta_x[foe->
                                                       pos_vaiss[POS_CURVE]];
      spr->ycoord +=
        (float) initial_curve[foe->num_courbe].delta_y[foe->
                                                       pos_vaiss[POS_CURVE]];
    }
  /* set current image of the foe sprite */
  spr->current_image =
    initial_curve[foe->num_courbe].angle[foe->pos_vaiss[POS_CURVE]];
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible
      || ((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible)
    {
      /* foe is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
    }
  else
    {
      /* foe is visible, perform the tests of collision */
      foe->visible = TRUE;
    }
  return TRUE;
}

/**
 * A meteor is considered as lonely foe 
 * @param foe pointer to enemy structure 
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely_meteor (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (!player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* update y and x coordinates of the meteor */
      spr->xcoord += foe->x_speed;
      spr->ycoord += foe->y_speed;
      /* decrease delay time before destruction */
      foe->timelife--;
    }
  /* check if the sprite is visible or not  */
  if (((Sint16) spr->ycoord + spr->img[spr->current_image]->h) <
      offscreen_starty
      || (Sint16) spr->ycoord >= offscreen_starty + offscreen_height_visible
      || ((Sint16) spr->xcoord + spr->img[spr->current_image]->w) <
      offscreen_startx
      || (Sint16) spr->xcoord >= offscreen_startx + offscreen_width_visible)
    {
      /* meteor is not visible, don't perform the tests of collision */
      foe->visible = FALSE;
      /* if time delay has passed we can destroy the meteor */
      if (foe->timelife < 0)
        {
          return FALSE;
        }
    }
  else
    {
      /* meteor is visible, perform the tests of collision */
      foe->visible = TRUE;
      /* increase animation delay counter */
      spr->anim_count++;
      /* value of delay between two images reached? */
      if (spr->anim_count >= spr->anim_speed)
        {
          /* clear counter delay between two images */
          spr->anim_count = 0;
          /* flip to the next image */
          spr->current_image = (Sint16) (spr->current_image + foe->sens_anim);
          /* check if last image has been reached */
          if (spr->current_image >= spr->numof_images)
            {
              /* resets the animation to the first image of the animation sequence */
              spr->current_image = 0;
            }
          /* check if first image has been reached */
          if (spr->current_image < 0)
            {
              /* resets the animation to the last image of the animation sequence */
              spr->current_image = METEOR_NUMOF_IMAGES - 1;
            }
        }
    }
  return TRUE;
}

/**
 * Add a new shot from a lonely foe
 * @param foe pointer to enemy structure 
 */
static void
lonely_foe_fire (enemy * foe)
{
  Sint32 canon_pos;
  if (player_pause || menu_status != MENU_OFF
      || menu_section != NO_SECTION_SELECTED)
    {
      return;
    }
  /* decrease delay before next shot */
  foe->fire_rate_count--;

  /* check if the foe fire a not shot */
  if (foe->fire_rate_count > 0 || num_of_shots >= (MAX_OF_SHOTS - 1))
    {
      return;
    }

  /* if foe aim the cannon at the spaceship, also fire it! */
  if ((foe->type == PERTURBIANS ||
       foe->type == GAFFIES ||
       foe->type == HOCKYS ||
       foe->type == BAINIES ||
       foe->type == TOUTIES ||
       foe->type == ARCHINIANS) && foe->img_angle != foe->spr.current_image)
    {
      return;
    }

  /* reset shot time-frequency */
  foe->fire_rate_count = foe->fire_rate;
  /* process each origin of the shot (location of the cannon) */
  for (canon_pos = 0;
       canon_pos < foe->spr.img[foe->spr.current_image]->numof_cannons;
       canon_pos++)
    {
      if (foe->type == NAGGYS)
        {
          lonely_foe_add_gozuky (foe, canon_pos);
        }
      else
        {
          shot_enemy_add (foe, canon_pos);
        }
    }
}

/**
 * Handle animation of a lonely foe with a adjustable canon
 * @param foe pointer to enemy structure 
 */
static void
lonely_foe_anim_adjustable_cannon (enemy * foe)
{
  sprite *spr = &foe->spr;
  Uint32 i;
  float angle;
  Sint32 anim_count, length1, length2;
  spaceship_struct *ship = spaceship_get ();
  /* foe has a cannon with adjustable direction */
  if (foe->type != PERTURBIANS
      && foe->type != GAFFIES
      && foe->type != HOCKYS
      && foe->type != BAINIES
      && foe->type != TOUTIES && foe->type != ARCHINIANS)
    {
      return;
    }
  /* cannon rotation to follow the player's spaceship */
  if (foe->img_angle != spr->current_image)
    {
      if (foe->sens_anim)
        {
          /* increase animation delay counter */
          spr->anim_count++;
          /* value of delay between two images reached? */
          if (spr->anim_count >= spr->anim_speed)
            {
              /* clear counter delay between two images */
              spr->anim_count = 0;
              /* flip to the previous image */
              spr->current_image--;
              /* check if first image has been reached */
              if (spr->current_image < 0)
                {
                  /* resets the animation to the last image of the animation sequence */
                  spr->current_image = (Sint16) (spr->numof_images - 1);
                }
            }
        }
      else
        {
          /* increase animation delay counter */
          spr->anim_count++;
          /* value of delay between two images reached? */
          if (spr->anim_count >= spr->anim_speed)
            {
              /* clear counter delay between two images */
              spr->anim_count = 0;
              /* flip to the next image */
              spr->current_image++;
              /* check if last image has been reached */
              if (spr->current_image >= spr->numof_images)
                {
                  /* resets the animation to the first image of the animation sequence */
                  spr->current_image = 0;
                }
            }
        }
      /* count length in the oposite trigonometrical direction */
      anim_count = spr->current_image;
      length1 = 0;
      /* process each angle */
      for (i = 0; i < 32; i++)
        {
          length1++;
          anim_count++;
          if (anim_count > 31)
            {
              anim_count = 0;
            }
          if (foe->img_angle == anim_count)
            {
              i = 32;
            }
        }
      /* count length in the trigonometrical direction */
      anim_count = spr->current_image;
      length2 = 0;
      /* process each angle */
      for (i = 0; i < 32; i++)
        {
          length2++;
          anim_count--;
          if (anim_count < 0)
            {
              anim_count = 31;
            }
          if (foe->img_angle == anim_count)
            {
              i = 32;
            }
        }
      if (length1 < length2)
        {
          /* clockwise */
          foe->sens_anim = 0;
        }
      else
        {
          /* anticlockwise */
          foe->sens_anim = 1;
        }
    }
  /* search cannon position compared to the direction of spaceship  */
  angle =
    calc_target_angle ((Sint16)
                       (spr->xcoord +
                        spr->img[foe->spr.current_image]->x_gc),
                       (Sint16) (spr->ycoord +
                                 spr->img[spr->current_image]->y_gc),
                       (Sint16) (ship->spr.xcoord +
                                 ship->spr.img[ship->spr.current_image]->
                                 x_gc),
                       (Sint16) (ship->spr.ycoord +
                                 ship->spr.img[ship->spr.current_image]->
                                 y_gc));
  /* search image to draw determined upon angle */
  if (sign (angle) < 0)
    {
      foe->img_angle = (Sint16) ((angle + TWO_PI) / PI_BY_16);
    }
  else
    {
      foe->img_angle = (Sint16) (angle / PI_BY_16);
    }
  /* avoid a negative table index */
  foe->img_angle = (Sint16) abs (foe->img_angle);
  /* avoid shot angle higher than the number of images of the sprite */
  if (foe->img_angle >= spr->numof_images)
    {
      foe->img_angle = (Sint16) (spr->numof_images - 1);
    }
}

/**
 * Handle a lonely foe
 * @param foe Pointer to a enemy structure
 * @return FALSE if the enemy must be removed from the list
 */
static bool
enemy_lonely (enemy * foe)
{
  //Sint32 k;
  sprite *spr = &foe->spr;

  /* ckeck type of lonely foe */
  switch (foe->type)
    {
      /* missiles homing head fired by the guardians */
    case SOUKEE:
      if (!enemy_lonely_soukee (foe))
        {
          return FALSE;
        }
      break;

      /* foe moving towards bottom of screen and goe back to the top */
    case VIONIEES:
    case ANGOUFF:
    case CLOWNIES:
    case EFFIES:
    case SAPOUCH:
      if (!enemy_lonely_down_and_back (foe))
        {
          return FALSE;
        }
      break;

      /* foe moving from the top of the screen towards the bottom  */
    case MILLOUZ:
    case TOUBOUG:
    case BELCHOUTIES:
    case DEMONIANS:
    case FIDGETINIANS:
    case DIMITINIANS:
    case ENSLAVEERS:
    case DIVERTIZERS:
    case HORRIBIANS:
    case DEVILIANS:
    case ROUGHLEERS:
    case ABASCUSIANS:
    case PERTURBIANS:
    case EARTHINIANS:
    case DISGOOSTEES:
    case BIRIANSTEES:
    case TODHAIRIES:
    case DEFECTINIANS:
    case SOONIEES:
    case GAFFIES:
    case SHURIKY:
    case GOZUKY:
    case QUIBOULY:
      if (!enemy_lonely_towards_bottom (foe))
        {
          return FALSE;
        }
      break;

      /* foe moving from the bottom of the screen towards the top  */
    case SWORDINIANS:
    case HOCKYS:
    case CARRYONIANS:
    case ARCHINIANS:
    case TOURNADEE:
      if (!enemy_lonely_towards_top (foe))
        {
          return FALSE;
        }
      break;

      /* foe moving from the right of the screen towards the left  */
    case FEABILIANS:
    case BITTERIANS:
    case TOUTIES:
    case SUBJUGANEERS:
      if (!enemy_lonely_towards_left (foe))
        {
          return FALSE;
        }
      break;

      /* foe moving from the left of the screen towards the right */
    case PAINIANS:
    case BAINIES:
    case NAGGYS:
      if (!enemy_lonely_towards_right (foe))
        {
          return FALSE;
        }
      break;

      /* foe follow a curve */
    case ROTIES:
    case STENCHIES:
    case MADIRIANS:
    case BLAVIRTHE:
    case BLEUERCKS:
    case SAAKAMIN:
      if (!enemy_lonely_follow_curve (foe))
        {
          return FALSE;
        }
      break;

      /* meteor is considered as lonely foe, handle it */
    case BIGMETEOR:
    case NORMALMETEOR:
    case SMALLMETEOR:
      if (!enemy_lonely_meteor (foe))
        {
          return FALSE;
        }
      break;
    }

  /* the lonely foe is not visible, thus the display is not handled */
  if (!foe->visible)
    {
      return TRUE;
    }

  /* Cyclic animation of 32 images */
  if (foe->type != BIGMETEOR
      && foe->type != NORMALMETEOR
      && foe->type != SMALLMETEOR
      && foe->type != BLAVIRTHE
      && foe->type != BLEUERCKS
      && foe->type != ROTIES
      && foe->type != STENCHIES
      && foe->type != MADIRIANS
      && foe->type != PERTURBIANS
      && foe->type != HOCKYS
      && foe->type != BAINIES
      && foe->type != TOUTIES
      && foe->type != VIONIEES
      && foe->type != ANGOUFF
      && foe->type != CLOWNIES
      && foe->type != EFFIES
      && foe->type != SAPOUCH
      && foe->type != DIMITINIANS
      && foe->type != GAFFIES
      && foe->type != ARCHINIANS
      && foe->type != TODHAIRIES
      && foe->type != DEFECTINIANS
      && foe->type != SOONIEES && foe->type != SOUKEE)
    {
      /* increase animation delay counter */
      spr->anim_count++;
      /* value of delay between two images reached? */
      if (spr->anim_count >= spr->anim_speed)
        {
          /* clear counter delay between two images */
          spr->anim_count = 0;
          /* flip to the next image */
          spr->current_image++;
          /* check if last image has been reached */
          if (spr->current_image >= spr->numof_images)
            {
              /* resets the animation to the first image of the animation sequence */
              spr->current_image = 0;
            }
        }
    }

  /* check if foe move from right towards left and vice-versa, and update next
   * frame */
  if (foe->type == DIMITINIANS
      || foe->type == TODHAIRIES
      || foe->type == DEFECTINIANS || foe->type == SOONIEES)
    {
      /* reverse animation direction? */
      if (foe->sens_anim)
        {
          /* inc. counter delay between two images */
          spr->anim_count++;
          /* value of delay between two images reached? */
          if (spr->anim_count >= spr->anim_speed)
            {
              /* clear counter delay between two images */
              spr->anim_count = 0;
              /* flip to the previous image */
              spr->current_image--;
              /* check if first image has been reached */
              if (spr->current_image < 0)
                {
                  /* block animation to the first image of the animation sequence */
                  spr->current_image = 0;
                  /* set forward animation direction */
                  foe->sens_anim = 0;
                }
            }
        }
      /* forward animation direction */
      else
        {
          /* inc. counter delay between two images */
          spr->anim_count++;
          /* value of delay between two images reached? */
          if (spr->anim_count >= spr->anim_speed)
            {
              /* clear counter delay between two images */
              spr->anim_count = 0;
              /* flip to the next image */
              spr->current_image++;
              /* check if last image has been reached */
              if (spr->current_image >= spr->numof_images)
                {
                  /* block animation to the last image of the animation sequence */
                  spr->current_image = (Sint16) (spr->numof_images - 1);
                  /* set reverse animation direction */
                  foe->sens_anim = 1;
                }
            }
        }
    }


  /* Handle animation of a lonely foe with a adjustable canon */
  lonely_foe_anim_adjustable_cannon (foe);

  /* foe is dead? */
  if (foe->dead)
    {
      /* gradual disappearance of the foe */
      if (enemy_dead_fadeout (foe))
        {
          /* foe must be removed from the list of ennmies */
          foe->dead = FALSE;
          return FALSE;
        }
      else
        {
          return TRUE;
        }
    }

  /* draw the foe */
  enemy_draw (foe);
  /* collision between shockwave and current enemy? */
  if (shockwave_collision (foe))
    {
      enemy_set_fadeout (foe);
      /* add a serie of explosions */
      explosions_add_serie (foe);
      /* add one or more explosions fragments */
      explosions_fragments_add (spr->xcoord +
                                spr->img[foe->spr.current_image]->x_gc - 8,
                                spr->ycoord +
                                spr->img[foe->spr.current_image]->y_gc - 8,
                                1.0, 4, 0, 1);
      return TRUE;
    }

  /* add a new shot from a lonely foe */
  lonely_foe_fire (foe);

  if (!gameover_enable)
    {
      enemy_satellites_collisions (foe);
      enemy_guns_collisions (foe);
      enemy_spaceship_collision (foe);
    }
  return TRUE;
}

/** 
 * Enable the gradual disappearance of an enemy
 * @param foe Pointer to a enemy structure
 */
void
enemy_set_fadeout (enemy * foe)
{

  /* fade-out effect enable */
  foe->dead = TRUE;
  foe->visible_dead = TRUE;
  /* initialize the index of color table */
  foe->dead_color_index = 0;
  foe->invincible = 20;
  foe->spr.energy_level = 0;
  /* handle the second sprite of the guardien
   * in the cases where the guardian is composed of 2 sprites */
  if (guardian->number > 0 && foe->displacement == DISPLACEMENT_GUARDIAN
      && guardian->foe[1] != NULL
      && guardian->foe[1]->displacement == DISPLACEMENT_GUARDIAN)
    {
      guardian->foe[1]->dead = TRUE;
      guardian->foe[1]->visible_dead = TRUE;
      guardian->foe[1]->dead_color_index = 0;
      guardian->foe[1]->invincible = 20;
      guardian->foe[1]->spr.energy_level = 0;
    }
}

/** 
 * Gradual disappearance of an enemy
 * @param foe Pointer to a enemy structure
 * @return TRUE enemy must be removed from the list
 */
static bool
enemy_dead_fadeout (enemy * foe)
{
  /* display the sprite mask */
  if (foe->visible_dead)
    {
      /* the sprite will be invisible in the next loop */
      foe->visible_dead = FALSE;
      /* draw the sprite mask with fade-out effect */
      draw_sprite_mask (enemy_dead_colors[foe->dead_color_index],
                        foe->spr.img[foe->spr.current_image],
                        (Sint32) (foe->spr.xcoord),
                        (Sint32) (foe->spr.ycoord));
      /* next color */
      foe->dead_color_index++;
      /* block on the last color */
      if (foe->dead_color_index >= NUMOF_DEAD_COLORS)
        {
          foe->dead_color_index = NUMOF_DEAD_COLORS - 1;
        }
    }

  /* don't display the sprite mask */
  else
    {
      foe->visible_dead = TRUE;
      /* invincibility duration is decreased by one */
      foe->invincible--;
      if (foe->invincible <= 0)
        {
          foe->visible_dead = TRUE;
          /* enemy must be removed from the list */
          return TRUE;
        }
    }

  /* enemy must not be removed from the list yet */
  return FALSE;
}

/**
 * Check validty of enemies chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
enemy_check_chained_list (void)
{
  Uint32 i;
  enemy *foe;
  Sint32 count = 0;
  for (i = 0; i < MAX_OF_ENEMIES; i++)
    {
      foe = &enemies[i];
      if (foe->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_enemies)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, num_of_enemies=%i", count, num_of_enemies);
    }
  count = 0;
  foe = enemy_first;
  do
    {
      count++;
      foe = foe->next;
    }
  while (foe != NULL && count <= (MAX_OF_ENEMIES + 1));
  if (count != num_of_enemies)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_enemies=%i", count, num_of_enemies);
    }
  count = 0;
  foe = enemy_last;
  do
    {
      count++;
      foe = foe->previous;
    }
  while (foe != NULL && count <= (MAX_OF_ENEMIES + 1));
  if (count != num_of_enemies)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_enemies=%i", count, num_of_enemies);
    }
}
#endif

/** 
 * Return a free enemy element 
 * @return Pointer to a enemy structure, NULL if not enemy available 
 */
enemy *
enemy_get (void)
{
  Uint32 i;
  enemy *foe;
  for (i = 0; i < MAX_OF_ENEMIES; i++)
    {
      foe = &enemies[i];
      if (foe->is_enabled)
        {
          continue;
        }
      foe->is_enabled = TRUE;
      foe->next = NULL;
      if (num_of_enemies == 0)
        {
          enemy_first = foe;
          enemy_last = foe;
          enemy_last->previous = NULL;
        }
      else
        {
          enemy_last->next = foe;
          foe->previous = enemy_last;
          enemy_last = foe;
        }
      num_of_enemies++;
#ifdef UNDER_DEVELOPMENT
      enemy_check_chained_list ();
#endif
      return foe;
    }
  LOG_ERR ("no more element enemy is available");
  return NULL;
}

/** 
 * Return a first enemy element 
 * @return Pointer to a enemy structure 
 */
enemy *
enemy_get_first (void)
{
  return enemy_first;
}

/** 
 * Remove a enemy element from list
 * @param Pointer to a enemy structure 
 */
void
enemy_delete (enemy * foe)
{
  foe->is_enabled = FALSE;
  num_of_enemies--;
  if (enemy_first == foe)
    {
      enemy_first = foe->next;
    }
  if (enemy_last == foe)
    {
      enemy_last = foe->previous;
    }
  if (foe->previous != NULL)
    {
      foe->previous->next = foe->next;
    }
  if (foe->next != NULL)
    {
      foe->next->previous = foe->previous;
    }
}

/**
 * Draw the sprite of a vessel enemy
 * @param enemy_num enemy element index
 */
void
enemy_draw (enemy * foe)
{
  /* display white mask */
  if (foe->is_white_mask_displayed)
    {
      draw_sprite_mask (coulor[WHITE],
                        foe->spr.img[foe->spr.current_image],
                        (Sint32) (foe->spr.xcoord),
                        (Sint32) (foe->spr.ycoord));
      foe->is_white_mask_displayed = FALSE;
    }
  else
    {
      draw_sprite (foe->spr.img[foe->spr.current_image],
                   (Uint32) foe->spr.xcoord, (Uint32) foe->spr.ycoord);
    }
}

/** 
 * Collisions between extra guns and a guardian or loney foe
 * @param Pointer to a enemy structure
 */
void
enemy_guns_collisions (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (gameover_enable)
    {
      return;
    }
  if (!guns_enemy_collisions (foe, 0))
    {
      return;
    }
  explosions_add_serie (foe);
  explosions_fragments_add (spr->xcoord +
                            spr->img[spr->current_image]->x_gc - 8,
                            spr->ycoord +
                            spr->img[spr->current_image]->y_gc - 8, 0.5, 8, 0,
                            3);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.0, 8, 0, 2);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.5, 8, 0, 1);

  /* enable the gradual disappearance of the enemy */
  enemy_set_fadeout (foe);
}

/** 
 * Collisions between satellite protections and a guardian or loney foe
 * @param Pointer to a enemy structure
 */
void
enemy_satellites_collisions (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (gameover_enable)
    {
      return;
    }
  if (!satellites_enemy_collisions (foe, 0))
    {
      return;
    }
  explosions_add_serie (foe);
  explosions_fragments_add (spr->xcoord +
                            spr->img[spr->current_image]->x_gc - 8,
                            spr->ycoord +
                            spr->img[spr->current_image]->y_gc - 8, 0.5, 8, 0,
                            3);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.0, 8, 0, 2);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.5, 8, 0, 1);
  /* enable the gradual disappearance of the enemy */
  enemy_set_fadeout (foe);
}

/** 
 * Collision between spaceship and a guardian or loney foe
 * @param Pointer to a enemy structure
 */
void
enemy_spaceship_collision (enemy * foe)
{
  sprite *spr = &foe->spr;
  if (gameover_enable)
    {
      return;
    }
  if (!spaceship_enemy_collision (foe, foe->x_speed, foe->y_speed))
    {
      return;
    }
  explosions_add_serie (foe);
  explosions_fragments_add (spr->xcoord +
                            spr->img[spr->current_image]->x_gc - 8,
                            spr->ycoord +
                            spr->img[spr->current_image]->y_gc - 8, 0.5, 8, 0,
                            3);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.0, 8, 0, 2);
  explosions_fragments_add (spr->xcoord + spr->img[spr->current_image]->x_gc -
                            8,
                            spr->ycoord + spr->img[spr->current_image]->y_gc -
                            8, 1.5, 8, 0, 1);
  enemy_set_fadeout (foe);
}
