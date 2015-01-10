/**
 * @file grid_phase.c
 * @brief Handle grid phase (enemy wave like Space Invaders)
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: grid_phase.c,v 1.19 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "enemies.h"
#include "shots.h"
#include "grid_phase.h"
#include "guardians.h"
#include "meteors_phase.h"
#include "spaceship.h"

/** Frid level data structure */
grid_struct grid;

/**
 * Handle grid phase
 * enemies that assault by waves, sometimes led by a chief
 */
void
grid_handle (void)
{
  /* grid phase currently in progress? */
  if (!grid.is_enable)
    {
      /* grid phase not enable */
      return;
    }

  /* appearance of the grid of enemies? */
  if (grid.is_appearing)
    {
      grid.coor_y++;
      /* grid has reached its final position? */
      if (grid.coor_y >= offscreen_clipsize - 4)
        {
          /* the appearance is finished */
          grid.is_appearing = FALSE;
        }
      return;
    }

  /* movement toward right */
  if (grid.right_movement)
    {
      grid.coor_x += grid.vit_dep_x;
      grid.coor_y += grid.vit_dep_y;
      /* x coordinate maximum? */
      if (grid.coor_x > offscreen_clipsize + 32)
        {
          grid.coor_x = (float) (offscreen_clipsize + 32);
          /*set movement toward left */
          grid.right_movement = FALSE;
        }
      grid.speed_x = grid.vit_dep_x;
    }
  else
    /* movement toward left */
    {
      grid.coor_x -= grid.vit_dep_x;
      grid.coor_y += grid.vit_dep_y;
      /* x coordinate minimum? */
      if (grid.coor_x < offscreen_clipsize - 32)
        {
          grid.coor_x = (float) (offscreen_clipsize - 32);
          /*set movement toward right */
          grid.right_movement = TRUE;
        }
      grid.speed_x = -grid.vit_dep_x;
    }
}

/**
 * Check if the end of the grid phase
 */
void
grid_finished (void)
{
  /* grid phase currently in progress? */
  if (!grid.is_enable)
    {
      /* grid phase not enable */
      return;
    }
  /* grid is out of the screen or all enemies are dead? */
  if ((Sint16) grid.coor_y > 184 || num_of_enemies == 0)
    {
      /* meteors phase enable */
      meteor_activity = TRUE;
      num_of_meteors = 0;
      courbe.activity = FALSE;
      guardian->number = 0;
      grid.is_enable = FALSE;
    }
}

/**
 * Load grid level and initialize grid structure
 * @param num_grid Grid level number from 0 to 41
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
grid_load (Sint32 num_grid)
{
  Sint16 *ptr16;
  float *ptr32;
  Sint16 *dest;
  Sint32 i;
  char *source;
  meteors_images_free ();
  if (num_grid > MAX_NUM_OF_LEVELS || num_grid < 0)
    {
      num_grid = 0;
    }

  /* load grid level file */
  source = loadfile_num ("data/levels/grids_phase/grid_%02d.bin", num_grid);
  if (source == NULL)
    {
      return FALSE;
    }

  /* read grid speed of the displacement
   * read little endian float */
  ptr32 = (float *) source;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  convert32bits_2bigendian ((unsigned char *) ptr32);
#endif
  grid.vit_dep_x = (float) *(ptr32++);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  convert32bits_2bigendian ((unsigned char *) ptr32);
#endif
  grid.vit_dep_y = (float) *(ptr32++);
  grid.speed_x = grid.vit_dep_x;

  ptr16 = (Sint16 *) ptr32;
  /* read type of enemies on the grid */
  dest = &grid.enemy_num[0][0];
  for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }
  dest = &grid.shoot_speed[0][0];
  /* read shot time-frequency */
  for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
      *(dest++) = little_endian_to_short (ptr16++);
    }
  free_memory (source);
  return TRUE;
}

/**
 * Initialize grid phase
 */
void
grid_start (void)
{
  Sint32 i, j, k;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  for (i = 0; i < GRID_WIDTH; i++)
    {
      for (j = 0; j < GRID_HEIGHT; j++)
        {
          /* enemy exists? */
          if (grid.enemy_num[i][j] != -1)
            {
              /* get a new enemy element */
              foe = enemy_get ();
              if (foe == NULL)
                {
                  continue;
                }

              /* little enemy or big enemy (Bloodsuckers, chief of the wave)? */
              if (grid.enemy_num[i][j] < ENEMIES_MAX_SMALL_TYPES)
                {
                  /* set power of the destruction */
                  foe->spr.pow_of_dest = (Sint16) (1 + grid.enemy_num[i][j]);
                  /* the level of energy of the sprite */
                  foe->spr.energy_level =
                    (Sint16) ((ship->type << 1) + foe->spr.pow_of_dest);
                  /* size of the enemy sprite as 16x16 pixels */
                  foe->type = 0;
                }
              else
                /*
                 * Bloodsuckers, the chief of the wave
                 */
                {
                  /* set power of the destruction */
                  foe->spr.pow_of_dest =
                    (Sint16) (6 +
                              (grid.enemy_num[i][j] -
                               ENEMIES_MAX_SMALL_TYPES) * 2);
                  /* the level of energy of the sprite */
                  foe->spr.energy_level =
                    (Sint16) ((ship->type << 2) + foe->spr.pow_of_dest + 10);
                  /* size of the enemy sprite as 32x32 pixels */
                  foe->type = 1;
                }
              /* set number of images of the sprite */
              foe->spr.numof_images = 8;

              /* set current image of the enemy sprite */
              foe->spr.current_image = (Sint16) (rand () % 8);
              /* delay before next image: speed of the animation */
              foe->spr.anim_speed = 10;
              /* counter delay before next image */
              foe->spr.anim_count = 0;

              /* set addresses of the images buffer */
              for (k = 0; k < foe->spr.numof_images; k++)
                {
                  if (grid.enemy_num[i][j] < ENEMIES_MAX_SMALL_TYPES)
                    /* enemy sprite of 16x16 pixels */
                    foe->spr.img[k] =
                      (image *) & enemi[grid.enemy_num[i][j]][32 + k];
                  else
                    /* enemy sprite of 32x32 pixels */
                    foe->spr.img[k] =
                      (image *) & enemi[grid.enemy_num[i][j]][k];
                }
              /* delay value before next shot */
              foe->fire_rate_count = grid.shoot_speed[i][j];
              /* type of displacement */
              foe->displacement = DISPLACEMENT_GRID;
              foe->pos_vaiss[XCOORD] = (Sint16) i;
              foe->pos_vaiss[YCOORD] = (Sint16) j;
            }
        }
    }
  /* set grid x coordinate */
  grid.coor_x = 128;
  /* set grid y coordinate */
  grid.coor_y = -176;
  /* enable the appearance of the grid of enemies */
  grid.is_appearing = TRUE;
  /* grid phase disable */
  grid.is_enable = FALSE;
  /* set movement toward right or left */
  grid.right_movement = rand () % 2 ? TRUE : FALSE;
  if (grid.right_movement)
    {
      grid.speed_x = grid.vit_dep_x;
    }
  else
    {
      grid.speed_x = -grid.vit_dep_x;
    }
}
