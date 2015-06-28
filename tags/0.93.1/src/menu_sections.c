/** 
 * @file menu_sections.c 
 * @brief hanlde high score table, about and order menu sections 
 * @created 1998-06-29 
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: menu_sections.c,v 1.42 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "menu.h"
#include "scrolltext.h"
#include "sprites_string.h"
#include "log_recorder.h"
#include "menu_sections.h"
#include "texts.h"
#include "config_file.h"
/** Names of the score filenames follow the difficulty level */
static const char *score_filenames[] =
  { SCOREFILE "-easy", SCOREFILE, SCOREFILE "-hard" };
/** Code of the current menu section */
Uint32 menu_section = NO_SECTION_SELECTED;

/*
 * Order section: display text like Commodore 64
 */
const Uint32 KEYSTROKE_NUM_OF_LINES = 11;
const Uint32 KEYSTOKE_NUM_OF_COLS = 16;
/** Size of the file for keystroke */
const Uint32 KEYSTROKE_SIZE_OF_FILE = 20480;
/** Pause code of keystroke */
const Sint16 KEYSTROKE_PAUSE = 0x4c1;
/** End text code of keystroke */
const Sint16 KEYSTROKE_EOF = 0x4c2;

/*
 * High score table
 */
/** Maximum number of chars for the 'about' section */
#define ABOUT_MAX_OF_CHARS 100
/** X coordinates of the playernames in the high score table */
#define SCORE_TABLE_XCOORD1 128 + 32
/** X coordinates of the scores in the high score table */
#define SCORE_TABLE_XCOORD2 128 + 96
/** Y coordinates of the playernames in the high score table */
#define SCORE_TABLE_YCOORD 128 + 50
/* Vertical space between two lines in the high score table */
#define SCORE_TABLE_VSPACE 20
/** Maximum number of high scores in the high score file */
#define MAX_OF_HIGH_SCORES 5
/** Lenght of the player name */
#define PLAYERNAME_LENGHT 3
/** Time delay of the appareance of high score table before Game Over */
#define SCORE_TABLE_DELAY 400
#define SCORE_TABLE_SIZE MAX_OF_HIGH_SCORES * (PLAYERNAME_LENGHT + 8)
/** Size of the high score file in bytes */
#define SIZE_OF_SCORES_FILE MAX_OF_HIGH_SCORES * (PLAYERNAME_LENGHT + sizeof (Sint32)) + 4

static void destroy (void);
static bool about_load_text (void);
static void about_release_memory (void);
static bool order_load_textdata (void);
static void order_release_data (void);
static void game_over_initialize (Sint32 rank);
static bool high_scores_initialize (void);
static bool about_initialize (void);
static bool order_initialize (void);
static bool high_score_rotate (void);
static void high_score_anim (void);
static Sint32 high_scores_sort (void);
static void high_scores_load (void);
static void high_score_save (void);
static void high_scores_create (void);
static Sint32 generate_byte_checksum (unsigned char *buffer, Uint32 size);
static void high_score_draw (void);
static void gameover_run (void);
static void high_scores_draw (void);
static void about_draw (void);
static void order_run (void);
static void order_clear_screen (void);
static void order_insert_line (Uint32 line_num);
static void order_delete_line (Uint32 line_num);
static bool order_load_data (void);
static void order_save_keystroke (void);
static void order_set_keycode (Uint16 cValeur);
static Uint16 order_get_keycode (void);
static void check_if_enable_menu (void);
/*
 * 'game over' section
 */
/* working table */
static char high_scores_values[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

/* default scores table */
static char high_scores_names[MAX_OF_HIGH_SCORES][4] = {
  {'T', 'L', 'K', 0},
  {'J', 'M', 'M', 0},
  {'E', 'T', 'B', 0},
  {'D', 'A', 'V', 0},
  {'F', 'L', 'O', 0}
};

/* players score */
static Sint32 high_scores_points[MAX_OF_HIGH_SCORES] = { 10, 90, 80, 7, 5 };

/* 0: no action; 1: score input; 6: display scores */
static Uint32 current_phase = 0;
/** List containing a pointer on every char composing high score table */
static sprite_char_struct **scores_chars_sprites = NULL;
/** List of all the strings composing the high score table */
static sprite_string_struct **scores_strings = NULL;
/** String sprites stucture used for the input of the player name */
static sprite_string_struct *playername = NULL;

/*
 * strings of about texts
 */
static char **about_strings_list = NULL;
static char *about_text_data = NULL;
/* index of the animated characters */
static Uint32 scores_anim_current_index = 0;
/* list text index */
static Uint32 about_list_index = 0;
static Uint32 scores_anim_speed_count = 0;
/* delay time during which the score still display */
static Uint32 delay_counter = 0;
static sprite_string_struct *about_string = NULL;

/* 
 * text of order menu
 */
/** List of all the strings composing the high score table */
static sprite_string_struct **order_strings = NULL;
Sint32 order_x_cursor = 0;
Uint32 order_y_cursor = 0;
char *order_text_data = NULL;
/** Command table: containt text and movement cursor */
Uint16 *order_keystroke = NULL;
/** Index on the command table */
Uint32 order_index = 0;
/** Time delay before next command */
Sint16 order_delay_counter = 0;
Uint16 order_last_keycode = 0;
/** Type of order section file  */
bool order_cmd_type = FALSE;

/**
 * Load about text file
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
menu_sections_once_init (void)
{
  return about_load_text ();
}

/**
 * Release memory used in menu sections
 */
void
menu_sections_free (void)
{
  destroy ();
  about_release_memory ();
  order_release_data ();
}

/**
 * About section: load and recopy text data
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
about_load_text (void)
{
  Uint32 filesize, index;
  char *filedata;
  Uint32 chars_index, words_count, strings_index, strings_count;

  /*
   * load file text
   * file format:
   * 0,MAIN, 0,PROGRAMMING, 1,JEAN MICHEL, 1,MARTIN
   * 0,LINUX,0,VERSION,     1,BRUNO,       1,ETHVIGNOT
   */
  filedata = loadfile_with_lang ("texts/about_%s.txt", &filesize);
  if (filedata == NULL)
    {
      return FALSE;
    }

  /*
   * determine total number of strings
   */
  /* counter of number of strings */
  strings_count = 0;
  /* filedata index */
  index = 0;
  /* counter of number of words */
  words_count = 0;
  while (index < filesize)
    {
      switch (filedata[index])
        {
        case ',':
          words_count++;
          break;
        case '\n':
          strings_count += ((words_count + 1) / 2) + 1;
          words_count = 0;
          break;
        default:
          break;
        }
      index++;
    }
  strings_count += 3;

  about_text_data = memory_allocation (filesize);
  if (about_text_data == NULL)
    {
      free_memory (filedata);
      return FALSE;
    }
  about_strings_list =
    (char **) memory_allocation (strings_count * sizeof (char *));
  if (about_strings_list == NULL)
    {
      free_memory (filedata);
      return FALSE;
    }

  /*
   * create the strings
   */
  strings_index = 0;
  words_count = 0;
  index = 0;
  chars_index = 0;
  about_strings_list[strings_index++] = &about_text_data[0];
  while (index < filesize)
    {
      switch (filedata[index])
        {
        case ',':
          if (words_count & 1)
            {
              about_text_data[chars_index++] = '\0';
              about_strings_list[strings_index++] =
                &about_text_data[chars_index];
            }
          words_count++;
          break;

        case '\n':
          about_text_data[chars_index++] = '\0';
          about_strings_list[strings_index++] = &about_text_data[chars_index];
          about_text_data[chars_index++] = '@';
          about_text_data[chars_index++] = '\0';
          about_strings_list[strings_index++] = &about_text_data[chars_index];
          words_count = 0;
          break;

        default:
          if (filedata[index] >= ' ')
            {
              if (words_count & 1)
                {
                  about_text_data[chars_index++] = filedata[index];
                }
              else
                {
                  if (filedata[index] != ' ')
                    {
                      about_text_data[chars_index++] = filedata[index];
                    }
                }
            }
          break;
        }
      index++;
    }

  /* end of text */
  about_strings_list[strings_index++] = &about_text_data[chars_index];
  about_text_data[chars_index++] = '#';
  about_text_data[chars_index++] = '\0';

  /* release file was loaded in memory */
  free_memory (filedata);
  return TRUE;
}

