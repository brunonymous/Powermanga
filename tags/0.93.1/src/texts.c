/** 
 * @file texts.c
 * @brief Handle texts PAUSE, LEVEL, GAME OVER, PAUSE, enemy's name
 * @created 1999-09-05 
 * @date 2012-08-26 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: texts.c,v 1.17 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "enemies.h"
#include "shots.h"
#include "gfx_wrapper.h"
#include "guardians.h"
#include "log_recorder.h"
#include "meteors_phase.h"
#include "menu.h"
#include "spaceship.h"
#include "sprites_string.h"
#include "texts.h"
const Sint32 POS_Y_GAME = 179;
const Sint32 POS_Y_OVER = 222;

/** New player's score value */
Sint32 player_score;
/** Old player's score value */
static Sint32 old_player_score;
/** Level number: from 0 to 41 */
Sint32 num_level = -1;
/** If TRUE refresh player's score points */
bool is_player_score_displayed = FALSE;

static bool texts_load (void);

/** Strings loaded from "texts/text_en.txt" or "texts/text_fr.txt" file */
static char **texts_loaded = NULL;
static Uint32 texts_loaded_count = 0;
typedef enum
{
  TEXT_LEVEL,
  TEXT_PAUSE,
  TEXT_GAME,
  TEXT_OVER,
  TEXT_NUMOFLINES
}
TEXT_ENUM;

static sprite_string_struct *text_pause = NULL;
static sprite_string_struct *text_level = NULL;
static sprite_string_struct *text_game = NULL;
static sprite_string_struct *text_over = NULL;
static sprite_string_struct *text_score = NULL;
static sprite_string_struct *text_enemy_name = NULL;
static Uint32 text_gameover_case = 0;

/**
 * First initialization, load strings file, create structures for texts like
 * "PAUSE", "LEVEL", "GAME OVER"
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
texts_init_once (void)
{
  Uint32 i;
  /* load game texts */
  if (!texts_load ())
    {
      return FALSE;
    }

  /* create structures for PAUSE text */
  text_pause =
    sprites_string_new (texts_loaded[TEXT_PAUSE], 0, FONT_BIG, 0, 0);
  if (text_pause == NULL)
    {
      LOG_ERR ("creation of 'PAUSE' sprites string failed");
      return FALSE;
    }
  sprite_string_centerx (text_pause, (float) (77 * pixel_size),
                         2 * pixel_size);
  sprite_string_init_anim (text_pause, 250, 1000, FALSE, 0);
  text_pause->space = text_pause->sprites_chars[0].font_size + 1 * pixel_size;

  /* create structures for LEVEL text */
  text_level =
    sprites_string_new (texts_loaded[TEXT_LEVEL], 3, FONT_GAME, 0, 0);
  if (text_level == NULL)
    {
      LOG_ERR ("creation of 'LEVEL' sprites string failed");
      return FALSE;
    }
  sprite_string_init_anim (text_level, 500, 1000, TRUE, 0);

  /* create structures for "GAME" text */
  text_game = sprites_string_new (texts_loaded[TEXT_GAME], 0, FONT_BIG, 0, 0);
  if (text_game == NULL)
    {
      LOG_ERR ("creation of 'GAME' sprites string failed");
      return FALSE;
    }
  sprite_string_set_center (text_game, -1, POS_Y_GAME);
  sprite_string_init_anim (text_game, 1, 6, FALSE, FONTS_MAX_OF_IMAGES * 5);
  text_game->delay_next_char = FONTS_MAX_OF_IMAGES * 6;
  text_game->delay_next_char_count = text_game->delay_next_char;

  /* create structures for "OVER" text */
  text_over = sprites_string_new (texts_loaded[TEXT_OVER], 0, FONT_BIG, 0, 0);
  if (text_over == NULL)
    {
      LOG_ERR ("creation of 'OVER' sprites string failed!");
      return FALSE;
    }
  sprite_string_set_center (text_over, -1, POS_Y_OVER);
  sprite_string_init_anim (text_over, 1, 6, FALSE, FONTS_MAX_OF_IMAGES * 5);
  text_over->delay_next_char = FONTS_MAX_OF_IMAGES * 6;
  text_over->delay_add_to_last = 0;

  /* create structures for the player's score */
  text_score =
    sprites_string_new ("00000000", 0, FONT_SCORE,
                        (float) (68 * pixel_size), 0);
  if (text_score == NULL)
    {
      LOG_ERR ("creation of score sprites string failed!");
      return FALSE;
    }
  sprite_string_init_anim (text_score, 500, 1000, TRUE, 0);
  for (i = 0; i < text_score->num_of_chars; i++)
    {
      text_score->sprites_chars[i].anim_speed_inc =
        (350 + text_score->num_of_chars * 25) - 25 * i;
    }
  sprite_string_coords (text_score);

  /* create structures for the enemy name in congratulations */
  text_enemy_name =
    sprites_string_new ("                                      ", 0,
                        FONT_GAME, 0, 0);
  if (text_enemy_name == NULL)
    {
      LOG_ERR ("creation of congratulations sprites string failed!");
      return FALSE;
    }
  sprite_string_init_anim (text_enemy_name, 1, 6, TRUE, 0);
  return TRUE;
}

