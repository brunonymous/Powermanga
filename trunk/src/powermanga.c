/** 
 * @file powermanga.c
 * @brief The main loop of the game Powermanga 
 * @date 2012-08-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: powermanga.c,v 1.32 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "curve_phase.h"
#include "config_file.h"
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
#include "movie.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "scrolltext.h"
#include "shockwave.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "spaceship_weapons.h"
#include "starfield.h"
#include "texts.h"
#include "text_overlay.h"

Sint32 global_counter;
/** Pause mode is enable */
bool player_pause;
/** TRUE if "PAUSE" string is currently displayed */
bool is_pause_draw;
/** Display "GAME OVER" sprites string */
bool gameover_enable = TRUE;
#ifdef DEVELOPPEMENT
bool mouse_here = TRUE;
bool curve_editor_enable = FALSE;
#endif
/** Current image index of the TLK Games logo */
Sint32 tlk_logo_image_index;
/** TRUE if flip to the nex image */
bool tlk_logo_is_next_image;
/** X coordinate of the TLK Games logo */
Sint32 tlk_logo_xcoord;
/** Y coordinate of the TLK Games logo */
Sint32 tlk_logo_ycoord;
/** TRUE if the TLK Games logo is enabled */
bool tlk_logo_is_move;
bitmap logotlk[TLKLOGO_MAXOF_IMAGES];

/**
 * The main loop of game
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
update_frame (void)
{
  spaceship_struct *ship = spaceship_get ();
#ifdef DEVELOPPEMENT
  /* 1 = phase grid editor enable (don't work) */
  static bool grid_editor = FALSE;
#endif
  /* global frame counter */
  global_counter++;

  /* play start and congratulations animations files
   * ("movie_congratulation.gca" and "movie_introduction.gca") */
  if (movie_playing_switch != MOVIE_NOT_PLAYED)
    {
      if (!movie_player ())
        {
          LOG_ERR ("movie_player() failed!");
          return FALSE;
        }
      else
        {
          return TRUE;
        }
    }

  display_clear_offscreen ();

  /* restores the level of energy of the player spaceship */
  spaceship_energy_restore ();

  /* phase grid and curve phase editor (don't work) */
#ifdef DEVELOPPEMENT
  if (grid_editor || curve_editor_enable)
    {
      if (grid_editor)
        {
          Grid_Edit ();
        }
      else
        {
          courbe_editeur ();
        }
    }
  else
#endif

    /* pause or main menu enable */
  if (!player_pause && menu_status == MENU_OFF &&
        menu_section == NO_SECTION_SELECTED)
    {
      /* 
       * handle the phases of the game 
       */
      /* phase 2: grids (enemy wave like Space Invaders) */
      grid_handle ();
      /* phase 1: curves (little skirmish) */
      curve_phase ();
      /* phase 3: meteor storm */
      meteors_handle ();
    }

  /* draw the starfield background */
  starfield_handle ();

  /* handle bonus: green, red, yellow, blue and purple gems */
  bonus_handle ();

  /* handle protection satellites and extra gun of the player spaceship  */
  if (!gameover_enable && menu_section == NO_SECTION_SELECTED)
    {
      /* orbital protection satellites gravitate around player's spaceship */
      satellites_handle ();
      /* extra gun positioned on the sides */
      guns_handle ();
    }

  /* handle enemies */
  if (!is_congratulations_enabled)
    {
      /* handling of all the possible types of enemies */
      enemies_handle ();
    }
  else
    {
      /* congratulations, end of the game */
      congratulations ();
    }

  /* spaceship temporary invincibility  */
  spaceship_invincibility ();

  /* handle the powerful electrical shocks */
  electrical_shock ();

  /* draw the player's spaceship */
  spaceship_draw ();

  /* handle explosions */
  explosions_handle ();

  /* handle shots */
  shots_handle ();

  /* wait until all enemies are dead before jumping on next phase */
  if (num_of_enemies == 0 && !player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      /* end of a guardian phase? */
      if (!guardian_finished ())
        {
          LOG_ERR ("guardian_finished() failed!");
          return FALSE;
        }
      /* check if curve phase is finished */
      curve_finished ();
      /* check if grid phase is finished */
      grid_finished ();
      /* check if meteors phase is finished */
      if (!meteors_finished ())
        {
          LOG_ERR ("meteors_finished failed!");
          return FALSE;
        }
    }

  /* display pixel mouse pointer */
#ifdef DEVELOPPEMENT
  if (mouse_here)
    {
      put_pixel (game_offscreen, mouse_x, mouse_y, 5);
    }