/**
 * About section: Release text data of the 
 */
static void
about_release_memory ()
{
  if (about_strings_list != NULL)
    {
      free_memory ((char *) about_strings_list);
      about_strings_list = NULL;
    }
  if (about_text_data != NULL)
    {
      free_memory (about_text_data);
      about_text_data = NULL;
    }
}

/**
 * Run menu sections
 */
void
menu_sections_run (void)
{
  switch (menu_section)
    {
      /* game over: the player lost his last spaceship */
    case SECTION_GAME_OVER:
      gameover_run ();
      break;

      /* display high score table: ESC key is pressed by the user */
    case SECTION_HIGH_SCORE:
      high_scores_draw ();
      break;

      /* about section: display credits */
    case SECTION_ABOUT:
      about_draw ();
      break;

      /* order section: display license information */
    case SECTION_ORDER:
      order_run ();
      break;
    }
}

/**
 * Enable or disable a menu section
 * @param section_code the section code: SECTION_GAME_OVER, SECTION_ABOUT, ...
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
menu_section_set (Uint32 section_code)
{
  Sint32 rank;
  switch (section_code)
    {
    case NO_SECTION_SELECTED:
      {
        menu_section = NO_SECTION_SELECTED;
        current_phase = 0;
        break;
      }
      /* initialize 'GAME OVER' section */
    case SECTION_GAME_OVER:
      {
        high_scores_load ();
        /* calculations of ranking */
        rank = high_scores_sort ();
        if (rank == -1)
          {
            /* this a non-ranked player */
            delay_counter = SCORE_TABLE_DELAY;
          }
        else
          {
            /* no delay because player enter his name */
            delay_counter = 0;
          }
        if (!high_scores_initialize ())
          {
            return FALSE;
          }
        game_over_initialize (rank);
        menu_section = SECTION_GAME_OVER;
        current_phase = 1;
      }
      break;

      /* initialize the 'SCORE TABLE' section */
    case SECTION_HIGH_SCORE:
      high_scores_load ();
      if (!high_scores_initialize ())
        {
          return FALSE;
        }
      menu_section = SECTION_HIGH_SCORE;
      current_phase = 0;
      break;

      /* initialize the 'ABOUT' section */
    case SECTION_ABOUT:
      about_initialize ();
      menu_section = SECTION_ABOUT;
      current_phase = 0;
      break;

      /* initialize the 'ORDER' section */
    case SECTION_ORDER:
      if (!order_initialize ())
        {
          return FALSE;
        }
      menu_section = SECTION_ORDER;
      break;
    }
  return TRUE;
}

/**
 * Check if the player enter currently his name
 * @return TRUE if the player enter his name 
 */
bool
is_playername_input (void)
{
  if (playername != NULL)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/**
 * Release memory used by the menu sections
 */
static void
destroy (void)
{
  Uint32 i;
  LOG_DBG ("Release memory used by the menu sections");
  if (about_string != NULL)
    {
      sprites_string_delete (about_string);
      about_string = NULL;
    }

  /* release memory used for the high score */
  if (scores_strings != NULL)
    {
      for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
        {
          if (scores_strings[i] == NULL)
            {
              continue;
            }
          sprites_string_delete (scores_strings[i]);
          scores_strings[i] = NULL;
        }
      free_memory ((char *) scores_strings);
    }
  if (scores_chars_sprites != NULL)
    {
      free_memory ((char *) scores_chars_sprites);
      scores_chars_sprites = NULL;
    }

  if (order_text_data != NULL)
    {
      free_memory (order_text_data);
      order_text_data = NULL;
    }
  if (order_strings != NULL)
    {
      for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
        {
          if (order_strings[i] == NULL)
            {
              continue;
            }
          sprites_string_delete (order_strings[i]);
          order_strings[i] = NULL;
        }
      free_memory ((char *) order_strings);
    }
}

/**
 * Game over: initialize the high score table
 * @param rank Rank or -1 if this'is a non-ranked player
 */
static void
game_over_initialize (Sint32 rank)
{
  Uint32 index = 0;
  Uint32 angle = 0;
  Sint32 inc_angle = 1;
  Uint32 xradius = 200;
  Uint32 yradius = 160;
  Uint32 i = 0;

  /* create two sprites strings structures for 'GAME' and 'OVER' */
  text_gameover_init ();

  /* initialize player names */
  playername = NULL;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++, index++)
    {
      sprite_string_init_rotation (scores_strings[index], angle, inc_angle,
                                   xradius, yradius);
      sprite_string_restart_anim (scores_strings[index]);
      if (rank == (Sint32) (i))
        {
          playername = scores_strings[index];
          playername->angle = 0;
          playername->cursor_pos = 0;
        }
      angle = (angle + 1) & MAX_ANGLE;
      inc_angle = -inc_angle;
    }

  /* initialize scores of players */
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++, index++)
    {
      sprite_string_init_rotation (scores_strings[index], angle, inc_angle,
                                   xradius, yradius);
      sprite_string_restart_anim (scores_strings[index]);
      angle = (angle + 1) & MAX_ANGLE;
      inc_angle = -inc_angle;
    }
}

