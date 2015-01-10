/**
 * @file config_file.c 
 * @brief Handle configuratio file 
 * @created 2005-12-12 
 * @date 2015-01-10
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: config_file.c,v 1.30 2012/08/26 19:16:07 gurumeditation Exp $
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
#include "config_file.h"
#include "lispreader.h"
#include "log_recorder.h"

#define CONFIG_DIR_NAME "tlk-games"
#define CONFIG_FILE_NAME "powermanga.conf"

config_file *power_conf = NULL;
static const char *lang_to_text[MAX_OF_LANGUAGES] = { "en", "fr", "it" };

static char *config_dir = NULL;
static char config_filename[] = CONFIG_DIR_NAME;
static char config_file_name[] = CONFIG_FILE_NAME;
static char *configname = NULL;
static bool configfile_check_dir ();
static void configfile_reset_values ();

/** 
 * Reset all configuration values
 */
void
configfile_reset_values ()
{
  power_conf->fullscreen = TRUE;
  power_conf->nosound = FALSE;
  power_conf->resolution = 640;
  power_conf->verbose = 0;
  power_conf->difficulty = 1;
  power_conf->scale_x = 2;
#if defined(_WIN32_WCE)
/* FIXME use GetLocaleInfo() function to retrieve langage */
  power_conf->lang = EN_LANG;
#else
  if (getenv ("LANG") != NULL)
    {
      if (strncmp (getenv ("LANG"), "fr", 2) == 0)
        {
          power_conf->lang = FR_LANG;
        }
      else if (strncmp (getenv ("LANG"), "it", 2) == 0)
        {
          power_conf->lang = IT_LANG;
        }
      else
        {
          power_conf->lang = EN_LANG;
        }
    }
  else
    {
      power_conf->lang = EN_LANG;
    }
#endif
  power_conf->extract_to_png = FALSE;
  power_conf->joy_x_axis = 0;
  power_conf->joy_y_axis = 1;
  power_conf->joy_fire = 0;
  power_conf->joy_option = 1;
  power_conf->joy_start = 2;
}

/** 
 * Display values of the configuration file
 */
void
configfile_print (void)
{
  LOG_INF ("fullscreen: %i; nosound: %i; resolution: %i; "
           "verbose: %i; difficulty: %i; lang: %s; scale_x: %i"
           "; joy_config %i %i %i %i %i; nosync: %i",
           power_conf->fullscreen, power_conf->nosound,
           power_conf->resolution, power_conf->verbose,
           power_conf->difficulty, lang_to_text[power_conf->lang],
           power_conf->scale_x, power_conf->joy_x_axis,
           power_conf->joy_y_axis, power_conf->joy_fire,
           power_conf->joy_option, power_conf->joy_start, power_conf->nosync);
}

/** 
 * Check if config directory exists; if not create it and set config_dir
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
configfile_check_dir (void)
{
  Uint32 length;
  DIR *dir = NULL;
  if (config_dir == NULL)
    {
      /* Determines the size of the string of the directory
       * and allocates memory */
      length = strlen (config_filename) + 3;
#ifdef _WIN32
      length++;
#else
      if (getenv ("XDG_CONFIG_HOME") != NULL)
        {
          length += strlen (getenv ("XDG_CONFIG_HOME")) + 1;
        }
      else
        {
          if (getenv ("HOME") != NULL)
            {
              length += strlen (getenv ("HOME"));
            }
          else
            {
              length++;
            }
          length += strlen ("/.config") + 1;
        }
#endif
      config_dir = memory_allocation (length);
      if (config_dir == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes!", length);
          return FALSE;
        }
    }
#ifdef _WIN32
  _snprintf (config_dir, strlen (config_dir) - 1, "./%s", config_filename);
  /* create directory if not exist */
  create_dir (config_dir);
#else
  if (getenv ("XDG_CONFIG_HOME") != NULL)
    {
      snprintf (config_dir, strlen (config_dir) - 1, "%s/%s",
                getenv ("XDG_CONFIG_HOME"), config_filename);
    }
  else
    {
      snprintf (config_dir, strlen (config_dir) - 1, "%s/.config/%s",
                (getenv ("HOME") ? getenv ("HOME") : "."), config_filename);
    }
  /* test and create .tlkgames */
  dir = opendir (config_dir);
  if (dir == NULL)
    {
      LOG_WARN ("couldn't find/open config directory '%s'", config_dir);
      LOG_WARN ("attempting to create it...");
      create_dir (config_dir);
      if (!opendir (config_dir))
        {
          LOG_ERR ("opendir(%s) fail", config_dir);
          return FALSE;
        }
      else
        {
          LOG_INF ("opendir(%s) successful", config_dir);
        }
    }
  else
    {
      closedir (dir);
      dir = NULL;
    }
