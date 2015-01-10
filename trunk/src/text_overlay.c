/**
 * @file text_overlay.c
 * @brief Handle text overlay to display abouts, cheats menu and variables  
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: text_overlay.c,v 1.41 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "display.h"
#include "curve_phase.h"
#include "config_file.h"
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "options_panel.h"
#include "grid_phase.h"
#include "guardians.h"
#include "lonely_foes.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#include "meteors_phase.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "shockwave.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "texts.h"
#include "text_overlay.h"

#ifdef USE_SDLMIXER
#ifdef UNDER_DEVELOPMENT
static Uint32 current_sound = 0;
#endif
#endif

typedef enum
{
  ITEM_LEVEL_NUMBER,
  ITEM_GUARDIAN_NUM,
  ITEM_MOST_POWERFULL,
  ITEM_DESTROY_GUARDIAN,
  ITEM_ADD_SATELLITES,
  ITEM_ADD_GUNS,
  ITEM_UPGRADE,
  ITEM_LONELY_FOE,
  ITEM_WHOLE_LONELY_FOES,
  ITEM_MAX_FRONT_SHOT_1,
  MAX_OF_CHEATS_ITEMS
} CHEATS_MENU_ITEMS;

typedef enum
{
  NO_SECTION,
  SECTION_CREDITS,
  SECTION_VAR1,
  SECTION_CHEATS_MENU,
  SECTION_VAR2
} SECTIONS;

static void draw_text (Sint32 xcoord, Sint32 ycoord, const char *string);
static void draw_about (void);
#ifdef UNDER_DEVELOPMENT
static void draw_variables_1 (void);
static void draw_variables_2 (void);
static void draw_cheats_menu (void);
static void cheats_menu_check ();
static void jump_to_level (void);
static void jump_to_guardian (void);
#endif
static Uint32 decode (unsigned char character);

static unsigned char *bitmap_font = NULL;
static Uint32 text_overlay_section = 0;
/** Keyboard list index of the last key down */
static Sint32 last_key_down = 0;
#ifdef UNDER_DEVELOPMENT
static Uint32 key_delay_counter = 0;
/*** Current line selected in the menu */
static Sint32 menu_selection_y = 0;
/** Level number to jump */
static Sint32 cheat_level_num = 0;
/** Guardian number to jump */
static Sint32 cheat_guardian_num = 14;
/** Loney foe number to launch */
static Sint32 cheat_lonely_foe_num = 0;
static bool cheat_button_pressed = FALSE;
#endif

/**
 * Load font file
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
text_overlay_once_init (void)
{
  if (bitmap_font != NULL)
    {
      return TRUE;
    }
  bitmap_font = (unsigned char *) load_pcx_file ("graphics/font_overlay.pcx");
  if (bitmap_font == NULL)
    {
      LOG_ERR ("could not load font file");
      return FALSE;
    }
  return TRUE;
}

/**
 * Release font
 */
void
text_overlay_release (void)
{
  if (bitmap_font == NULL)
    {
      return;
    }
  free_memory ((char *) bitmap_font);
  bitmap_font = NULL;
}

/** 
 * Check if the button that enables or disables the cheat menu is 
 * pressed ([P] key, or mouse button into the panel option)
 * @return TRUE if the button is pressed, or FALSE otherwise
 */
#ifdef UNDER_DEVELOPMENT
static bool
menu_check_cheat_button (void)
{
  bool result;
  bool pressed = keys_down[K_P] || (mouse_b == 1
                                    && mouse_x >
                                    (offscreen_width_visible +
                                     SCORE_MULTIPLIER_XCOORD * pixel_size)
                                    && mouse_y >
                                    (score_offscreen_height +
                                     SCORE_MULTIPLIER_BOTTOM_YCOORD *
                                     pixel_size));

  if (!cheat_button_pressed && pressed)
    {
      result = TRUE;
    }
  else
    {
      result = FALSE;
    }
  cheat_button_pressed = pressed;
  return result;
}
#endif

/**
 * Draw text overlay
 */
