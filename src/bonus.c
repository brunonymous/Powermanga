/** 
 * @file bonus.c 
 * @brief Handle gems, bonus and penality 
 * @date 2014-10-12
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 * @author Laurent Duperval
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: bonus.c,v 1.30 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "enemies.h"
#include "bonus.h"
#include "electrical_shock.h"
#include "energy_gauge.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#include "lonely_foes.h"
#include "options_panel.h"
#include "shots.h"
#include "satellite_protections.h"
#include "shockwave.h"
#include "sdl_mixer.h"
#include "spaceship.h"

image bonus[GEM_NUMOF_TYPES][GEM_NUMOF_IMAGES];

/* max. bonus active at any time */
const Sint32 MAX_NUMOF_GEMS_ON_SCREEN = 20;

/** Bonus and penality availables */
typedef enum
{
  BONUS_INC_BY_1,
  BONUS_INC_BY_2,
  BONUS_ADD_SATELLITE,
  BONUS_INC_ENERGY,
  BONUS_SCR_MULTIPLIER,
  PENALITY_LONELY_FOE
} BONUS_AND_PENALTY;

/** Gem data structure */
typedef struct gem_str
{
  /** Type of bonus (BONUS_INC_BY_1, BONUS_INC_BY_2, ...) */
  Sint32 type;
  /** 0=linear trajectory (only one available) */
  Sint16 trajectory;
  /** Number of images in the sprite */
  Sint16 numof_images;
  /** Current image index */
  Sint16 current_image;
  /** Delay before next image */
  Sint16 next_image_pause;
  /** Delay's counter */
  Sint16 next_image_pause_cnt;
  /** Structures of images bonuses  */
  image *img[IMAGES_MAXOF];
  /* X-coordinate */
  float xcoord;
  /* Y-coordinate */
  float ycoord;
  /** Speed of the sprite */
  float speed;
  /** Previous element of the chained list */
  struct gem_str *previous;
  /** Next element of the chained list */
  struct gem_str *next;
  /** TRUE if the element is already chained */
  bool is_enabled;
}
gem_str;

/** Data structure of the gems */
static gem_str *gems = NULL;
static gem_str *gem_first = NULL;
static gem_str *gem_last = NULL;
static Sint32 num_of_gems = 0;

static void bonus_new (float pos_x, float pos_y);
static void bonus_meteor_new (float pos_x, float pos_y);
static Sint32 bonus_get (Sint32 value);
static Sint32 bonus_meteor_get (Sint32 value);
static bool bonus_collision (const gem_str * const gem);
static void bonus_del_gem (gem_str * gem);
static gem_str *bonus_get_gem (void);

/**
 * Load images of the gems and allocate gems structures 
 * @return TRUE if it completed successfully or FALSE otherwise 
 */
bool
bonus_once_init (void)
{
  bonus_free ();

  /* extract gems sprites images (80,868 bytes) */
  if (!image_load
      ("graphics/sprites/bonus_gems.spr", &bonus[0][0], GEM_NUMOF_TYPES,
       GEM_NUMOF_IMAGES))
    {
      return FALSE;
    }

  /* allocate gems data structure */
  if (gems == NULL)
    {
      gems =
        (gem_str *) memory_allocation (MAX_NUMOF_GEMS_ON_SCREEN *
                                       sizeof (gem_str));
      if (gems == NULL)
        {
          LOG_ERR ("not enough memory to allocate 'gems' structure");
          return FALSE;
        }
    }
  return 1;
}