#endif
  return TRUE;
}

/**
 * Load configuration file from "~/.tlkgames/powermanga.conf"
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
configfile_load (void)
{
#if !defined(_WIN32_WCE)
  Uint32 length;
#endif
  lisp_object_t *root_obj, *lst, *sub;
  char *str;
  /* allocate config structure */
  if (power_conf == NULL)
    {
      power_conf = (config_file *) memory_allocation (sizeof (config_file));
      if (power_conf == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'power_conf'!");
          return FALSE;
        }
    }
  configfile_reset_values ();
  if (!configfile_check_dir ())
    {
      return TRUE;
    }

  if (configname == NULL)
    {
#if defined(_WIN32_WCE)
      configname = locate_data_file (config_file_name);
      if (configname == NULL)
        {
          LOG_ERR ("can't locate file: %s", config_file_name);
          return FALSE;
        }
#else
      length = strlen (config_dir) + strlen (config_file_name) + 2;
      configname = memory_allocation (length);
      if (configname == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes!", length);
          return FALSE;
        }
#endif
    }
  sprintf (configname, "%s/%s", config_dir, config_file_name);
  LOG_INF ("configuration filename: %s", configname);
  root_obj = lisp_read_file (configname);
  if (root_obj == NULL)
    {
      LOG_ERR ("lisp_read_file(%s) failed!", configname);
      return TRUE;
    }
  if (root_obj->type == LISP_TYPE_EOF
      || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      LOG_ERR ("lisp_read_file(%s) failed!", configname);
      lisp_free (root_obj);
      return TRUE;
    }
  if (strcmp (lisp_symbol (lisp_car (root_obj)), "powermanga-config") != 0)
    {
      LOG_ERR ("lisp_read_file(%s) failed!", configname);
      lisp_free (root_obj);
      return TRUE;
    }
  lst = lisp_cdr (root_obj);
  if (!lisp_read_string (lst, "lang", &str))
    {
      power_conf->lang = EN_LANG;
    }
  else
    {
      if (strcmp (str, "fr") == 0)
        {
          power_conf->lang = FR_LANG;
        }
      else if (strcmp (str, "it") == 0)
        {
          power_conf->lang = IT_LANG;
        }
      else
        {
          power_conf->lang = EN_LANG;
        }
    }
  if (!lisp_read_bool (lst, "fullscreen", &power_conf->fullscreen))
    {
      power_conf->fullscreen = TRUE;
    }
  if (!lisp_read_bool (lst, "nosound", &power_conf->nosound))
    {
      power_conf->nosound = FALSE;
    }
  if (!lisp_read_bool (lst, "nosync", &power_conf->nosync))
    {
      power_conf->nosync = FALSE;
    }
  if (!lisp_read_int (lst, "verbose", &power_conf->verbose))
    {
      power_conf->verbose = 0;
    }
  if (!lisp_read_int (lst, "scale_x", &power_conf->scale_x))
    {
      power_conf->scale_x = 2;
    }
  if (power_conf->scale_x < 1 || power_conf->scale_x > 4)
    {
      power_conf->scale_x = 2;
    }
  if (!lisp_read_int (lst, "resolution", &power_conf->resolution))
    {
      power_conf->resolution = 640;
    }
  if (power_conf->resolution != 320 && power_conf->resolution != 640)
    {
      power_conf->resolution = 640;
    }
  if (power_conf->scale_x > 1)
    {
      power_conf->resolution = 640;
    }
  sub = search_for (lst, "joy_config");
  if (sub)
    sub = lisp_car_int (sub, &power_conf->joy_x_axis);
  if (sub)
    sub = lisp_car_int (sub, &power_conf->joy_y_axis);
  if (sub)
    sub = lisp_car_int (sub, &power_conf->joy_fire);
  if (sub)
    sub = lisp_car_int (sub, &power_conf->joy_option);
  if (sub)
    sub = lisp_car_int (sub, &power_conf->joy_start);
  lisp_free (root_obj);

