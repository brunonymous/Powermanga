/**
 * @file main.c
 * @brief Main function is where the program starts execution
 * @created 1999-08-17
 * @date 2012-08-26
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: main.c,v 1.45 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "gfx_wrapper.h"
#include "guardians.h"
#include "menu.h"
#include "meteors_phase.h"
#include "movie.h"
#include "log_recorder.h"
#include "options_panel.h"
#include "scrolltext.h"
#include "satellite_protections.h"
#include "script_page.h"
#include "shockwave.h"
#include "spaceship.h"
#include "sprites_string.h"
#include "sdl_mixer.h"
#include "starfield.h"
#include "texts.h"
#include "text_overlay.h"

/* TRUE = leave the Powermanga game */
bool quit_game = FALSE;
#ifdef POWERMANGA_SDL
/* game speed : 70 frames/sec (1000 <=> 1 seconde ; 1000 / 70 =~ 14) */
static const Uint32 GAME_FRAME_RATE = 14;
/* movie speed: 28 frames/sec */
static const Uint32 MOVIE_FRAME_RATE = 35;
#else
/* game speed : 70 frames/sec (1000000 <=> 1 seconde ; 1000000 / 70 =~ 14286) */
static const Uint32 GAME_FRAME_RATE = 14286;
/* movie speed: 28 frames/sec */
static const Uint32 MOVIE_FRAME_RATE = 35715;
#endif

static bool initialize_and_run (void);
static void main_loop (void);

/**
 * Returns to the standard GP2X menu.
 */
