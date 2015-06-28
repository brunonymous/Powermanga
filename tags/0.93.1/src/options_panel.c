/** 
 * @file options_panel.c 
 * @brief Handle right options panel 
 * @date 2011-02-26 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: options_panel.c,v 1.29 2012/06/03 17:06:15 gurumeditation Exp $
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
#include "config_file.h"
#include "tools.h"
#include "images.h"
#include "display.h"
#include "electrical_shock.h"
#include "enemies.h"
#include "energy_gauge.h"
#include "shots.h"
#include "extra_gun.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "options_panel.h"
#include "satellite_protections.h"
#include "sdl_mixer.h"
#include "spaceship.h"

/** Maximum number of images that can be combined in an option box */
#define OPTION_BOX_MAX_IMAGES 33
/** The numbers of different images multipliers scores */
typedef enum
{
  MULTIPLIER_CLEAR,
  MULTIPLIER_X2_RED,
  MULTIPLIER_X2_YELLOW,
  MULTIPLIER_X4_RED,
  MULTIPLIER_X4_YELLOW,
  MULTIPLIERS_NUM_OF_IMAGES
} MULTIPLIERS_ENUM;
/** Images data of the scores multiplier */
static bitmap multiplier_bmp[MULTIPLIERS_NUM_OF_IMAGES];
/** Structures data of the state of the options box */
option option_boxes[OPTIONS_MAX_OF_TYPES];
/** Images data of the options box */
static bitmap options[OPTIONS_MAX_OF_TYPES][OPTION_BOX_MAX_IMAGES];
option_change_coord *options_refresh = NULL;
/** Score are multiplied by 2 or 4 */
Uint32 score_multiplier = 0;
Sint32 opt_refresh_index = -1;
/** Selected option has changed */
bool option_change = FALSE;
/** Tempo display for option selected */
static Sint32 option_cursor_delay_count = 0;
static Sint32 option_selected_pos = 0;
/** Option box (28 width x 28 height) coordinates into the left panel */
static Uint32 options_positions[OPTIONS_PANEL_NUMOF][2] = {
  {260 - 256, 169 - 16},
  {290 - 256, 154 - 16},
  {260 - 256, 139 - 16},
  {290 - 256, 124 - 16},
  {260 - 256, 109 - 16},
  {290 - 256, 94 - 16},
  {260 - 256, 79 - 16},
  {290 - 256, 64 - 16},
  {260 - 256, 49 - 16},
  {290 - 256, 34 - 16},
  {260 - 256, 19 - 16}
};

typedef struct options_collision_coord
{
  Sint32 x;
  Sint32 y;
} options_collision_coord;
static options_collision_coord *options_collision = NULL;
static bool options_load (void);
static void option_selected_cursor (Sint32);
static void option_close_playanim (Sint32);
static void option_open_playanim (Sint32);
static void option_box_animation (Sint32);
static void option_clear (Sint32);
static Uint32 score_multiplier_clear = 2;
/** [Crtl] key or option button pressed */
static bool option_button_pressed = FALSE;
bool score_x2_refresh = FALSE;
bool score_x4_refresh = FALSE;
/** Option number to clear */
static Sint32 old_option;
static Sint32 cmpt_vbls_x2, cmpt_vbls_x4, aff_x2_rj, aff_x4_rj;

