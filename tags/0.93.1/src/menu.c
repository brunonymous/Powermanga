/**
 * @file menu.c
 * @brief Handle the main menu of Powermanga
 * @date 2014-10-12 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: menu.c,v 1.35 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "congratulations.h"
#include "curve_phase.h"
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "bonus.h"
#include "energy_gauge.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "grid_phase.h"
#include "guardians.h"
#include "log_recorder.h"
#include "lonely_foes.h"
#include "menu.h"
#include "menu_sections.h"
#include "meteors_phase.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "scrolltext.h"
#include "shockwave.h"
#include "sdl_mixer.h"
#include "spaceship.h"
#include "texts.h"

extern Sint32 tlk_logo_xcoord;
extern Sint32 tlk_logo_ycoord;
extern bool tlk_logo_is_move;

/** Status of the menu MENU_OFF(0)=menu disable; MENU_ON(1)=menu enable */
Sint32 menu_status;
/** Y coordinates of the top menu items */
Sint32 menu_coord_y = 0;
/** Height of the text sprite of the menu */
static const Sint32 MENU_ITEM_HIGH = 41;
/** Number of images of the option menu */
#define MENU_NUMOF_IMAGES 32
/** Y coordinate of the menu */
#define MENU_TOP_COORD 136
static bool menu_up_pressed = FALSE;
static bool menu_down_pressed = FALSE;
typedef enum
{
  MENU_NO_ITEM = -1,
  MENU_PLAY_ITEM = 0,
  MENU_ORDER_ITEM,
  MENU_ABOUT_ITEM,
  MENU_QUIT_ITEM,
  /** Number of menu options */
  MENU_ITEMS_NUMOF
} MENU_SELECTED;

static MENU_SELECTED menu_item_pos = MENU_PLAY_ITEM;

static Sint32 menu_images_index[MENU_ITEMS_NUMOF];
static bitmap menu_spr[MENU_ITEMS_NUMOF][MENU_NUMOF_IMAGES];
static void draw_menu (Sint32 pos_y, MENU_SELECTED menu_item_pos);
static void init_new_game (void);

