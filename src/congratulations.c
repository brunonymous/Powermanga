/** 
 * @file congratulations.c
 * @brief Handle the congratulations 
 * @created 1999-03-26 
 * @date 2012-08-26
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: congratulations.c,v 1.23 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "congratulations.h"
#include "display.h"
#include "enemies.h"
#include "shots.h"
#include "guardians.h"
#include "menu_sections.h"
#include "movie.h"
#include "options_panel.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "starfield.h"
#include "spaceship_weapons.h"
#include "texts.h"

static Sint32 starfield_speed_angle;
static Sint32 congrat_angle_pos_x;
static Sint32 congratulations_case;
static Sint32 starfield_delay_counter;
static Sint32 congrat_enemy_count;
static Sint32 screen_center_x;
static Sint32 screen_center_y;
/** If true display congratulations */
bool is_congratulations_enabled = FALSE;
static bool is_left_movement = FALSE;
static enemy *foe_view = NULL;
static Uint32 congrat_enemy_typeof = 0;
static Uint32 current_enemy_index = 0;
static Uint32 enemie_names_list[] = {
  BOONIES, MOOARCKS, ACHEES, EVILIANS, RAGOONS,
  FLABBIES, BOWANS, HIJACKERS, DISTASIANS, BRITTERS,
  SICKIES, ARIANS, STINCKIES, BZEURCKS, WEAKIES,
  MASKIES, SCRAPIES, SCROLLIANS, SHAMISTEES, FOOLINIANS,
  GLAMOOSH,
  BIGBOONIES, BIGMOOARCKS, BIGACHEES, BIGEVILIANS, BIGRAGOONS,
  BIGFLABBIES, BIGBOWANS, BIGHIJACKERS, BIGDISTASIANS, BIGBRITTERS,
  BIGSICKIES, BIGARIANS, BIGSTINCKIES, BIGBZEURCKS, BIGWEAKIES,
  BIGMASKIES, BIGSCRAPIES, BIGSCROLLIANS, BIGSHAMISTEES, BIGFOOLINIANS,
  BIGGLAMOOSH,
  SUBJUGANEERS, MILLOUZ, SWORDINIANS, TOUBOUG, DISGOOSTEES,
  EARTHINIANS, BIRIANSTEES, BELCHOUTIES, VIONIEES, HOCKYS,
  TODHAIRIES, DEFECTINIANS, BLAVIRTHE, SOONIEES, ANGOUFF,
  GAFFIES, BITTERIANS, BLEUERCKS, ARCHINIANS, CLOWNIES,
  DEMONIANS, TOUTIES, FIDGETINIANS, EFFIES, DIMITINIANS,
  PAINIANS, ENSLAVEERS, FEABILIANS, DIVERTIZERS, SAPOUCH,
  HORRIBIANS, CARRYONIANS, DEVILIANS, ROUGHLEERS, ABASCUSIANS,
  ROTIES, STENCHIES, PERTURBIANS, MADIRIANS, BAINIES,
  SHURIKY, NAGGYS, GOZUKY, SOUKEE, QUIBOULY, TOURNADEE, SAAKAMIN
};

static const char *all_enemies_names[] =
  { "BOONIES", "MOOARCKS", "ACHEES", "EVILIANS", "RAGOONS",
  "FLABBIES", "BOWANS", "HIJACKERS", "DISTASIANS", "BRITTERS",
  "SICKIES", "ARIANS", "STINCKIES", "BZEURCKS", "WEAKIES",
  "MASKIES", "SCRAPIES", "SCROLLIANS", "SHAMISTEES", "FOOLINIANS",
  "GLAMOOSH",
  "BIGBOONIES", "BIGMOOARCKS", "BIGACHEES", "BIGEVILIANS", "BIGRAGOONS",
  "BIGFLABBIES", "BIGBOWANS", "BIGHIJACKERS", "BIGDISTASIANS", "BIGBRITTERS",
  "BIGSICKIES", "BIGARIANS", "BIGSTINCKIES", "BIGBZEURCKS", "BIGWEAKIES",
  "BIGMASKIES", "BIGSCRAPIES", "BIGSCROLLIANS", "BIGSHAMISTEES",
  "BIGFOOLINIANS",
  "BIGGLAMOOSH",
  "SUBJUGANEERS", "MILLOUZ", "SWORDINIANS", "TOUBOUG", "DISGOOSTEES",
  "EARTHINIANS", "BIRIANSTEES", "BELCHOUTIES", "VIONIEES", "HOCKYS",
  "TODHAIRIES", "DEFECTINIANS", "BLAVIRTHE", "SOONIEES", "ANGOUFF",
  "GAFFIES", "BITTERIANS", "BLEUERCKS", "ARCHINIANS", "CLOWNIES",
  "DEMONIANS", "TOUTIES", "FIDGETINIANS", "EFFIES", "DIMITINIANS",
  "PAINIANS", "ENSLAVEERS", "FEABILIANS", "DIVERTIZERS", "SAPOUCH",
  "HORRIBIANS", "CARRYONIANS", "DEVILIANS", "ROUGHLEERS", "ABASCUSIANS",
  "ROTIES", "STENCHIES", "PERTURBIANS", "MADIRIANS", "BAINIES",
  "SHURIKY", "NAGGYS", "GOZUKY", "SOUKEE", "QUIBOULY", "TOURNADEE", "SAAKAMIN"
};