/**
 * High score table: create sprites characters
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
high_scores_initialize ()
{
  char *str;
  Uint32 i, j;
  Sint32 score;
  Sint32 ycoord = SCORE_TABLE_YCOORD;
  Uint32 str_index = 0;
  Uint32 char_index = 0;

  if (scores_chars_sprites == NULL)
    {
      scores_chars_sprites =
        (sprite_char_struct **) memory_allocation (SCORE_TABLE_SIZE *
                                                   sizeof (sprite_char_struct
                                                           **));
      if (scores_chars_sprites == NULL)
        {
          LOG_ERR ("allocation of 'scores_chars_sprites' failed!");
          return FALSE;
        }
    }

  if (scores_strings == NULL)
    {
      scores_strings =
        (sprite_string_struct **) memory_allocation (MAX_OF_HIGH_SCORES * 2 *
                                                     sizeof
                                                     (sprite_string_struct
                                                      **));
      if (scores_strings == NULL)
        {
          LOG_ERR ("allocation of 'scores_strings' failed!");
          return FALSE;
        }
    }

  /* convert the characters of players names strings to sprites */
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++, str_index++)
    {
      str = &high_scores_names[i][0];
      if (scores_strings[str_index] == NULL)
        {
          scores_strings[str_index] =
            sprites_string_create (str, strlen (str), FONT_GAME,
                                   SCORE_TABLE_XCOORD1, (float) ycoord);
          if (scores_strings[str_index] == NULL)
            {
              LOG_ERR ("allocation of 'sprite_string_struct' failed!");
              return FALSE;
            }
        }
      else
        {
          scores_strings[str_index]->coord_x = SCORE_TABLE_XCOORD1;
          scores_strings[str_index]->coord_y = (float) ycoord;
          /* sprite_string_set_char (scores_strings[str_index], str); */
        }

      ycoord += SCORE_TABLE_VSPACE;
    }

  /* convert the characters of players scores strings to sprites */
  ycoord = SCORE_TABLE_YCOORD;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++, str_index++)
    {
      score = high_scores_points[i];
      for (j = 0; j <= 7; j++)
        {
          high_scores_values[j] = '0';
        }
      sprintf (high_scores_values, "%08d", score);
      if (scores_strings[str_index] == NULL)
        {
          scores_strings[str_index] =
            sprites_string_new (high_scores_values, 0, FONT_GAME,
                                SCORE_TABLE_XCOORD2, (float) ycoord);
          if (scores_strings[str_index] == NULL)
            {
              LOG_ERR ("allocation of 'sprite_string_struct' failed!");
              return FALSE;
            }
        }
      else
        {
          scores_strings[str_index]->coord_x = SCORE_TABLE_XCOORD2;
          scores_strings[str_index]->coord_y = (float) ycoord;
          sprite_string_set_char (scores_strings[str_index],
                                  high_scores_values);
        }
      ycoord += SCORE_TABLE_VSPACE;
    }

  /* copy all chars sprites pointer in a list */
  char_index = 0;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++, str_index++)
    {
      for (j = 0; j < scores_strings[i]->num_of_chars; j++)
        {
          scores_chars_sprites[char_index++] =
            &scores_strings[i]->sprites_chars[j];
        }
      for (j = 0; j < scores_strings[i + MAX_OF_HIGH_SCORES]->num_of_chars;
           j++)
        {
          scores_chars_sprites[char_index++] =
            &scores_strings[i + MAX_OF_HIGH_SCORES]->sprites_chars[j];
        }
    }
  return TRUE;
}

/**
 * Rotate chars of the high score strings
 * @return TRUE if [Enter] key being pressed, otherwise FALSE
 */
static bool
high_score_rotate (void)
{
  Uint32 i, keycode;
  bool is_enter_pressed = FALSE;
  bool is_finished = TRUE;
  for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
    {
      if (!sprites_string_rotation_dec (scores_strings[i], 2, 2, 0))
        {
          is_finished = FALSE;
        }
      sprite_string_coords (scores_strings[i]);
    }

  if (playername != NULL && is_finished)
    {
      /* input the playername */
      keycode = sprites_string_input (playername);

      /* "return" or "enter" key is pressed? */
#ifdef POWERMANGA_SDL
      if (keycode == SDLK_RETURN)
#else
      if (keycode == XK_Return)
#endif
        {
          playername = NULL;
          high_score_save ();
          is_enter_pressed = TRUE;
        }
      else
        {
          is_enter_pressed = FALSE;
        }
    }
  high_score_draw ();
  return is_enter_pressed;
}

/**
 * Draw all chars sprites of the high score table
 */
static void
high_score_draw (void)
{
  Uint32 i;
  for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
    {
      sprite_string_draw (scores_strings[i]);
    }
}

/**
 * Sort high scores table
 * @return Rank or -1 if this'is a non-ranked player
 */
static Sint32
high_scores_sort (void)
{
  Uint32 i, j;
  Sint32 rank = -1;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      if (player_score > high_scores_points[i])
        {
          for (j = MAX_OF_HIGH_SCORES - 1; j > i; j--)
            {
              high_scores_points[j] = high_scores_points[j - 1];
              high_scores_names[j][0] = high_scores_names[j - 1][0];
              high_scores_names[j][1] = high_scores_names[j - 1][1];
              high_scores_names[j][2] = high_scores_names[j - 1][2];
            }
          high_scores_points[i] = player_score;
          high_scores_names[i][0] = ' ';
          high_scores_names[i][1] = ' ';
          high_scores_names[i][2] = ' ';
          rank = i;
          break;
        }
    }
  return rank;
}

/** 
 * Load high scores file 
 * @return File data buffer pointer
 */
static char *
high_scores_load_file (void)
{
  size_t fsize;
  FILE *fstream;
  char *filedata;
  const char *filename = score_filenames[power_conf->difficulty];
#if defined(_WIN32_WCE)
  char *pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      LOG_ERR ("can't locate file: %s", filename);
      return FALSE;
    }
  filename = pathname;
#endif

  fstream = fopen (filename, "rb");
#if defined(_WIN32_WCE)
  free_memory (pathname);
#endif
  if (fstream == NULL)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("fopen(%s) failed", filename);
#else
      LOG_ERR ("fopen(%s) return %s", filename, strerror (errno));
#endif
      return NULL;
    }
  fsize = get_file_size (fstream);
  /* fix by Andrey Bogomolov, prevent segmentation
   * fault if file is empty under FreeBSD 6.2! */
  if (fsize == 0)
    {
      LOG_ERR ("%s file is empty!", filename);
      return NULL;
    }
  /* allocate memory for the file data */
  filedata = memory_allocation (fsize);
  if (filedata == NULL)
    {
      LOG_ERR ("no enough memory to allocate %i bytes", (Sint32) fsize);
      return NULL;
    }
  if (fread (filedata, sizeof (char), fsize, fstream) != fsize)
    {
      free_memory (filedata);
#if defined(_WIN32_WCE)
      LOG_ERR ("fread(%s) failed", filename);
#else
      LOG_ERR ("fread(%s) return %s", filename, strerror (errno));
#endif
      return NULL;
    }
  LOG_DBG ("file \"%s\" was loaded in memory", filename);
  fclose (fstream);
  return filedata;
}

