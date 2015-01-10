/**
 * @file special_keys.c 
 * @brief Check specials keys to enhanced shots 
 * available only under development
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: special_keys.c,v 1.18 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "lonely_foes.h"
#include "meteors_phase.h"
#include "spaceship.h"

#ifdef UNDER_DEVELOPMENT
static bool k1, k2, k3, k4, k5, k6, k7, k8, k9, k0, f8;
static bool f1;

/**
 * Check specials keys to enhanced shots
 */
void
special_keys (void)
{
  spaceship_struct *ship = spaceship_get ();
  /*
   * shift key up
   */
  if (!keys_down[K_SHIFT])
    {
      if (keys_down[K_1] && !k1)
        {
          ship->shot_front_basic++;
          if (ship->shot_front_basic > SPACESHIP_MAX_OPTION_LEVELS)
            ship->shot_front_basic = SPACESHIP_MAX_OPTION_LEVELS;
        }
      k1 = keys_down[K_1];
      if (keys_down[K_2] && !k2)
        {
          ship->shot_left_basic++;
          if (ship->shot_left_basic > SPACESHIP_MAX_OPTION_LEVELS)
            ship->shot_left_basic = SPACESHIP_MAX_OPTION_LEVELS;
        }
      k2 = keys_down[K_2];
      if (keys_down[K_3] && !k3)
        {
          ship->shot_right_basic++;
          if (ship->shot_right_basic > SPACESHIP_MAX_OPTION_LEVELS)
            ship->shot_right_basic = SPACESHIP_MAX_OPTION_LEVELS;
        }
      k3 = keys_down[K_3];
      if (keys_down[K_4] && !k4)
        {
          ship->shot_rear_basic++;
          if (ship->shot_rear_basic > SPACESHIP_MAX_OPTION_LEVELS)
            ship->shot_rear_basic = SPACESHIP_MAX_OPTION_LEVELS;
        }
      k4 = keys_down[K_4];
      if (keys_down[K_5] && !k5)
        {
          ship->shot_front_enhanced++;
          if (ship->shot_front_enhanced > SPACESHIP_MAX_OPTION_LEVELS)
            ship->shot_front_enhanced = SPACESHIP_MAX_OPTION_LEVELS;
        }
      k5 = keys_down[K_5];
      if (keys_down[K_6] && !k6)
        {
          ship->shot_left_enhanced++;
          if (ship->shot_left_enhanced > SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_left_enhanced = SPACESHIP_MAX_OPTION_LEVELS;
            }
        }
      k6 = keys_down[K_6];
      if (keys_down[K_7] && !k7)
        {
          ship->shot_right_enhanced++;
          if (ship->shot_right_enhanced > SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_right_enhanced = SPACESHIP_MAX_OPTION_LEVELS;
            }
        }
      k7 = keys_down[K_7];
      if (keys_down[K_8] && !k8)
        {
          ship->shot_rear_enhanced++;
          if (ship->shot_rear_enhanced > SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_rear_enhanced = SPACESHIP_MAX_OPTION_LEVELS;
            }
        }
      k8 = keys_down[K_8];
      if (keys_down[K_9] && !k9)
        {
          lonely_foe_add (LONELY_STENCHIES);
        }
      k9 = keys_down[K_9];
      if (keys_down[K_0] && !k0)
        {
          lonely_foe_add (LONELY_PERTURBIANS);
        }
      k0 = keys_down[K_0];
      if (keys_down[K_F1] && !f1)
        {
          lonely_foe_add (LONELY_ABASCUSIANS);
        }
      f1 = keys_down[K_F1];
      if (keys_down[K_F8] && !f8)
        {
          if (meteor_activity)
            {
              num_of_meteors += (40 + (num_level << 1));
              enemies_kill ();
            }
          else
            {
              if (grid.is_enable)
                {
                  enemies_kill ();
                  meteor_activity = TRUE;
                  num_of_meteors = 0;
                  courbe.activity = FALSE;
                  guardian->number = 0;
                  grid.is_enable = FALSE;
                }
              else
                {
                  if (courbe.activity)
                    {
                      enemies_kill ();
                      grid_start ();
                      courbe.activity = FALSE;
                      meteor_activity = FALSE;
                      guardian->number = 0;
                      grid.is_enable = TRUE;
                    }
                }
            }
        }
      f8 = keys_down[K_F8];
    }

  /*
   * shift key down 
   */
  else
    {
      if (keys_down[K_1] && !k1)
        {
          ship->shot_front_basic--;
          if (ship->shot_front_basic < 0)
            {
              ship->shot_front_basic = 0;
            }
        }
      k1 = keys_down[K_1];
      if (keys_down[K_2] && !k2)
        {
          ship->shot_left_basic--;
          if (ship->shot_left_basic < 0)
            {
              ship->shot_left_basic = 0;
            }
        }
      k2 = keys_down[K_2];
      if (keys_down[K_3] && !k3)
        {
          ship->shot_right_basic--;
          if (ship->shot_right_basic < 0)
            {
              ship->shot_right_basic = 0;
            }
        }
      k3 = keys_down[K_3];
      if (keys_down[K_4] && !k4)
        {
          ship->shot_rear_basic--;
          if (ship->shot_rear_basic < 0)
            {
              ship->shot_rear_basic = 0;
            }
        }
      k4 = keys_down[K_4];
      if (keys_down[K_5] && !k5)
        {
          ship->shot_front_enhanced--;
          if (ship->shot_front_enhanced < 0)
            {
              ship->shot_front_enhanced = 0;
            }
        }
      k5 = keys_down[K_5];
      if (keys_down[K_6] && !k6)
        {
          ship->shot_left_enhanced--;
          if (ship->shot_left_enhanced < 0)
            {
              ship->shot_left_enhanced = 0;
            }
        }
      k6 = keys_down[K_6];
      if (keys_down[K_7] && !k7)
        {
          ship->shot_right_enhanced--;
          if (ship->shot_right_enhanced < 0)
            {
              ship->shot_right_enhanced = 0;
            }
        }
      k7 = keys_down[K_7];
      if (keys_down[K_8] && !k8)
        {
          ship->shot_rear_enhanced--;
          if (ship->shot_rear_enhanced < 0)
            {
              ship->shot_rear_enhanced = 0;
            }
        }
      k8 = keys_down[K_8];
      if (keys_down[K_9] && !k9)
        {
          lonely_foe_add (LONELY_ARCHINIANS);
        }
      k9 = keys_down[K_9];
      if (keys_down[K_0] && !k0)
        {
          /* lonely_foe_add (LONELY_ABASCUSIANS); */
          /*
             lonely_foe_add (LONELY_VIONIEES);
             lonely_foe_add (LONELY_ANGOUFF);
             lonely_foe_add (LONELY_CLOWNIES);
             lonely_foe_add (LONELY_EFFIES);
             lonely_foe_add (LONELY_SAPOUCH);
           */

          /*
             lonely_foe_add (LONELY_MILLOUZ);
             lonely_foe_add (LONELY_TOUBOUG);
             lonely_foe_add (LONELY_BELCHOUTIES);
             lonely_foe_add (LONELY_DEMONIANS);
             lonely_foe_add (LONELY_FIDGETINIANS);
             lonely_foe_add (LONELY_DIMITINIANS);
             lonely_foe_add (LONELY_ENSLAVEERS);
             lonely_foe_add (LONELY_DIVERTIZERS);
             lonely_foe_add (LONELY_HORRIBIANS);
             lonely_foe_add (LONELY_DEVILIANS);
             lonely_foe_add (LONELY_ROUGHLEERS);
             lonely_foe_add (LONELY_ABASCUSIANS);
             lonely_foe_add (LONELY_PERTURBIANS);
             lonely_foe_add (LONELY_EARTHINIANS);
             lonely_foe_add (LONELY_DISGOOSTEES);
             lonely_foe_add (LONELY_BIRIANSTEES);
             lonely_foe_add (LONELY_TODHAIRIES);
             lonely_foe_add (LONELY_DEFECTINIANS);
             lonely_foe_add (LONELY_SOONIEES);
             lonely_foe_add (LONELY_GAFFIES);
             lonely_foe_add (SHURIKY);
             lonely_foe_add (GOZUKY);
             lonely_foe_add (QUIBOULY);
           */


          /* foes moving from the bottom of the screen towards the top  */
          lonely_foe_add (LONELY_SWORDINIANS);
          lonely_foe_add (LONELY_HOCKYS);
          lonely_foe_add (LONELY_CARRYONIANS);
          lonely_foe_add (LONELY_ARCHINIANS);

          /*
             lonely_foe_add (LONELY_FEABILIANS);
             lonely_foe_add (LONELY_BITTERIANS);
             lonely_foe_add (LONELY_TOUTIES);
             lonely_foe_add (LONELY_SUBJUGANEERS);
           */


          /*
             lonely_foe_add (LONELY_PAINIANS);
             lonely_foe_add (LONELY_BAINIES);
             lonely_foe_add (NAGGYS);
           */

          /* foes follow a curve */
          /*
             lonely_foe_add (LONELY_ROTIES);
             lonely_foe_add (LONELY_STENCHIES);
             lonely_foe_add (LONELY_MADIRIANS);
             lonely_foe_add (LONELY_BLAVIRTHE);
             lonely_foe_add (LONELY_BLEUERCKS);
           */

        }
      k0 = keys_down[K_0];
      if (keys_down[K_F1] && !f1)
        {
          lonely_foe_add (LONELY_ARCHINIANS);
          lonely_foe_add (LONELY_ABASCUSIANS);
          lonely_foe_add (LONELY_STENCHIES);
          lonely_foe_add (LONELY_PERTURBIANS);
        }
      f1 = keys_down[K_F1];
    }
}
#endif