void congratulations_initialize (void);
void congratulations_release (void);
static void congratulations_new_enemy (void);
static void congratulations_enemy_animation (void);
static void congratulations_play (void);

/** 
 * Initialize congratulations: player have completed all 42 levels 
 */
void
congratulations_initialize (void)
{
  /* hide sprite of the guardian */
  guardian->foe[0]->spr.ycoord =
    (float) (offscreen_starty - guardian->foe[0]->spr.img[15]->h);
  congrat_enemy_count = 0;
  starfield_delay_counter = 0;
  current_enemy_index = 0;
  starfield_speed_angle = 0;
  starfield_speed_angle = 2;
  congratulations_case = 0;
  foe_view = enemy_get ();
  if (foe_view == NULL)
    {
      return;
    }

#ifdef USE_SDLMIXER
  /* play congratulations music */
  sound_music_play (MUSIC_CONGRATULATIONS);
#endif
}

/* 
 * Handle congratulations: end of the game!
 */
void
congratulations (void)
{
  if (foe_view == NULL)
    {
      return;
    }
  switch (congratulations_case)
    {
    case 0:
      if (starfield_speed == 0.0)
        {
          /* play congratulations movie */
          movie_playing_switch = MOVIE_CONGRATULATIONS;
          congratulations_case = 1;
          starfield_speed_angle = 0;
          congrat_angle_pos_x = 0;
        }
      break;
    case 1:
      {
        if (menu_section == NO_SECTION_SELECTED)
          {
            menu_section_set (SECTION_GAME_OVER);
            gameover_enable = TRUE;
            options_close_all ();
            clear_keymap ();
            congratulations_case = 2;
          }
      }
      break;
    case 2:
      congratulations_play ();
      break;
    }
}

/**
 * Handle starflied speed and presentation of all enemies
 */
static void
congratulations_play (void)
{
  bool restart;
  Sint32 angle, ycoord;
  float sin_value;
  enemy *foe = foe_view;
  is_congratulations_enabled = TRUE;

  /* handle starfield */
  if (starfield_delay_counter <= 0)
    {
      starfield_speed_angle = (starfield_speed_angle + 1) & 127;
      if (starfield_speed_angle == 96 || starfield_speed_angle == 32)
        {
          starfield_delay_counter = 100;
        }
    }
  else
    {
      starfield_delay_counter--;
    }
  sin_value = precalc_sin128[starfield_speed_angle];
  starfield_speed = (40 * sin_value) / 10;

  congratulations_new_enemy ();
  congratulations_enemy_animation ();
  if (congrat_enemy_count > 0)
    {
      if (is_left_movement)
        {
          congrat_angle_pos_x--;
          if (congrat_angle_pos_x <= 32)
            {
              congrat_angle_pos_x = 32;
              is_left_movement = rand () % 2 ? TRUE : FALSE;
            }
        }
      else
        {
          congrat_angle_pos_x++;
          if (congrat_angle_pos_x >= 32)
            {
              congrat_angle_pos_x = 32;
              is_left_movement = rand () % 2 ? TRUE : FALSE;
            }
        }
    }
  else
    {
      if (is_left_movement)
        {
          if (--congrat_angle_pos_x <= 0)
            {
              congrat_angle_pos_x = 0;
            }
        }
      else
        {
          if (++congrat_angle_pos_x >= 64)
            {
              congrat_angle_pos_x = 64;
            }
        }
    }
  foe->spr.xcoord =
    screen_center_x + (precalc_cos128[congrat_angle_pos_x] * 256);
  foe->spr.ycoord = (float) screen_center_y;
  enemy_draw (foe);
  angle = (congrat_angle_pos_x + 64) & 127;
  if (menu_section == NO_SECTION_SELECTED)
    {
      ycoord = (Sint32) (screen_center_y + foe->spr.img[0]->h);
    }
  else
    {
      ycoord = offscreen_height - offscreen_clipsize - 16;
    }
  restart = (congrat_enemy_count > 0
             && congrat_angle_pos_x != 32) ? TRUE : FALSE;
  text_enemy_name_draw ((Sint32) (precalc_cos128[angle] * 256), ycoord,
                        restart);
}


