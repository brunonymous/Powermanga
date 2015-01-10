/** 
 * @file electrical_shock.c
 * @brief Handle powerful electrical shocks
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: electrical_shock.c,v 1.20 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "energy_gauge.h"
#include "electrical_shock.h"
#include "gfx_wrapper.h"
#include "shots.h"
#include "menu.h"
#include "options_panel.h"
#include "enemies.h"
#include "explosions.h"
#include "bonus.h"
#include "spaceship.h"

Eclair eclair1;
bool electrical_shock_enable = FALSE;
/* colors of */
static unsigned char color_eclair[2];
static Sint32 electrical_delay_count = 0;

/**
 * Initialize colors of electrical shock
 */
bool
electrical_shock_once_init (void)
{
  /* color of middle of the electrical shock */
  color_eclair[0] = search_color (50, 150, 250);
  /* color border of the electrical shock */
  color_eclair[1] = search_color (50, 0, 180);
  return TRUE;
}

/** 
 * Handle the powerful electrical shocks 
 */
void
electrical_shock (void)
{
  Sint32 xcenter, ycenter;
  enemy *foe;
  sprite *spr;
  spaceship_struct *ship = spaceship_get ();

  if (!num_of_enemies || !electrical_shock_enable
      || (mouse_b == 0 && !keys_down[K_SPACE] && !fire_button_down))
    {
      return;
    }
  foe = enemy_get_first ();
  if (foe == NULL)
    {
      return;
    }
  spr = &foe->spr;
  xcenter = (Sint32) (spr->xcoord + spr->img[spr->current_image]->x_gc);
  ycenter = (Sint32) (spr->ycoord + spr->img[spr->current_image]->y_gc);

  /* enemy is alive, no pause, no "game over */
  if (!foe->dead && !gameover_enable && !player_pause
      && menu_status == MENU_OFF && xcenter > offscreen_clipsize
      && xcenter < (offscreen_clipsize + offscreen_width_visible)
      && ycenter > offscreen_clipsize
      && ycenter < (offscreen_clipsize + offscreen_height_visible))
    {
      eclair1.sx =
        (Sint32) (ship->spr.xcoord +
                  ship->spr.img[ship->spr.current_image]->x_gc);
      eclair1.sy =
        (Sint32) (ship->spr.ycoord +
                  ship->spr.img[ship->spr.current_image]->y_gc);
      eclair1.dx = xcenter;
      eclair1.dy = ycenter;
      eclair1.col1 = color_eclair[0];
      eclair1.col2 = color_eclair[1];
      if (electrical_delay_count == 0)
        {
          eclair1.r1 = (Sint32) rand () * (Sint32) rand ();
          eclair1.r2 = (Sint32) rand () * (Sint32) rand ();
          eclair1.r3 = (Sint32) rand () * (Sint32) rand ();
          /* decrease enemy's damage */
          spr->energy_level--;
          /* guardian enemy (big-boss)? */
          if (foe->displacement == DISPLACEMENT_GUARDIAN)
            {
              /* yes, update the energy gauge */
              energy_gauge_guard_is_update = TRUE;
            }
          /* display white mask of sprite */
          foe->is_white_mask_displayed = TRUE;
        }
      electrical_delay_count = (electrical_delay_count + 1) & 3;

      /* draw electrical shock */
      draw_electrical_shock (game_offscreen, &eclair1, 4);

      /* enemy is dead? */
      if (spr->energy_level <= 0)
        {


          /* enmey is a meteor or a big-boss */
          if (foe->type >= BIGMETEOR)
            {
              bonus_meteor_add (foe);
              explosions_fragments_add (spr->xcoord +
                                        spr->img[spr->current_image]->x_gc -
                                        8,
                                        spr->ycoord +
                                        spr->img[spr->current_image]->y_gc -
                                        8, 1.0, 5, 0, 2);
              explosions_add_serie (foe);
            }

          /* other enemy (curve or grid phase)  */
          else
            {
              bonus_add (foe);
              explosion_add (spr->xcoord, spr->ycoord, 0.25, foe->type, 0);
              explosions_fragments_add (spr->xcoord +
                                        spr->img[spr->current_image]->x_gc -
                                        8,
                                        spr->ycoord +
                                        spr->img[spr->current_image]->y_gc -
                                        8, 1.0, 3, 0, 1);
            }

          /* increase player's score */
          player_score += spr->pow_of_dest << 2 << score_multiplier;
          enemy_set_fadeout (foe);
        }
    }
  else
    {
      electrical_delay_count = 0;
    }
}