/**
 * Convert bonus gems from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
bonus_extract (void)
{
  Uint32 type, frame;
  const char *model = EXPORT_DIR "/bonus-gems/bonus-gem-x/bonus-gem-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/bonus-gems"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (type = 0; type < GEM_NUMOF_TYPES; type++)
    {
      sprintf (filename, EXPORT_DIR "/bonus-gems/bonus-gem-%01d", type + 1);
      if (!create_dir (filename))
        {
          free_memory (filename);
          return FALSE;
        }
      for (frame = 0; frame < GEM_NUMOF_IMAGES; frame++)
        {
          sprintf (filename,
                   EXPORT_DIR "/bonus-gems/bonus-gem-%01d/bonus-gem-%02d.png",
                   type + 1, frame);
          if (!image_to_png (&bonus[type][frame], filename))
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
 * Initialize gem structure and chained list 
 */
void
bonus_init (void)
{
  Sint32 i;
  gem_str *gem;
  num_of_gems = 0;
  for (i = 0; i < MAX_NUMOF_GEMS_ON_SCREEN; i++)
    {
      gem = &gems[i];
      gem->is_enabled = FALSE;
    }
  gem_first = NULL;
  gem_last = NULL;
}

/**
 * Release images and structures of the gems
 */
void
bonus_free (void)
{
  images_free (&bonus[0][0], GEM_NUMOF_TYPES, GEM_NUMOF_IMAGES,
               GEM_NUMOF_IMAGES);
  if (gems != NULL)
    {
      free_memory ((char *) gems);
      gems = NULL;
    }
}

/*
 * Disables all gems on the screen
 */
void
bonus_disable_all (void)
{
  gem_str *gem;
  Sint32 i, n;
  gem = gem_first;
  if (gem == NULL)
    {
      return;
    }
  n = num_of_gems;
  for (i = 0; i < n; i++, gem = gem->next)
    {
     bonus_del_gem (gem);
    }
}

/**
 * Animate, move and draw the gems
 */
void
bonus_handle (void)
{
  gem_str *gem;
  Sint32 i, n;
  gem = gem_first;
  if (gem == NULL)
    {
      return;
    }
  n = num_of_gems;
  for (i = 0; i < n; i++, gem = gem->next)
    {

#ifdef UNDER_DEVELOPMENT
      if (gem == NULL && i < (num_of_gems - 1))
        {
          LOG_ERR ("gem->next is null %i/%i", i, num_of_gems);
          break;
        }
#endif

      /* check the current trajectory (only one available) */
      switch (gem->trajectory)
        {
          /*
           * bonus go straight (from top to bottom)
           */
        case 0:
          {
            if (!player_pause && menu_status == MENU_OFF && menu_section == 0)
              {
                /* moves from top to bottom */
                gem->ycoord += gem->speed;
              }
            /* check if the gem sprite is visible or not */
            if (((Sint16) gem->ycoord >=
                 (offscreen_height_visible + offscreen_clipsize))
                || ((Sint16) gem->xcoord +
                    (Sint16) gem->img[gem->current_image]->w >=
                    (offscreen_width - 1))
                || ((Sint16) gem->xcoord +
                    (Sint16) gem->img[gem->current_image]->w <
                    offscreen_clipsize))
              {
                /* gem sprite disappear from the screen, remove it */
                bonus_del_gem (gem);
              }
            else
              {
                /* inc. counter of delay between two images */
                gem->next_image_pause_cnt++;
                /* value of delay between two images reached? */
                if (gem->next_image_pause_cnt >= gem->next_image_pause)
                  {
                    /* clear counter of delay between two images */
                    gem->next_image_pause_cnt = 0;
                    /* flip to the next image */
                    gem->current_image++;
                    /* last image has been reached  */
                    if (gem->current_image >= gem->numof_images)
                      {
                        /* resets the animation to the first image of
                         * the animation sequence */
                        gem->current_image = 0;
                      }
                  }

                /* 
                 * collision between gem and spaceship 
                 */
                if (!gameover_enable)
                  {
                    if (bonus_collision (gem))
                      {
                        bonus_del_gem (gem);
                        break;
                      }
                  }

                /* draw bonus sprite */
                draw_sprite (gem->img[gem->current_image],
                             (Sint32) gem->xcoord, (Sint32) gem->ycoord);
              }
          }
          break;
        }
    }
}