#endif

  /* draw powerful circular shock wave propagated by the player spaceship */
  shockwave_draw ();

  /* animations of the options box on the right options panel */
  option_execution ();

  /* handle high score table, game over, about and order sections */
  menu_sections_run ();

  /* display "PAUSE" chars sprites */
  if (is_pause_draw)
    {
      text_pause_draw ();
    }

  /* player's spaceship come */
  if (spaceship_appears_count > 0)
    {
      spaceship_appears_count--;
      /* accelerate the speed of the starfield */
      starfield_speed += 0.028f;
      if (starfield_speed > 2.0)
        {
          starfield_speed = 2.0;
        }
      /* invincibility time */
      spaceship_set_invincibility (SPACESHIP_INVINCIBILITY_TIME / 3);
    }

  /* player's spaceship disappearing to the bottom of the screen. */
  if (spaceship_disappears && is_congratulations_enabled == 0)
    {
      /* decelerate the speed of the starfield */
      starfield_speed -= 0.02f;
      if (starfield_speed <= 0.0)
        {
          starfield_speed = 0.0;
          ship->y_speed = 0.0;
        }
      else
        {
          /* accelerate the speed of the spaceship */
          ship->y_speed += -0.15f;
        }
    }

  /* display number level */
  text_level_draw ();

  /* display scrolltext in the main menu */
  scrolltext_handle ();

  /* handle the main menu of Powermanga */
  menu_handle ();

  /* [F1] spaceship_appears / [F2] spaceship disappears */
#ifdef DEVELOPPEMENT
  if (keys_down[K_F1])
    {
      spaceship_disappears = 1;
    }
  if (keys_down[K_F2] && starfield_speed == 0.0)
    {
      spaceship_show ();
    }
#endif

  /* handle "TLK Games" sprite logo */
  if (tlk_logo_is_move)
    {
      draw_bitmap (&logotlk[tlk_logo_image_index], tlk_logo_xcoord,
                   tlk_logo_ycoord);
      if (tlk_logo_is_next_image)
        {
          tlk_logo_image_index++;
          if (tlk_logo_image_index >= TLKLOGO_MAXOF_IMAGES)
            {
              tlk_logo_image_index = 0;
            }
          tlk_logo_is_next_image = FALSE;
          tlk_logo_ycoord--;
        }
      else
        {
          tlk_logo_is_next_image = TRUE;
        }
      if (tlk_logo_ycoord <= 32)
        {
          tlk_logo_is_move = FALSE;
        }
    }

  /* display text overlay (about, cheats menu and variables) */
  text_overlay_draw ();

  /* handle the loss and the regression of the spaceship or cause game over */
  spaceship_downgrading ();

  /* handle spaceship's energy level */
  energy_gauge_spaceship_update ();

  /* handle guardian's energy level */
  energy_gauge_guardian_update ();

  /* draw player's score into the top panel */
  text_draw_score ();

#ifdef DEVELOPPEMENT
  if (keys_down[K_E] && keys_down[K_G])
    {
      /* [e]+[g] = enable the grid editor */
      grid_editor = 1;
    }
  if (keys_down[K_E] && keys_down[K_C])
    {
      /* [e]+[c] enable the curve editor */
      curve_editor_enable = 1;
    }
  /* [p]+[g] back to the game */
  if (keys_down[K_P] && keys_down[K_G])
    {
      grid_editor = 0;
      curve_editor_enable = 0;
    }
#endif

  /* control the spaceship movements */
  if (!player_pause && !gameover_enable && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      spaceship_control_movements ();
    }

  /* [P] or [Pause] enable/disable pause */
  if (!keys_down[K_CTRL] && (keys_down[K_P] || keys_down[K_PAUSE]))
    {
      toggle_pause ();
    }
  keys_down[K_P] = FALSE;
  keys_down[K_PAUSE] = FALSE;   /* clear flag pause key */


  /* switch between full screen and windowed mode */
#ifdef POWERMANGA_SDL
  if ((keys_down[K_F] && !is_playername_input ()
       && menu_section != SECTION_ORDER) || keys_down[K_F11])
    {
      if (power_conf->fullscreen)
        {
          power_conf->fullscreen = FALSE;
        }
      else
        {
          power_conf->fullscreen = TRUE;
        }
      init_video_mode ();
    }
  keys_down[K_F] = FALSE;
  keys_down[K_F11] = FALSE;
#endif

  /* control the speed of the spaceship */
  spaceship_speed_control ();

  /* cheat code keys */
#ifdef UNDER_DEVELOPMENT
  special_keys ();
#endif

  /* handle weapon's player spaceship */
#ifdef DEVELOPPEMENT
  if (!grid_editor && !curve_editor_enable && !spaceship_is_dead
      && !player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
#else
  if (!spaceship_is_dead && !player_pause && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
#endif
    {
      spaceship_weapons ();
    }
  return TRUE;
}

/**
 * Enable/disable pause
 **/
bool
toggle_pause (void)
{
  if (!spaceship_is_dead && spaceship_appears_count <= 0
      && !spaceship_disappears && menu_status == MENU_OFF
      && menu_section == NO_SECTION_SELECTED)
    {
      if (player_pause)
        {
          player_pause = FALSE;
        }
      else
        {
          player_pause = TRUE;
        }
      if (is_pause_draw)
        {
          is_pause_draw = FALSE;
        }
      else
        {
          is_pause_draw = TRUE;
        }
    }
  return player_pause;
}