/** 
 * Load high scores table
 */
static void
high_scores_load (void)
{
  Uint32 i, j;
  Sint32 sum1, sum2;
  Sint32 *ptr32;
  char *ptr8;
  char *filedata = high_scores_load_file ();

  /* score file could not be loaded */
  if (filedata == NULL)
    {
      high_scores_create ();
      return;
    }

  /* 
   * copy data file into memory structure 
   */
  ptr8 = filedata;
  /* copy players names */
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      for (j = 0; j < PLAYERNAME_LENGHT; j++)
        {
          high_scores_names[i][j] = *(ptr8++);
        }
    }
  /* copy players scores */
  ptr32 = (Sint32 *) ptr8;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      high_scores_points[i] = little_endian_to_int (ptr32++);
    }

  /* file checksum */
  sum2 = little_endian_to_int (ptr32);
  sum1 =
    generate_byte_checksum ((unsigned char *) filedata,
                            SIZE_OF_SCORES_FILE - sizeof (Sint32));

  /* file corrupt: create a new high score table */
  if (sum2 != sum1)
    {
      LOG_ERR ("high score table: bad checksum read:%i != generate:%i "
               "/ read:%x != generate:%x", sum2, sum1, sum2, sum1);
      high_scores_create ();
    }
  free_memory (filedata);
}

/**
 * Create a new high scores table 
 */
static void
high_scores_create (void)
{
  Uint32 i, j;
  Sint32 score = 100;
  char name[PLAYERNAME_LENGHT] = { 'T', 'L', 'K' };

  /* create the names */
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      for (j = 0; j < PLAYERNAME_LENGHT; j++)
        {
          high_scores_names[i][j] = name[j];
        }
    }
  /* create the scores */
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      high_scores_points[i] = score;
      score = score / 2;
    }
}

/**
 * Save high table score file
 */
static void
high_score_save (void)
{
  Sint32 sum;
  Uint32 i, j;
  Sint32 *buffer32;
  char *filedata, *buffer;
#if defined(_WIN32_WCE)
  char *pathname;
#endif
  const char *filename = score_filenames[power_conf->difficulty];

  /* allocate a temporary buffer */
  filedata = memory_allocation (SIZE_OF_SCORES_FILE);
  if (filedata == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i "
               " bytes!", (Uint32) (SIZE_OF_SCORES_FILE));
      return;
    }

  /* crate the filedata */
  buffer = filedata;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      for (j = 0; j < PLAYERNAME_LENGHT; j++)
        {
          *(buffer++) = high_scores_names[i][j];
        }
    }
  buffer32 = (Sint32 *) buffer;
  for (i = 0; i < MAX_OF_HIGH_SCORES; i++)
    {
      int_to_little_endian (high_scores_points[i], buffer32++);
    }
  sum =
    generate_byte_checksum ((unsigned char *) filedata,
                            SIZE_OF_SCORES_FILE - sizeof (Sint32));
  int_to_little_endian (sum, buffer32);

#if defined(_WIN32_WCE)
  pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      LOG_ERR ("can't locate file: %s", filename);
      return;
    }
  filename = pathname;
#endif
  if (!file_write (filename, filedata, SIZE_OF_SCORES_FILE))
    {
      LOG_ERR ("file_write (%s) failed", filename);
    }
  else
    {
      LOG_INF ("\"%s\" file was saved", filename);
    }
#if defined(_WIN32_WCE)
  free_memory (pathname);
#endif
  free_memory (filedata);
  return;
}

/**
 * Generate checksum for the high score file and text order file
 * @param buffer Pointer to the buffer to generate checksum fot
 * @param size Size of the buffer
 * @return The checksum
 */
static Sint32
generate_byte_checksum (unsigned char *buffer, Uint32 size)
{
  Uint32 c;
  Uint32 i;
  Sint32 val = 0;
  for (i = 0; i < size; i++)
    {
      c = *(buffer++);
      val += c;
    }
  return val;
}

/**
 * Game over: appearance of score table, input name, display 'GAME * OVER'
 * @input is_menu_enabled FALSE if main menu is disable, otherwise TRUE
 */
static void
gameover_run (void)
{
  Uint32 i;
  bool is_finished, is_enter_pressed;
  switch (current_phase)
    {
      /* high score table appears from the upper of the screen  */
    case 1:
      {
        is_enter_pressed = high_score_rotate ();
        /* if delay_counter == 0 then player input her name */
        if (delay_counter == 0)
          {
            /* [Enter] key is pressed?  */
            if (is_enter_pressed)
              {
                /* yes, high score table going to disappear */
                current_phase = 2;
              }
          }
        /*  this a non-ranked player,
         *  display the high score table a short moment */
        else
          {
            delay_counter--;
            if (delay_counter == 0
                || (key_code_down != 0
                    && delay_counter < SCORE_TABLE_DELAY - 10))
              {
                current_phase = 2;
              }
          }

      }
      break;

      /* high score table disappears */
    case 2:
      {
        is_finished = TRUE;
        for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
          {
            if (!sprites_string_rotation_inc
                (scores_strings[i], 2, 2, 250, 210))
              {
                is_finished = FALSE;
              }
            sprite_string_coords (scores_strings[i]);
          }
        if (is_finished)
          {
            current_phase = 3;
          }
        high_score_draw ();
      }
      break;

      /* display chars "GAME OVER" */
    case 3:
      {
        /* high_score_draw (); */
        if (text_gameover_draw ())
          {
            check_if_enable_menu ();
            if (menu_status == MENU_ON)
              {
                menu_section = NO_SECTION_SELECTED;
              }
          }
      }
      break;
    }
}

/** 
* High score: animation of each char separately, one by one 
*/
static void
high_score_anim (void)
{
  Uint32 i;
  if (scores_anim_speed_count == 0)
    {
      scores_chars_sprites[scores_anim_current_index++]->is_anim = TRUE;
      if (scores_anim_current_index >= SCORE_TABLE_SIZE)
        {
          scores_anim_current_index = 0;
        }
      scores_anim_speed_count = 5;
    }
  else
    {
      scores_anim_speed_count--;
    }

  for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
    {
      sprite_string_anim (scores_strings[i], FALSE);
    }
}

/**
 * High score: appearance when the player presses [Esc] key
 */