/** 
 * Collision between a gem and the spaceship 
 * @param gem_str pointer to a gem structure
 * @return TRUE if the gem touched the spaceship 
 */
static bool
bonus_collision (const gem_str * const gem)
{
  image *gem_img, *ship_img;
  Sint32 i;
  Sint32 collisionx, collisiony, gemx, gemy;
  spaceship_struct *ship = spaceship_get ();
  gem_img = gem->img[gem->current_image];
  ship_img = ship->spr.img[ship->spr.current_image];
  gemx = (Sint32) gem->xcoord + gem_img->collisions_coords[0][XCOORD];
  gemy = (Sint32) gem->ycoord + gem_img->collisions_coords[0][YCOORD];

  /* for each collision point of the spaceship */
  for (i = 0; i < ship_img->numof_collisions_points; i++)
    {
      collisionx =
        (Sint32) ship->spr.xcoord + ship_img->collisions_points[i][XCOORD];
      collisiony =
        (Sint32) ship->spr.ycoord + ship_img->collisions_points[i][YCOORD];
      /* check if collision point is into gem collision zone */
      if (collisionx >= gemx &&
          collisiony >= gemy &&
          collisionx < (gemx + gem_img->collisions_sizes[0][IMAGE_WIDTH])
          && collisiony < (gemy + gem_img->collisions_sizes[0][IMAGE_HEIGHT]))
        {
          /* increase score of the player */
          player_score += 250 << score_multiplier;
          switch (gem->type)
            {
              /* 
               * add one level in the options range (green gem)  
               */
            case BONUS_INC_BY_1:
              {
                ship->gems_count++;
                option_change = TRUE;
#ifdef USE_SDLMIXER
                sound_play (SOUND_GREEN_GEM);
#endif
              }
              break;

              /* 
               * add two levels in the options range (red gem)  
               */
            case BONUS_INC_BY_2:
              {
                ship->gems_count += 2;
                option_change = TRUE;
#ifdef USE_SDLMIXER
                sound_play (SOUND_RED_GEM);
#endif
              }
              break;

              /* 
               * add a satellite protection (yellow gem) 
               */
            case BONUS_ADD_SATELLITE:
              {
                satellite_add ();
#ifdef USE_SDLMIXER
                sound_play (SOUND_YELLOW_GEM);
#endif
              }
              break;

              /* 
               * restore energy level of the spaceship
               * (purple gem)
               */
            case BONUS_INC_ENERGY:
              {
                if (ship->spr.energy_level < ship->spr.pow_of_dest)
                  {
                    option_boxes[1].close_option = FALSE;
#ifdef USE_SDLMIXER
                    sound_play (SOUND_PURPLE_GEM);
#endif
                  }
                /* maximum energy level reached: 
                 * circular shock wave is propagated */
                else
                  {
                    shockwave_add ();
                  }

                /* increase the level of energy of the spaceship */
                ship->spr.energy_level += 20;
                if (ship->spr.energy_level >= ship->spr.pow_of_dest)
                  {
                    ship->spr.energy_level = ship->spr.pow_of_dest;
                    if (!option_boxes[OPTION_PANEL_REPAIR].close_option)
                      {
                        /* maximum energy level:
                         * start anim of closing option box */
                        option_anim_init (OPTION_PANEL_REPAIR, TRUE);
                      }
                  }
                energy_gauge_spaceship_is_update = TRUE;
              }
              break;

              /* 
               * add a score multiplier (blue gem) 
               */
            case 4:
              {
                score_multiplier++;
                /* multiplier x4 maximum is allowed */
                if (score_multiplier > 2)
                  {
                    score_multiplier = 2;
                  }
              }
              break;
            }
          return TRUE;
        }
    }
  return FALSE;
}

/** 
 * Add a bonus (green, red, yellow, or purple gem) 
 * or a lonely foe, into curve, grid, or guardian phase   
 * @param foe pointer to the structure of an enemy
 */