void
text_overlay_draw (void)
{
  switch (text_overlay_section)
    {
    case SECTION_CREDITS:
      draw_about ();
      break;

#ifdef UNDER_DEVELOPMENT
    case SECTION_VAR1:
      draw_variables_1 ();
      break;
    case SECTION_CHEATS_MENU:
      draw_cheats_menu ();
      break;
    case SECTION_VAR2:
      draw_variables_2 ();
      break;
#endif
    }

  if (last_key_down != -1)
    {
      if (!keys_down[last_key_down])
        {
          last_key_down = -1;
        }
    }
  else
    {
      /* [Ctrl] key pressed? */
#if !defined(_WIN32_WCE)
      if (keys_down[K_CTRL])
#endif
        {
          if (keys_down[K_A])
            {
              last_key_down = K_A;
              if (text_overlay_section == SECTION_CREDITS)
                {
                  text_overlay_section = NO_SECTION;
                }
              else if (text_overlay_section == NO_SECTION)
                {
                  text_overlay_section = SECTION_CREDITS;
                }
            }
#ifdef UNDER_DEVELOPMENT
          if (keys_down[K_V])
            {
              last_key_down = K_V;
              if (text_overlay_section == SECTION_VAR1)
                {
                  text_overlay_section = NO_SECTION;
                }
              else if (text_overlay_section == NO_SECTION)
                {
                  text_overlay_section = SECTION_VAR1;
                }
            }
          if (keys_down[K_B])
            {
              last_key_down = K_B;
              if (text_overlay_section == SECTION_VAR2)
                {
                  text_overlay_section = NO_SECTION;
                }
              else if (text_overlay_section == NO_SECTION)
                {
                  text_overlay_section = SECTION_VAR2;
                }
            }
          if (menu_status == MENU_OFF && menu_check_cheat_button ())
            {
              last_key_down = K_P;
              if (text_overlay_section == SECTION_CHEATS_MENU)
                {
                  text_overlay_section = NO_SECTION;
                  player_pause = FALSE;
                  is_pause_draw = FALSE;
                }
              else
                {
                  if (text_overlay_section == NO_SECTION)
                    {
                      text_overlay_section = SECTION_CHEATS_MENU;
                      cheat_level_num = num_level;
                    }
                }
            }
#endif
        }
#ifdef UNDER_DEVELOPMENT
#if !defined(_WIN32_WCE)
      else
#endif
        {
          if (keys_down[K_RETURN] || keys_down[K_SPACE])
            {
              switch (text_overlay_section)
                {
                case SECTION_CHEATS_MENU:
                  cheats_menu_check ();
                  break;
                }
            }
        }
#endif
    }
}

/**
 * Check items of the cheats menu
 */
#ifdef UNDER_DEVELOPMENT
static void
cheats_menu_check (void)
{
  Uint32 i;
  bool enabled = FALSE;
  spaceship_struct *ship = spaceship_get ();
  switch (menu_selection_y)
    {
      /* jump to level */
    case ITEM_LEVEL_NUMBER:
      if (cheat_level_num != num_level)
        {
          num_level = cheat_level_num;
          text_level_move (num_level - 1);
          /* text_level->offset_x = 0.0; */
          jump_to_level ();
        }
      enabled = TRUE;
      break;
      /* jump to a guardian */
    case ITEM_GUARDIAN_NUM:
      while (spaceship_upgrading ())
        {
        }
      jump_to_guardian ();
      enabled = TRUE;
      break;
      /* most powefull spaceship */
    case ITEM_MOST_POWERFULL:
      spaceship_most_powerfull ();
      enabled = TRUE;
      break;
      /* destroy guardian */
    case ITEM_DESTROY_GUARDIAN:
      if (guardian->number)
        {
          if (guardian->foe[0]->is_enabled)
            {
              guardian->foe[0]->spr.energy_level = -1;
            }
        }
      enabled = TRUE;
      break;
      /* */
    case ITEM_ADD_SATELLITES:
      satellites_add ();
      enabled = TRUE;
      break;
    case ITEM_ADD_GUNS:
      while (gun_add ())
        {
        }
      enabled = TRUE;
      break;
    case ITEM_UPGRADE:
      spaceship_upgrading ();
      enabled = TRUE;
      break;
    case ITEM_LONELY_FOE:
      enabled = TRUE;
      lonely_foe_add (cheat_lonely_foe_num);
      break;
    case ITEM_WHOLE_LONELY_FOES:
      for (i = 0; i < LONELY_FOES_MAX_OF; i++)
        {
          lonely_foe_add (i);
        }
      enabled = TRUE;
      break;
    case ITEM_MAX_FRONT_SHOT_1:
      ship->shot_front_basic = SPACESHIP_MAX_OPTION_LEVELS;
      enabled = TRUE;
      break;
    }
  if (enabled)
    {
      player_pause = FALSE;
      is_pause_draw = FALSE;
      text_overlay_section = 0;
    }
}
#endif