static void
high_scores_draw (void)
{
  bool is_finished;
  Uint32 index = 0;
  Uint32 i;
  Sint32 ycoord;

  switch (current_phase)
    {
      /* initialize the appearance of the characters */
    case 0:
      {
        current_phase = 1;
        scores_anim_speed_count = 0;
        scores_anim_current_index = 0;
        for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
          {
            sprite_string_set_anim (scores_strings[i]);
            sprite_string_init_anim (scores_strings[i], 1, 6, TRUE, 0);
          }
      }
      break;

      /* animate and move char toward the top/bottom of the screen */
    case 1:
      {
        ycoord = SCORE_TABLE_YCOORD;
        /* process strings of playenames */
        for (i = 0; i < MAX_OF_HIGH_SCORES; i++, index++)
          {
            scores_strings[index]->coord_y =
              (float) (ycoord + menu_coord_y - (offscreen_clipsize +
                                                offscreen_height_visible));
            sprite_string_coords (scores_strings[index]);
            sprite_string_anim (scores_strings[index], TRUE);
            sprite_string_draw (scores_strings[index]);
            ycoord += SCORE_TABLE_VSPACE;
          }
        /* process strings of scores */
        ycoord = SCORE_TABLE_YCOORD;
        for (i = 0; i < MAX_OF_HIGH_SCORES; i++, index++)
          {
            scores_strings[index]->coord_y =
              (float) (ycoord + menu_coord_y - (offscreen_clipsize +
                                                offscreen_height_visible));
            sprite_string_coords (scores_strings[index]);
            sprite_string_anim (scores_strings[index], TRUE);
            sprite_string_draw (scores_strings[index]);
            ycoord += SCORE_TABLE_VSPACE;
          }
        /* the main menu is completely hidden? */
        if (menu_coord_y >= offscreen_clipsize + offscreen_height_visible)
          {
            current_phase = 2;
          }
        check_if_enable_menu ();
        if (menu_status == MENU_ON)
          {
            menu_section = NO_SECTION_SELECTED;
          }
      }
      break;

      /* chars arrived at their final positions, end the animation of chars */
    case 2:
      {
        is_finished = TRUE;
        for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
          {
            sprite_string_anim (scores_strings[i], FALSE);
            sprite_string_draw (scores_strings[i]);
            if (!scores_strings[index]->none_char_animated)
              {
                is_finished = FALSE;
              }
          }
        if (is_finished)
          {
            current_phase = 3;
          }

        if (menu_coord_y < offscreen_clipsize + offscreen_height_visible)
          {
            current_phase = 1;
          }
        check_if_enable_menu ();
      }
      break;

      /* the characters are static, animation of each char separately */
    case 3:
      {
        high_score_anim ();
        high_score_draw ();
        if (menu_coord_y < offscreen_clipsize + offscreen_height_visible)
          {
            /* anim and moves char toward the top of the screen */
            current_phase = 1;
            for (i = 0; i < MAX_OF_HIGH_SCORES * 2; i++)
              {
                sprite_string_set_anim (scores_strings[i]);
              }
          }
        check_if_enable_menu ();
      }
      break;
    }
}

/**
 * About section: initialize, create structure of the sprites string 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
about_initialize (void)
{
  about_list_index = 0;
  if (about_string == NULL)
    {
      /* create structure for 'about' sprites string */
      about_string =
        sprites_string_new (NULL, ABOUT_MAX_OF_CHARS, FONT_GAME, 0, 0);
    }
  if (about_string == NULL)
    {
      LOG_ERR ("creation of 'about' sprites string failed");
      return FALSE;
    }
  sprite_string_init_anim (about_string, 1, 6, TRUE, 0);
  about_string->delay_next_char = 26;
  return TRUE;
}

/**
 * About section: draw and move chars
 */
static void
about_draw (void)
{
  Uint32 i, font_type;
  Sint32 ycoord, xcoord;
  char c;
  bool is_finished;
  sprite_char_struct *sprite_char;
  char *str;
  Sint32 angle_inc;

  switch (current_phase)
    {
      /* initialize all chars */
    case 0:
      {
        ycoord = 158;
        angle_inc = 1;
        about_string->num_of_chars = 0;
        str = about_strings_list[about_list_index++];
        /* last string? */
        if (*(str) == '#')
          {
            /* text restart */
            str = about_strings_list[0];
            about_list_index = 1;
          }

        while (*(str) != '@')
          {
            xcoord = 160;
            if (about_string->num_of_chars >= ABOUT_MAX_OF_CHARS)
              {
                break;
              }
            /* first char get font type used */
            c = *(str++);
            if (c == '0')
              {
                font_type = FONT_GAME;
              }
            else
              {
                font_type = FONT_BIG;
              }

            if (font_type == FONT_GAME)
              {
                xcoord = 128 + (256 - ((strlen (str)) * 16)) / 2 - 8;
              }
            else
              {
                xcoord = 128 + (256 - ((strlen (str)) * 22)) / 2 - 11;
              }

            c = *(str++);
            while (c != 0)
              {
                if (about_string->num_of_chars >= ABOUT_MAX_OF_CHARS)
                  {
                    break;
                  }
                sprite_char =
                  &about_string->sprites_chars[about_string->num_of_chars++];
                sprite_char_initialize (sprite_char, c, xcoord, ycoord,
                                        font_type);
                sprite_char_init_rotate (sprite_char, 0, angle_inc, 200, 200);
                if (angle_inc > 0)
                  {
                    angle_inc = -angle_inc;
                  }
                else
                  {
                    angle_inc = -angle_inc;
                    angle_inc++;
                    if (angle_inc > 2)
                      {
                        angle_inc = 1;
                      }
                  }
                c = *(str++);
                if (font_type == FONT_GAME)
                  {
                    xcoord += 16;
                  }
                else
                  {
                    xcoord += 22;
                  }
              }
            ycoord += 30;
            str = about_strings_list[about_list_index++];
          }

        current_phase = 1;
        sprite_string_set_anim (about_string);
      }
      break;

      /* rotation and appearance of all chars */
    case 1:
      {
        is_finished = TRUE;
        for (i = 0; i < about_string->num_of_chars; i++)
          {
            sprite_char = &about_string->sprites_chars[i];
            if (!sprite_char_rotate (sprite_char, 1, 1, 0))
              {
                is_finished = FALSE;
              }
            sprite_char_anim (sprite_char, TRUE);
          }
        sprite_string_draw (about_string);
        if (is_finished)
          {
            current_phase = 2;
          }
      }
      break;

      /* chars arrived at their final positions, end the animation of chars */
    case 2:
      {
        sprite_string_anim (about_string, FALSE);
        if (about_string->none_char_animated)
          {
            /* begin to animate first char in the next phase  */
            about_string->current_char = 0;
            current_phase = 3;
            delay_counter = 300;
          }
        sprite_string_draw (about_string);
      }
      break;

      /* the characters are static, animation of each char separately */
    case 3:
      {
        /* animation of each char separately */
        sprite_string_anim_one (about_string);
        sprite_string_draw (about_string);
        delay_counter--;
        if (delay_counter <= 0 || menu_status == MENU_ON ||
            menu_coord_y < offscreen_clipsize + offscreen_height_visible)
          {
            current_phase = 4;
          }
        check_if_enable_menu ();
      }
      break;

      /* end the animation of chars */
    case 4:
      {
        sprite_string_anim (about_string, FALSE);
        sprite_string_draw (about_string);
        if (about_string->none_char_animated)
          {
            sprite_string_set_anim (about_string);
            current_phase = 5;
          }
      }
      break;

      /* rotation and disappearance of the chars */
    case 5:
      {
        is_finished = TRUE;
        for (i = 0; i < about_string->num_of_chars; i++)
          {
            sprite_char = &about_string->sprites_chars[i];
            if (!sprite_char_rotate_enlarge (sprite_char, 3, 3, 300, 300))
              {
                is_finished = FALSE;
              }
            sprite_char_anim (sprite_char, TRUE);
          }
        sprite_string_draw (about_string);
        if (is_finished)
          {
            current_phase = 0;
            if (menu_status == MENU_ON
                || menu_coord_y <
                offscreen_clipsize + offscreen_height_visible)
              {
                menu_section = NO_SECTION_SELECTED;
              }
          }
      }
      break;
    }
  check_if_enable_menu ();
}