/**
 * Initialize options panel structure
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
options_once_init (void)
{
  Sint32 i;
  options_free ();

  if (!options_load ())
    {
      return FALSE;
    }

  /* allocate fontes elements */
  if (options_refresh == NULL)
    {
      options_refresh =
        (option_change_coord *) memory_allocation (88 *
                                                   sizeof
                                                   (option_change_coord));
      if (options_refresh == NULL)
        {
          LOG_ERR ("not enough memory to alloc %i bytes!",
                   sizeof (option_change_coord));
          return FALSE;
        }
    }
  for (i = 0; i < OPTIONS_MAX_OF_TYPES; i++)
    {
      /* delay between two images */
      option_boxes[i].next_image_pause = 1000;
      option_boxes[i].next_image_pause_offset = 550;
      option_boxes[i].current_image = 0;
      option_boxes[i].next_image_pause_cnt = 0;
    }

  /* generates the table to test the collision of the mouse or stylus */
  if (options_collision == NULL)
    {
      options_collision =
        (options_collision_coord *) memory_allocation (OPTIONS_PANEL_NUMOF *
                                                       sizeof
                                                       (options_collision_coord));
      if (options_collision == NULL)
        {
          LOG_ERR ("not enough memory to alloc %i bytes!",
                   sizeof (options_collision_coord));
          return FALSE;
        }
      for (i = 0; i < OPTIONS_PANEL_NUMOF; i++)
        {
          options_collision[i].x =
            options_positions[i][0] + offscreen_width_visible;
          options_collision[i].y =
            options_positions[i][1] + score_offscreen_height;
        }
    }
  options_close_all ();
  return TRUE;
}

/**
 * Load options boxes and scores multiplier animations
 * scanline width which has the origin for a screen width of 320 pixels
 * 13 options boxes animations of 33 images each
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
options_load (void)
{
  Sint32 i, j;
  Uint32 offset;
  Uint32 size;
  Uint32 *repeats;
  bitmap *bmp;

  /* extract box options animations  bitmap images (387,761 bytes) */
  if (!bitmap_load ("graphics/bitmap/options_panel_anims.spr",
                    &options[0][0], OPTIONS_MAX_OF_TYPES,
                    OPTION_BOX_MAX_IMAGES))
    {
      return FALSE;
    }

  /* extract score multipliers bitmap images (760 bytes) */
  if (!bitmap_load
      ("graphics/bitmap/scores_multiplier.spr", &multiplier_bmp[0], 1,
       MULTIPLIERS_NUM_OF_IMAGES))
    {
      return FALSE;
    }

  if (power_conf->extract_to_png)
    {
      return TRUE;
    }

  /* modifies the display offsets of box animations */
  for (i = 0; i < OPTIONS_MAX_OF_TYPES; i++)
    {
      for (j = 0; j < OPTION_BOX_MAX_IMAGES; j++)
        {
          bmp = &options[i][j];
          repeats = (Uint32 *) bmp->compress;
          size = bmp->nbr_data_comp >> 2;
          do
            {
              offset = *repeats;
              if (offset > (26 * bytes_per_pixel))
                {
                  /* original width screen's 320 pixels */
                  offset -= (320 * bytes_per_pixel);
                  /* actual width screen's 64 pixels */
                  offset += (OPTIONS_WIDTH * bytes_per_pixel);
                }
              *repeats = offset;
              repeats += 2;
            }
          while (--size > 0);
        }
    }

  /* modifies the display offsets of score multipliers */
  for (j = 0; j < MULTIPLIERS_NUM_OF_IMAGES; j++)
    {
      bmp = &multiplier_bmp[j];
      repeats = (Uint32 *) bmp->compress;
      size = bmp->nbr_data_comp >> 2;
      do
        {
          offset = *repeats;
          if (offset > (16 * bytes_per_pixel))
            {
              /* original width screen's 320 pixels */
              offset -= (320 * bytes_per_pixel);
              /* actual width screen's 64 pixels */
              offset += (OPTIONS_WIDTH * bytes_per_pixel);
            }
          *repeats = offset;
          repeats += 2;
        }
      while (--size > 0);
    }
  return TRUE;
}

/**
 * Convert menu items from data bitmaps to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
options_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/options/option-xx/option-xx.png";
  //const char *model = EXPORT_DIR "/multipliers/multiplier-x.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/options"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < OPTIONS_MAX_OF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/options/option-%02d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < OPTION_BOX_MAX_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/options/option-%02d/option-%02d.png",
                   type + 1, frame);
          if (!bitmap_to_png (&options[type][frame], filename, 28, 28, 320))
            {
              free_memory (filename);
              return FALSE;
            }
        }
    }
  if (!create_dir (EXPORT_DIR "/multipliers"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (frame = 0; frame < MULTIPLIERS_NUM_OF_IMAGES; frame++)
    {
      sprintf (filename, EXPORT_DIR "/multipliers/multiplier-%1d", frame);
      if (!bitmap_to_png (&multiplier_bmp[frame], filename, 14, 8, 320))
        {
          free_memory (filename);
          return FALSE;
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif

/** 
 * Free some options panel data
 */
