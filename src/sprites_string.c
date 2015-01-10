/** 
 * @file sprites_string.c 
 * @brief  handle the strings fonts of sprites 
 * @created 2006-12-18 
 * @date 2012-08-25
 * @author Bruno Ethvignot 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: sprites_string.c,v 1.31 2012/08/25 15:55:00 gurumeditation Exp $
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
#include "display.h"
#include "electrical_shock.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "scrolltext.h"
#include "sprites_string.h"

#define FONT_SCORE_MAXOF_CHARS 10
#define FONT_BIG_MAXOF_CHARS 27
#define FONT_GAME_MAXOF_CHARS 90

bitmap fnt_game[FONT_GAME_MAXOF_CHARS][FONTS_MAX_OF_IMAGES];
bitmap fnt_big[FONT_BIG_MAXOF_CHARS][FONTS_MAX_OF_IMAGES];
bitmap fnt_score[FONT_SCORE_MAXOF_CHARS][FONTS_MAX_OF_IMAGES];

const Uint32 MAX_OF_STRINGS = 100;
static sprite_string_struct **strings_list = NULL;
static Uint32 num_of_sprites_strings = 0;

static Uint32 keycode_down_prev = 0;
static Uint32 keycode_down = 0;
static Sint32 input_delay = 0;
static Uint32 keycode_up = 0;
static Uint32 keysym_up = 0;
static Uint32 keysym_down = 0;

static Uint32 joy_code_down = 0;
static Uint32 joy_code_down_prev = 0;
static Uint32 joy_up;
static Sint32 input_joy_tempo = 0;

static unsigned char cycling_table[] = {
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 78,
  77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 0
};

static Uint32 cycling_delay = 0;
static Uint32 cycling_index = 0;

/**
 * Color cycling, return new color
 * @return: color index
 */
static unsigned char
get_next_color ()
{
  unsigned char color;
  /* change color? */
  if (cycling_delay++ > 1)
    {
      cycling_delay = 0;
      cycling_index++;
    }
  color = cycling_table[cycling_index];
  /* end of table? */
  if (color == 0)
    {
      color = cycling_table[0];
      cycling_index = 0;
    }
  return color;
}