/**  
 * Initialize the structure enemy with the next enemy of the list 
 */
static void
congratulations_new_enemy (void)
{
  Uint32 i;
  enemy *foe = foe_view;
  if (congrat_enemy_count > 0
      || (congrat_angle_pos_x != 64 && congrat_angle_pos_x != 0))
    {
      congrat_enemy_count--;
      return;
    }
  congrat_enemy_count = 300;
  /* string which display enemy name */
  text_enemy_name_init (all_enemies_names[current_enemy_index]);
  is_left_movement = rand () % 2 ? TRUE : FALSE;
  if (is_left_movement)
    {
      congrat_angle_pos_x = 64;
    }
  else
    {
      congrat_angle_pos_x = 0;
    }
  congrat_enemy_typeof = enemie_names_list[current_enemy_index++];
  if (congrat_enemy_typeof == SAAKAMIN)
    {
      current_enemy_index = 0;
    }
  foe->spr.pow_of_dest = 1000;
  foe->spr.energy_level = 1000;
  foe->spr.max_energy_level = 1000;
  if (congrat_enemy_typeof <= GLAMOOSH)
    {
      foe->spr.numof_images = 32;
    }
  else
    {
      if (congrat_enemy_typeof >= BIGBOONIES
          && congrat_enemy_typeof <= BIGGLAMOOSH)
        {
          foe->spr.numof_images = 8;
        }
      else
        {
          foe->spr.numof_images = 32;
        }
    }
  foe->spr.current_image = 0;
  foe->spr.anim_count = 0;
  foe->spr.anim_speed = 4;
  for (i = 0; i < (Uint32) foe->spr.numof_images; i++)
    {
      foe->spr.img[i] = (image *) & enemi[congrat_enemy_typeof][i];
    }
  foe->fire_rate = 60;
  foe->fire_rate_count = 60;
  foe->displacement = DISPLACEMENT_LONELY_FOE;
  foe->spr.xcoord = (float) (offscreen_width - foe->spr.img[0]->w) / 2;
  foe->spr.ycoord = (float) (offscreen_height - foe->spr.img[0]->h) / 2;
  screen_center_x = (Sint32) (offscreen_width - foe->spr.img[15]->w) / 2;
  if (menu_section == NO_SECTION_SELECTED)
    {
      screen_center_y = (Sint32) (offscreen_height - foe->spr.img[0]->h) / 2;
    }
  else
    {
      screen_center_y = offscreen_clipsize;
    }
  foe->spr.speed = 0.5;
  foe->type = congrat_enemy_typeof;
  foe->dead = FALSE;
  foe->visible = TRUE;
}

/*
 * Handle enemy's animation
 */
static void
congratulations_enemy_animation (void)
{
  enemy *foe = foe_view;
  if (foe->sens_anim)
    {
      foe->spr.anim_count++;
      if (foe->spr.anim_count >= foe->spr.anim_speed)
        {
          foe->spr.anim_count = 0;
          foe->spr.current_image--;
          if (foe->spr.current_image < 0)
            {
              foe->spr.current_image = 0;
              foe->sens_anim = 0;
            }
        }
    }
  else
    {
      foe->spr.anim_count++;
      if (foe->spr.anim_count >= foe->spr.anim_speed)
        {
          foe->spr.anim_count = 0;
          foe->spr.current_image++;
          if (foe->spr.current_image >= foe->spr.numof_images)
            {
              foe->spr.current_image = (Sint16) (foe->spr.numof_images - 1);
              foe->sens_anim = 1;
            }
        }
    }
}