void
options_free (void)
{
  bitmap_free (&options[0][0], OPTIONS_MAX_OF_TYPES, OPTION_BOX_MAX_IMAGES,
               OPTION_BOX_MAX_IMAGES);
  bitmap_free (&multiplier_bmp[0], 1, 5, 5);
  if (options_refresh != NULL)
    {
      free_memory ((char *) options_refresh);
      options_refresh = NULL;
    }
  if (options_collision != NULL)
    {
      free_memory ((char *) options_collision);
      options_collision = NULL;
    }
}

/** 
 * Handle options box on the left panel 
 */
void
option_execution (void)
{
  Sint32 i;
  spaceship_struct *ship = spaceship_get ();

  /* promotes up of one or two level in the range of the available options */
  if (option_change)
    {

      /* restore the old option box background */
      option_clear (old_option);

      /* out of the options number? */
      if (ship->gems_count > 11)
        {
          /* clear the number of bonus collected */
          ship->gems_count = 0;
          if (!gun_add ())
            {
              /* cannot install more than 2 guns */
            }
        }
      /* change the option box selected */
      option_selected_pos = ship->gems_count;
      option_change = FALSE;
    }

  /* 
   * play the animations of opening or closing of a box options 
   */
  for (i = 0; i < 11; i++)
    {
      if (option_boxes[i].anim_close)
        {
          option_close_playanim (i);
        }
      if (option_boxes[i].anim_open)
        {
          option_open_playanim (i);
        }
    }

  /*
   * display the option box animation, when it's selected
   */
  switch (ship->gems_count)
    {

      /* increase the speed of the spaceship option box */
    case 1:
      /* check that no animation of opening or closing is in progress */
      if (!option_boxes[0].anim_close && !option_boxes[0].anim_open)
        {
          if (ship->speed_booster < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* spaceship reach max speed: close option box */
              option_clear (0);
            }
        }
      /* display cursor on option box selected */
      option_selected_cursor (0);
      old_option = ship->gems_count - 1;
      break;

      /* spaceship repair option box  */
    case 2:
      if (!option_boxes[1].anim_close && !option_boxes[1].anim_open)
        {
          if (ship->spr.energy_level != ship->spr.pow_of_dest)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* maximum energy level: close option box */
              option_clear (1);
            }
        }
      option_selected_cursor (1);
      old_option = ship->gems_count - 1;
      break;

      /* basic front fire shot option box */
    case 3:
      if (!option_boxes[2].anim_close && !option_boxes[2].anim_open)
        {
          if (ship->shot_front_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max of level of the basic front fire shot: close option box */
              option_clear (2);
            }
        }
      option_selected_cursor (2);
      old_option = ship->gems_count - 1;
      break;

      /* enhanced front fire shot option box */
    case 4:
      if (!option_boxes[3].anim_close && !option_boxes[3].anim_open)
        {
          if (ship->shot_front_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max of level of the enhanced front fire shot: close option */
              option_clear (3);
            }
        }
      option_selected_cursor (3);
      old_option = ship->gems_count - 1;
      break;

      /* basic lateral left fire shot option box */
    case 5:
      if (!option_boxes[4].anim_close && !option_boxes[4].anim_open)
        {
          if (ship->shot_left_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max of level of basic lateral left fire shot: close option */
              option_clear (4);
            }
        }
      option_selected_cursor (4);
      old_option = ship->gems_count - 1;
      break;

      /* enhanced lateral left fire shot enhanced option box */
    case 6:
      if (!option_boxes[5].anim_close && !option_boxes[5].anim_open)
        {
          if (ship->shot_left_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max level of enhanced lateral left fire shot: close option */
              option_clear (5);
            }
        }
      option_selected_cursor (5);
      old_option = ship->gems_count - 1;
      break;

      /* basic lateral right fire shot option box */
    case 7:
      if (!option_boxes[6].anim_close && !option_boxes[6].anim_open)
        {
          if (ship->shot_right_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max level of basic latetal right fire shot: close option */
              option_clear (6);
            }
        }
      option_selected_cursor (6);
      old_option = ship->gems_count - 1;
      break;

      /* enhanced lateral right fire shot option box */
    case 8:
      if (!option_boxes[7].anim_close && !option_boxes[7].anim_open)
        {
          if (ship->shot_right_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max level of enhanced latetal right fire shot: close option */
              option_clear (7);
            }
        }
      option_selected_cursor (7);
      old_option = ship->gems_count - 1;
      break;

      /* basic rear fire shot option box */
    case 9:
      if (!option_boxes[8].anim_close && !option_boxes[8].anim_open)
        {
          if (ship->shot_rear_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max level of basic rear fire shot: close option */
              option_clear (8);
            }
        }
      option_selected_cursor (8);
      old_option = ship->gems_count - 1;
      break;

      /* enhanced rear fire shot option box */
    case 10:
      if (!option_boxes[9].anim_close && !option_boxes[9].anim_open)
        {
          if (ship->shot_rear_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              /* max level of enhanced rear fire shot: close option */
              option_clear (9);
            }
        }
      option_selected_cursor (9);
      old_option = ship->gems_count - 1;
      break;

      /* get a new spaceship option box */
    case 11:
      if (!option_boxes[10].anim_close && !option_boxes[10].anim_open)
        {
          if (ship->type < (SPACESHIP_NUM_OF_TYPES - 1))
            {
              option_box_animation (ship->gems_count - 1);
            }
          else
            {
              option_clear (10);
            }
        }
      option_selected_cursor (10);
      old_option = ship->gems_count - 1;
      break;
    }
}