/**
 * Text of the cheats menu
 */
#ifdef UNDER_DEVELOPMENT
static char cheats_menu_text[] = {
  "################################@"
    "# ] level number       :??     #@"
    "#   go to guardian     :??     #@"
    "#   most powefull spaceship    #@"
    "#   destroy guardian   :??     #@"
    "#   add protection satellites  #@"
    "#   add extra guns             #@"
    "#   upgrade spaceship          #@"
    "#   lonely foe         :??     #@"
    "#   whole lonely foes          #@"
    "#   maximum front shot 1       #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#                              #@"
    "#  > PRESS CTRL-P TO CANCEL <  #@"
    "################################@"
};
#endif

/**
 * Draw cheats menu used under development
 */
#ifdef UNDER_DEVELOPMENT
static void
draw_cheats_menu (void)
{
  Sint32 increase = 0;
  char *text = cheats_menu_text;
  player_pause = TRUE;
  is_pause_draw = TRUE;
  *(text + (33 * (1 + menu_selection_y) + 2)) = ' ';
  if (key_delay_counter < 1)
    {
      if (keys_down[K_DOWN])
        {
          menu_selection_y++;
          key_delay_counter = 8;
        }
      else
        {
          if (keys_down[K_UP])
            {
              menu_selection_y--;
              key_delay_counter = 8;
            }
          else
            {
              if (keys_down[K_LEFT])
                {
                  increase = -1;
                  key_delay_counter = 8;
                }
              else
                {
                  if (keys_down[K_RIGHT])
                    {
                      increase = 1;
                      key_delay_counter = 8;
                    }
                }
            }
        }
    }
  else
    {
      key_delay_counter--;
    }
  if (menu_selection_y < 0)
    {
      menu_selection_y = MAX_OF_CHEATS_ITEMS - 1;
    }
  if (menu_selection_y >= MAX_OF_CHEATS_ITEMS)
    {
      menu_selection_y = 0;
    }
  *(text + (33 * (1 + menu_selection_y) + 2)) = ']';
  if (increase != 0)
    {
      switch (menu_selection_y)
        {
          /* change level number */
        case ITEM_LEVEL_NUMBER:
          cheat_level_num = cheat_level_num + increase;
          if (cheat_level_num < 0)
            {
              cheat_level_num = MAX_NUM_OF_LEVELS;
            }
          if (cheat_level_num > MAX_NUM_OF_LEVELS)
            {
              cheat_level_num = 0;
            }
          break;
          /* change guardian number */
        case ITEM_GUARDIAN_NUM:
          cheat_guardian_num = cheat_guardian_num + increase;
          if (cheat_guardian_num < 1)
            {
              cheat_guardian_num = 14;
            }
          if (cheat_guardian_num > 14)
            {
              cheat_guardian_num = 1;
            }
          break;

        case ITEM_LONELY_FOE:
          cheat_lonely_foe_num += increase;
          if (cheat_lonely_foe_num < 0)
            {
              cheat_lonely_foe_num = LONELY_FOES_MAX_OF - 1;
            }
          if (cheat_lonely_foe_num >= LONELY_FOES_MAX_OF)
            {
              cheat_lonely_foe_num = 0;
            }
          break;
        }
    }
  integer_to_ascii (cheat_level_num, 2, text + (33 * 1) + 24);
  integer_to_ascii (cheat_guardian_num, 2, text + (33 * 2) + 24);
  integer_to_ascii (guardian->number, 2, text + (33 * 4) + 24);
  integer_to_ascii (cheat_lonely_foe_num, 2, text + (33 * 8) + 24);
  draw_text (0, 0, text);
}
#endif

/**
 * About text displaying with a [Ctrl] + [A]
 */
