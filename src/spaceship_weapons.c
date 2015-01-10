/** 
 * @file spaceship_weapons.c
 * @brief Handle the shots of the player's spaceship 
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: spaceship_weapons.c,v 1.19 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "enemies.h"
#include "options_panel.h"
#include "shots.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "spaceship_weapons.h"

static void spacheship_basic_shot (void);
static void spacheship_enhanced_shot (void);

/**
 * Handle the shots of the player's spaceship
 */
void
spaceship_weapons (void)
{
  spaceship_struct *ship = spaceship_get ();

  /* player's spaceship coming or disappearing? */
  if (spaceship_appears_count > 0 || spaceship_disappears)
    {
      return;
    }

  if ((mouse_b == 1 || keys_down[K_SPACE] || fire_button_down)
      && num_of_shots < (MAX_OF_SHOTS - 1) && ship->fire_rate <= 0)
    {
      spacheship_basic_shot ();
    }
  ship->fire_rate--;

  if (!ship->fire_rate_enhanced && ship->fire_rate > 0)
    {
      spacheship_enhanced_shot ();
    }
  ship->fire_rate_enhanced--;

  /* check if an option is selected by the player */
  options_check_selected ();
}

/*
 * basic shot (force 1) is fired
 */
static void
spacheship_basic_shot (void)
{
  spaceship_struct *ship = spaceship_get ();
  /* set time delay before spaceship can again shot */
  ship->fire_rate = 50 - (ship->type * 5 + 5);
  ship->fire_rate_enhanced = ship->fire_rate >> 1;
#ifdef USE_SDLMIXER
  sound_play (SOUND_SPACESHIP_FIRE);
#endif
  /* check the current spaceship used */
  switch (ship->type)
    {
      /* 
       * first spaceship 
       */
    case SPACESHIP_TYPE_1:
      {
        /* front shot force 1 */
        switch (ship->shot_front_basic)
          {
          case 1:
            shot_linear_spaceship_add (1, 3, V1TN1, 24, 0, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (1, 3, V1TN2, 23, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN2, 25, 0, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (1, 3, V1TN3, 24, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 23, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 25, 0, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (1, 3, V1TN3, 23, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 25, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 22, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 26, 0, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (1, 3, V1TN3, 23, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 25, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 22, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 26, 0, 8.0);
            shot_linear_spaceship_add (1, 3, V1TN3, 24, 0, 8.0);
            break;
          }
        /* rear shot force 1 */
        switch (ship->shot_rear_basic)
          {
          case 1:
            shot_linear_spaceship_add (1, 0, TIR1P1J, 8, 3, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (1, 0, TIR1P1J, 9, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 7, 3, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (1, 0, TIR1P1J, 8, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 9, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 7, 3, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (1, 0, TIR1P1J, 9, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 7, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 10, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 6, 3, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (1, 0, TIR1P1J, 8, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 9, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 7, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 10, 3, 8.0);
            shot_linear_spaceship_add (1, 0, TIR1P1J, 6, 3, 8.0);
            break;
          }
        /* right shot force 1 */
        switch (ship->shot_right_basic)
          {
          case 1:
            shot_linear_spaceship_add (1, 4, V1TDN1, 0, 6, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (1, 4, V1TDN2, 31, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 1, 6, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (1, 4, V1TDN2, 31, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 1, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN3, 0, 6, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (1, 4, V1TDN2, 31, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 1, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 30, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 2, 6, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (1, 4, V1TDN2, 31, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 1, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 30, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN2, 2, 6, 8.0);
            shot_linear_spaceship_add (1, 4, V1TDN3, 0, 6, 8.0);
            break;
          }
        /* left shot force 1 */
        switch (ship->shot_left_basic)
          {
          case 1:
            shot_linear_spaceship_add (1, 4, V1TGN1, 16, 9, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (1, 4, V1TGN2, 17, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 15, 9, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (1, 4, V1TGN2, 17, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 15, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN3, 16, 9, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (1, 4, V1TGN2, 17, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 15, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 18, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 14, 9, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (1, 4, V1TGN2, 17, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 15, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 18, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN2, 14, 9, 8.0);
            shot_linear_spaceship_add (1, 4, V1TGN3, 16, 9, 8.0);
            break;
          }
      }
      break;

      /* 
       * second spaceship 
       */
    case SPACESHIP_TYPE_2:
      {
        /* front shot force 1 */
        switch (ship->shot_front_basic)
          {
          case 1:
            shot_linear_spaceship_add (3, 4, V2THN1, 24, 0, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+2, 4, V2THN2, 24, 0, 105);
            shot_curve_spaceship_add (+2, 4, V2THN2, 24, 0, 106);
            break;
          case 3:
            shot_curve_spaceship_add (+1, 4, V2THN2, 24, 0, 105);
            shot_curve_spaceship_add (+1, 4, V2THN2, 24, 0, 106);
            shot_linear_spaceship_add (3, 4, V2THN3, 24, 0, 9.0);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 4, V2THN2, 24, 0, 105);
            shot_curve_spaceship_add (+2, 4, V2THN2, 24, 0, 106);
            shot_curve_spaceship_add (1, 4, V2THN1, 24, 1, 108);
            shot_curve_spaceship_add (1, 4, V2THN1, 24, 2, 107);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 4, V2THN2, 24, 0, 105);
            shot_curve_spaceship_add (+1, 4, V2THN2, 24, 0, 106);
            shot_curve_spaceship_add (1, 4, V2THN1, 24, 1, 108);
            shot_curve_spaceship_add (1, 4, V2THN1, 24, 2, 107);
            shot_linear_spaceship_add (3, 4, V2THN3, 24, 0, 9.0);
            break;
          }
        /* rear shot force 1 */
        switch (ship->shot_rear_basic)
          {
          case 1:
            shot_linear_spaceship_add (3, 0, T2NP1J, 8, 3, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+2, 0, T2NP1J, 8, 3, 109);
            shot_curve_spaceship_add (+2, 0, T2NP1J, 8, 3, 110);
            break;
          case 3:
            shot_linear_spaceship_add (3, 0, T2NP1J, 8, 3, 9.0);
            shot_curve_spaceship_add (+1, 0, T2NP1J, 8, 3, 109);
            shot_curve_spaceship_add (+1, 0, T2NP1J, 8, 3, 110);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 0, T2NP1J, 8, 3, 109);
            shot_curve_spaceship_add (+2, 0, T2NP1J, 8, 3, 110);
            shot_curve_spaceship_add (1, 0, T2NP1J, 8, 5, 111);
            shot_curve_spaceship_add (1, 0, T2NP1J, 8, 4, 112);
            break;
          case 5:
            shot_linear_spaceship_add (3, 0, T2NP1J, 8, 3, 9.0);
            shot_curve_spaceship_add (+1, 0, T2NP1J, 8, 3, 109);
            shot_curve_spaceship_add (+1, 0, T2NP1J, 8, 3, 110);
            shot_curve_spaceship_add (1, 0, T2NP1J, 8, 5, 111);
            shot_curve_spaceship_add (1, 0, T2NP1J, 8, 4, 112);
            break;
          }
        /* right shot force 1 */
        switch (ship->shot_right_basic)
          {
          case 1:
            shot_linear_spaceship_add (3, 4, V2TDN1, 0, 6, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+2, 4, V2TDN2, 0, 6, 113);
            shot_curve_spaceship_add (+2, 4, V2TDN2, 0, 6, 114);
            break;
          case 3:
            shot_curve_spaceship_add (+1, 4, V2TDN2, 0, 6, 113);
            shot_curve_spaceship_add (+1, 4, V2TDN2, 0, 6, 114);
            shot_linear_spaceship_add (3, 4, V2TDN3, 0, 6, 9.0);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 4, V2TDN2, 0, 6, 113);
            shot_curve_spaceship_add (+2, 4, V2TDN2, 0, 6, 114);
            shot_curve_spaceship_add (1, 4, V2TDN2, 0, 7, 115);
            shot_curve_spaceship_add (1, 4, V2TDN2, 0, 8, 116);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 4, V2TDN2, 0, 6, 113);
            shot_curve_spaceship_add (+1, 4, V2TDN2, 0, 6, 114);
            shot_curve_spaceship_add (1, 4, V2TDN2, 0, 7, 115);
            shot_curve_spaceship_add (1, 4, V2TDN2, 0, 8, 116);
            shot_linear_spaceship_add (3, 4, V2TDN3, 0, 6, 9.0);
            break;
          }
        /* left shot force 1 */
        switch (ship->shot_left_basic)
          {
          case 1:
            shot_linear_spaceship_add (3, 4, V2TGN1, 16, 9, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+2, 4, V2TGN2, 16, 9, 117);
            shot_curve_spaceship_add (+2, 4, V2TGN2, 16, 9, 118);
            break;
          case 3:
            shot_linear_spaceship_add (3, 4, V2TGN3, 16, 9, 9.0);
            shot_curve_spaceship_add (+1, 4, V2TGN2, 16, 9, 117);
            shot_curve_spaceship_add (+1, 4, V2TGN2, 16, 9, 118);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 4, V2TGN2, 16, 9, 117);
            shot_curve_spaceship_add (+2, 4, V2TGN2, 16, 9, 118);
            shot_curve_spaceship_add (1, 4, V2TGN2, 16, 10, 119);
            shot_curve_spaceship_add (1, 4, V2TGN2, 16, 11, 120);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 4, V2TGN2, 16, 9, 117);
            shot_curve_spaceship_add (+1, 4, V2TGN2, 16, 9, 118);
            shot_curve_spaceship_add (1, 4, V2TGN2, 16, 10, 119);
            shot_curve_spaceship_add (1, 4, V2TGN2, 16, 11, 120);
            shot_linear_spaceship_add (3, 4, V2TGN3, 16, 9, 9.0);
            break;
          }
      }
      break;

      /* 
       * third spaceship 
       */
    case SPACESHIP_TYPE_3:
      {
        /* front shot force 1 */
        switch (ship->shot_front_basic)
          {
          case 1:
            shot_linear_spaceship_add (+5, 0, V3THN1, 24, 0, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+3, 0, V3THN2, 24, 0, 59);
            shot_curve_spaceship_add (+3, 0, V3THN2, 24, 0, 61);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V3THN1, 24, 0, 9.0);
            shot_curve_spaceship_add (+2, 0, V3THN2, 24, 0, 59);
            shot_curve_spaceship_add (+2, 0, V3THN2, 24, 0, 61);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 0, V3THN2, 24, 0, 59);
            shot_curve_spaceship_add (+2, 0, V3THN2, 24, 0, 61);
            shot_curve_spaceship_add (2, 0, V3THN3, 24, 2, 60);
            shot_curve_spaceship_add (2, 0, V3THN3, 24, 1, 62);
            break;
          case 5:
            shot_linear_spaceship_add (+3, 0, V3THN1, 24, 0, 9.0);
            shot_curve_spaceship_add (+1, 0, V3THN2, 24, 0, 59);
            shot_curve_spaceship_add (+1, 0, V3THN2, 24, 0, 61);
            shot_curve_spaceship_add (2, 0, V3THN3, 24, 2, 60);
            shot_curve_spaceship_add (2, 0, V3THN3, 24, 1, 62);
            break;
          }
        /* rear shot force 1 */
        switch (ship->shot_rear_basic)
          {
          case 1:
            shot_linear_spaceship_add (+5, 0, V3TBB, 8, 3, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+3, 0, V3TBA, 24, 3, 85);
            shot_curve_spaceship_add (+3, 0, V3TBA, 24, 3, 86);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V3TBB, 8, 3, 9.0);
            shot_curve_spaceship_add (+2, 0, V3TBA, 24, 3, 85);
            shot_curve_spaceship_add (+2, 0, V3TBA, 24, 3, 86);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 0, V3TBA, 24, 3, 85);
            shot_curve_spaceship_add (+2, 0, V3TBA, 24, 3, 86);
            shot_curve_spaceship_add (2, 0, V3TBB, 24, 5, 87);
            shot_curve_spaceship_add (2, 0, V3TBB, 24, 4, 88);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 0, V3TBA, 24, 3, 85);
            shot_curve_spaceship_add (+1, 0, V3TBA, 24, 3, 86);
            shot_curve_spaceship_add (2, 0, V3TBB, 24, 5, 87);
            shot_curve_spaceship_add (2, 0, V3TBB, 24, 4, 88);
            shot_linear_spaceship_add (3, 0, V3TBC, 8, 3, 9.0);
            break;
          }
        /* right shot force 1 */
        switch (ship->shot_right_basic)
          {
          case 1:
            shot_linear_spaceship_add (+5, 0, V3TDB, 0, 6, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+3, 0, V3TDA, 24, 6, 77);
            shot_curve_spaceship_add (+3, 0, V3TDA, 24, 6, 78);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V3TDB, 0, 6, 9.0);
            shot_curve_spaceship_add (+2, 0, V3TDA, 24, 6, 77);
            shot_curve_spaceship_add (+2, 0, V3TDA, 24, 6, 78);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 0, V3TDA, 24, 6, 77);
            shot_curve_spaceship_add (+2, 0, V3TDA, 24, 6, 78);
            shot_curve_spaceship_add (2, 0, V3TDB, 24, 7, 79);
            shot_curve_spaceship_add (2, 0, V3TDB, 24, 8, 80);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 0, V3TDA, 24, 6, 77);
            shot_curve_spaceship_add (+1, 0, V3TDA, 24, 6, 78);
            shot_curve_spaceship_add (2, 0, V3TDB, 24, 7, 79);
            shot_curve_spaceship_add (2, 0, V3TDB, 24, 8, 80);
            shot_linear_spaceship_add (3, 0, V3TDC, 0, 6, 9.0);
            break;
          }
        /* left shot force 1 */
        switch (ship->shot_left_basic)
          {
          case 1:
            shot_linear_spaceship_add (+5, 0, V3TGB, 16, 9, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+3, 0, V3TGA, 24, 9, 81);
            shot_curve_spaceship_add (+3, 0, V3TGA, 24, 9, 82);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V3TGB, 16, 9, 9.0);
            shot_curve_spaceship_add (+2, 0, V3TGA, 24, 9, 81);
            shot_curve_spaceship_add (+2, 0, V3TGA, 24, 9, 82);
            break;
          case 4:
            shot_curve_spaceship_add (+2, 0, V3TGA, 24, 9, 81);
            shot_curve_spaceship_add (+2, 0, V3TGA, 24, 9, 82);
            shot_curve_spaceship_add (2, 0, V3TGB, 24, 10, 83);
            shot_curve_spaceship_add (2, 0, V3TGB, 24, 11, 84);
            break;
          case 5:
            shot_curve_spaceship_add (+1, 0, V3TGA, 24, 9, 81);
            shot_curve_spaceship_add (+1, 0, V3TGA, 24, 9, 82);
            shot_curve_spaceship_add (2, 0, V3TGB, 24, 10, 83);
            shot_curve_spaceship_add (2, 0, V3TGB, 24, 11, 84);
            shot_linear_spaceship_add (3, 0, V3TGC, 16, 9, 9.0);
            break;
          }
      }
      break;

      /* 
       * fourth spaceship 
       */
    case SPACESHIP_TYPE_4:
      {
        /* front shot force 1 */
        switch (ship->shot_front_basic)
          {
          case 1:
            shot_linear_spaceship_add (7, 0, V4THB, 24, 0, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (4, 0, V4THA, 24, 0, 89);
            shot_curve_spaceship_add (4, 0, V4THA, 24, 0, 90);
            break;
          case 3:
            shot_linear_spaceship_add (3, 0, V4THB, 24, 0, 9.0);
            shot_curve_spaceship_add (3, 0, V4THA, 24, 0, 89);
            shot_curve_spaceship_add (3, 0, V4THA, 24, 0, 90);
            break;
          case 4:
            shot_curve_spaceship_add (3, 0, V4THA, 24, 0, 89);
            shot_curve_spaceship_add (3, 0, V4THA, 24, 0, 90);
            shot_curve_spaceship_add (2, 0, V4THB, 24, 1, 92);
            shot_curve_spaceship_add (2, 0, V4THB, 24, 2, 91);
            break;
          case 5:
            shot_linear_spaceship_add (3, 0, V4THC, 24, 0, 9.0);
            shot_curve_spaceship_add (2, 0, V4THA, 24, 0, 89);
            shot_curve_spaceship_add (2, 0, V4THA, 24, 0, 90);
            shot_curve_spaceship_add (2, 0, V4THB, 24, 1, 92);
            shot_curve_spaceship_add (2, 0, V4THB, 24, 2, 91);
            break;
          }
        /* rear shot force 1 */
        switch (ship->shot_rear_basic)
          {
          case 1:
            shot_linear_spaceship_add (7, 0, V4TBB, 8, 3, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (4, 0, V4TBA, 24, 5, 101);
            shot_curve_spaceship_add (4, 0, V4TBA, 24, 4, 102);
            break;
          case 3:
            shot_linear_spaceship_add (3, 0, V4TBB, 8, 3, 9.0);
            shot_curve_spaceship_add (3, 0, V4TBA, 24, 5, 101);
            shot_curve_spaceship_add (3, 0, V4TBA, 24, 4, 102);
            break;
          case 4:
            shot_curve_spaceship_add (3, 0, V4TBA, 24, 5, 101);
            shot_curve_spaceship_add (3, 0, V4TBA, 24, 4, 102);
            shot_curve_spaceship_add (2, 0, V4TBB, 24, 5, 103);
            shot_curve_spaceship_add (2, 0, V4TBB, 24, 4, 104);
            break;
          case 5:
            shot_linear_spaceship_add (3, 0, V4TBC, 8, 3, 9.0);
            shot_curve_spaceship_add (2, 0, V4TBA, 24, 5, 101);
            shot_curve_spaceship_add (2, 0, V4TBA, 24, 4, 102);
            shot_curve_spaceship_add (2, 0, V4TBB, 24, 5, 103);
            shot_curve_spaceship_add (2, 0, V4TBB, 24, 4, 104);
            break;
          }
        /* right shot force 1 */
        switch (ship->shot_right_basic)
          {
          case 1:
            shot_linear_spaceship_add (+7, 0, V4TDB, 0, 6, 8.0);
            break;
          case 2:
            shot_curve_spaceship_add (+4, 0, V4TDA, 24, 6, 97);
            shot_curve_spaceship_add (+4, 0, V4TDA, 24, 6, 98);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V4TDB, 0, 6, 8.0);
            shot_curve_spaceship_add (+3, 0, V4TDA, 24, 6, 97);
            shot_curve_spaceship_add (+3, 0, V4TDA, 24, 6, 98);
            break;
          case 4:
            shot_curve_spaceship_add (+3, 0, V4TDA, 24, 6, 97);
            shot_curve_spaceship_add (+3, 0, V4TDA, 24, 6, 98);
            shot_curve_spaceship_add (2, 0, V4TDB, 24, 7, 99);
            shot_curve_spaceship_add (2, 0, V4TDB, 24, 8, 100);
            break;
          case 5:
            shot_linear_spaceship_add (3, 0, V4TDC, 0, 6, 8.0);
            shot_curve_spaceship_add (+2, 0, V4TDA, 24, 6, 97);
            shot_curve_spaceship_add (+2, 0, V4TDA, 24, 6, 98);
            shot_curve_spaceship_add (2, 0, V4TDB, 24, 7, 99);
            shot_curve_spaceship_add (2, 0, V4TDB, 24, 8, 100);
            break;
          }
        /* left shot force 1 */
        switch (ship->shot_left_basic)
          {
          case 1:
            shot_linear_spaceship_add (+7, 0, V4TGB, 16, 9, 8.0);
            break;
          case 2:
            shot_curve_spaceship_add (+4, 0, V4TGA, 24, 9, 93);
            shot_curve_spaceship_add (+4, 0, V4TGA, 24, 9, 94);
            break;
          case 3:
            shot_linear_spaceship_add (+3, 0, V4TGB, 16, 9, 8.0);
            shot_curve_spaceship_add (+3, 0, V4TGA, 24, 9, 93);
            shot_curve_spaceship_add (+3, 0, V4TGA, 24, 9, 94);
            break;
          case 4:
            shot_curve_spaceship_add (+3, 0, V4TGA, 24, 9, 93);
            shot_curve_spaceship_add (+3, 0, V4TGA, 24, 9, 94);
            shot_curve_spaceship_add (2, 0, V4TGB, 24, 10, 95);
            shot_curve_spaceship_add (1, 0, V4TGB, 24, 11, 96);
            break;
          case 5:
            shot_linear_spaceship_add (3, 0, V4TGC, 16, 9, 8.0);
            shot_curve_spaceship_add (+2, 0, V4TGA, 24, 9, 93);
            shot_curve_spaceship_add (+2, 0, V4TGA, 24, 9, 94);
            shot_curve_spaceship_add (2, 0, V4TGB, 24, 10, 95);
            shot_curve_spaceship_add (1, 0, V4TGB, 24, 11, 96);
            break;
          }
      }
      break;

      /* 
       * fifth and last spaceship 
       */
    case SPACESHIP_TYPE_5:
      {
        /* front shot force 1 */
        switch (ship->shot_front_basic)
          {
          case 1:
            shot_linear_spaceship_add (9, 4, V2TN1, 24, 0, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+5, 4, V2TN2, 24, 0, 55);
            shot_curve_spaceship_add (+5, 4, V2TN2, 24, 0, 57);
            break;
          case 3:
            shot_linear_spaceship_add (5, 4, V2TN3, 24, 0, 9.0);
            shot_curve_spaceship_add (+3, 4, V2TN2, 24, 0, 55);
            shot_curve_spaceship_add (+3, 4, V2TN2, 24, 0, 57);
            break;
          case 4:
            shot_curve_spaceship_add (+4, 4, V2TN2, 24, 0, 55);
            shot_curve_spaceship_add (+4, 4, V2TN2, 24, 0, 57);
            shot_curve_spaceship_add (2, 4, V2TN2, 24, 1, 58);
            shot_curve_spaceship_add (2, 4, V2TN2, 24, 2, 56);
            break;
          case 5:
            shot_linear_spaceship_add (5, 4, V2TN3, 24, 0, 9.0);
            shot_curve_spaceship_add (+2, 4, V2TN2, 24, 0, 55);
            shot_curve_spaceship_add (+2, 4, V2TN2, 24, 0, 57);
            shot_curve_spaceship_add (2, 4, V2TN2, 24, 1, 58);
            shot_curve_spaceship_add (2, 4, V2TN2, 24, 2, 56);
            break;
          }
        /* rear shot force 1 */
        switch (ship->shot_rear_basic)
          {
          case 1:
            shot_linear_spaceship_add (9, 0, T2NP1J, 8, 3, 9.0);
            break;
          case 2:
            shot_curve_spaceship_add (+5, 0, T2NP1J, 24, 5, 73);
            shot_curve_spaceship_add (+5, 0, T2NP1J, 24, 4, 74);
            break;
          case 3:
            shot_linear_spaceship_add (5, 0, T2NP1J, 8, 3, 9.0);
            shot_curve_spaceship_add (+3, 0, T2NP1J, 24, 5, 73);
            shot_curve_spaceship_add (+3, 0, T2NP1J, 24, 4, 74);
            break;
          case 4:
            shot_curve_spaceship_add (+4, 0, T2NP1J, 24, 5, 73);
            shot_curve_spaceship_add (+4, 0, T2NP1J, 24, 4, 74);
            shot_curve_spaceship_add (2, 0, T2NP1J, 24, 5, 75);
            shot_curve_spaceship_add (2, 0, T2NP1J, 24, 4, 76);
            break;
          case 5:
            shot_linear_spaceship_add (5, 0, T2NP1J, 8, 3, 9.0);
            shot_curve_spaceship_add (+2, 0, T2NP1J, 24, 5, 73);
            shot_curve_spaceship_add (+2, 0, T2NP1J, 24, 4, 74);
            shot_curve_spaceship_add (2, 0, T2NP1J, 24, 5, 75);
            shot_curve_spaceship_add (2, 0, T2NP1J, 24, 4, 76);
            break;
          }
        /* right shot force 1 */
        switch (ship->shot_right_basic)
          {
          case 1:
            shot_curve_spaceship_add (9, 4, V2TDN1, 24, 6, 72);
            break;
          case 2:
            shot_curve_spaceship_add (+5, 4, V2TDN2, 24, 6, 71);
            shot_curve_spaceship_add (+5, 4, V2TDN2, 24, 6, 70);
            break;
          case 3:
            shot_curve_spaceship_add (5, 4, V2TDN3, 24, 6, 72);
            shot_curve_spaceship_add (+3, 4, V2TDN2, 24, 6, 71);
            shot_curve_spaceship_add (+3, 4, V2TDN2, 24, 6, 70);
            break;
          case 4:
            shot_curve_spaceship_add (+4, 4, V2TDN2, 24, 6, 71);
            shot_curve_spaceship_add (+4, 4, V2TDN2, 24, 6, 70);
            shot_curve_spaceship_add (2, 4, V2TDN2, 24, 6, 69);
            shot_curve_spaceship_add (2, 4, V2TDN2, 24, 6, 68);
            break;
          case 5:
            shot_curve_spaceship_add (5, 4, V2TDN3, 24, 6, 72);
            shot_curve_spaceship_add (+2, 4, V2TDN2, 24, 6, 71);
            shot_curve_spaceship_add (+2, 4, V2TDN2, 24, 6, 70);
            shot_curve_spaceship_add (2, 4, V2TDN2, 24, 6, 69);
            shot_curve_spaceship_add (2, 4, V2TDN2, 24, 6, 68);
            break;
          }
        /* left shot force 1 */
        switch (ship->shot_left_basic)
          {
          case 1:
            shot_curve_spaceship_add (9, 4, V2TGN1, 24, 9, 67);
            break;
          case 2:
            shot_curve_spaceship_add (+5, 4, V2TGN2, 24, 9, 66);
            shot_curve_spaceship_add (+5, 4, V2TGN2, 24, 9, 65);
            break;
          case 3:
            shot_curve_spaceship_add (5, 4, V2TGN3, 24, 9, 67);
            shot_curve_spaceship_add (+3, 4, V2TGN2, 24, 9, 66);
            shot_curve_spaceship_add (+3, 4, V2TGN2, 24, 9, 65);
            break;
          case 4:
            shot_curve_spaceship_add (+4, 4, V2TGN2, 24, 9, 66);
            shot_curve_spaceship_add (+4, 4, V2TGN2, 24, 9, 65);
            shot_curve_spaceship_add (2, 4, V2TGN2, 24, 9, 64);
            shot_curve_spaceship_add (2, 4, V2TGN2, 24, 9, 63);
            break;
          case 5:
            shot_curve_spaceship_add (5, 4, V2TGN3, 24, 9, 67);
            shot_curve_spaceship_add (+2, 4, V2TGN2, 24, 9, 66);
            shot_curve_spaceship_add (+2, 4, V2TGN2, 24, 9, 65);
            shot_curve_spaceship_add (2, 4, V2TGN2, 24, 9, 64);
            shot_curve_spaceship_add (2, 4, V2TGN2, 24, 9, 63);
            break;
          }
      }
      break;
    }
}

/**
 * Enhance shot (force 2) is fired
 */
static void
spacheship_enhanced_shot (void)
{

  spaceship_struct *ship = spaceship_get ();
  switch (ship->type)
    {
      /* 
       * first spaceship 
       */
    case SPACESHIP_TYPE_1:
      {
        /* front shot force 2 */
        switch (ship->shot_front_enhanced)
          {
          case 1:
            shot_linear_spaceship_add (2, 0, SP1V1J, 24, 0, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (4, 0, SP1V2J, 24, 0, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (6, 0, SP1V3J, 24, 0, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (8, 0, SP1V4J, 24, 0, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (10, 0, SP1V5J, 24, 0, 8.0);
            break;
          }
        /* rear shot force 2 */
        switch (ship->shot_rear_enhanced)
          {
          case 1:
            shot_linear_spaceship_add (2, 0, SP1V1J, 8, 3, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (4, 0, SP1V2J, 8, 3, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (6, 0, SP1V3J, 8, 3, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (8, 0, SP1V4J, 8, 3, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (10, 0, SP1V5J, 8, 3, 8.0);
            break;
          }
        /* right shot force 2 */
        switch (ship->shot_right_enhanced)
          {
          case 1:
            shot_linear_spaceship_add (2, 0, SP1H1J, 0, 6, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (4, 0, SP1H2J, 0, 6, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (6, 0, SP1H3J, 0, 6, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (8, 0, SP1H4J, 0, 6, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (10, 0, SP1H5J, 0, 6, 8.0);
            break;
          }
        /* left shot force 2 */
        switch (ship->shot_left_enhanced)
          {
          case 1:
            shot_linear_spaceship_add (2, 0, SP1H1J, 16, 9, 8.0);
            break;
          case 2:
            shot_linear_spaceship_add (4, 0, SP1H2J, 16, 9, 8.0);
            break;
          case 3:
            shot_linear_spaceship_add (6, 0, SP1H3J, 16, 9, 8.0);
            break;
          case 4:
            shot_linear_spaceship_add (8, 0, SP1H4J, 16, 9, 8.0);
            break;
          case 5:
            shot_linear_spaceship_add (10, 0, SP1H5J, 16, 9, 8.0);
            break;
          }
      }
      break;

      /* 
       * second spaceship 
       */
    case SPACESHIP_TYPE_2:
      {
        switch (ship->shot_front_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (4, MISSx0, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (6, MISSx1, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (8, MISSx2, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (10, MISSx3, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (12, SP2MIJ, 24, 0, PI_PLUS_HALF_PI);
            break;
          }
        switch (ship->shot_rear_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (4, MISSx0, 8, 3, HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (6, MISSx1, 8, 3, HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (8, MISSx2, 8, 3, HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (10, MISSx3, 8, 3, HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (12, SP2MIJ, 8, 3, HALF_PI);
            break;
          }
        switch (ship->shot_right_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (4, MISSx0, 0, 6, 0.0);
            break;
          case 2:
            shot_calulated_spaceship_add (6, MISSx1, 0, 6, 0.0);
            break;
          case 3:
            shot_calulated_spaceship_add (8, MISSx2, 0, 6, 0.0);
            break;
          case 4:
            shot_calulated_spaceship_add (10, MISSx3, 0, 6, 0.0);
            break;
          case 5:
            shot_calulated_spaceship_add (12, SP2MIJ, 0, 6, 0.0);
            break;
          }
        switch (ship->shot_left_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (4, MISSx0, 16, 9, PI);
            break;
          case 2:
            shot_calulated_spaceship_add (6, MISSx1, 16, 9, PI);
            break;
          case 3:
            shot_calulated_spaceship_add (8, MISSx2, 16, 9, PI);
            break;
          case 4:
            shot_calulated_spaceship_add (10, MISSx3, 16, 9, PI);
            break;
          case 5:
            shot_calulated_spaceship_add (12, SP2MIJ, 16, 9, PI);
            break;
          }
      }
      break;

      /* 
       * third spaceship 
       */
    case SPACESHIP_TYPE_3:
      {
        switch (ship->shot_front_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (6, V3TSA, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (8, V3TSB, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (10, V3TSC, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (12, V3TSD, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (14, V3TSE, 24, 0, PI_PLUS_HALF_PI);
            break;
          }
        switch (ship->shot_rear_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (6, V3TSA, 8, 3, HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (8, V3TSB, 8, 3, HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (10, V3TSC, 8, 3, HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (12, V3TSD, 8, 3, HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (14, V3TSE, 8, 3, HALF_PI);
            break;
          }
        switch (ship->shot_right_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (6, V3TSA, 0, 6, 0.0);
            break;
          case 2:
            shot_calulated_spaceship_add (8, V3TSB, 0, 6, 0.0);
            break;
          case 3:
            shot_calulated_spaceship_add (10, V3TSC, 0, 6, 0.0);
            break;
          case 4:
            shot_calulated_spaceship_add (12, V3TSD, 0, 6, 0.0);
            break;
          case 5:
            shot_calulated_spaceship_add (14, V3TSE, 0, 6, 0.0);
            break;
          }
        switch (ship->shot_left_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (6, V3TSA, 16, 9, PI);
            break;
          case 2:
            shot_calulated_spaceship_add (8, V3TSB, 16, 9, PI);
            break;
          case 3:
            shot_calulated_spaceship_add (10, V3TSC, 16, 9, PI);
            break;
          case 4:
            shot_calulated_spaceship_add (12, V3TSD, 16, 9, PI);
            break;
          case 5:
            shot_calulated_spaceship_add (14, V3TSE, 16, 9, PI);
            break;
          }
      }
      break;

      /* 
       * fourth spaceship 
       */
    case SPACESHIP_TYPE_4:
      {
        switch (ship->shot_front_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (8, V4TSA, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (10, V4TSB, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (12, V4TSC, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (14, V4TSD, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (16, V4TSE, 24, 0, PI_PLUS_HALF_PI);
            break;
          }
        switch (ship->shot_rear_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (8, V4TSA, 8, 3, HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (10, V4TSB, 8, 3, HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (12, V4TSC, 8, 3, HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (14, V4TSD, 8, 3, HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (16, V4TSE, 8, 3, HALF_PI);
            break;
          }
        switch (ship->shot_right_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (8, V4TSA, 0, 6, 0.0);
            break;
          case 2:
            shot_calulated_spaceship_add (10, V4TSB, 0, 6, 0.0);
            break;
          case 3:
            shot_calulated_spaceship_add (12, V4TSC, 0, 6, 0.0);
            break;
          case 4:
            shot_calulated_spaceship_add (14, V4TSD, 0, 6, 0.0);
            break;
          case 5:
            shot_calulated_spaceship_add (16, V4TSE, 0, 6, 0.0);
            break;
          }
        switch (ship->shot_left_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (8, V4TSA, 16, 9, PI);
            break;
          case 2:
            shot_calulated_spaceship_add (10, V4TSB, 16, 9, PI);
            break;
          case 3:
            shot_calulated_spaceship_add (12, V4TSC, 16, 9, PI);
            break;
          case 4:
            shot_calulated_spaceship_add (14, V4TSD, 16, 9, PI);
            break;
          case 5:
            shot_calulated_spaceship_add (16, V4TSE, 16, 9, PI);
            break;
          }
      }
      break;

      /* 
       * fifth and last spaceship 
       */
    case SPACESHIP_TYPE_5:
      {
        switch (ship->shot_front_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (10, MISSx0, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (12, SP2MIJ, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (14, MISSx1, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (16, MISSx2, 24, 0, PI_PLUS_HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (18, MISSx3, 24, 0, PI_PLUS_HALF_PI);
            break;
          }
        switch (ship->shot_rear_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (10, MISSx0, 8, 3, HALF_PI);
            break;
          case 2:
            shot_calulated_spaceship_add (12, SP2MIJ, 8, 3, HALF_PI);
            break;
          case 3:
            shot_calulated_spaceship_add (14, MISSx1, 8, 3, HALF_PI);
            break;
          case 4:
            shot_calulated_spaceship_add (16, MISSx2, 8, 3, HALF_PI);
            break;
          case 5:
            shot_calulated_spaceship_add (18, MISSx3, 8, 3, HALF_PI);
            break;
          }
        switch (ship->shot_right_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (10, MISSx0, 0, 6, 0.0);
            break;
          case 2:
            shot_calulated_spaceship_add (12, SP2MIJ, 0, 6, 0.0);
            break;
          case 3:
            shot_calulated_spaceship_add (14, MISSx1, 0, 6, 0.0);
            break;
          case 4:
            shot_calulated_spaceship_add (16, MISSx2, 0, 6, 0.0);
            break;
          case 5:
            shot_calulated_spaceship_add (18, MISSx3, 0, 6, 0.0);
            break;
          }
        switch (ship->shot_left_enhanced)
          {
          case 1:
            shot_calulated_spaceship_add (10, MISSx0, 16, 9, PI);
            break;
          case 2:
            shot_calulated_spaceship_add (12, SP2MIJ, 16, 9, PI);
            break;
          case 3:
            shot_calulated_spaceship_add (14, MISSx1, 16, 9, PI);
            break;
          case 4:
            shot_calulated_spaceship_add (16, MISSx2, 16, 9, PI);
            break;
          case 5:
            shot_calulated_spaceship_add (18, MISSx3, 16, 9, PI);
            break;
          }
        break;
      }
    }
}