#ifdef POWERMANGA_GP2X
void
returnToMenu (void)
{
  /* This is how to quit back to the menu - calling exit() will just cause
   * the GP2X to "hang". execl() will replace the current process image
   *  with that of the menu program, and then execute it */
  chdir ("/usr/gp2x");
  execl ("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
}
#else
#ifdef POWERMANGA_PSP
void
returnToMenu (void)
{
  sceKernelExitGame ();
}
#endif
#endif

/**
 * The main function is where the program starts execution.
 */
Sint32
main (Sint32 args_count, char **arguments)
{
#if defined(POWERMANGA_LOG_ENABLED)
  LOG_LEVELS log_level = LOG_NOTHING;
#endif
  /* GP2X or PSP port */
#ifdef POWERMANGA_HANDHELD_CONSOLE
  /* Use atexit() to call the return-to-menu function,
   * in case of crashes, etc. */
  atexit (returnToMenu);
#endif


  /* allocate memory table */
#if defined (USE_MALLOC_WRAPPER)
  if (!memory_init (22000))
    {
      exit (0);
    }
#endif
#if defined(POWERMANGA_LOG_ENABLED)
  log_initialize (LOG_INFO);
#endif

  /* load config file */
  if (!configfile_load ())
    {
#if defined (USE_MALLOC_WRAPPER)
      memory_releases_all ();
#endif
      exit (0);
    }
#ifdef _WIN32_WCE
  power_conf->verbose = 1;
  display_windows_ce_infos ();
#endif
  if (configfile_scan_arguments (args_count, arguments))
    {
#if defined(POWERMANGA_LOG_ENABLED)
      switch (power_conf->verbose)
        {
        case 1:
          log_level = LOG_WARNING;
          break;
        case 2:
          log_level = LOG_DEBUG;
          break;
        default:
          log_level = LOG_ERROR;
          break;
        }
      log_set_level (log_level);
#endif
#if defined(POWERMANGA_HANDHELD_CONSOLE) || defined(_WIN32_WCE)
      /* We require a 320x200 output size to fit on
       * the GP2X or PSP's display */
      power_conf->scale_x = 1;
      power_conf->resolution = 320;
      power_conf->fullscreen = 1;
      pixel_size = 1;
#endif
      if (power_conf->extract_to_png)
        {
          power_conf->scale_x = 1;
          power_conf->resolution = 320;
          power_conf->fullscreen = 0;
          power_conf->nosound = TRUE;
          pixel_size = 1;
        }
      if (power_conf->resolution == 320)
        {
          vmode = 0;
        }
      else
        {
          if (power_conf->scale_x > 1)
            {
              vmode = 2;
            }
          else
            {
              vmode = 1;
            }
        }
      initialize_and_run ();
    }
  release_game ();

#if defined(POWERMANGA_LOG_ENABLED)
  log_close ();
#endif

  /* releases all memory allocated */
#if defined (USE_MALLOC_WRAPPER)
  memory_releases_all ();
#endif

  /* launch html donation page before leaving */
#if !defined(_WIN32_WCE) && defined(_WIN32)
  ShellExecute (0, "open", ".\\html\\ar01s06s02.html", 0, 0, SW_SHOWNORMAL);
#endif
  return 0;
}

/**
 * Initialize and run the game!
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
initialize_and_run (void)
{
  LOG_INF (POWERMANGA_VERSION);
  configfile_print ();
  type_routine_gfx ();
  if (!inits_game ())
    {
      return FALSE;
    }

#ifdef PNG_EXPORT_ENABLE
  if (power_conf->extract_to_png)
    {
      LOG_INF ("Extracting sprites Powermanga in PNG");
      if (!create_dir (EXPORT_DIR))
        {
          return FALSE;
        }
      enemies_extract ();
      shots_extract ();
      bonus_extract ();
      energy_gauge_extract ();
      explosions_extract ();
      guns_extract ();
      guardians_extract ();
      meteors_extract ();
      satellite_extract ();
      spaceship_extract ();
      starfield_extract ();

      menu_extract ();
      options_extract ();
      tlk_games_logo_extract ();
      sprites_font_extract ();
      scrolltext_extract ();
      return TRUE;
    }
#endif

#ifdef SHAREWARE_VERSION
  /* update counter */
  Sint32 cpt = counter_shareware_update ("PowerManga", 0, 8);
  if (cpt >= 8)
    {
      /* display first page order */
      switch (power_conf->lang)
        {
        case FR_LANG:
          show_page_order (1, "fr", cpt);
          break;
        case EN_LANG:
          show_page_order (1, "en", cpt);
          break;
        default:
          show_page_order (1, "en", cpt);
          break;
        }
    }
  counter_shareware_free ();
#endif

  fps_init ();
  main_loop ();
  fps_print ();

#ifdef SHAREWARE_VERSION
  /* displaying of the third page order */
  switch (power_conf->lang)
    {
    case FR_LANG:
      show_page_order (3, "fr", 0);
      break;
    case EN_LANG:
      show_page_order (3, "en", 0);
      break;
    default:
      show_page_order (3, "en", 0);
      break;
    }
  TTF_Quit ();
#endif

  LOG_INF ("Powermanga exited normally");
  return TRUE;
}

/**
 * Main loop of the Powermanga game
 */
void
main_loop (void)
{
  Sint32 pause_delay = 0;
  Sint32 frame_diff = 0;
  do
    {
      loops_counter++;
      if (!power_conf->nosync)
        {
          frame_diff = get_time_difference ();
          if (movie_playing_switch != MOVIE_NOT_PLAYED)
            {
              pause_delay =
                wait_next_frame (MOVIE_FRAME_RATE - frame_diff + pause_delay,
                                 MOVIE_FRAME_RATE);
            }
          else
            {
              pause_delay =
                wait_next_frame (GAME_FRAME_RATE - frame_diff + pause_delay,
                                 GAME_FRAME_RATE);
            }
        }
      /* handle Powermanga game */
      if (!update_frame ())
        {
          quit_game = TRUE;
        }
      /* handle keyboard and joystick events */
      display_handle_events ();

      /* update our main window */
      display_update_window ();

#ifdef USE_SDLMIXER
      /* play music and sounds */
      sound_handle ();
#endif
    }
  while (!quit_game);
}