void
bonus_add (const enemy * const foe)
{
  image *i = foe->spr.img[foe->spr.current_image];
  bonus_new (foe->spr.xcoord + i->x_gc - 8, foe->spr.ycoord + i->y_gc - 8);
}

/**
 * Add a new gem
 * @param type Type of gem BONUS_INC_BY_1, BONUS_INC_BY_2, ...
 * @param xcoord X coordinate of the gem
 * @param xcoord Y coordinate of the gem
 * @param speed speed of the gem
 */
static void
bonus_new_gem (Sint32 type, float xcoord, float ycoord, float speed)
{
  Sint32 i;
  gem_str *gem;
  gem = bonus_get_gem ();
  if (gem == NULL)
    {
      return;
    }
  gem->type = type;
  gem->trajectory = 0;
  gem->numof_images = GEM_NUMOF_IMAGES;
  gem->current_image = 0;
  gem->next_image_pause = 4;
  gem->next_image_pause_cnt = 0;
  for (i = 0; i < gem->numof_images; i++)
    {
      gem->img[i] = (image *) & bonus[type][i];
    }
  gem->xcoord = xcoord;
  gem->ycoord = ycoord;
  gem->speed = speed;
}

/** 
 * Initialize a new bonus (green, red, yellow, or purple gem) 
 * or a lonely foe, into curve, grid, or guardian phase   
 * @param pos_x x coordinate of the gem
 * @param pos_y y coordinate of the gem
 **/
static void
bonus_new (float pos_x, float pos_y)
{
  Sint32 btype = 0;
  spaceship_struct *ship = spaceship_get ();

  /* get a bonus or penality value according to the difficulty */
  if (num_level == 0)
    btype = (((Sint32) rand () % ((ship->type << 2) + 35)));
  if (num_level == 1)
    btype = (((Sint32) rand () % ((ship->type << 2) + 40)));
  if (num_level == 2)
    btype = (((Sint32) rand () % ((ship->type << 2) + 45)));
  if (num_level >= 3)
    btype = (((Sint32) rand () % ((ship->type << 2) + 50)));
  btype = bonus_get (btype);
  switch (btype)
    {
      /* one level in the options range (green gem)  */
    case BONUS_INC_BY_1:
      bonus_new_gem (BONUS_INC_BY_1, pos_x, pos_y, 0.25f);
      break;

      /* two levels in the options range (red gem)  */
    case BONUS_INC_BY_2:
      bonus_new_gem (BONUS_INC_BY_2, pos_x, pos_y, 0.35f);
      break;

      /* add a satellite protection bonus (yellow gem) */
    case BONUS_ADD_SATELLITE:
      bonus_new_gem (BONUS_ADD_SATELLITE, pos_x, pos_y, 0.55f);
      break;

      /* add a energy bonus (purple gem) */
    case BONUS_INC_ENERGY:
      bonus_new_gem (BONUS_INC_ENERGY, pos_x, pos_y, 0.45f);
      break;

      /* add a new lonely foe */
    case PENALITY_LONELY_FOE:
      lonely_foe_add (-1);
      break;

    default:
      break;
    }
}

/** 
 * Get a bonus or penality value according to the difficulty
 * @return Type of bonus or penalty
 */
