/**
 * @file inits_games.c
 * @brief Initialize some structures and load some data 
 * @date 2012-08-25
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: inits_game.c,v 1.29 2012/08/25 13:58:37 gurumeditation Exp $
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
#include "curve_phase.h"
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "bonus.h"
#include "energy_gauge.h"
#include "explosions.h"
#include "shots.h"
#include "extra_gun.h"
#include "guardians.h"
#include "images.h"
#include "log_recorder.h"
#include "meteors_phase.h"
#include "menu.h"
#include "menu_sections.h"
#include "movie.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "scrolltext.h"
#include "sdl_mixer.h"
#include "shockwave.h"
#include "spaceship.h"
#include "sprites_string.h"
#include "starfield.h"
#include "text_overlay.h"
#include "texts.h"

#ifdef SHAREWARE_VERSION
#include <SDL/SDL_ttf.h>
#include "counter_shareware.h"
#endif

/* logo TLK Games */
extern bitmap logotlk[TLKLOGO_MAXOF_IMAGES];

/**
 * Initialization code that is only run once
 * @return TRUE if successful
 */
bool
inits_game (void)
{
  if (!menu_sections_once_init ())
    {
      return FALSE;
    }
#ifdef USE_SDLMIXER
  if (!sound_once_init ())
    {
      return FALSE;
    }
#endif
#ifdef SHAREWARE_VERSION
  if (TTF_Init () < 0)
    {
      LOG_ERR ("TTF_Init() return: %s", TTF_GetError ());
      return FALSE;
    }
  LOG_INF ("SHAREWARE_VERSION TTF_Init() successful!");
#endif
  /* initialize SDL or X11 display */
  if (!display_initialize ())
    {
      return FALSE;
    }
  if (!text_overlay_once_init ())
    {
      return FALSE;
    }
  if (!sprites_string_once_init ())
    {
      return FALSE;
    }
  if (!texts_init_once ())
    {
      return FALSE;
    }
  /* allocate and precalculate sinus and cosinus curves */
  if (!alloc_precalulate_sinus ())
    {
      return FALSE;
    }
  /* load TLK logo 92 949 bytes */
  if (!bitmap_load
      ("graphics/bitmap/tlk_games_logo.spr", &logotlk[0], 1,
       TLKLOGO_MAXOF_IMAGES))
    {
      return FALSE;
    }
  if (!guardians_once_init ())
    {
      return FALSE;
    }
  if (!meteors_once_init ())
    {
      return FALSE;
    }
  if (!starfield_once_init ())
    {
      return FALSE;
    }
  /* allocate and clear fontes list and elements */
  if (!scrolltext_once_init ())
    {
      return FALSE;
    }
  /* initialize bonus data structure */
  if (!bonus_once_init ())
    {
      return FALSE;
    }
  /* initialize shots data structure */
  if (!shots_once_init ())
    {
      return FALSE;
    }
  /* allocate buffers and initialize structure of the enemies vessels */
  if (!enemies_once_init ())
    {
      return FALSE;
    }
  if (!options_once_init ())
    {
      return FALSE;
    }
  /* initialize explosions */
  if (!explosions_once_init ())
    {
      return FALSE;
    }
  /* initialize extra guns */
  if (!guns_once_init ())
    {
      return FALSE;
    }
  if (!energy_gauge_once_init ())
    {
      return FALSE;
    }
  /* initialize spaceship's structure */
  if (!spaceship_once_init ())
    {
      return FALSE;
    }
  /* laod all bezier curves */
  if (!curve_once_init ())
    {
      return FALSE;
    }
  /* initialize some predefined colors */
  display_colors_init ();
  if (!electrical_shock_once_init ())
    {
      return FALSE;
    }
  if (!shockwave_once_init ())
    {
      return FALSE;
    }
  if (!satellites_once_init ())
    {
      return FALSE;
    }
  /* main menu enable */
  if (!menu_once_init ())
    {
      return FALSE;
    }
  if (!create_offscreens ())
    {
      return FALSE;
    }
  /* load right options panel  */
  if (!
      (load_pcx_into_buffer
       ("graphics/right_options_panel.pcx", options_offscreen)))
    {
      return FALSE;
    }
  /* load top scores panel */
  if (!
      (load_pcx_into_buffer
       ("graphics/top_scores_panel.pcx", scores_offscreen)))
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * Convert TLK Games logo from data bitmaps to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
tlk_games_logo_extract (void)
{
  Uint32 frame;
  const char *model = EXPORT_DIR "/menu/tlk-games-logo/tlk-games-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/tlk-games-logo"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (frame = 0; frame < TLKLOGO_MAXOF_IMAGES; frame++)
    {
      sprintf (filename,
               EXPORT_DIR "/tlk-games-logo/tlk-games-%02d.png", frame);
      if (!bitmap_to_png (&logotlk[frame], filename, 82, 58, offscreen_pitch))
        {
          free_memory (filename);
          return FALSE;
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif

/***
 * Release all resources used by the game
 */
void
release_game (void)
{
#if defined(_WIN32_WCE)
  free_wince_module_pathname ();
#endif
  bitmap_free (&logotlk[0], 1, TLKLOGO_MAXOF_IMAGES, TLKLOGO_MAXOF_IMAGES);
  /* free video ressources (xorg-x11 or SDL) */
  display_release ();
#ifdef USE_SDLMIXER
  sound_free ();
#endif
  menu_sections_free ();
  text_overlay_release ();
  menu_free ();
  satellites_free ();
  shockwave_free ();
  curve_free ();
  spaceship_free ();
  energy_gauge_free ();
  guns_free ();
  explosions_free ();
  options_free ();
  enemies_free ();
  shots_free ();
  scrolltext_free ();
  starfield_free ();
  meteors_free ();
  guardians_free ();
  bonus_free ();
  texts_free ();
  sprites_string_free ();
  movie_free ();
  free_precalulate_sinus ();
  configfile_save ();
  configfile_free ();
}