static const char about_texts[] = {
  "********************************@"
    "*                              *@* "
    POWERMANGA_VERSION
    "*@*                LINUX VERSION *@"
    "* (c) 1998-2015 TLK-GAMES      *@"
    "*                              *@"
    "* LINUX.TLK.FR                 *@"
    "*                              *@"
    "* BY:                          *@"
    "* ANDRE MAJOREL                *@"
    "* BRUNO ETHVIGNOT              *@"
    "* DAVID IGREJA                 *@"
    "* EMMANUEL FOUNAUD             *@"
    "* ETIENNE SOBOLE               *@"
    "* GUILLAUME COTTENCEAU         *@"
    "* JEAN MICHEL MARTIN DE SANTERO*@"
    "* LAURENT DUPERVAL             *@"
    "* MICHEL DOGNIAUX              *@"
    "* PATRICE DUHAMEL              *@"
    "* SAM HOCEVAR                  *@"
    "*                              *@"
    "*  > PRESS CTRL-A TO CANCEL <  *@" 
    "********************************@"
};

/**
 * Draw information about all the wonderful and talented contributors to this code
 */
static void
draw_about (void)
{
  draw_text (0, 0, about_texts);
}


/**
 * variables displaying with a [Ctrl] + [V]
 */
#ifdef UNDER_DEVELOPMENT
static char variables_text_1[] = {
  " * * * * * * * * * * * * * * * *@"
    "* num-level:??    etat-menu:?   @"
    "  player-pause:?  aff-pause:?  *@"
    "* game-over-player-one:?        @"
    "  aff-game-over:?  quit-game:? *@"
    "* guardian->is_appearing:?      @"
    "  gardian-activity:??          *@"
    "* meteor-activity:?             @"
    "  grid.is-enable:?             *@"
    "* courbe.activity:?             @"
    "  touch-90-:?                  *@"
    "* nbr-nmis:???                  @"
    "  menu-section:               ?*@"
    "*                  vmode:?      @"
    "  tmp_apparition-vj:           *@"
    "* disparition-vj:               @"
    "  ship->fire_rate:             *@"
    "* mem-numof_zones:?????         @"
    "  mem-total-size:???????? ?????*@"
    "* taille-mem-glob:????????      @"
    "                               *@"
    "*  > PRESS CTRL-V TO CANCEL <   @" " * * * * * * * * * * * * * * * *@"
};
#endif

/**
 * Display some variable values
 */
#ifdef UNDER_DEVELOPMENT
static void
draw_variables_1 (void)
{
  char *str = variables_text_1;
  spaceship_struct *ship = spaceship_get ();
  integer_to_ascii (num_level, 2, str + (33 * 1) + 12);
  integer_to_ascii (menu_status, 1, str + (33 * 1) + 28);
  integer_to_ascii (player_pause, 1, str + (33 * 2) + 15);
  integer_to_ascii (is_pause_draw, 1, str + (33 * 2) + 28);
  integer_to_ascii (spaceship_is_dead, 1, str + (33 * 3) + 23);
  integer_to_ascii (gameover_enable, 1, str + (33 * 4) + 16);
  integer_to_ascii (quit_game, 1, str + (33 * 4) + 29);
  integer_to_ascii (guardian->is_appearing, 1, str + (33 * 5) + 21);
  integer_to_ascii (guardian->number, 2, str + (33 * 6) + 19);
  integer_to_ascii (meteor_activity, 1, str + (33 * 7) + 18);
  integer_to_ascii (grid.is_enable, 1, str + (33 * 8) + 18);
  integer_to_ascii (courbe.activity, 1, str + (33 * 9) + 18);
  integer_to_ascii (keys_down[K_SPACE], 1, str + (33 * 10) + 12);
  integer_to_ascii (num_of_enemies, 3, str + (33 * 11) + 11);
  integer_to_ascii (menu_section, 1, str + (33 * 12) + 30);
  integer_to_ascii (vmode, 1, str + (33 * 13) + 25);
  integer_to_ascii (spaceship_appears_count, 4, str + (33 * 14) + 20);
  integer_to_ascii (spaceship_disappears, 4, str + (33 * 15) + 18);
  integer_to_ascii (ship->fire_rate, 4, str + (33 * 16) + 17);
#if defined (USE_MALLOC_WRAPPER)
  integer_to_ascii (mem_numof_zones, 5, str + (33 * 17) + 17);
  integer_to_ascii (mem_total_size, 8, str + (33 * 18) + 17);
  integer_to_ascii (mem_total_size / 1024, 5, str + (33 * 18) + 26);
#endif
  /* integer_to_ascii (taille_mem_glob, 8, str + (33 * 19) + 18); */
  draw_text (0, 0, str);
}
#endif