static Sint32
bonus_get (Sint32 value)
{
  Sint32 btype = -1;
  if (power_conf->difficulty == 2)
    {
      if (value < 5)
        {
          btype = BONUS_INC_BY_1;
        }
      else if (value == 10)
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value == 11)
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if (value == 12)
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value == 13 || value == 26 || value == 33 || value == 40
               || value == 49)
        {
          if (num_of_enemies < (MAX_OF_ENEMIES))
            {
              btype = PENALITY_LONELY_FOE;
            }
        }
    }
  else if (power_conf->difficulty == 1)
    {
      if (value < 10)
        {
          btype = BONUS_INC_BY_1;
        }
      else if (value == 10)
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value == 11)
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if (value == 12 || value == 35)
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value == 13 || value == 26 || value == 49)
        {
          if (num_of_enemies < (MAX_OF_ENEMIES - 2))
            {
              btype = PENALITY_LONELY_FOE;
            }
        }
    }
  else if (power_conf->difficulty == 0)
    {
      if (value < 15)
        {
          btype = BONUS_INC_BY_1;
        }
      else if (value < 22)
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value < 25)
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if (value < 30)
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value < 32 || value == 49)
        {
          if (num_of_enemies < (MAX_OF_ENEMIES - 4))
            {
              btype = PENALITY_LONELY_FOE;
            }
        }
    }
  /* test only
     btype = BONUS_INC_ENERGY;
   */
  return btype;
}

/** 
 * Add a bonus (green, red, yellow, or purple gem) 
 * or a lonely foe, into meteors phase   
 * @param foe Pointer to the structure of an enemy
 */
void
bonus_meteor_add (const enemy * const foe)
{
  image *i = foe->spr.img[foe->spr.current_image];
  bonus_meteor_new (foe->spr.xcoord + i->x_gc - 8,
                    foe->spr.ycoord + i->y_gc - 8);
}

/** 
 * Initialize a bonus (green, red, yellow, or purple gem) 
 * or a lonely foe, into meteors phase   
 * @param pos_x X-coordinate of the gem
 * @param pos_y Y-coordinate of the gem
 */
static void
bonus_meteor_new (float pos_x, float pos_y)
{
  Sint32 btype = 0;
  spaceship_struct *ship = spaceship_get ();

  /* get a bonus or penality value according to the difficulty */
  if (num_level == 0)
    btype = (((Sint32) rand () % ((ship->type << 2) + 30)));
  if (num_level == 1)
    btype = (((Sint32) rand () % ((ship->type << 2) + 40)));
  if (num_level == 2)
    btype = (((Sint32) rand () % ((ship->type << 2) + 50)));
  if (num_level >= 3)
    btype = (((Sint32) rand () % ((ship->type << 2) + 60)));
  btype = bonus_meteor_get (btype);

  switch (btype)
    {
      /* one level in the options range (green gem)  */
    case BONUS_INC_BY_1:
      bonus_new_gem (BONUS_INC_BY_1, pos_x, pos_y, 0.25f);
      break;

      /* two levels in the options range (red gem)  */
    case BONUS_INC_BY_2:
      bonus_new_gem (BONUS_INC_BY_2, pos_x, pos_y, 0.35f);
      break;

      /* add a satellite protection bonus (yellow gem) */
    case BONUS_ADD_SATELLITE:
      bonus_new_gem (BONUS_ADD_SATELLITE, pos_x, pos_y, 0.55f);
      break;

      /* add a energy bonus (purple gem) */
    case BONUS_INC_ENERGY:
      bonus_new_gem (BONUS_INC_ENERGY, pos_x, pos_y, 0.45f);
      break;

      /* add a bonus score multiplier (blue gem) */
    case BONUS_SCR_MULTIPLIER:
      bonus_new_gem (BONUS_SCR_MULTIPLIER, pos_x, pos_y, 0.65f);
      break;

      /* add a new lonely foe */
    case PENALITY_LONELY_FOE:
      if (num_of_enemies < (MAX_OF_ENEMIES - 2))
        {
          lonely_foe_add (-1);
        }
      break;

    default:
      break;
    }
}

/** 
 * Get a bonus or penality value according to the difficulty
 * into the meteors phase
 * @return Type of bonus or penalty
 */