/**
 * Performs this menu initialization once, only at startup
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
menu_once_init (void)
{
  Sint32 i;

  /* extract menu animations bitmap (452,598 bytes) */
  if (!bitmap_load
      ("graphics/bitmap/main_menu.spr", &menu_spr[0][0], MENU_ITEMS_NUMOF,
       MENU_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* set menu y coordintate, at the bottom of the screen */
  menu_coord_y = offscreen_clipsize + offscreen_height_visible;

  /* main menu enable */
  menu_status = MENU_UP;

  for (i = 0; i < MENU_ITEMS_NUMOF; i++)
    {
      menu_images_index[i] = 0;
    }
  return TRUE;
}

/**
 * Convert menu items from data bitmaps to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
menu_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/menu/menu-x/menu-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/menu"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < MENU_ITEMS_NUMOF; type++)
    {
      sprintf (filename, EXPORT_DIR "/menu/menu-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < MENU_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/menu/menu-%01d/menu-%02d.png",
                   type + 1, frame);
          if (!bitmap_to_png
              (&menu_spr[type][frame], filename, 192, 31, offscreen_pitch))
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
 * Release memory used for the menu aminations
 */
void
menu_free (void)
{
  LOG_DBG ("deallocates the memory used by the bitmap");
  bitmap_free (&menu_spr[0][0], MENU_ITEMS_NUMOF, MENU_NUMOF_IMAGES, 32);
}

/**
 * Check if the mouse button is pressed on the menu
 * @return
 */
static MENU_SELECTED
menu_check_mouse_button (void)
{
  Sint32 i;
  Sint32 item_pos = MENU_NO_ITEM;
  Sint32 posy = MENU_TOP_COORD - OFFSCREEN_STARTY + SCORES_HEIGHT;
  if (mouse_b == 0)
    {
      return item_pos;
    }
  if (mouse_x < 64 || mouse_x > 191)
    for (i = 0; i < MENU_ITEMS_NUMOF; i++)
      {
        if (mouse_y > posy && mouse_y <= posy + MENU_ITEM_HIGH)
          {
            item_pos = i;
            break;
          }
        posy += MENU_ITEM_HIGH;
      }
  return item_pos;
}

/**
 * Handles selection of menu
 * @param item_num
 */
static void
menu_item_selected (MENU_SELECTED item_num)
{
  switch (item_num)
    {
      /*
       * start a new game
       */
    case MENU_PLAY_ITEM:
#if defined(POWERMANGA_SDL) && defined(USE_SDL_JOYSTICK)
      display_open_joysticks ();
#endif
      /* reset player parameters */
      init_new_game ();
      /* disable main menu */
      menu_status = MENU_DOWN;
      /* disable scroll text */
      scrolltext_disable (SCROLL_PRESENT);
      menu_section_set (NO_SECTION_SELECTED);
#ifdef USE_SDLMIXER
      /* start music of the game */
      sound_music_play (MUSIC_GAME);
#endif
      break;

      /*
       * start order section (display GPL and greetings)
       */
    case MENU_ORDER_ITEM:
#ifdef SHAREWARE_VERSION
      /* draw the third page order */
      switch (power_conf->lang)
        {
        case FR_LANG:
          show_page_order (2, "fr", 0);
          break;
        case EN_LANG:
          show_page_order (2, "en", 0);
          break;
        default:
          show_page_order (2, "en", 0);
          break;
        }

      if (keys_down[K_RETURN])
        {
          keys_down[K_RETURN] = FALSE;
        }
      if (keys_down[K_SPACE])
        {
          keys_down[K_SPACE] = FALSE;
        }
      if (fire_button_down)
        {
          fire_button_down = FALSE;
        }

#else
      /* initialize order section */
      menu_section_set (SECTION_ORDER);
      menu_status = MENU_DOWN;
      /* disable scroll text */
      scrolltext_disable (SCROLL_PRESENT);
#endif
      break;

      /* start about section */
    case MENU_ABOUT_ITEM:
      /* initialize about section */
      menu_section_set (SECTION_ABOUT);
      /* disable main menu */
      menu_status = MENU_DOWN;
      /* disable scroll text */
      scrolltext_disable (SCROLL_PRESENT);
      break;

      /* quit Powermanga */
    case MENU_QUIT_ITEM:
      quit_game = TRUE;
      break;

    default:
      break;

    }
}

/**
 * Main menu's handle
 */
static void
menu_run (void)
{
  MENU_SELECTED item_num;
  /*
   * appearance and disappearance of main menu
   */
  /* menu appearing? */
  if (menu_status == MENU_UP)
    {
      if (menu_coord_y > MENU_TOP_COORD)
        {
          /* moving menu to top */
          menu_coord_y -= 2 * pixel_size;
        }
    }
  /* menu disappearing? */
  if (menu_status == MENU_DOWN)
    {
      if (menu_coord_y < offscreen_clipsize + offscreen_height_visible)
        {
          /* moving menu to bottom */
          menu_coord_y += 2 * pixel_size;
        }
    }
  /* menu take her final position? */
  if (menu_coord_y <= MENU_TOP_COORD)
    {
      /* enable main menu */
      menu_status = MENU_ON;
    }
  /* menu not visible on the screen? */
  if (menu_coord_y >= offscreen_clipsize + offscreen_height_visible)
    {
      /* disable main menu */
      menu_status = MENU_OFF;
    }

  /* menu is enable? */
  if (menu_status == MENU_ON)
    {
      /* main navigation menu option */
      /* up cursor key? */
      if ((keys_down[K_UP] || joy_top) && !menu_up_pressed)
        {
          /* previous option */
          menu_item_pos--;
          if (menu_item_pos < 0)
            {
              menu_item_pos = MENU_ITEMS_NUMOF - 1;
            }
        }
      menu_up_pressed = (keys_down[K_UP] | joy_top);
      /* down cursor key? */
      if ((keys_down[K_DOWN] || joy_down) && !menu_down_pressed)
        {
          /* next option */
          menu_item_pos++;
          if (menu_item_pos >= MENU_ITEMS_NUMOF)
            {
              menu_item_pos = 0;
            }
        }
      menu_down_pressed = (keys_down[K_DOWN] | joy_down);

      /* moving "TLK Games" logo sprite */
      /* "TLK Games" logo appearing? */
      if (!tlk_logo_is_move && (rand () % 2500) == 500)
        {
          if (rand () % 2)
            {
              tlk_logo_xcoord = 120;
            }
          else
            {
              tlk_logo_xcoord = 310;
            }
          tlk_logo_ycoord = 328;
          tlk_logo_is_move = TRUE;
        }

      /*
       * space bar, enter or fire button pressed
       */
      item_num = menu_check_mouse_button ();
      if (item_num != MENU_NO_ITEM)
        {
          menu_item_selected (item_num);
        }

      if (keys_down[K_RETURN] || keys_down[K_SPACE] || fire_button_down)
        {
          menu_item_selected (menu_item_pos);
        }

      /*
       * ESC key or start button pressed
       */
      if (menu_check_button ())
        {
          if (gameover_enable)
            {
              /* initialize high score table */
              menu_section_set (SECTION_HIGH_SCORE);
            }
          menu_status = MENU_DOWN;
          /* disable scroll text */
          scrolltext_disable (SCROLL_PRESENT);
        }
    }

  /* draw menu on the screen */
  draw_menu (menu_coord_y, menu_item_pos);
}

/**
 * Anim and draw the bitmap menu
 * @params pos_y ordinate of the menu (min = 136, max = 312)
 * @params item_pos number of the selected option
 */
static void
draw_menu (Sint32 pos_y, MENU_SELECTED item_pos)
{
  MENU_SELECTED i;
  for (i = MENU_PLAY_ITEM; i < MENU_ITEMS_NUMOF; i++)
    {
      if (i == item_pos)
        {
          menu_images_index[i] =
            (menu_images_index[i] + 1) % MENU_NUMOF_IMAGES;
        }
      else if (menu_images_index[i] != 0)
        {
          menu_images_index[i] =
            (menu_images_index[i] + 1) % MENU_NUMOF_IMAGES;
        }
      /* draw a sprite */
      if ((pos_y + i * MENU_ITEM_HIGH) >=
          (offscreen_clipsize - MENU_ITEM_HIGH)
          && (pos_y + i * MENU_ITEM_HIGH) <
          (offscreen_clipsize + offscreen_height_visible))
        {
          draw_bitmap (&menu_spr[i][menu_images_index[i]],
                       offscreen_clipsize + 32, pos_y + (MENU_ITEM_HIGH * i));
        }
    }
}

/**
 * Initialize values before beging the game
 */
static void
init_new_game (void)
{
  LOG_DBG ("Initialize values before beging the game");
  /* initialize random generator */
  srand (1);
  texts_init ();
  energy_gauge_init ();
  /* close all options boxes (except spaceship repair)
   * on the right options panel */
  options_open_all (OPTION_PANEL_REPAIR);
  /* initialize spaceship's structure */
  spaceship_initialize ();
  /* clear score multiplier */
  score_multiplier = 0;
  spaceship_set_invincibility (SPACESHIP_INVINCIBILITY_TIME);
  enemies_init ();
  shots_init ();
  /* remove protection satellites */
  satellites_init ();
  /* remove extra guns */
  guns_init ();
  shockwave_init ();
  meteor_activity = TRUE;
  num_of_meteors = 1000;
  courbe.activity = FALSE;
  grid.is_enable = FALSE;
  guardian->number = 0;
  guardian->is_appearing = FALSE;
  spaceship_is_dead = FALSE;
  gameover_enable = FALSE;
  /* level number (-1 start a new game, else 0 to 41) */
  num_level = -1;
  /* refresh spaceship's energy gauge */
  energy_gauge_spaceship_is_update = TRUE;
  /* refresh guardian's energy gauge  */
  energy_gauge_guard_is_update = TRUE;
  lonely_foes_init ();
  option_change = TRUE;
  bonus_init ();
  spaceship_show ();
  starfield_speed = 2.0;
  /* disable congratulations */
  is_congratulations_enabled = FALSE;
}


/**
 * Handle the main menu of Powermanga
 */
void
menu_handle (void)
{
  if (menu_status != MENU_OFF)
    {
      menu_run ();
    }
  else
    {
      /* [ESC] key or joystick's start button? */
      if (menu_check_button ())
        {
          menu_status = MENU_UP;
          scrolltext_init ();
        }
    }
}

/** 
 * Check if the button that enables or disables the main menu is 
 * pressed ([Esc] key, option button, or mouse button into the
 * score panel
 * @return TRUE if the button is pressed, or FALSE otherwise
 */
bool
menu_check_button (void)
{
  if (keys_down[K_ESCAPE] || start_button_down ||
      (mouse_b == 1 && mouse_y < score_offscreen_height))
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}