#ifdef UNDER_DEVELOPMENT
static char variables_text_2[] = {
  " * * * * * * * * * * * * * * * *@"
    "* test channel is playing       @"
    "  ????????????????             *@"
    "* current-sound:                @"
    "                               *@"
    "*                               @"
    "  sound-samples-len:           *@"
    "* difficulty:                   @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*                               @"
    "                               *@"
    "*  > PRESS CTRL-B TO CANCEL <   @" 
    " * * * * * * * * * * * * * * * *@"
};
#endif

/**
 * Display other variable values and play sound
 */
#ifdef UNDER_DEVELOPMENT
static void
draw_variables_2 (void)
{
#ifdef USE_SDLMIXER
  Uint32 i;
  bool is_sound_played;
  Sint32 offset;
  Sint32 status = 0;
  char *text = variables_text_2;
  char *str = text + (33 * 2) + 2;
  for (i = 0; i < MAX_OF_CHANNELS; i++)
    {
      if (!power_conf->nosound)
        {
          status = Mix_Playing (i);
        }
      integer_to_ascii (status, 1, str);
      str += 1;
    }
  integer_to_ascii (current_sound, 2, text + (33 * 3) + 17);
  integer_to_ascii (sound_samples_len, 8, text + (33 * 6) + 21);
  is_sound_played = FALSE;
  offset = 0;
  if (keys_down[K_SHIFT])
    {
      offset = 10;
    }
  if (keys_down[K_CTRL])
    {
      offset = 20;
    }
  if (keys_down[K_1])
    {
      is_sound_played = TRUE;
      current_sound = 0 + offset;
    }
  if (keys_down[K_2])
    {
      is_sound_played = TRUE;
      current_sound = 1 + offset;
    }
  if (keys_down[K_3])
    {
      is_sound_played = TRUE;
      current_sound = 2 + offset;
    }
  if (keys_down[K_4])
    {
      is_sound_played = TRUE;
      current_sound = 3 + offset;
    }
  if (keys_down[K_5])
    {
      is_sound_played = TRUE;
      current_sound = 4 + offset;
    }
  if (keys_down[K_6])
    {
      is_sound_played = TRUE;
      current_sound = 5 + offset;
    }
  if (keys_down[K_7])
    {
      is_sound_played = TRUE;
      current_sound = 6 + offset;
    }
  if (offset < 20)
    {
      if (keys_down[K_8])
        {
          is_sound_played = TRUE;
          current_sound = 7 + offset;
        }
      if (keys_down[K_9])
        {
          is_sound_played = TRUE;
          current_sound = 8 + offset;
        }
      if (keys_down[K_0])
        {
          is_sound_played = TRUE;
          current_sound = 9 + offset;
        }
    }
  current_sound = current_sound % SOUND_NUMOF;
  if (is_sound_played && !power_conf->nosound)
    {
      sound_play (current_sound);
    }
#else
  char *text = variables_text_2;
#endif
  integer_to_ascii (power_conf->difficulty, 1, text + (33 * 7) + 14);
  draw_text (0, 0, text);
}
#endif

/**
 * Draw text overlay
 * @param xcoord top-left x coordinate of the text
 * @param ycoord top-left y coordinate of the text
 * @param string chars to draw
 */
static void
draw_text (Sint32 xcoord, Sint32 ycoord, const char *string)
{
  Uint32 offset;
  unsigned char *source, *screen, *dest, c;
  screen = (unsigned char *)
    (game_offscreen +
     ((offscreen_starty + ycoord) * offscreen_width +
      offscreen_startx + xcoord) * bytes_per_pixel);
  dest = screen;
  c = *(string++);
  while (c != 0)
    {
      /* next line */
      if (c == '@')
        {
          screen = screen + (8 * offscreen_pitch);
          dest = screen;
        }
      else
        {
          offset = decode (c);
          if (offset != 26 * 8)
            {
              source = bitmap_font + offset * bytes_per_pixel;
              draw_bitmap_char (dest, source);
            }
          dest = dest + (8 * bytes_per_pixel);
        }
      c = *(string++);
    }
}