/** 
 * Check if an option is selected by the player 
 */
void
options_check_selected (void)
{
  spaceship_struct *ship = spaceship_get ();
  bool leftb = FALSE;

  /* detects the collision of the stylus or the mouse 
   * in the options panel */
  if (option_selected_pos > 0 && mouse_b == 1)
    {
      if (mouse_x >= options_collision[option_selected_pos - 1].x &&
          mouse_x < options_collision[option_selected_pos - 1].x + OPTION_SIZE
          && mouse_y >= options_collision[option_selected_pos - 1].y
          && mouse_y <
          options_collision[option_selected_pos - 1].y + OPTION_SIZE)
        {
          leftb = TRUE;
        }
    }

  if ((keys_down[K_CTRL] || option_button_down || leftb)
      && option_selected_pos > 0 && !option_button_pressed)


    {
      switch (option_selected_pos)
        {
          /* increase the speed of the spaceship option seleted */
        case 1:
          if (ship->speed_booster < SPACESHIP_MAX_OPTION_LEVELS)
            {
              /* increase the speed of the spaceship */
              ship->speed_booster++;
              ship->gems_count = 0;
              option_change = TRUE;
              /* if spaceship reach max speed then close option box */
              if (ship->speed_booster >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_INC_SPEED, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* spaceship repair option selected */
        case 2:
          if (ship->spr.energy_level != ship->spr.pow_of_dest)
            {
              /* increase energy level */
              ship->spr.energy_level += 20;
              if (ship->spr.energy_level > ship->spr.pow_of_dest)
                {
                  ship->spr.energy_level = ship->spr.pow_of_dest;
                }
              energy_gauge_spaceship_is_update = TRUE;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->spr.energy_level == ship->spr.pow_of_dest)
                {
                  option_anim_init (OPTION_PANEL_REPAIR, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* basic front fire shot option selected */
        case 3:
          if (ship->shot_front_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_front_basic++;
              ship->gems_count = 0;
              option_change = TRUE;
              /* if max of level of the basic front fire shot close option */
              if (ship->shot_front_basic >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_FRONT_FIRE1, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* enhanced front fire shot option selected */
        case 4:
          if (ship->shot_front_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_front_enhanced++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_front_enhanced >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_FRONT_FIRE2, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* basic lateral left fire shot option selected */
        case 5:
          if (ship->shot_left_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_left_basic++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_left_basic >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_LEFT_FIRE1, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* enhanced lateral left fire shot enhanced option selected */
        case 6:
          if (ship->shot_left_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              ship->shot_left_enhanced++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_left_enhanced >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_LEFT_FIRE2, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* basic lateral right fire shot option selected */
        case 7:
          if (ship->shot_right_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              /* increase basic lateral right fire shot */
              ship->shot_right_basic++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_right_basic >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_RIGHT_FIRE1, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* enhanced lateral right fire shot option selected */
        case 8:
          if (ship->shot_right_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              /* increase enhanced lateral right fire shot */
              ship->shot_right_enhanced++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_right_enhanced >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_RIGHT_FIRE2, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* basic rear fire shot option selected */
        case 9:
          if (ship->shot_rear_basic < SPACESHIP_MAX_OPTION_LEVELS)
            {
              /* increase basic rear fire shot */
              ship->shot_rear_basic++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_rear_basic >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_REAR_FIRE1, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* enhanced rear fire shot option selected */
        case 10:
          if (ship->shot_rear_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
            {
              /* increase enhanced rear fire shot */
              ship->shot_rear_enhanced++;
              ship->gems_count = 0;
              option_change = TRUE;
              if (ship->shot_rear_enhanced >= SPACESHIP_MAX_OPTION_LEVELS)
                {
                  option_anim_init (OPTION_PANEL_REAR_FIRE2, TRUE);
                }
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;

          /* new spaceship option selected */
        case 11:
          if (spaceship_upgrading ())
            {

              /* 
               * downgrade all options by one 
               */
              /* decrease the speed */
              if (option_boxes[OPTION_PANEL_INC_SPEED].close_option)
                {
                  /* open option box if closed */
                  option_anim_init (OPTION_PANEL_INC_SPEED, FALSE);
                }
              ship->speed_booster--;
              if (ship->speed_booster < 1)
                {
                  ship->speed_booster = 1;
                }
              /* decrease basic front fire shot */
              if (option_boxes[OPTION_PANEL_FRONT_FIRE1].close_option)
                {
                  option_anim_init (OPTION_PANEL_FRONT_FIRE1, FALSE);
                }
              ship->shot_front_basic--;
              if (ship->shot_front_basic < 1)
                {
                  ship->shot_front_basic = 1;
                }
              /* decrease enhanced front fire shot */
              if (option_boxes[OPTION_PANEL_FRONT_FIRE2].close_option)
                {
                  option_anim_init (OPTION_PANEL_FRONT_FIRE2, FALSE);
                }
              ship->shot_front_enhanced--;
              if (ship->shot_front_enhanced < 0)
                {
                  ship->shot_front_enhanced = 0;
                }
              /* decrease basic lateral left fire shot */
              if (option_boxes[OPTION_PANEL_LEFT_FIRE1].close_option)
                {
                  option_anim_init (OPTION_PANEL_LEFT_FIRE1, FALSE);
                }
              ship->shot_left_basic--;
              if (ship->shot_left_basic < 0)
                {
                  ship->shot_left_basic = 0;
                }
              /* decrease enhanced lateral left fire shot */
              if (option_boxes[OPTION_PANEL_LEFT_FIRE2].close_option)
                {
                  option_anim_init (OPTION_PANEL_LEFT_FIRE2, FALSE);
                }
              ship->shot_left_enhanced--;
              if (ship->shot_left_enhanced < 0)
                {
                  ship->shot_left_enhanced = 0;
                }
              /* decrease basic lateral right fire shot */
              if (option_boxes[OPTION_PANEL_RIGHT_FIRE1].close_option)
                {
                  option_anim_init (OPTION_PANEL_RIGHT_FIRE1, FALSE);
                }
              ship->shot_right_basic--;
              if (ship->shot_right_basic < 0)
                {
                  ship->shot_right_basic = 0;
                }
              /* decrease enhanced lateral right fire shot */
              if (option_boxes[OPTION_PANEL_RIGHT_FIRE2].close_option)
                {
                  option_anim_init (OPTION_PANEL_RIGHT_FIRE2, FALSE);
                }
              ship->shot_right_enhanced--;
              if (ship->shot_right_enhanced < 0)
                {
                  ship->shot_right_enhanced = 0;
                }
              /* decrease basic rear fire shot */
              if (option_boxes[OPTION_PANEL_REAR_FIRE1].close_option)
                {
                  option_anim_init (OPTION_PANEL_REAR_FIRE1, FALSE);
                }
              ship->shot_rear_basic--;
              if (ship->shot_rear_basic < 0)
                {
                  ship->shot_rear_basic = 0;
                }
              /* decrease enhanced rear fire shot */
              if (option_boxes[OPTION_PANEL_REAR_FIRE2].close_option)
                {
                  option_anim_init (OPTION_PANEL_REAR_FIRE2, FALSE);
                }
              ship->shot_rear_enhanced--;
              if (ship->shot_rear_enhanced < 0)
                {
                  ship->shot_rear_enhanced = 0;
                }
              score_multiplier = 0;
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_OPTION);
#endif
            }
          else
            {
#ifdef USE_SDLMIXER
              sound_play (SOUND_SELECT_CLOSED_OPTION);
#endif
            }
          break;
        }
    }
  option_button_pressed = (keys_down[K_CTRL] | option_button_down);

  /* 
   * display or clear score multiplier 
   * */
  switch (score_multiplier)
    {
    case 0:
      if (score_multiplier_clear == 2)
        {
          /* clear X4 score multiplier */
          draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_CLEAR],
                                  SCORE_MULTIPLIER_XCOORD,
                                  SCORE_MULTIPLIER_TROP_YCOORD);
          score_x4_refresh = TRUE;
          score_multiplier_clear--;
        }
      if (score_multiplier_clear == 1)
        {
          /* clear X2 score multiplier */
          draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_CLEAR],
                                  SCORE_MULTIPLIER_XCOORD,
                                  SCORE_MULTIPLIER_BOTTOM_YCOORD);
          score_x2_refresh = TRUE;
          score_multiplier_clear--;
        }
      break;

      /* display X4 score multiplier */
    case 2:
      if (!(cmpt_vbls_x4 & 15))
        {
          score_x4_refresh = TRUE;
          /* display in red */
          if (aff_x4_rj & 1)
            {
              draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_X4_RED],
                                      SCORE_MULTIPLIER_XCOORD,
                                      SCORE_MULTIPLIER_TROP_YCOORD);
            }
          /* display in yellow */
          else
            {
              draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_X4_YELLOW],
                                      SCORE_MULTIPLIER_XCOORD,
                                      SCORE_MULTIPLIER_TROP_YCOORD);
            }
          aff_x4_rj++;
        }
      cmpt_vbls_x4++;

      /* display X2 score multiplier */
    case 1:
      if (!(cmpt_vbls_x2 & 15))
        {
          score_x2_refresh = TRUE;
          /* display in red */
          if (aff_x2_rj & 1)
            {
              draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_X2_RED],
                                      SCORE_MULTIPLIER_XCOORD,
                                      SCORE_MULTIPLIER_BOTTOM_YCOORD);
            }
          /* display in yellow */
          else
            {
              draw_bitmap_in_options (&multiplier_bmp[MULTIPLIER_X2_YELLOW],
                                      SCORE_MULTIPLIER_XCOORD,
                                      SCORE_MULTIPLIER_BOTTOM_YCOORD);
            }
          aff_x2_rj++;
          /* clear X4 score multiplier */
          if (score_multiplier == 1)
            {
              draw_bitmap_in_options (&multiplier_bmp[0],
                                      SCORE_MULTIPLIER_XCOORD,
                                      SCORE_MULTIPLIER_TROP_YCOORD);
            }
        }
      cmpt_vbls_x2++;
      break;
    }
  score_multiplier_clear = score_multiplier;
}

/** 
 * Display option box animation, when it's selected 
 * @param num_option Option number from 0 to 11
 */
static void
option_box_animation (Sint32 num_option)
{
  Sint32 coord_x;
  Sint32 coord_y;

  bitmap *img;
  option_boxes[num_option].next_image_pause_cnt =
    (Sint16) (option_boxes[num_option].next_image_pause_cnt +
              option_boxes[num_option].next_image_pause_offset);

  if (option_boxes[num_option].next_image_pause_cnt >=
      option_boxes[num_option].next_image_pause)
    {
      option_boxes[num_option].next_image_pause_cnt =
        (option_boxes[num_option].next_image_pause_cnt -
         option_boxes[num_option].next_image_pause);
      option_boxes[num_option].current_image++;
      if (option_boxes[num_option].current_image >= 32)
        {
          option_boxes[num_option].current_image = 0;
        }
    }
  img = &options[num_option][option_boxes[num_option].current_image];
  coord_x = options_positions[num_option][0];
  coord_y = options_positions[num_option][1];

  /* display option box image */
  draw_bitmap_in_options (img, coord_x, coord_y);
  options_refresh[++opt_refresh_index].coord_x = coord_x;
  options_refresh[opt_refresh_index].coord_y = coord_y;
}

/** 
 * Display last image of a option box animation,        
 * when one option box is not selected any more, 
 * or that one option box is closed 
 * @param num_option option number from 0 to 11
 */
static void
option_clear (Sint32 num_option)
{
  Sint32 coord_x;
  Sint32 coord_y;
  spaceship_struct *ship = spaceship_get ();
  bitmap *img = &options[12][32];
  switch (num_option)
    {
    case OPTION_PANEL_INC_SPEED:
      if (ship->speed_booster < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_REPAIR:
      if (ship->spr.energy_level != ship->spr.pow_of_dest)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_FRONT_FIRE1:
      if (ship->shot_front_basic < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_FRONT_FIRE2:
      if (ship->shot_front_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_LEFT_FIRE1:
      if (ship->shot_left_basic < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_LEFT_FIRE2:
      if (ship->shot_left_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_RIGHT_FIRE1:
      if (ship->shot_right_basic < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_RIGHT_FIRE2:
      if (ship->shot_right_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_REAR_FIRE1:
      if (ship->shot_rear_basic < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_REAR_FIRE2:
      if (ship->shot_rear_enhanced < SPACESHIP_MAX_OPTION_LEVELS)
        {
          img = &options[num_option][32];
        }
      break;
    case OPTION_PANEL_NEW_SPACESHIP:
      if (ship->type < (SPACESHIP_NUM_OF_TYPES - 1))
        {
          img = &options[num_option][32];
        }
      break;
    }
  coord_x = options_positions[num_option][0];
  coord_y = options_positions[num_option][1];
  draw_bitmap_in_options (img, coord_x, coord_y);
  options_refresh[++opt_refresh_index].coord_x = coord_x;
  options_refresh[opt_refresh_index].coord_y = coord_y;
}

/** 
 * Display close animation option
 * @param num_option option number from 0 to 11
 */
static void
option_close_playanim (Sint32 num_option)
{
  Sint32 coord_x;
  Sint32 coord_y;
  option_boxes[num_option].next_image_pause_cnt =
    (Sint16) (option_boxes[num_option].next_image_pause_cnt +
              option_boxes[num_option].next_image_pause_offset);
  if (option_boxes[num_option].next_image_pause_cnt >=
      option_boxes[num_option].next_image_pause)
    {
      option_boxes[num_option].next_image_pause_cnt =
        (option_boxes[num_option].next_image_pause_cnt -
         option_boxes[num_option].next_image_pause);
      option_boxes[num_option].current_image++;
      if (option_boxes[num_option].current_image >= 31)
        {
          option_boxes[num_option].anim_close = 0;
        }
    }
  /* state of the option box is currently closing */
  option_boxes[num_option].close_option = 1;
  coord_x = options_positions[num_option][0];
  coord_y = options_positions[num_option][1];
  draw_bitmap_in_options (&options[11]
                          [option_boxes[num_option].current_image], coord_x,
                          coord_y);
  options_refresh[++opt_refresh_index].coord_x = coord_x;
  options_refresh[opt_refresh_index].coord_y = coord_y;
}

/** 
 * Display open animation option 
 * @param num_option option number from 0 to 11
 */
static void
option_open_playanim (Sint32 num_option)
{
  Sint32 num_ouverture;
  Sint32 coord_x;
  Sint32 coord_y;

  num_ouverture = 12;
  /* incrase delay counter */
  option_boxes[num_option].next_image_pause_cnt =
    (Sint16) (option_boxes[num_option].next_image_pause_cnt +
              option_boxes[num_option].next_image_pause_offset);
  if (option_boxes[num_option].next_image_pause_cnt >=
      option_boxes[num_option].next_image_pause)
    {
      option_boxes[num_option].next_image_pause_cnt =
        (option_boxes[num_option].next_image_pause_cnt -
         option_boxes[num_option].next_image_pause);
      /* next image */
      option_boxes[num_option].current_image++;
      if (option_boxes[num_option].current_image >= 32)
        {
          /* last image, option box is completely open */
          option_boxes[num_option].anim_open = 0;
          num_ouverture = num_option;
        }
    }
  /* state of the option box is currently opening */
  option_boxes[num_option].close_option = 0;
  coord_x = options_positions[num_option][0];
  coord_y = options_positions[num_option][1];
  draw_bitmap_in_options (&options[num_ouverture]
                          [option_boxes[num_option].current_image], coord_x,
                          coord_y);
  options_refresh[++opt_refresh_index].coord_x = coord_x;
  options_refresh[opt_refresh_index].coord_y = coord_y;
}

/** 
 * Initialize options open or close animations 
 * @param num_option option number from 0 to 11
 * @param box_closex TRUE if close anim, else open anim
 * */
void
option_anim_init (Sint32 num_option, bool box_close)
{
  option_boxes[num_option].current_image = 0;
  option_boxes[num_option].next_image_pause_cnt = 0;
  if (box_close)
    {
      option_boxes[num_option].anim_open = 0;
      option_boxes[num_option].anim_close = 1;
    }
  else
    {
      option_boxes[num_option].anim_open = 1;
      option_boxes[num_option].anim_close = 0;
    }
}

/**
 * Display the cursor on the selected option box
 * @param num_option option number from 0 to 11
 */
static void
option_selected_cursor (Sint32 num_option)
{
  Sint32 coord_x;
  Sint32 coord_y;
  if (((option_cursor_delay_count++) & 7) > 4)
    {
      coord_x = options_positions[num_option][0];
      coord_y = options_positions[num_option][1];
      draw_bitmap_in_options (&options[11][32], coord_x, coord_y);
      options_refresh[++opt_refresh_index].coord_x = coord_x;
      options_refresh[opt_refresh_index].coord_y = coord_y;
    }
}

/** 
 * Close all options boxes on the right options panel 
 */
void
options_close_all (void)
{
  Sint32 i;
  for (i = 0; i < OPTIONS_PANEL_NUMOF; i++)
    {
      option_anim_init (i, TRUE);
    }
}

/** 
 * Open all options boxes on the right options panel 
 * @param except Option number not to be opened
 */
void
options_open_all (Sint32 opt_except)
{
  Sint32 i;
  for (i = 0; i < OPTIONS_PANEL_NUMOF; i++)
    {
      if (opt_except == i)
        {
          continue;
        }
      option_anim_init (i, FALSE);
    }
}
