/**
 * @file config_file.h
 * @brief Handle configuration file 
 * @created 2005-12-12 
 * @date 2015-01-10 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: config_file.h,v 1.11 2012/06/03 17:06:14 gurumeditation Exp $
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
#ifndef __CONFIGFILE__
#define __CONFIGFILE__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    EN_LANG,
    FR_LANG,
    IT_LANG,
    MAX_OF_LANGUAGES
  } LANGUAGES;

  typedef struct config_file
  {
    /** FALSE if windowed mode or TRUE if full screen */
    bool fullscreen;
    /** TRUE if force no sound */
    bool nosound;
    /** TRUE if disable timer */
    bool nosync;
    /** 1, 2, 3 or 4 */
    Sint32 scale_x;
    /** 320 or 640 */
    Sint32 resolution;
    /** Joystick configuration **/
    Sint32 joy_x_axis;
    Sint32 joy_y_axis;
    Sint32 joy_fire;
    Sint32 joy_option;
    Sint32 joy_start;
    /** verbose mode leve 1 or 2 if more messages */
    Sint32 verbose;
    /** 0 = easy, 1 = normal or 2 = hard */
    Sint32 difficulty;
    /** 0 = EN or 1 = FR */
    Sint32 lang;
    /** True if extract sprites to PNG format */
    bool extract_to_png;
  } config_file;
  extern config_file *power_conf;
  void configfile_print (void);
  bool configfile_load (void);
  void configfile_save (void);
  void configfile_free (void);
  bool configfile_scan_arguments (Sint32 arg_count, char **arg_values);
  const char *configfile_get_lang (void);

#ifdef __cplusplus
}
#endif

#endif