/**
 * Convert ASCII code to bitmap offset
 * @param ASCII character
 * @return bitmap offset
 */
Uint32
decode (unsigned char character)
{
  if (character >= 'A' && character <= 'Z')
    {
      character = character - 'A';
    }
  else
    {
      if (character >= 'a' && character <= 'z')
        {
          character = character - 'a';
        }
      else
        {
          if (character >= '0' && character <= '9')
            {
              character = character - '0' + 27;
            }
          else
            {
              switch (character)
                {
                case ' ':
                  character = 26;
                  break;
                case '!':
                  character = 37;
                  break;
                case ';':
                  character = 38;
                  break;
                case ':':
                  character = 39;
                  break;
                  /* apostrophe */
                case 39:
                  character = 40;
                  break;
                case '-':
                  character = 41;
                  break;
                case '.':
                  character = 42;
                  break;
                case '?':
                  character = 43;
                  break;
                case '<':
                  character = 44;
                  break;
                case '>':
                  character = 45;
                  break;
                case '&':
                  character = 46;
                  break;
                case '*':
                  character = 47;
                  break;
                case '(':
                  character = 48;
                  break;
                case ')':
                  character = 49;
                  break;
                case '#':
                  character = 50;
                  break;
                case '_':
                  character = 51;
                  break;
                case '=':
                  character = 52;
                  break;
                case '[':
                  character = 53;
                  break;
                case ']':
                  character = 54;
                  break;
                case '|':
                  character = 55;
                  break;
                default:
                  character = 43;
                  break;
                }
            }

        }
    }
  return character * 8;
}

/**
 * Jump to a new level, used under development
 */
#ifdef UNDER_DEVELOPMENT
static void
jump_to_level (void)
{
  if (num_level >= 0 && num_level < 4)
    {
      guardian_load (1);
    }
  if (num_level > 3 && num_level < 8)
    {
      guardian_load (2);
    }
  if (num_level > 7 && num_level < 12)
    {
      guardian_load (3);
    }
  if (num_level > 11 && num_level < 16)
    {
      guardian_load (4);
    }
  if (num_level > 15 && num_level < 20)
    {
      guardian_load (5);
    }
  if (num_level > 19 && num_level < 24)
    {
      guardian_load (6);
    }
  if (num_level > 23 && num_level < 28)
    {
      guardian_load (7);
    }
  if (num_level > 27 && num_level < 32)
    {
      guardian_load (8);
    }
  if (num_level > 31 && num_level < 36)
    {
      guardian_load (9);
    }
  if (num_level > 35 && num_level < 40)
    {
      guardian_load (10);
    }
  if (num_level > 39 && num_level < 42)
    {
      guardian_load (11);
    }
  grid_load (num_level);
  curve_load_level (num_level);
  meteors_load (num_level);
  curve_enable_level ();
  courbe.activity = TRUE;
  grid.is_enable = FALSE;
  meteor_activity = FALSE;
  guardian->number = 0;
}
#endif

/**
 * Jump to a guardian, used under development
 */
#ifdef UNDER_DEVELOPMENT
static void
jump_to_guardian (void)
{
  switch (cheat_guardian_num)
    {
    case 1:
      cheat_level_num = 3;
      break;
    case 2:
      cheat_level_num = 7;
      break;
    case 3:
      cheat_level_num = 11;
      break;
    case 4:
      cheat_level_num = 15;
      break;
    case 5:
      cheat_level_num = 19;
      break;
    case 6:
      cheat_level_num = 23;
      break;
    case 7:
      cheat_level_num = 27;
      break;
    case 8:
      cheat_level_num = 31;
      break;
    case 9:
      cheat_level_num = 35;
      break;
    case 10:
      cheat_level_num = 39;
      break;
    case 11:
      cheat_level_num = 41;
      break;
    case 12:
      cheat_level_num = 41;
      break;
    case 13:
      cheat_level_num = 41;
      break;
    case 14:
      cheat_level_num = 41;
      break;
    }
  num_level = cheat_level_num;
  enemies_init ();
  shots_init ();
  shockwave_init ();
  guardian_load (cheat_guardian_num);
  guardian_new (cheat_guardian_num);
}
#endif