configfile_print();

  return TRUE;
}

/** 
 * Save config file "~/.tlkgames/powermanga.conf"
 */
void
configfile_save (void)
{
  FILE *config;
  if (power_conf->extract_to_png)
    {
      return;
    }
  if (configname == NULL)
    {
      LOG_ERR ("config filename is not defined");
      return;
    }
  config = fopen_data (configname, "w");
  if (config == NULL)
    {
      return;
    }
  fprintf (config, "(powermanga-config\n");
  fprintf (config, "\t;; the following options can be set to #t or #f:\n");
  fprintf (config, "\t(fullscreen %s)\n",
           power_conf->fullscreen ? "#t" : "#f");
  fprintf (config, "\t(nosound %s)\n", power_conf->nosound ? "#t" : "#f");
  fprintf (config, "\t(nosync %s)\n", power_conf->nosync ? "#t" : "#f");

  fprintf (config, "\n\t;; window size (320 or 640):\n");
  fprintf (config, "\t(resolution  %d)\n", power_conf->resolution);

  fprintf (config, "\n\t;; scale_x (1, 2, 3 or 4):\n");
  fprintf (config, "\t(scale_x   %d)\n", power_conf->scale_x);

  fprintf (config,
           "\n\t;; joy_config x_axis y_axis fire_button option_button start_button):\n");
  fprintf (config, "\t(joy_config %d %d %d %d %d)\n", power_conf->joy_x_axis,
           power_conf->joy_y_axis, power_conf->joy_fire,
           power_conf->joy_option, power_conf->joy_start);

  fprintf (config,
           "\n\t;; verbose mode 0 (disabled), 1 (enable) or 2 (more messages)\n");
  fprintf (config, "\t(verbose   %d)\n", power_conf->verbose);

  fprintf (config, "\n\t;; difficulty 0 (easy), 1 (normal) or 2 (hard)\n");
  fprintf (config, "\t(difficulty   %d)\n", power_conf->difficulty);

  fprintf (config, "\n\t;; langage en or fr\n");
  fprintf (config, "\t(lang      ");
  switch (power_conf->lang)
    {
    case FR_LANG:
      fprintf (config, "\"fr\")\n");
      break;
    default:
      fprintf (config, "\"en\")\n");
      break;
    }
  fprintf (config, ")\n");

  fclose (config);
}

/**
 * Release the configuration data structure and filenames strings
 */
void
configfile_free (void)
{
  if (power_conf != NULL)
    {
      free_memory ((char *) power_conf);
      power_conf = NULL;
    }
  if (config_dir != NULL)
    {
      free_memory (config_dir);
      config_dir = NULL;
    }
  if (configname != NULL)
    {
      free_memory (configname);
      configname = NULL;
    }
}

/** 
 * Scan command line arguments
 * @param arg_count the number of arguments
 * @param arg_values he command line arguments
 * @return FALSE if exit, TRUE otherwise 
 */
