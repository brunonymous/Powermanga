/** 
 * @file sprites_string.h 
 * @brief Handle the strings fonts of sprites 
 * @created 2006-12-19 
 * @date 2012-08-25 
 * @author Bruno Ethvignot 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: sprites_string.h,v 1.38 2012/08/25 15:55:00 gurumeditation Exp $
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
#ifndef __SPRITES_STRING__
#define __SPRITES_STRING__

#ifdef __cplusplus
extern "C"
{
#endif

#define FONTS_MAX_OF_IMAGES 16
#define MAX_ANGLE 127

  typedef enum
  {
    IJOY_LEFT = 1,
    IJOY_RIGHT,
    IJOY_TOP,
    IJOY_DOWN,
    IJOY_FIRE,
    IJOY_OPT
  }
  JOYCODE_ENUM;

  typedef enum
  {
    FONT_GAME,
    FONT_BIG,
    FONT_SCROLL,
    FONT_SCORE
  }
  FONTS_TYPEOF;

  typedef enum
  {
    FONT_DRAW_OFFSCREEN,
    FONT_DRAW_TOP_PANEL
  }
  FONTS_WITCH_OFFSCREEN;

  typedef struct sprite_char_struct
  {
    /** If TRUE char is displayed */
    bool is_display;
    Uint32 coord_x;
    Uint32 coord_y;
    Uint32 center_x;
    Uint32 center_y;
    /** If TRUE char is animated */
    bool is_anim;
    /** Number of sprite images of the animation */
    Uint32 num_of_images;
    Uint32 current_char;
    Uint32 current_image;
    /** Delay before next image: speed of the animation */
    Uint32 anim_speed;
    Uint32 anim_speed_inc;
    Uint32 anim_speed_count;
    /** Type of font used: FONT_GAME, FONT_BIG or FONT_SCROLL */
    Uint32 type_of_font;
    /* Width and height of a font in pixels */
    Uint32 font_size;
    /** Indicate in which screen the chars must be drawed */
    Uint32 which_offscreen;
    Uint32 xmax;
    Uint32 ymax;
    Uint32 xmin;
    Uint32 ymin;
    Sint32 radius_x;
    Sint32 radius_y;
    Sint32 angle;
    Sint32 angle_inc;
  }
  sprite_char_struct;

/**
 * Data structure for string of chars sprites
 */
  typedef struct sprite_string_struct
  {
    /** Current number of the chars */
    Uint32 num_of_chars;
    /** Maximum number of the chars */
    Uint32 max_of_chars;
    /** Indicate in which screen the chars must be drawed */
    Uint32 which_offscreen;
    /** X and y coordinates of the string */
    float coord_x;
    float coord_y;
    float offset_x;
    float offset_y;
    bool is_move;
    /** At least a character was animated */
    bool at_least_one_char_changed;
    /** No character was animated */
    bool none_char_animated;
    /** Space in pixel between 2 chars */
    Uint32 space;
    /** Delay before process next char */
    Sint32 delay_next_char;
    Sint32 delay_next_char_count;
    Sint32 delay_add_to_last;
    /** Index of the next char to be animated */
    Uint32 current_char;
    Uint32 center_x;
    Uint32 center_y;
    Sint32 angle;
    Sint32 angle_increment;
    Sint32 radius_x;
    Sint32 radius_y;
    /** ASCII string */
    char *string;
    /** True if the string was allocated */
    bool is_string_allocated;
    sprite_char_struct *sprites_chars;
    Sint32 cursor_pos;
    Uint32 cursor_status;
  } sprite_string_struct;

  bool sprites_string_once_init (void);
  void sprites_string_free (void);
#ifdef PNG_EXPORT_ENABLE
  bool sprites_font_extract (void);
#endif
  sprite_string_struct *sprites_string_new (const char *const string,
                                            Uint32 more_chars, Uint32 type,
                                            float coordx, float coordy);
  sprite_string_struct *sprites_string_create (char *string, Uint32 size,
                                               Uint32 type, float coordx,
                                               float coordy);
  void sprites_string_delete (sprite_string_struct * sprite_str);
  void sprite_char_initialize (sprite_char_struct * sprite_char, char code,
                               Sint32 xcoord, Sint32 ycoord, Uint32 type);
  void sprite_set_char (sprite_char_struct * sprite_char, char code);
  void sprite_char_init_rotate (sprite_char_struct * sprite_char,
                                Uint32 angle, Sint32 angle_inc,
                                Uint32 radius_x, Uint32 radius_y);
  bool sprite_char_anim (sprite_char_struct * sprite_char, bool restart);
  bool sprite_char_rotate (sprite_char_struct * sprite_char, Sint32 decx,
                           Sint32 decy, Sint32 min);
  bool sprite_char_rotate_enlarge (sprite_char_struct * sprite_char,
                                   Sint32 incx, Sint32 incy, Sint32 max_x,
                                   Sint32 max_y);

  bool sprite_string_set_char (sprite_string_struct * sprite_str,
                               const char *str);
  void sprite_chars_to_image (sprite_string_struct * sprite_str);

  void sprite_string_init_rotation (sprite_string_struct * sprite_str,
                                    Uint32 angle, Uint32 angle_increment,
                                    Uint32 radius_x, Uint32 radius_y);
  void sprite_string_set_center (sprite_string_struct * sprite_str,
                                 Sint32 centerx, Sint32 centery);
  void sprite_string_centerx (sprite_string_struct * sprite_str, float coordy,
                              Uint32 offsetx);
  void sprite_string_center (sprite_string_struct * sprite_str);
  bool sprites_string_rotation_dec (sprite_string_struct * sprite_str,
                                    Uint32 dec_x, Uint32 dec_y,
                                    Sint32 min_pos);
  bool sprites_string_rotation_inc (sprite_string_struct * sprite_str,
                                    Uint32 inc_x, Uint32 inc_y,
                                    Sint32 xmax_pos, Sint32 ymax_pos);
  void sprite_string_coords (sprite_string_struct * sprite_str);
  void sprite_string_init_anim (sprite_string_struct * sprite_str,
                                Uint32 anim_speed_inc, Uint32 anim_speed,
                                bool is_anim, Uint32 anim_speed_count);
  void sprite_string_restart_anim (sprite_string_struct * sprite_str);
  void sprite_string_set_anim (sprite_string_struct * sprite_str);
  Sint32 sprite_string_anim (sprite_string_struct * sprite_str, bool restart);
  bool sprite_string_anim_one (sprite_string_struct * sprite_str);
  void sprite_string_draw (sprite_string_struct * sprite_str);

  Sint32 sprites_string_input (sprite_string_struct * sprite_str);
  void sprites_string_input_code (sprite_string_struct * sprite_str,
                                  Sint32 keycode);
  void sprites_string_set_joy (Uint32 code);
  void sprites_string_clr_joy (Uint32 code);
  void sprites_string_key_down (Uint32 code, Uint32 sym);
  void sprites_string_key_up (Uint32 code, Uint32 sym);
  void sprites_string_set_cursor_pos (sprite_string_struct * sprite_str,
                                      Sint32 cursor_pos);

#ifdef __cplusplus
}
#endif

#endif