/**
 * Release sprites strings structures and other strings
 */
void
texts_free (void)
{
  Uint32 i;

  /* free text loaded from "texts/text_en.txt" file */
  if (texts_loaded != NULL)
    {
      for (i = 0; i < texts_loaded_count; i++)
        {
          if (texts_loaded[i] != NULL)
            {
              free_memory (texts_loaded[i]);
              texts_loaded[i] = NULL;
            }
        }
      free_memory ((char *) texts_loaded);
      texts_loaded = NULL;
    }

  if (text_level != NULL)
    {
      sprites_string_delete (text_level);
      text_level = NULL;
    }
  if (text_pause != NULL)
    {
      sprites_string_delete (text_pause);
      text_pause = NULL;
    }
  if (text_game != NULL)
    {
      sprites_string_delete (text_game);
      text_game = NULL;
    }
  if (text_over != NULL)
    {
      sprites_string_delete (text_over);
      text_over = NULL;
    }
  if (text_score != NULL)
    {
      sprites_string_delete (text_score);
      text_score = NULL;
    }
  if (text_enemy_name != NULL)
    {
      sprites_string_delete (text_enemy_name);
      text_enemy_name = NULL;
    }
}

/** 
 * Initialize a sprites string with a enemy's name in the
 * congratulations phase
 * @param name a enemy's name
 */
void
text_enemy_name_init (const char *name)
{
  if (!sprite_string_set_char (text_enemy_name, name))
    {
      return;
    }
  /* enable animation of each char */
  sprite_string_set_anim (text_enemy_name);
  /* center the string horizontally at the screen */
  sprite_string_centerx (text_enemy_name, 0, 0);
}

/** 
 * Draw name of the enemy 
 * @param offsetx
 * @param ycoord y coordinate of the first char
 * @param restart TRUE if the animation of a char restarts when it is
 * finished 
 */
void
text_enemy_name_draw (Sint32 offsetx, Sint32 ycoord, bool restart)
{
  text_enemy_name->offset_x = (float) offsetx;
  text_enemy_name->coord_y = (float) ycoord;
  /* set coordinates for each chars */
  sprite_string_coords (text_enemy_name);
  sprite_string_anim (text_enemy_name, restart);
  sprite_string_draw (text_enemy_name);
}

/** 
 * Display score number
 */
void
text_draw_score (void)
{
  bool is_updated = FALSE;
  if (player_score != old_player_score)
    {
      sprite_string_set_anim (text_score);
      sprintf (text_score->string, "%08d", player_score);
      sprite_chars_to_image (text_score);
      old_player_score = player_score;
      is_updated = TRUE;
    }
  sprite_string_anim (text_score, FALSE);
  if (text_score->at_least_one_char_changed || is_updated)
    {
      sprite_string_draw (text_score);
      /* refresh player's score into the top panel */
      is_player_score_displayed = TRUE;
    }
}

/**
 * Initialize the appearance of the "LEVEL n" chars if it is 
 * not active, and check if the text is centered horizontally
 * on the screen
 * @return "LEVEL n" string is located at the center of the screen 
 */