Sint32
bonus_meteor_get (Sint32 value)
{
  Sint32 btype = -1;
  switch (power_conf->difficulty)
    {
    case 0:
      if (value < 10)
        {
          btype = BONUS_INC_BY_1;
        }
      if (value == 10 || value == 11)
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value == 6 || value == 12 || value == 36)
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if (value == 7 || value == 21 || value == 35 || value == 50
               || value == 56)
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value == 40 || value == 20)
        {
          btype = BONUS_SCR_MULTIPLIER;
        }
      else if (value == 8 || value == 49)
        {
          btype = PENALITY_LONELY_FOE;
        }
      break;

    case 1:
      if (value < 5)
        {
          btype = BONUS_INC_BY_1;
        }
      if (value == 5)
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value == 6)
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if (value == 7 || value == 35 || value == 50)
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value == 40 && !(rand () % 3))
        {
          btype = BONUS_SCR_MULTIPLIER;
        }
      else if (value == 8 || value == 26 || value == 49 || value == 59)
        {
          btype = PENALITY_LONELY_FOE;
        }
      break;

    case 2:
      if (value < 3)
        {
          btype = BONUS_INC_BY_1;
        }
      if (value == 5 && (rand () % 2))
        {
          btype = BONUS_INC_BY_2;
        }
      else if (value == 6 && (rand () % 2))
        {
          btype = BONUS_ADD_SATELLITE;
        }
      else if ((value == 7 || value == 35 || value == 50) && (rand () % 2))
        {
          btype = BONUS_INC_ENERGY;
        }
      else if (value == 40 && !(rand () % 6))
        {
          btype = BONUS_SCR_MULTIPLIER;
        }
      else if (!(rand () % 6))
        {
          btype = PENALITY_LONELY_FOE;
        }
    }
  return btype;
}

/**
 * Check validty of bonus.chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
bonus_check_chained_list (void)
{
  Sint32 i;
  gem_str *gem;
  Sint32 count = 0;
  for (i = 0; i < MAX_NUMOF_GEMS_ON_SCREEN; i++)
    {
      gem = &gems[i];
      if (gem->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_gems)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, num_of_gems=%i", count, num_of_gems);
    }
  count = 0;
  gem = gem_first;
  do
    {
      count++;
      gem = gem->next;
    }
  while (gem != NULL && count <= (MAX_NUMOF_GEMS_ON_SCREEN + 1));
  if (count != num_of_gems)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_gems=%i", count, num_of_gems);
    }
  count = 0;
  gem = gem_last;
  do
    {
      count++;
      gem = gem->previous;
    }
  while (gem != NULL && count <= (MAX_NUMOF_GEMS_ON_SCREEN + 1));
  if (count != num_of_gems)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_gems=%i", count, num_of_gems);
    }
}
#endif

/** 
 * Return a free gem element 
 * @return Pointer to a gem structure, NULL if not gem available 
 */
static gem_str *
bonus_get_gem (void)
{
  Sint32 i;
  gem_str *gem;
  for (i = 0; i < MAX_NUMOF_GEMS_ON_SCREEN; i++)
    {
      gem = &gems[i];
      if (gem->is_enabled)
        {
          continue;
        }
      gem->is_enabled = TRUE;
      gem->next = NULL;
      if (num_of_gems == 0)
        {
          gem_first = gem;
          gem_last = gem;
          gem_last->previous = NULL;
        }
      else
        {
          gem_last->next = gem;
          gem->previous = gem_last;
          gem_last = gem;
        }
      num_of_gems++;
#ifdef UNDER_DEVELOPMENT
      bonus_check_chained_list ();
#endif
      return gem;
    }
  LOG_ERR ("no more element gem is available");
  return NULL;
}

/** 
 * Remove a gem element from list
 */
static void
bonus_del_gem (gem_str * gem)
{
  gem->is_enabled = FALSE;
  num_of_gems--;
  if (gem_first == gem)
    {
      gem_first = gem->next;
    }
  if (gem_last == gem)
    {
      gem_last = gem->previous;
    }
  if (gem->previous != NULL)
    {
      gem->previous->next = gem->next;
    }
  if (gem->next != NULL)
    {
      gem->next->previous = gem->previous;
    }
}