/**
 * Order section: allocate memory and initialize sprites structures
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
order_initialize (void)
{
  char *str;
  Sint32 ycoord;
  Uint32 i;
  if (order_strings == NULL)
    {
      order_strings =
        (sprite_string_struct **) memory_allocation (KEYSTROKE_NUM_OF_LINES *
                                                     sizeof
                                                     (sprite_string_struct
                                                      **));
      if (order_strings == NULL)
        {
          LOG_ERR ("allocation of 'order_strings' failed!");
          return FALSE;
        }
    }

  /* allocate buffer of chars */
  if (order_text_data == NULL)
    {
      order_text_data =
        memory_allocation (KEYSTROKE_NUM_OF_LINES * KEYSTOKE_NUM_OF_COLS);
      if (order_text_data == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes!",
                   KEYSTROKE_NUM_OF_LINES * KEYSTOKE_NUM_OF_COLS);
          return FALSE;
        }
      str = order_text_data;
      for (i = 0; i < KEYSTROKE_NUM_OF_LINES * KEYSTOKE_NUM_OF_COLS; i++)
        {
          str[i] = ' ';
        }
    }
  ycoord = 128 + 7;

  str = order_text_data;
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      if (order_strings[i] == NULL)
        {
          order_strings[i] =
            sprites_string_create (str, KEYSTOKE_NUM_OF_COLS, FONT_GAME,
                                   128, (float) ycoord);
          if (order_strings[i] == NULL)
            {
              LOG_ERR ("allocation of 'sprite_string_struct' failed!");
              return FALSE;
            }
          sprite_string_coords (order_strings[i]);
        }
      ycoord += 16;
      str += KEYSTOKE_NUM_OF_COLS;
    }
  order_x_cursor = 0;
  order_y_cursor = 0;
  if (!order_load_data ())
    {
      LOG_ERR ("order_load_data() failed!");
      /* return FALSE; */
    }
  if (!order_cmd_type)
    {
      order_index = 0;
    }
  else
    {
      /* skip checksum */
      order_index = 2;
    }
  order_delay_counter = 0;
  order_last_keycode = 0;
  order_clear_screen ();
  return TRUE;
}

/**
 * Order section: display text Commodore-64 like 
 */