bool
text_level_move (Sint32 level_nu)
{
  if (!text_level->is_move)
    {
      text_level->is_move = TRUE;
      sprintf (text_level->string, "%s %d", texts_loaded[TEXT_LEVEL],
               level_nu + 1);
      text_level->num_of_chars = strlen (text_level->string);
      sprite_chars_to_image (text_level);
      text_level->offset_x = 128.0;
      text_level->offset_y = 0.0;
      sprite_string_center (text_level);
      /* set the first image of each char */
      sprite_string_restart_anim (text_level);
      /* enable animation of each char */
      sprite_string_set_anim (text_level);
    }
  if (text_level->offset_y == 0.0 && text_level->offset_x == 0.0)
    {
      /* "LEVEL n" string is located at the center of the screen */
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/**
 * Move and draw "LEVEL N" chars 
 */
void
text_level_draw (void)
{
  if (!text_level->is_move)
    {
      return;
    }

  /* meteors or guardian phase? */
  if (meteor_activity || guardian->number > 0)
    {
      /* horizontal displacement, toward left */
      text_level->offset_x -= 2 * pixel_size;
      if (text_level->offset_x < 0.0)
        {
          text_level->offset_x = 0.0;
        }
    }
  else
    {
      sprite_string_set_anim (text_level);
      text_level->offset_y += (float) (0.4 * pixel_size);
      if (text_level->offset_y > 100 * pixel_size)
        {
          text_level->is_move = FALSE;
        }
    }
  sprite_string_coords (text_level);
  sprite_string_anim (text_level, FALSE);
  sprite_string_draw (text_level);
}

/**
 * Initialize the appearance of the "GAME OVER" chars 
 */
void
text_gameover_init (void)
{
  sprite_string_init_rotation (text_game, 64, 0, 0, 120);
  sprite_string_init_rotation (text_over, 0, 0, 0, 120);
  /* set the first image of each char */
  sprite_string_restart_anim (text_game);
  sprite_string_restart_anim (text_over);
  /* enable animation of each char */
  sprite_string_set_anim (text_game);
  sprite_string_set_anim (text_over);
  text_game->is_move = TRUE;
  text_gameover_case = 0;
}

/**
 * Draw and move "GAME OVER" chars
 */
bool
text_gameover_draw (void)
{
  bool is_finished = FALSE;
  switch (text_gameover_case)
    {
      /* GAME OVER appearing in progress */
    case 0:
      is_finished = TRUE;
      if (!sprites_string_rotation_dec (text_game, 2, 2, 0))
        {
          is_finished = FALSE;
        }
      if (!sprites_string_rotation_dec (text_over, 2, 2, 0))
        {
          is_finished = FALSE;
        }
      sprite_string_anim (text_game, TRUE);
      sprite_string_anim (text_over, TRUE);
      if (is_finished)
        {
          text_gameover_case = 1;
          is_finished = FALSE;
        }
      break;

    case 1:
      is_finished = FALSE;
      if (sprite_string_anim (text_game, FALSE) >= 0)
        {
          is_finished = TRUE;
        }
      if (sprite_string_anim (text_over, FALSE) >= 0)
        {
          is_finished = TRUE;
        }
      if (is_finished)
        {
          text_gameover_case = 2;
        }
      break;

      /* animation of each char separately, one by one  */
    case 2:
      is_finished = TRUE;
      if (text_game->is_move)
        {
          if (sprite_string_anim_one (text_game))
            {
              text_game->is_move = FALSE;
              text_over->is_move = TRUE;
            }
        }
      else
        {
          if (sprite_string_anim_one (text_over))
            {
              text_over->is_move = FALSE;
              text_game->is_move = TRUE;
            }
        }
    }

  if (menu_status != MENU_OFF)
    {
      text_game->coord_y =
        (float) (POS_Y_GAME + menu_coord_y - (offscreen_clipsize +
                                              offscreen_height_visible));
      text_over->coord_y =
        (float) (POS_Y_OVER + menu_coord_y - (offscreen_clipsize +
                                              offscreen_height_visible));
    }
  sprite_string_coords (text_game);
  sprite_string_coords (text_over);
  sprite_string_draw (text_game);
  sprite_string_draw (text_over);
  return is_finished;
}

/** 
 * Display "PAUSE" chars sprites 
 */
void
text_pause_draw (void)
{
  sprite_string_anim_one (text_pause);
  sprite_string_draw (text_pause);
}

/** 
 * Initialize player score
 */
void
texts_init (void)
{
  player_score = 0;
  old_player_score = -1;
}

/**
 * Load game texts files 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
texts_load (void)
{
  Uint32 offset, char_count, start, line_num, filesize;
  char c, *str, *filedata;
  filedata = loadfile_with_lang ("texts/text_%s.txt", &filesize);
  if (filedata == NULL)
    {
      return FALSE;
    }

  /* 
   * caclulate the number of lines 
   */
  offset = 0;
  texts_loaded_count = 0;
  while (offset < filesize)
    {
      if (filedata[offset++] == '\n')
        {
          texts_loaded_count++;
        }
    }
  texts_loaded =
    (char **) memory_allocation (texts_loaded_count * sizeof (char *));
  if (texts_loaded == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'texts_loaded'!");
      return FALSE;
    }

  /* 
   * read line by line
   */
  offset = 0;
  start = offset;
  char_count = 0;
  line_num = 0;
  while (offset < filesize)
    {
      c = filedata[offset++];
      if (c == '\n')
        {
          if (char_count > 0)
            {
              str = memory_allocation (char_count * sizeof (char) + 1);
              if (str == NULL)
                {
                  LOG_ERR ("not enough memory to allocate 'str'!");
                  return FALSE;
                }
              texts_loaded[line_num] = str;
              while (start < offset)
                {
                  if (filedata[start] > ' ')
                    {
                      *(str++) = filedata[start];
                    }
                  start++;
                }
            }
          line_num++;
          start = offset;
          char_count = 0;
        }
      else
        {
          if (c >= ' ')
            {
              char_count++;
            }
        }
    }
  free_memory (filedata);
  return TRUE;
}
