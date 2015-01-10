/** 
 * @file energy_gauge.c
 * @brief Handle displaying of the energy gauge the top score panel
 *        for the player's spaceship and the current guardian
 * @created 2007-01-06
 * @date 2012-08-26
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: energy_gauge.c,v 1.20 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "energy_gauge.h"
#include "guardians.h"
#include "log_recorder.h"
#include "shots.h"
#include "gfx_wrapper.h"
#include "images.h"
#include "spaceship.h"

bool energy_gauge_spaceship_is_update = TRUE;
bool energy_gauge_guard_is_update = TRUE;
static image gauge_red;
static image gauge_green;
static image gauge_blue;
static const Sint32 GAUGE_SPACESHIP_WIDTH = 100;

static void draw_energy_gauge (Uint32 sizeof_bar, Sint32 energy,
                               Uint32 coordx, Uint32 energy_max);

/**
 * Load sprites images of the gauge
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
energy_gauge_once_init (void)
{

  if (!image_load_single
      ("graphics/sprites/energy_gauge_red.spr", &gauge_red))
    {
      return FALSE;
    }
  if (!image_load_single
      ("graphics/sprites/energy_gauge_green.spr", &gauge_green))
    {
      return FALSE;
    }
  if (!image_load_single
      ("graphics/sprites/energy_gauge_blue.spr", &gauge_blue))
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * Convert from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
energy_gauge_extract (void)
{
  const char *model = EXPORT_DIR "/gauges/energy_gauge-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/gauges"))
    {
      free_memory (filename);
      return FALSE;
    }
  free_memory (filename);
  return TRUE;
}
#endif

/**
 * Release memory used for the images sprites 
 */
void
energy_gauge_free (void)
{
  LOG_DBG ("deallocates the memory used by the sprites images");
  images_free (&gauge_blue, 1, 1, 1);
  images_free (&gauge_green, 1, 1, 1);
  images_free (&gauge_red, 1, 1, 1);
}

/**
 * Draw the entire gauge in blue
 */
void
energy_gauge_init (void)
{
  Sint32 i;
  for (i = 0; i < 100; i++)
    {
      draw_image_in_score (&gauge_blue, 210 + i, 3);
    }
}

/**
 * Display player's ernergy level bar
 */
void
energy_gauge_spaceship_update (void)
{
  spaceship_struct *ship = spaceship_get ();
  if (!energy_gauge_spaceship_is_update)
    {
      return;
    }
  draw_energy_gauge (GAUGE_SPACESHIP_WIDTH * pixel_size,
                     ship->spr.energy_level, 210 * pixel_size,
                     (ship->type * 20 + 20) * pixel_size);
}

/**
 * Draw guardian's energy gauge
 */
void
energy_gauge_guardian_update (void)
{
  Uint32 energy_level;
  enemy *guard;
  if (!energy_gauge_guard_is_update)
    {
      return;
    }
  guard = guardian->foe[0];
  if (guard != NULL && guard->displacement == DISPLACEMENT_GUARDIAN)
    {
      energy_level =
        (guard->spr.energy_level * 45) / guard->spr.max_energy_level;
      draw_energy_gauge (45, energy_level, 10, 45);
    }
  else
    {
      draw_energy_gauge (45, 0, 10, 45);
    }
}

/**
 * Draw a energy gauge into the top score panel
 * @param sizeof_bar maximum width of energy barline
 * @param energy current energy level
 * @param coordx x coordinate of the ernergy level bar
 * @param energy_max maximum energy level
 */
static void
draw_energy_gauge (Uint32 sizeof_bar, Sint32 energy, Uint32 coordx,
                   Uint32 energy_max)
{
  Sint32 width;
  Sint32 coordy = 3 * pixel_size;
  if (energy > 0)
    {
      draw_image_in_score_repeat (&gauge_green, coordx, coordy, energy);
      draw_image_in_score_repeat (&gauge_red, coordx + energy, coordy,
                                  energy_max - energy);
      coordx = coordx + energy_max;
      width = sizeof_bar - energy_max;
    }
  else
    {
      width = sizeof_bar;
    }
  draw_image_in_score_repeat (&gauge_blue, coordx, coordy, width);
}