/**
 * Allocating memory for the list of sprites strings structures 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
sprites_string_once_init (void)
{
  /* allocate list of sprite_char_struct */
  if (strings_list == NULL)
    {
      strings_list =
        (sprite_string_struct **) memory_allocation (MAX_OF_STRINGS *
                                                     sizeof
                                                     (sprite_string_struct
                                                      *));
      if (strings_list == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'strings_list'");
          return FALSE;
        }
    }
  num_of_sprites_strings = 0;


  /* 159,654 bytes */
  if (!bitmap_load
      ("graphics/bitmap/fonts/font_game.spr", &fnt_game[0][0],
       FONT_GAME_MAXOF_CHARS, FONTS_MAX_OF_IMAGES))
    {
      return FALSE;
    }
  /* 149 950 bytes */
  if (!bitmap_load
      ("graphics/bitmap/fonts/font_big.spr", &fnt_big[0][0],
       FONT_BIG_MAXOF_CHARS, FONTS_MAX_OF_IMAGES))
    {
      return FALSE;
    }
  /* 52 480 bytes */
  if (!bitmap_load
      ("graphics/bitmap/fonts/font_score.spr", &fnt_score[0][0],
       FONT_SCORE_MAXOF_CHARS, FONTS_MAX_OF_IMAGES))
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * Convert fonts from data bitmaps to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
sprites_font_extract (void)
{
  Uint32 i, frame;
  const char *model = EXPORT_DIR "/fonts/xxxxx/xx/xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/fonts"))
    {
      free_memory (filename);
      return FALSE;
    }

  if (!create_dir (EXPORT_DIR "/fonts/game"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (i = 0; i < FONT_GAME_MAXOF_CHARS; i++)
    {
      sprintf (filename, EXPORT_DIR "/fonts/game/%02d", i + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < FONTS_MAX_OF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/fonts/game/%02d/%02d.png", i + 1, frame);
          if (!bitmap_to_png
              (&fnt_game[i][frame], filename, 16, 16, offscreen_pitch))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }

  if (!create_dir (EXPORT_DIR "/fonts/big"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (i = 0; i < FONT_BIG_MAXOF_CHARS; i++)
    {
      sprintf (filename, EXPORT_DIR "/fonts/big/%02d", i + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < FONTS_MAX_OF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/fonts/big/%02d/%02d.png", i + 1, frame);
          if (!bitmap_to_png
              (&fnt_big[i][frame], filename, 32, 26, offscreen_pitch))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }

  if (!create_dir (EXPORT_DIR "/fonts/score"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (i = 0; i < FONT_SCORE_MAXOF_CHARS; i++)
    {
      sprintf (filename, EXPORT_DIR "/fonts/score/%02d", i + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < FONTS_MAX_OF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/fonts/score/%02d/%02d.png", i + 1, frame);
          if (!bitmap_to_png
              (&fnt_score[i][frame], filename, 16, 16, score_offscreen_pitch))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif

/**
 * Release all sprites strings structures
 */
void
sprites_string_free (void)
{
  Uint32 i;
  LOG_DBG ("deallocates the memory used by the bitmap");
  if (strings_list == NULL)
    {
      return;
    }
  for (i = 0; i < MAX_OF_STRINGS; i++)
    {
      if (strings_list[i] == NULL)
        {
          continue;
        }
      sprites_string_delete (strings_list[i]);
      strings_list[i] = NULL;
    }
  free_memory ((char *) strings_list);
  strings_list = NULL;
  bitmap_free (&fnt_game[0][0], FONT_GAME_MAXOF_CHARS, FONTS_MAX_OF_IMAGES,
               FONTS_MAX_OF_IMAGES);
  bitmap_free (&fnt_big[0][0], FONT_BIG_MAXOF_CHARS, FONTS_MAX_OF_IMAGES,
               FONTS_MAX_OF_IMAGES);
  bitmap_free (&fnt_score[0][0], FONT_SCORE_MAXOF_CHARS, FONTS_MAX_OF_IMAGES,
               FONTS_MAX_OF_IMAGES);
}

/**
 * Create a new string of sprites characters 
 * @param string A simple string
 * @param more_chars Additional chars to allow
 * @param type type of font (FONT_BIG, FONT_SCROLL, FONT_SCROLL, or FONT_SCORE)
 * @param coordx x coordinate in pixels
 * @param coordy y coordinate in pixels
 * @return pointer to a sprites string structure
 */
sprite_string_struct *
sprites_string_new (const char *const string, Uint32 more_chars, Uint32 type,
                    float coordx, float coordy)
{
  Uint32 i;
  sprite_string_struct *sprite_str;
  char *string_allocated;
  Uint32 numof_chars, maxof_chars;

  if (string != NULL)
    {
      numof_chars = strlen (string);
      maxof_chars = strlen (string) + more_chars;
    }
  else
    {
      numof_chars = maxof_chars = more_chars;
    }
  if (maxof_chars == 0)
    {
      LOG_ERR ("The string's lenght is null!");
      return NULL;
    }

  /* allocate the string */
  string_allocated = memory_allocation (maxof_chars + 1);
  if (string_allocated == NULL)
    {
      LOG_ERR ("not enough memory to allocate a string!");
      return NULL;
    }
  if (string != NULL)
    {
      strcpy (string_allocated, string);
    }
  else
    {
      for (i = 0; i < maxof_chars; i++)
        {
          string_allocated[i] = ' ';
        }
    }

  sprite_str =
    sprites_string_create (string_allocated, maxof_chars, type, coordx,
                           coordy);
  if (sprite_str == NULL)
    {
      free_memory (string_allocated);
      LOG_ERR ("sprites_string_create() failed!");
      return NULL;
    }
  sprite_str->num_of_chars = numof_chars;
  sprite_str->max_of_chars = maxof_chars;
  sprite_str->is_string_allocated = TRUE;
  return sprite_str;
}

/**
 * Create a new string of sprites characters 
 * @param string A simple string
 * @param size Length of the string 
 * @param type Type of font (FONT_BIG, FONT_SCROLL, FONT_SCROLL, or FONT_SCORE)
 * @param coordx X coordinate in pixels
 * @param coordy Y coordinate in pixels
 * @return Pointer to a sprites string structure
 */
sprite_string_struct *
sprites_string_create (char *string, Uint32 size, Uint32 type, float coordx,
                       float coordy)
{
  Uint32 i;
  sprite_char_struct *sprite_char;
  sprite_string_struct *sprite_str;
  if (num_of_sprites_strings >= MAX_OF_STRINGS)
    {
      LOG_ERR ("maximum number of sprites string reached!");
      return NULL;
    }

  /* allocate the sprite string structure */
  sprite_str =
    (sprite_string_struct *)
    memory_allocation (sizeof (sprite_string_struct));
  if (sprite_str == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'sprite_string_struct'!");
      return NULL;
    }
  sprite_str->string = string;
  sprite_str->is_string_allocated = FALSE;
  sprite_str->num_of_chars = size;
  sprite_str->max_of_chars = size;
  sprite_str->coord_x = coordx;
  sprite_str->coord_y = coordy;
  sprite_str->center_x = (Sint32) coordx;
  sprite_str->center_y = (Sint32) coordy;
  sprite_str->delay_next_char = 50;
  sprite_str->delay_next_char_count = 0;
  sprite_str->delay_add_to_last = 200;
  sprite_str->current_char = 0;
  sprite_str->at_least_one_char_changed = FALSE;
  sprite_str->cursor_pos = 0;
  sprite_str->cursor_status = 0;

  /* allocate sprites chars structures */
  sprite_str->sprites_chars =
    (sprite_char_struct *) memory_allocation (sprite_str->max_of_chars *
                                              sizeof (sprite_char_struct));
  if (sprite_str->sprites_chars == NULL)
    {
      free_memory ((char *) sprite_str);
      LOG_ERR ("not enough memory to allocate 'sprite_char_struct'!");
      return NULL;
    }
  for (i = 0; i < sprite_str->max_of_chars; i++)
    {
      sprite_char = &sprite_str->sprites_chars[i];
      sprite_char->anim_speed_inc = 500;
      sprite_char->anim_speed = 1000;
      sprite_char_initialize (sprite_char, sprite_str->string[i],
                              (Sint32) coordx, (Sint32) coordy, type);
    }
  sprite_str->space = sprite_str->sprites_chars[0].font_size;
  sprite_chars_to_image (sprite_str);
  strings_list[num_of_sprites_strings] = sprite_str;
  num_of_sprites_strings++;
  return sprite_str;
}

/**
 * Release a sprites string structure
 * @param a pointer to a sprites string structure
 */
void
sprites_string_delete (sprite_string_struct * sprite_str)
{
  Uint32 i, j;
  for (i = 0; i < num_of_sprites_strings; i++)
    {
      if (strings_list[i] != sprite_str)
        {
          continue;
        }
      if (sprite_str->sprites_chars != NULL)
        {
          free_memory ((char *) sprite_str->sprites_chars);
          sprite_str->sprites_chars = NULL;
        }
      if (sprite_str->string != NULL && sprite_str->is_string_allocated)
        {
          free_memory (sprite_str->string);
          sprite_str->string = NULL;
          sprite_str->is_string_allocated = FALSE;
        }
      free_memory ((char *) sprite_str);
      for (j = i; j < MAX_OF_STRINGS - 1; j++)
        {
          strings_list[j] = strings_list[j + 1];
        }
      strings_list[j] = NULL;
      return;
    }
  LOG_ERR ("sprite_string_struct not found");
}


/**
 * Initialize a char sprite
 * @param sprite_char a pointer to a char sprite structure
 * @param code char ascii code
 * @param xcoord x-coordinate of char and rotate center
 * @param ycoord y-coordinate of char and rotate center
*  @param type type of font
 */
void
sprite_char_initialize (sprite_char_struct * sprite_char, char code,
                        Sint32 xcoord, Sint32 ycoord, Uint32 type)
{
  sprite_char->type_of_font = type;
  switch (sprite_char->type_of_font)
    {
    case FONT_GAME:
      sprite_char->font_size = 16;
      break;
    case FONT_BIG:
      sprite_char->font_size = 32;
      break;
    case FONT_SCROLL:
      sprite_char->font_size = 32;
      break;
    case FONT_SCORE:
      sprite_char->font_size = 16;
      break;
    default:
      sprite_char->type_of_font = FONT_BIG;
      sprite_char->font_size = 32;
      break;
    }
  sprite_char->coord_x = xcoord;
  sprite_char->coord_y = ycoord;
  sprite_char->center_x = xcoord;
  sprite_char->center_y = ycoord;
  sprite_char->current_image = 0;

  switch (sprite_char->type_of_font)
    {
    case FONT_SCORE:
      sprite_char->which_offscreen = FONT_DRAW_TOP_PANEL;
      sprite_char->xmin = 0;
      sprite_char->ymin = 0;
      sprite_char->xmax = score_offscreen_width - sprite_char->font_size;
      sprite_char->ymax = score_offscreen_height - sprite_char->font_size;
      break;
    default:
      sprite_char->which_offscreen = FONT_DRAW_OFFSCREEN;
      sprite_char->xmin = offscreen_clipsize - sprite_char->font_size + 1;
      sprite_char->ymin = offscreen_clipsize - sprite_char->font_size + 1;
      sprite_char->xmax = offscreen_width - offscreen_clipsize - 1;
      sprite_char->ymax = offscreen_height - offscreen_clipsize - 1;
      break;
    }
  sprite_set_char (sprite_char, code);
}

/**
 * Initialize the char of  sprite
 * @param sprite_char a pointer to a char sprite structure
 * @param code char ascii code
 */
void
sprite_set_char (sprite_char_struct * sprite_char, char code)
{
  if (code == 32)
    {
      sprite_char->is_display = FALSE;
    }
  else
    {
      sprite_char->is_display = TRUE;
      switch (sprite_char->type_of_font)
        {
        case FONT_BIG:
          sprite_char->current_char = (Sint32) (code - 65);
          break;
        case FONT_SCORE:
          sprite_char->current_char = (Sint32) (code - 48);
          break;
        default:
          sprite_char->current_char = (Sint32) (code - 33);
          break;
        }
    }
}

/**
 * Initialize values for rotate a char sprite
 * @param sprite_char a pointer to a char sprite structure
 * @param angle 
 * @param angle_inc  
 * @param radius_x 
 * @param radius_y 
 */
void
sprite_char_init_rotate (sprite_char_struct * sprite_char, Uint32 angle,
                         Sint32 angle_inc, Uint32 radius_x, Uint32 radius_y)
{
  sprite_char->angle = angle;
  sprite_char->angle_inc = angle_inc;
  sprite_char->radius_x = radius_x;
  sprite_char->radius_y = radius_y;
}

/* Play animation for a single character
 * @param sprite_char a pointer to a char sprite structure
 * @param restart TRUE if the animation of char restarts
 *        when it is finished 
 * return TRUE if char finished animation
 */
bool
sprite_char_anim (sprite_char_struct * sprite_char, bool restart)
{
  if (!sprite_char->is_anim)
    {
      return FALSE;
    }
  sprite_char->anim_speed_count += sprite_char->anim_speed_inc;
  if (sprite_char->anim_speed_count < sprite_char->anim_speed)
    {
      return FALSE;
    }
  sprite_char->anim_speed_count -= sprite_char->anim_speed;
  sprite_char->current_image++;
  if (sprite_char->current_image >= FONTS_MAX_OF_IMAGES)
    {
      sprite_char->current_image = 0;
      sprite_char->is_anim = restart;
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/* Rotate a single character and reduce radius
 * @param sprite_char a pointer to a char sprite structure
 * @param decx value to decrease x radius
 * @param decy value to decrease y radius
 * @param min radius minimum in pixels
 * return TRUE if rotation is finish
 */
bool
sprite_char_rotate (sprite_char_struct * sprite_char, Sint32 decx,
                    Sint32 decy, Sint32 min)
{
  float sin_value, cos_value;
  bool is_finished = FALSE;
  sprite_char->angle =
    (sprite_char->angle + sprite_char->angle_inc) & MAX_ANGLE;
  if (sprite_char->radius_x > min)
    {
      sprite_char->radius_x -= decx;
    }
  if (sprite_char->radius_y > min)
    {
      sprite_char->radius_y -= decy;
    }
  if (sprite_char->radius_x < min)
    {
      sprite_char->radius_x = min;
    }
  if (sprite_char->radius_y < min)
    {
      sprite_char->radius_y = min;
    }
  if (sprite_char->radius_x == min && sprite_char->radius_y == min)
    {
      is_finished = TRUE;
    }
  sin_value = precalc_sin128[sprite_char->angle];
  cos_value = precalc_cos128[sprite_char->angle];
  sprite_char->coord_x =
    (Sint32) (sin_value * sprite_char->radius_x) + sprite_char->center_x;
  sprite_char->coord_y =
    (Sint32) (cos_value * sprite_char->radius_y) + sprite_char->center_y;
  return is_finished;
}

/* Rotate a single character and enlarge radius
 * @param sprite_char a pointer to a char sprite structure
 * @param incx value to decrease x radius
 * @param incy value to decrease y radius
 * @param max_x radius max_ximum in pixels
 * return TRUE if rotation is finish
 */
bool
sprite_char_rotate_enlarge (sprite_char_struct * sprite_char, Sint32 incx,
                            Sint32 incy, Sint32 max_x, Sint32 max_y)
{
  float sin_value, cos_value;
  bool is_finished = FALSE;
  sprite_char->angle =
    (sprite_char->angle + sprite_char->angle_inc) & MAX_ANGLE;
  sin_value = precalc_sin128[sprite_char->angle];
  cos_value = precalc_cos128[sprite_char->angle];
  if (sprite_char->radius_x < max_x)
    {
      sprite_char->radius_x += incx;
    }
  if (sprite_char->radius_y < max_y)
    {
      sprite_char->radius_y += incy;
    }
  if (sprite_char->radius_x > max_x)
    {
      sprite_char->radius_x = max_x;
    }
  if (sprite_char->radius_y > max_y)
    {
      sprite_char->radius_y = max_x;
    }
  if (sprite_char->radius_x == max_x && sprite_char->radius_y == max_y)
    {
      is_finished = TRUE;
    }
  sprite_char->coord_x =
    (Sint32) (sin_value * sprite_char->radius_x) + sprite_char->center_x;
  sprite_char->coord_y =
    (Sint32) (cos_value * sprite_char->radius_y) + sprite_char->center_y;
  return is_finished;
}

/**
 * Initialize a new string 
 * @param sprite_str a pointer to a sprites string structure
 * @param str a simple string
 */
bool
sprite_string_set_char (sprite_string_struct * sprite_str, const char *str)
{
  if (strlen (str) > sprite_str->max_of_chars)
    {
      LOG_ERR ("string too long");
      return FALSE;
    }
  strcpy (sprite_str->string, str);
  sprite_str->num_of_chars = strlen (sprite_str->string);
  /* convert ASCII code to pointer to the bitmap structure of the font */
  sprite_chars_to_image (sprite_str);
  return TRUE;
}

/**
 * Initialize values for chars rotation 
 * @param sprite_str a pointer to a sprites string structure
 * @param angle 
 * @param angle_increment
 * @radius_x
 * @radius_y 
 */
void
sprite_string_init_rotation (sprite_string_struct * sprite_str, Uint32 angle,
                             Uint32 angle_increment, Uint32 radius_x,
                             Uint32 radius_y)
{
  sprite_str->angle = angle;
  sprite_str->angle_increment = angle_increment;
  sprite_str->radius_x = radius_x;
  sprite_str->radius_y = radius_y;
}

/**
 * Center the string horizontally at the screen
 * @param sprite_str a pointer to a sprites string structure
 * @param coordy y-coordinate
 * @param offsetx horizontal offset to add to the x-coordinate 
 */
void
sprite_string_centerx (sprite_string_struct * sprite_str, float coordy,
                       Uint32 offsetx)
{
  Uint32 i, coordx, width;
  width = sprite_str->space;
  coordx =
    (offscreen_width_visible - sprite_str->num_of_chars * width) / 2 +
    offscreen_startx + offsetx;
  sprite_str->coord_x = (float) coordx;
  sprite_str->coord_y = coordy + offscreen_starty;
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_str->sprites_chars[i].coord_x = coordx;
      sprite_str->sprites_chars[i].coord_y = (Uint32) sprite_str->coord_y;
      coordx += width;
    }
}

/**
 * Center string horizontally and vertically at the screen 
 * @param sprite_str a pointer to a sprites string structure
 */
void
sprite_string_center (sprite_string_struct * sprite_str)
{
  Uint32 i, coordx, coordy, width;
  width = sprite_str->space;
  coordx =
    (offscreen_width_visible - sprite_str->num_of_chars * width) / 2 +
    offscreen_startx;
  coordy = (offscreen_height_visible - width) / 2 + offscreen_starty;
  sprite_str->coord_x = (float) coordx;
  sprite_str->coord_y = (float) coordy;
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_str->sprites_chars[i].coord_x = coordx;
      sprite_str->sprites_chars[i].coord_y = coordy;
      coordx += width;
    }
}

/**
 * Initialize radius center for chars rotation
 * @param sprite_str a pointer to a sprites string structure
 * @param centerx center x-coordinate, -1 if center in middle of screen vertically
 * @param centery center y-coordinate, -1 if center in middle of screen horizontally
 */
void
sprite_string_set_center (sprite_string_struct * sprite_str, Sint32 centerx,
                          Sint32 centery)
{
  if (centerx == -1)
    {
      sprite_str->center_x =
        (offscreen_width_visible -
         sprite_str->num_of_chars * sprite_str->space) / 2 + offscreen_startx;
    }
  else
    {
      sprite_str->center_x = centerx;
    }
  if (centery == -1)
    {
      sprite_str->center_y =
        (offscreen_height_visible - sprite_str->space) / 2 + offscreen_starty;
    }
  else
    {
      sprite_str->center_y = centery;
    }
}

/**
 * Decrease radius for chars rotations
 * @param sprite_str a pointer to a sprites string structure
 * @param dec_x
 * @param dec_y
 * @param min_pos
 */
bool
sprites_string_rotation_dec (sprite_string_struct * sprite_str, Uint32 dec_x,
                             Uint32 dec_y, Sint32 min_pos)
{
  float sin, cos;
  sprite_str->angle =
    (sprite_str->angle + sprite_str->angle_increment) & MAX_ANGLE;
  sin = precalc_sin128[sprite_str->angle];
  cos = precalc_cos128[sprite_str->angle];
  sprite_str->coord_x = (sin * sprite_str->radius_x) + sprite_str->center_x;
  sprite_str->coord_y = (cos * sprite_str->radius_y) + sprite_str->center_y;
  sprite_str->radius_x -= dec_x;
  sprite_str->radius_y -= dec_y;
  if (sprite_str->radius_x < min_pos)
    {
      sprite_str->radius_x = min_pos;
    }
  if (sprite_str->radius_y < min_pos)
    {
      sprite_str->radius_y = min_pos;
    }
  if (sprite_str->radius_x == min_pos && sprite_str->radius_y == min_pos)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/**
 * Increase radius for chars rotations
 * @param sprite_str a pointer to a sprites string structure
 * @param inc_x
 * @param inc_y
 * @param xmax_pos
 * @param ymax_pos
 */
bool
sprites_string_rotation_inc (sprite_string_struct * sprite_str, Uint32 inc_x,
                             Uint32 inc_y, Sint32 xmax_pos, Sint32 ymax_pos)
{
  float sin, cos;
  sprite_str->angle =
    (sprite_str->angle + sprite_str->angle_increment) & MAX_ANGLE;
  sin = precalc_sin128[sprite_str->angle];
  cos = precalc_cos128[sprite_str->angle];
  sprite_str->coord_x = (sin * sprite_str->radius_x) + sprite_str->center_x;
  sprite_str->coord_y = (cos * sprite_str->radius_y) + sprite_str->center_y;
  sprite_str->radius_x += inc_x;
  sprite_str->radius_y += inc_y;
  if (sprite_str->radius_x > xmax_pos)
    {
      sprite_str->radius_x = xmax_pos;
    }
  if (sprite_str->radius_y > ymax_pos)
    {
      sprite_str->radius_y = ymax_pos;
    }
  if (sprite_str->radius_x == xmax_pos && sprite_str->radius_y == ymax_pos)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/**
 * Initialize coordinates of each char
 * @param sprite_str pointer to a sprites string structure
 */
void
sprite_string_coords (sprite_string_struct * sprite_str)
{
  Uint32 i, coordx, coordy;
  sprite_char_struct *sprite_char;
  coordx = (Uint32) (sprite_str->coord_x + sprite_str->offset_x);
  coordy = (Uint32) (sprite_str->coord_y + sprite_str->offset_y);
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_char = &sprite_str->sprites_chars[i];
      sprite_char->coord_x = coordx;
      sprite_char->coord_y = coordy;
      coordx += sprite_str->space;
    }
}

/**
 * Initialization of the speed of animation of each char 
 * @param sprite_str pointer to a sprites string structure
 * @param anim_speed_inc
 * @param anim_speed
 * @param is_anim
 * @pram anim_speed_count
 */
void
sprite_string_init_anim (sprite_string_struct * sprite_str,
                         Uint32 anim_speed_inc, Uint32 anim_speed,
                         bool is_anim, Uint32 anim_speed_count)
{
  Uint32 i;
  sprite_char_struct *sprite_char;
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_char = &sprite_str->sprites_chars[i];
      sprite_char->anim_speed_count = anim_speed_count;
      sprite_char->anim_speed_inc = anim_speed_inc;
      sprite_char->anim_speed = anim_speed;
      sprite_char->is_anim = is_anim;
    }
}

/**
 * Convert ASCII code char to pointer to the bitmap structure of the char 
 * @param sprite_str pointer to a sprites string structure
 */
void
sprite_chars_to_image (sprite_string_struct * sprite_str)
{
  Uint32 i;
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_set_char (&sprite_str->sprites_chars[i], sprite_str->string[i]);
    }
}

/**
 * Set the first image of each char 
 * @param sprite_str pointer to a sprites string structure
 */
void
sprite_string_restart_anim (sprite_string_struct * sprite_str)
{
  Uint32 i;

  /* clear current image */
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_str->sprites_chars[i].current_image = 0;
    }
}

/**
 * Enable animation of each char 
 * @param sprite_str pointer to a sprites string structure
 */
void
sprite_string_set_anim (sprite_string_struct * sprite_str)
{
  Uint32 i;

  /* enable char animation */
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_str->sprites_chars[i].is_anim = TRUE;
    }
}

/**
 * Handle animation of each char 
 * @param sprite_str pointer to a sprites string structure
 * @param restart TRUE if the animation of a char restarts
 *        when it is finished 
 * return index of the last char updated, otherwise -1 
 */
Sint32
sprite_string_anim (sprite_string_struct * sprite_str, bool restart)
{
  Uint32 i;
  sprite_char_struct *sprite_char;
  Sint32 char_num = -1;
  sprite_str->at_least_one_char_changed = FALSE;
  sprite_str->none_char_animated = TRUE;

  /* animation of the chars */
  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_char = &sprite_str->sprites_chars[i];
      if (!sprite_char->is_anim)
        {
          continue;
        }
      sprite_str->none_char_animated = FALSE;
      sprite_char->anim_speed_count += sprite_char->anim_speed_inc;
      if (sprite_char->anim_speed_count < sprite_char->anim_speed)
        {
          continue;
        }
      sprite_str->at_least_one_char_changed = TRUE;
      sprite_char->anim_speed_count -= sprite_char->anim_speed;
      sprite_char->current_image++;
      if (sprite_char->current_image >= FONTS_MAX_OF_IMAGES)
        {
          sprite_char->current_image = 0;
          sprite_char->is_anim = restart;
          char_num = i;
        }
    }
  return char_num;
}

/**
 * Animation of each char separately, one by one 
 * (like PAUSE text)
 * @param sprite_str pointer to a sprites string structure
 */
bool
sprite_string_anim_one (sprite_string_struct * sprite_str)
{
  Uint32 count;
  bool is_display, is_last;
  is_last = FALSE;
  if (sprite_string_anim (sprite_str, FALSE) >=
      (Sint32) (sprite_str->num_of_chars - 1))
    {
      is_last = TRUE;
    }

  /* enable animation of the next char */
  sprite_str->delay_next_char_count++;
  if (sprite_str->delay_next_char_count > sprite_str->delay_next_char)
    {
      sprite_str->delay_next_char_count = 0;
      count = sprite_str->num_of_chars;
      do
        {
          count--;
          is_display =
            sprite_str->sprites_chars[sprite_str->current_char].is_display;
          if (is_display)
            {
              sprite_str->sprites_chars[sprite_str->current_char].is_anim =
                TRUE;
            }
          sprite_str->current_char++;
          if (sprite_str->current_char >= sprite_str->num_of_chars)
            {
              sprite_str->current_char = 0;
              sprite_str->delay_next_char_count -=
                sprite_str->delay_add_to_last;
            }
        }
      while (!is_display && count > 0);
    }
  return is_last;
}

/**
 * Draw the chars sprites
 * @param sprite_str pointer to a sprites string structure
 */
void
sprite_string_draw (sprite_string_struct * sprite_str)
{
  Uint32 i;
  sprite_char_struct *sprite_char;
  bitmap *img;

  for (i = 0; i < sprite_str->num_of_chars; i++)
    {
      sprite_char = &sprite_str->sprites_chars[i];
      if (!sprite_char->is_display)
        {
          continue;
        }
      if (sprite_char->coord_x < sprite_char->xmin
          || sprite_char->coord_x > sprite_char->xmax
          || sprite_char->coord_y < sprite_char->ymin
          || sprite_char->coord_y > sprite_char->ymax)
        {
          continue;
        }
      switch (sprite_char->type_of_font)
        {
        case FONT_GAME:
          img =
            &fnt_game[sprite_char->current_char][sprite_char->current_image];
          break;
        default:
        case FONT_BIG:
          img =
            &fnt_big[sprite_char->current_char][sprite_char->current_image];
          break;
        case FONT_SCROLL:
          img = &fnt_scroll[sprite_char->current_char];
          break;
        case FONT_SCORE:
          img =
            &fnt_score[sprite_char->current_char][sprite_char->current_image];
          break;
        }
      switch (sprite_char->which_offscreen)
        {
        case FONT_DRAW_TOP_PANEL:
          draw_bitmap_in_score (img, sprite_char->coord_x,
                                sprite_char->coord_y);
          break;
        default:
          draw_bitmap (img, sprite_char->coord_x, sprite_char->coord_y);
          break;
        }
    }
}

/**
 * Input string from keyboard and joystick
 * @param sprite_str pointer to a sprites string structure
 * @return keycode pressed
 */
Sint32
sprites_string_input (sprite_string_struct * sprite_str)
{
  sprite_char_struct *sprite_char;
  char c;
  Sint32 joycode;
  Sint32 keycode = 0;
  if (input_delay < 1)
    {
      keycode = keycode_down;
      if (keycode_down > 0)
        {
          /* key pressed for the first time? */
          if (keycode_down_prev != keycode_down)
            {
              keycode_down_prev = keycode_down;
              input_delay = 40;
            }
          else
            {
              input_delay = 5;
            }
        }
    }
  else
    {
      keycode = 0;
      input_delay--;
    }
  sprites_string_input_code (sprite_str, keycode);

  /* input string with the joystick */
  joycode = 0;
  if (input_joy_tempo < 1)
    {
      joycode = joy_code_down;
      if (joy_code_down > 0)
        {
          /* button pressed for the first time? */
          if (joy_code_down_prev != joy_code_down)
            {
              joy_code_down_prev = joy_code_down;
              input_joy_tempo = 40;
            }
          else
            {
              input_joy_tempo = 5;
            }
        }
    }
  else
    {
      joycode = 0;
      input_joy_tempo--;
    }
  if (joycode > 0)
    {
      switch (joycode)
        {
        case IJOY_FIRE:
#ifdef POWERMANGA_SDL
          keycode = SDLK_RETURN;
#else
          keycode = XK_Return;
#endif
          sprites_string_input_code (sprite_str, keycode);
          break;
        case IJOY_OPT:
          sprite_str->string[sprite_str->cursor_pos] = ' ';
          sprite_chars_to_image (sprite_str);
          break;
        case IJOY_LEFT:
#ifdef POWERMANGA_SDL
          sprites_string_input_code (sprite_str, SDLK_LEFT);
#else
          sprites_string_input_code (sprite_str, XK_Left);
#endif
          break;
        case IJOY_RIGHT:
#ifdef POWERMANGA_SDL
          sprites_string_input_code (sprite_str, SDLK_RIGHT);
#else
          sprites_string_input_code (sprite_str, XK_Right);
#endif
          break;
        case IJOY_TOP:
          {
            c = sprite_str->string[sprite_str->cursor_pos] + 1;
            if (c > 90 && c < 97)
              {
                c = 97;
              }
            else if (c > 122)
              {
                c = 32;
              }
            sprite_str->string[sprite_str->cursor_pos] = c;
            sprite_chars_to_image (sprite_str);
            sprite_char = &sprite_str->sprites_chars[sprite_str->cursor_pos];
            sprite_char->is_anim = TRUE;
          }
          break;
        case IJOY_DOWN:
          c = sprite_str->string[sprite_str->cursor_pos] - 1;
          if (c > 90 && c < 97)
            {
              c = 90;
            }
          else if (c < 32)
            {
              c = 122;
            }
          sprite_str->string[sprite_str->cursor_pos] = c;
          sprite_chars_to_image (sprite_str);
          sprite_char = &sprite_str->sprites_chars[sprite_str->cursor_pos];
          sprite_char->is_anim = TRUE;
          break;
        }

    }
  return keycode;
}

/**
 * Input string from keyboard
 * @param sprite_str pointer to a sprites string structure
 * @param keycode the keycode pressed
 */
void
sprites_string_input_code (sprite_string_struct * sprite_str, Sint32 keycode)
{
  Uint32 i, j;
  Sint32 xcoord, ycoord, cursor_size;
  unsigned char color;
  float sin_val;
  sprite_char_struct *sprite_char;
  sprite_str->cursor_status = 0;

  /* display blink cursor */
  color = get_next_color ();
  sprite_str->angle = (sprite_str->angle + 1) & MAX_ANGLE;
  sin_val = precalc_sin128[sprite_str->angle];
  cursor_size = (Sint32) (sin_val * sprite_str->space / 2);
  if (cursor_size < 0)
    {
      cursor_size = -cursor_size;
    }
  cursor_size++;
  xcoord =
    (Sint32) sprite_str->coord_x +
    (sprite_str->cursor_pos * sprite_str->space) + sprite_str->space / 2 -
    cursor_size;
  ycoord = (Sint32) sprite_str->coord_y + sprite_str->space / 2 - cursor_size;
  cursor_size += cursor_size;
  if (ycoord + cursor_size >= offscreen_starty)
    {
      draw_empty_rectangle (game_offscreen, xcoord, ycoord - 2, color,
                            cursor_size, cursor_size);
    }

  /* check key code */
  switch (keycode)
    {
#ifdef POWERMANGA_SDL
    case SDLK_LEFT:
#else
    case XK_Left:
#endif
      sprite_str->cursor_pos--;
      break;
#ifdef POWERMANGA_SDL
    case SDLK_RIGHT:
#else
    case XK_Right:
#endif
      sprite_str->cursor_pos++;
      break;
#ifdef POWERMANGA_SDL
    case SDLK_BACKSPACE:
#else
    case XK_BackSpace:
#endif
      if (sprite_str->cursor_pos > 0)
        {
          j = sprite_str->cursor_pos;
        }
      else
        {
          j = 1;
        }
      for (i = j; i < sprite_str->num_of_chars; i++)
        {
          sprite_str->string[i - 1] = sprite_str->string[i];
        }
      sprite_str->string[sprite_str->num_of_chars - 1] = ' ';
      sprite_str->cursor_pos--;
      sprite_chars_to_image (sprite_str);
      break;
#ifdef POWERMANGA_SDL
    case SDLK_DELETE:
#else
    case XK_Delete:
#endif
      for (i = sprite_str->cursor_pos; i <= sprite_str->num_of_chars - 2; i++)
        {
          sprite_str->string[i] = sprite_str->string[i + 1];
        }
      sprite_str->string[sprite_str->num_of_chars - 1] = ' ';
      sprite_chars_to_image (sprite_str);
      break;
#ifdef POWERMANGA_SDL
    case SDLK_RETURN:
      break;
    case SDLK_UP:
      break;
    case SDLK_DOWN:
      break;
    case SDLK_LSHIFT:
      break;
    case SDLK_RSHIFT:
      break;
    case SDLK_LCTRL:
      break;
    case SDLK_RCTRL:
      break;
    case SDLK_F1:
      break;
    case SDLK_F2:
      break;
    case SDLK_F3:
      break;
    case SDLK_F4:
      break;
    case SDLK_F5:
      break;
    case SDLK_F6:
      break;
    case SDLK_F7:
      break;
    case SDLK_F8:
      break;
    case SDLK_F9:
      break;
    case SDLK_F10:
      break;
    case SDLK_F11:
      break;
    case SDLK_F12:
      break;
#else
    case XK_Return:
      break;
    case XK_Up:
      break;
    case XK_Down:
      break;
    case XK_Shift_L:
      break;
    case XK_Shift_R:
      break;
    case XK_Control_L:
      break;
    case XK_Control_R:
      break;
    case XK_F1:
      break;
    case XK_F2:
      break;
    case XK_F3:
      break;
    case XK_F4:
      break;
    case XK_F5:
      break;
    case XK_F6:
      break;
    case XK_F7:
      break;
    case XK_F8:
      break;
    case XK_F9:
      break;
    case XK_F10:
      break;
    case XK_F11:
      break;
    case XK_F12:
      break;
#endif
    default:
      keycode = keycode & 127;
      if (keycode >= ' ' && keycode <= 'z')
        {
          for (i = sprite_str->num_of_chars - 1;
               i > (Uint32) sprite_str->cursor_pos; i--)
            {
              sprite_str->string[i] = sprite_str->string[i - 1];
            }
          sprite_str->string[sprite_str->cursor_pos] = (char) keycode;
          sprite_char = &sprite_str->sprites_chars[sprite_str->cursor_pos];
          sprite_str->cursor_pos++;
          sprite_chars_to_image (sprite_str);
        }
    }

  /* checks the cursor position */
  if (sprite_str->cursor_pos < 0)
    {
      sprite_str->cursor_pos = 0;
      sprite_str->cursor_status = 1;
    }
  if ((Uint32) sprite_str->cursor_pos > sprite_str->num_of_chars - 1)
    {
      sprite_str->cursor_pos = sprite_str->num_of_chars - 1;
      sprite_str->cursor_status = 2;
    }
  sprite_char = &sprite_str->sprites_chars[sprite_str->cursor_pos];
  sprite_char->is_anim = TRUE;
}


/**
 * Set a joystick code
 * @param code A joystick code, a buttom is down
 */
void
sprites_string_set_joy (Uint32 code)
{
  joy_code_down = code;
  input_joy_tempo = 0;
}

/**
 * Clear a joystick code, a button is up
 * @param code A joystick code
 */
void
sprites_string_clr_joy (Uint32 code)
{
  joy_up = code;
  if (joy_up == joy_code_down)
    {
      joy_code_down = 0;
      input_joy_tempo = 0;
      input_delay = 0;
      joy_code_down_prev = 0;
    }
}

/**
 * Set the code of the pressed key
 * @param code
 * @param sym
 */
void
sprites_string_key_down (Uint32 code, Uint32 sym)
{
  keycode_down = code;
  keysym_down = sym;
  input_delay = 0;
}

/**
 * Set the code of the released key
 * @param code
 * @param sym
 */
void
sprites_string_key_up (Uint32 code, Uint32 sym)
{
  keycode_up = code;
  keysym_up = sym;
  if (keysym_up == keysym_down)
    {
      keycode_down = 0;
      keysym_down = 0;
      input_delay = 0;
      keycode_down_prev = 0;
    }
}

/**
 * Set the cursor position 
 * @param sprite_str Pointer to a sprites string structure
 * @param cursor_pos The cursor position 
 */
void
sprites_string_set_cursor_pos (sprite_string_struct * sprite_str,
                               Sint32 cursor_pos)
{
  sprite_str->cursor_pos = cursor_pos;
  if (sprite_str->cursor_pos < 0)
    {
      sprite_str->cursor_pos = 0;
    }
  if ((Uint32) sprite_str->cursor_pos > sprite_str->num_of_chars)
    {
      sprite_str->cursor_pos = sprite_str->num_of_chars;
    }
}