bool
configfile_scan_arguments (Sint32 arg_count, char **arg_values)
{
  Sint32 i;
  for (i = 1; i < arg_count; i++)
    {
      if (*arg_values[i] != '-')
        {
          continue;
        }

      /* display help */
      if (!strcmp (arg_values[i], "-h") || !strcmp (arg_values[i], "--help"))
        {
          fprintf (stdout, "\noptions:\n"
                   "-h, --help     print Help (this message) and exit\n"
                   "--version      print version information and exit\n"
                   "-x             extract sprites in PNG format and exit\n"
                   "--320          game run in a 320*200 window (slow machine)\n"
                   "--2x           scale2x\n"
                   "--3x           scale3x\n" "--4x           scale4x\n"
                   "--joyconfig x,y,f,o,s\n"
                   "               use the indicated joystick axes and buttons for the\n"
                   "               x-axis, y-axis, fire button, option button, and start button,\n"
                   "               respectively.  The following argument must be 5 integers\n"
                   "               seperated by commas.  The default is 0,1,0,1,2\n");
#ifdef POWERMANGA_SDL
          fprintf (stdout, "--window       windowed mode\n");
          fprintf (stdout, "--fullscreen   fullscreen mode\n");
#endif
          fprintf (stdout,
#if defined(POWERMANGA_LOG_ENABLED)
                   "-q             \n"
                   "-v             verbose mode\n"
                   "--verbose      verbose mode (more messages)\n"
#endif
                   "--nosound      disable sound and musics\n"
                   "--sound        enable sound and musics\n"
                   "--nosync       disable timer\n"
                   "--easy         easy bonuses\n"
                   "--hard         hard bonuses\n"
                   "--------------------------------------------------------------\n"
                   "keys recognized during the game:\n"
                   "[Ctrl] + [S]   enable/disable the music\n"
                   "[Ctrl] + [Q]   finish the play current\n"
                   "[Ctrl] + [A]   about Powermanga\n"
                   "[F10]          quit Powermanga\n"
                   "[P]            enable/disable pause\n"
                   "[Page Down]    volume down\n"
                   "[Page Up]      volume up\n");
#ifdef POWERMANGA_SDL
          fprintf (stdout,
                   "F              switch between full screen and windowed mode\n");
#endif
          return FALSE;

        }

      /* print version information and exit */
      if (!strcmp (arg_values[i], "--version"))
        {
          printf (POWERMANGA_VERSION);
          printf ("\n");
          printf ("copyright (c) 1998-2015 TLK Games\n");
          printf ("website: http://linux.tlk.fr/\n");
          return FALSE;
        }
      /* force window mode */
      if (!strcmp (arg_values[i], "--window"))
        {
          power_conf->fullscreen = FALSE;
          continue;
        }

      /* force fullscreen mode */
      if (!strcmp (arg_values[i], "--fullscreen"))
        {
          power_conf->fullscreen = TRUE;
          continue;
        }

      /* resolution, low-res or high-res */
      if (!strcmp (arg_values[i], "--320"))
        {
          power_conf->resolution = 320;
          power_conf->scale_x = 1;
          continue;
        }
      if (!strcmp (arg_values[i], "--640"))
        {
          power_conf->resolution = 640;
          power_conf->scale_x = 1;
          continue;
        }
      if (!strcmp (arg_values[i], "--2x"))
        {
          power_conf->scale_x = 2;
          power_conf->resolution = 640;
          continue;
        }
      if (!strcmp (arg_values[i], "--3x"))
        {
          power_conf->scale_x = 3;
          power_conf->resolution = 640;
          continue;
        }
      if (!strcmp (arg_values[i], "--4x"))
        {
          power_conf->scale_x = 4;
          power_conf->resolution = 640;
          continue;
        }


      /* Joystick configuration */
      if (!strcmp (arg_values[i], "--joyconfig"))
        {
          if (sscanf (arg_values[++i], "%d,%d,%d,%d,%d",
                      &power_conf->joy_x_axis,
                      &power_conf->joy_y_axis,
                      &power_conf->joy_fire,
                      &power_conf->joy_option, &power_conf->joy_start) != 5)
            {
              LOG_ERR
                ("Invalid joystick configuration, expecting 5 integers seperated by commas, found %s",
                 arg_values[i]);
              return FALSE;
            }
          continue;
        }

      /* enable verbose mode */
      if (!strcmp (arg_values[i], "-q"))
        {
          power_conf->verbose = 0;
          continue;
        }
      if (!strcmp (arg_values[i], "-v"))
        {
          power_conf->verbose = 1;
          continue;
        }
      if (!strcmp (arg_values[i], "--verbose"))
        {
          power_conf->verbose = 2;
          continue;
        }

      /* enable extract sprites to png */
      if (!strcmp (arg_values[i], "-x"))
        {
          power_conf->extract_to_png = TRUE;
          continue;
        }

      /* disable sound */
      if (!strcmp (arg_values[i], "--nosound"))
        {
          power_conf->nosound = TRUE;
          continue;
        }

      /* disable sound */
      if (!strcmp (arg_values[i], "--sound"))
        {
          power_conf->nosound = FALSE;
          continue;
        }

      /* disable timer */
      if (!strcmp (arg_values[i], "--nosync"))
        {
          power_conf->nosync = TRUE;
          continue;
        }

      /* difficulty: easy or hard (normal bu default) */
      if (!strcmp (arg_values[i], "--easy"))
        {
          power_conf->difficulty = 0;
          continue;
        }
      if (!strcmp (arg_values[i], "--hard"))
        {
          power_conf->difficulty = 2;
          continue;
        }
    }
  return TRUE;
}

/**
 * Return current language
 * @return current language 'en' or 'fr'
 */
const char *
configfile_get_lang (void)
{
  return lang_to_text[power_conf->lang];
}