static void
order_run (void)
{
  Uint32 i, ycoord;
  sprite_string_struct *sprite_str;
  Sint32 keycode = 0;
  Sint32 cursor_status = 0;

  ycoord = menu_coord_y - offscreen_height_visible + 7;
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      order_strings[i]->coord_y = (float) ycoord;
      sprite_string_coords (order_strings[i]);
      ycoord += 16;
    }

  /* string in which the cursor is */
  sprite_str = order_strings[order_y_cursor];

  switch (current_phase)
    {
      /*
       * input a new text
       */
    case 0:
      {
        /* keycode entered by user */
        keycode = sprites_string_input (sprite_str);
        order_x_cursor = sprite_str->cursor_pos;
        cursor_status = sprite_str->cursor_status;
        if (keycode == 0)
          {
            order_delay_counter++;
            if (order_delay_counter >= 32767)
              {
                /* set a wait code */
                order_set_keycode (KEYSTROKE_PAUSE);
                /* set time delay */
                order_set_keycode (order_delay_counter);
                order_delay_counter = 0;
              }
          }
        else
          {
            if (order_delay_counter > 0)
              {
                /* set a wait code */
                order_set_keycode (KEYSTROKE_PAUSE);
                /* set time delay from 1 to 32767 */
                order_set_keycode (order_delay_counter);
                order_delay_counter = 0;
              }
            order_set_keycode ((Sint16) keycode);
          }
        if (menu_status == MENU_ON || menu_status == MENU_UP)
          {
            menu_section = NO_SECTION_SELECTED;
            /* set end of file */
            order_set_keycode (KEYSTROKE_EOF);
            order_save_keystroke ();
          }
      }
      break;

      /*
       * display text from the file was loaded in memory
       */
    case 1:
      {
        keycode = 0;
        if (--order_delay_counter <= 0)
          {
            Uint16 _valeur = order_get_keycode ();
            if (_valeur == KEYSTROKE_PAUSE)
              {
                if (order_cmd_type)
                  {
                    order_delay_counter = order_get_keycode ();
                    order_delay_counter = order_delay_counter / 3;
                  }
                else
                  {
                    if (order_last_keycode == (Uint16) ' ')
                      {
                        /* fast spaces */
                        order_delay_counter = 10;
                      }
                    else
                      {
                        order_delay_counter = (Sint16) (30 + rand () % 30);
                        if ((rand () % 100) < 25)
                          {
                            order_delay_counter += 20;
                          }
                        order_delay_counter = order_delay_counter / 3;
                      }
                  }
              }
            else
              {
                /* force unsigned */
                keycode = _valeur;
                order_last_keycode = _valeur;
#ifdef POWERMANGA_SDL
                switch (keycode)
                  {
                    /* XK_BackSpace */
                  case 0xff08:
                    keycode = SDLK_BACKSPACE;
                    break;
                    /* XK_Delete */
                  case 0xffff:
                    keycode = SDLK_DELETE;
                    break;
                    /* XK_Right */
                  case 0xff53:
                    keycode = SDLK_RIGHT;
                    break;
                    /* XK_Left */
                  case 0xff51:
                    keycode = SDLK_LEFT;
                    break;
                    /* XK_Up */
                  case 0xff52:
                    keycode = SDLK_UP;
                    break;
                    /* XK_Down */
                  case 0xff54:
                    keycode = SDLK_DOWN;
                    break;
                    /* XK_Return */
                  case 0xff0d:
                    keycode = SDLK_RETURN;
                    break;
                    /* XK_F3 */
                  case 0xffc0:
                    keycode = SDLK_F3;
                    break;
                    /* XK_F4 */
                  case 0xffc1:
                    keycode = SDLK_F4;
                    break;
                    /* XK_F5 */
                  case 0xffc2:
                    keycode = SDLK_F5;
                    break;
                    /* XK_Shift_L */
                  case 0xffe1:
                    keycode = SDLK_LSHIFT;
                    break;
                    /* XK_Shift_R */
                  case 0xffe2:
                    keycode = SDLK_RSHIFT;
                    break;
                  }
#endif
              }
          }
        sprites_string_input_code (sprite_str, keycode);
        order_x_cursor = sprite_str->cursor_pos;
        cursor_status = sprite_str->cursor_status;
        if (menu_status == MENU_ON || menu_status == MENU_UP)
          {
            current_phase = 2;
          }
        check_if_enable_menu ();
      }
      break;

      /* menu up */
    case 2:
      {
        keycode = 0;
        cursor_status = 0;
        if (menu_status == MENU_ON)
          {
            menu_section = NO_SECTION_SELECTED;
          }
      }
      break;
    }

  switch (keycode)
    {
#ifdef POWERMANGA_SDL
    case SDLK_DOWN:
#else
    case XK_Down:
#endif
      if (++order_y_cursor >= KEYSTROKE_NUM_OF_LINES)
        {
          order_y_cursor = KEYSTROKE_NUM_OF_LINES - 1;
          order_delete_line (0);
        }
      break;
#ifdef POWERMANGA_SDL
    case SDLK_UP:
#else
    case XK_Up:
#endif
      if (order_y_cursor == 0)
        {
          order_insert_line (order_y_cursor);
        }
      else
        {
          order_y_cursor--;
        }
      break;
#ifdef POWERMANGA_SDL
    case SDLK_INSERT:
#else
    case XK_Insert:
#endif
      order_insert_line (order_y_cursor);
      break;
#ifdef POWERMANGA_SDL
    case SDLK_RETURN:
#else
    case XK_Return:
#endif
      if (++order_y_cursor >= KEYSTROKE_NUM_OF_LINES)
        {
          order_y_cursor = KEYSTROKE_NUM_OF_LINES - 1;
          order_delete_line (0);
        }
      order_x_cursor = 0;
      break;
      /* [F3]  moves the cursor to top-left screen corner */
#ifdef POWERMANGA_SDL
    case SDLK_F3:
#else
    case XK_F3:
#endif
      order_x_cursor = 0;
      order_y_cursor = 0;
      break;
      /* [F4] clear screen */
#ifdef POWERMANGA_SDL
    case SDLK_F4:
#else
    case XK_F4:
#endif
      order_clear_screen ();
      break;
      /* [F5] erase the current linge */
#ifdef POWERMANGA_SDL
    case SDLK_F5:
#else
    case XK_F5:
#endif
      order_delete_line (order_y_cursor);
      break;
    }

  /* check cursor override */
  switch (cursor_status)
    {
      /* left override */
    case 1:
      if (order_y_cursor == 0)
        {
          order_insert_line (order_y_cursor);
        }
      else
        {
          order_y_cursor--;
        }
      order_x_cursor = KEYSTOKE_NUM_OF_COLS - 1;
      break;
      /* right override */
    case 2:
      if (++order_y_cursor >= KEYSTROKE_NUM_OF_LINES)
        {
          order_y_cursor = KEYSTROKE_NUM_OF_LINES - 1;
          order_delete_line (0);
          order_x_cursor = 0;
        }
      else
        order_x_cursor = 0;
      break;
    }
  sprites_string_set_cursor_pos (order_strings[order_y_cursor],
                                 order_x_cursor);

  /* draw and animation of all chars */
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      sprite_string_draw (order_strings[i]);
      sprite_string_anim (order_strings[i], FALSE);
    }
}

/**
 * Order section: clear all screen
 */
static void
order_clear_screen (void)
{
  char *c;
  Uint32 i;
  order_x_cursor = 0;
  order_y_cursor = 0;
  c = order_text_data;
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES * KEYSTOKE_NUM_OF_COLS; i++)
    {
      *(c++) = ' ';
    }
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      sprite_chars_to_image (order_strings[i]);
    }
}

/**
 * Order section: insert a line of text
 * @param line_num Line number from 0 to 11
 */
static void
order_insert_line (Uint32 line_num)
{
  Uint32 i;
  char *dest =
    order_text_data + (KEYSTOKE_NUM_OF_COLS * KEYSTROKE_NUM_OF_LINES) - 1;
  char *src = dest - KEYSTOKE_NUM_OF_COLS;
  for (i = 0;
       i < (KEYSTROKE_NUM_OF_LINES - 1 - line_num) * KEYSTOKE_NUM_OF_COLS;
       i++)
    {
      *(dest--) = *(src--);
    }
  src++;
  for (i = 0; i < KEYSTOKE_NUM_OF_COLS; i++)
    {
      *(src++) = ' ';
    }
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      sprite_chars_to_image (order_strings[i]);
    }
}

/**
 * Order section: remove  a line of text
 * @param line_num Line number from 0 to 10
 */
static void
order_delete_line (Uint32 line_num)
{
  Uint32 i;
  char *dest = order_text_data + (line_num * KEYSTOKE_NUM_OF_COLS);
  char *src = dest + KEYSTOKE_NUM_OF_COLS;
  for (i = 0;
       i < (KEYSTROKE_NUM_OF_LINES - 1 - line_num) * KEYSTOKE_NUM_OF_COLS;
       i++)
    {
      *(dest++) = *(src++);
    }
  for (i = 0; i < KEYSTOKE_NUM_OF_COLS; i++)
    {
      *(--src) = ' ';
    }
  for (i = 0; i < KEYSTROKE_NUM_OF_LINES; i++)
    {
      sprite_chars_to_image (order_strings[i]);
    }
}

