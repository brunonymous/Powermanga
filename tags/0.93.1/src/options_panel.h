/** 
 * @file options_panel.h
 * @brief handle right options panel 
 * @created 2006-11-12 
 * @date 2011-02-21 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: options_panel.h,v 1.22 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __OPTIONS_PANEL__
#define __OPTIONS_PANEL__

#ifdef __cplusplus
extern "C"
{
#endif

#define OPTIONS_MAX_OF_TYPES 13
#define OPTION_SIZE 28
#define SCORE_MULTIPLIER_XCOORD 41
#define SCORE_MULTIPLIER_TROP_YCOORD 5
#define SCORE_MULTIPLIER_BOTTOM_YCOORD 171

  typedef enum
  {
    OPTION_PANEL_INC_SPEED,
    OPTION_PANEL_REPAIR,
    OPTION_PANEL_FRONT_FIRE1,
    OPTION_PANEL_FRONT_FIRE2,
    OPTION_PANEL_LEFT_FIRE1,
    OPTION_PANEL_LEFT_FIRE2,
    OPTION_PANEL_RIGHT_FIRE1,
    OPTION_PANEL_RIGHT_FIRE2,
    OPTION_PANEL_REAR_FIRE1,
    OPTION_PANEL_REAR_FIRE2,
    OPTION_PANEL_NEW_SPACESHIP,
    OPTIONS_PANEL_NUMOF
  }
  OPTIONS_PANEL_ENUM;

  bool options_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool options_extract ();
#endif
  void options_free (void);
  void option_execution (void);
  void options_check_selected (void);
  void option_anim_init (Sint32, bool);
  void options_close_all (void);
  void options_open_all (Sint32);

  /** Right side panel */
  typedef struct option
  {
    Sint32 num_option;
    bool anim_open;
    bool anim_close;
    bool close_option;
    /** Current image number */
    Sint16 current_image;
    /** Delay between two images */
    Sint16 next_image_pause;
    /** Additional delay before next image */
    Sint16 next_image_pause_offset;
    /** Current tempo counter */
    Sint16 next_image_pause_cnt;
  }
  option;

  typedef struct option_change_coord
  {
    Sint32 coord_x;
    Sint32 coord_y;
  }
  option_change_coord;

  extern bool option_change;
  extern option option_boxes[OPTIONS_MAX_OF_TYPES];
  extern option_change_coord *options_refresh;
  extern Sint32 opt_refresh_index;
  extern bool score_x2_refresh;
  extern bool score_x4_refresh;
  extern Uint32 score_multiplier;

#ifdef __cplusplus
}
#endif

#endif