/**
 * Order display: load command table file
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
order_load_data (void)
{
  Sint32 *ptr32;
  Uint32 i;
  Sint32 sum1, sum2;
  if (order_keystroke == NULL)
    {
      order_keystroke = (Uint16 *) memory_allocation (KEYSTROKE_SIZE_OF_FILE);
      if (order_keystroke == NULL)
        {
          return FALSE;
        }
    }

  /* try load keystroke recording file */
  if (loadfile_into_buffer
      ("data/menu_order_keystrokes.bin", (char *) order_keystroke))
    {
      /* checksum: integrity of data */
      ptr32 = (Sint32 *) order_keystroke;
      sum1 = little_endian_to_int (ptr32);
      sum2 =
        generate_byte_checksum ((unsigned char *) (order_keystroke) +
                                sizeof (Sint32),
                                KEYSTROKE_SIZE_OF_FILE - sizeof (Sint32));
      if (sum2 != sum1)
        {
          LOG_ERR ("bad checksum read:%i != generate:%i"
                   " / read:%x != generate:%x", sum1, sum2, sum1, sum2);
          /* input text */
          current_phase = 0;
        }
      else
        {
          /* display text */
          current_phase = 1;
        }
      order_cmd_type = TRUE;
      return TRUE;
    }

  order_release_data ();

  /* try a load plain text file */
  if (order_load_textdata ())
    {
      /* display text */
      current_phase = 1;
      /* keystroke table is disable */
      order_cmd_type = FALSE;
      return TRUE;
    }

  order_release_data ();
  order_keystroke = (Uint16 *) memory_allocation (KEYSTROKE_SIZE_OF_FILE);
  if (order_keystroke == NULL)
    {
      return FALSE;
    }
  for (i = 0; i < KEYSTROKE_SIZE_OF_FILE / 2; i++)
    {
      order_keystroke[i] = 0;
    }
  /* input text */
  current_phase = 0;
  order_cmd_type = TRUE;
  return TRUE;
}

/**
 * Order section: load plain text data for linear text display
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
order_load_textdata ()
{
  Uint32 i, filesize, index;
  char *filedata = loadfile_with_lang ("texts/order_%s.txt", &filesize);
  if (filedata == NULL)
    {
      return FALSE;
    }
  order_keystroke =
    (Uint16 *) memory_allocation (filesize * 2 * sizeof (Uint16));
  if (order_keystroke == NULL)
    {
      return FALSE;
    }
  memset (order_keystroke, 0, filesize * 2 * sizeof (Uint16));
  index = 0;
  for (i = 0; i < filesize; i++)
    {
      if (filedata[i] == '"')
        {
          continue;
        }
      if ((filedata[i] == 10) || (filedata[i] >= ' '))
        {
          order_keystroke[index++] = (unsigned char) filedata[i];
          order_keystroke[index++] = KEYSTROKE_PAUSE;

        }
    }
  free_memory (filedata);
  return TRUE;
}

/**
 * Order section: release memory used for order text
 */
static void
order_release_data ()
{
  if (order_keystroke != NULL)
    {
      free_memory ((char *) order_keystroke);
      order_keystroke = NULL;
    }
}

/**
 * Order section: save keystroke file
 */
static void
order_save_keystroke (void)
{
#if defined(_WIN32_WCE)
  /* FIXME to implement */
  LOG_ERR ("not implemented");
#else
  Sint32 *ptr32;
  Sint32 filehandle;
  Uint32 size = 0;
  Sint32 sum =
    generate_byte_checksum ((unsigned char *) (order_keystroke) +
                            sizeof (Sint32),
                            KEYSTROKE_SIZE_OF_FILE - sizeof (Sint32));
  LOG_INF ("checksum: %i", sum);
  ptr32 = (Sint32 *) order_keystroke;
  int_to_little_endian (sum, ptr32);
  filehandle =
    open ("data/menu_order_keystrokes.bin", O_WRONLY | O_CREAT, 0666);
  if (filehandle == -1)
    {
      LOG_ERR ("can't open file : \"menu_order_keystrokes.bin\"");
    }
  else
    {
      size = write (filehandle, order_keystroke, KEYSTROKE_SIZE_OF_FILE);
      if (size != KEYSTROKE_SIZE_OF_FILE)
        {
          LOG_ERR ("write() return: %s", strerror (errno));
        }
      close (filehandle);
    }
#endif
}

/**
 * Order srction: set keycode value
 * @return a keycode
 */
static void
order_set_keycode (Uint16 value)
{
  if (order_index < KEYSTROKE_SIZE_OF_FILE - 1)
    {
      order_keystroke[order_index++] = value;
    }
}

/**
 * Order section: read keycode value
 * @return a keycode
 */
static Uint16
order_get_keycode (void)
{
  Uint16 val;
  Uint16 val2;
  if (!order_cmd_type)
    {
      val = (Uint16) order_keystroke[order_index++];
      if (order_index >= KEYSTROKE_SIZE_OF_FILE || val == 0)
        {
          order_clear_screen ();
          order_index = 2;
          LOG_DBG ("end of file 'menu_order_keystrokes.bin'");
        }
    }
  else
    {
      val =
        (Uint16) little_endian_to_short ((Sint16 *) &
                                         order_keystroke[order_index++]);
      val2 =
        (Uint16) little_endian_to_short ((Sint16 *) &
                                         order_keystroke[order_index]);
      if (order_index >= KEYSTROKE_SIZE_OF_FILE || val == 0
          || val2 == KEYSTROKE_EOF)
        {
          order_clear_screen ();
          order_index = 2;
          LOG_DBG ("end of file 'menu_order_keystrokes.bin'");
        }
    }
  return val;
}

/**
 * Check if the player want to return to main menu
 */
static void
check_if_enable_menu (void)
{
  if ((key_code_down != 0 || fire_button_down) && menu_status == MENU_OFF)
    {
      /* enable main menu */
      menu_status = MENU_UP;
      /* enable scroll text */
      scrolltext_init ();
    }
}

/*
 F3:  curseur en haut a gauche
 F4:  curseur en haut a gauche & efface l'ecran
 F5:  supprime la ligne en cours
.THIS..PROGRAM..
IS FREE SOFTWARE
....YOU..CAN.... 
..REDISTRIBUTE..
...IT..AND/OR...
...MODIFY..IT...
...UNDER..THE...
TERMS OF THE GNU
.GENERAL PUBLIC.
...LICENSE AS...
..PUBLISHED.BY..
....THE FREE....
....SOFTWARE....
...FOUNDATION...
EITHER VERSION 3
.OF.THE LICENSE.
.......OR.......
(AT.YOUR.OPTION)
...ANY..LATER...
................
SEE THE GNU.....
GENERAL PUBLIC..
LICENSE FOR MORE
DETAILS.........
................
..(c)1999-2008..
...TLK..GAMES...
................
..Greetings.to..
..Alex.Senesse..
.Alexis.Sukrieh.
.Andre..Majorel.
Aurelien.Anselme
.David..Alardet.
..David.Igreja..
...Delphine.....
Didier..Florentz
Emeric.Pourcelot
.Etienne.Sobole.
.Gael.Roualland.
.Gaelle.Richard.
.Gautier.Portet.
....Hafida.....
J-Michel..Martin
Karine.Philippon
.Laurent..Cance.
Laurent.Guibelin
.Laurent..Tenza.
..Marie-Jeanne..
.Nicolas.Arnaud.
Olivier..Simonin
Patrice..Duhamel
Pascal.Ethvignot
..Pascal.Lauly..
.Pierre.Olivier.
.Roderic.Moitie.
Sandrine..Berger
.Sophie.Bibanov.
..Sylvain.Cote..
..Yoan.Daubeze..

*/
