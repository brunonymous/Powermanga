/** 
 * @file meteors_phase.c
 * @brief Handle the meteor storm 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: meteors_phase.c,v 1.27 2012/08/25 19:18:32 gurumeditation Exp $
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
#include "curve_phase.h"
#include "display.h"
#include "enemies.h"
#include "shots.h"
#include "grid_phase.h"
#include "guardians.h"
#include "images.h"
#include "log_recorder.h"
#include "meteors_phase.h"
#include "spaceship.h"
#include "starfield.h"
#include "texts.h"

bool meteor_activity = FALSE;
Sint32 num_of_meteors = 0;

/** Maximum number of differents meteors: big, medium and small */
#define METEOR_MAXOF_TYPES 3

static image meteor_images[METEOR_MAXOF_TYPES][METEOR_NUMOF_IMAGES];
static Sint32 meteor_delay_next = 0;
static bool next_level_without_guardian (void);

/**
 * First initialization of meteors
 */
bool
meteors_once_init (void)
{
  meteors_free ();
  return TRUE;
}

/**
 * Release memory used for the meteors
 */
void
meteors_free (void)
{
  meteors_images_free ();
}

/**
 * Generate big, medium or small meteors 
 */
void
meteors_handle (void)
{
  Sint32 k, meteor_size;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  /* meteors phase currently in progress? */
  if (!meteor_activity)
    {
      /* meteors phase not enable */
      return;
    }

  /* appearance of new a meteor? */
  if (meteor_delay_next <= (12 + (MAX_NUM_OF_LEVELS - num_level)))
    {
      meteor_delay_next++;
      return;
    }
  meteor_delay_next = 1;

  /* max. of meteors reached? */
  if (num_of_meteors >= (40 + (num_level << 1)))
    {
      return;
    }

  /* get new enemy element */
  foe = enemy_get ();
  if (foe == NULL)
    {
      return;
    }

  /* size of the meteor: 0, 1 or 2 */
  meteor_size = rand () % 3;
  switch (meteor_size)
    {
    case 0:
      /* set meteor's power of destruction */
      foe->spr.pow_of_dest = (Sint16) (6 + (num_level >> 1));
      /* set vertical speed of the displacement */
      foe->y_speed = 0.7f + (float) ((float) num_level / 25.0);
      /* value of delay between two images */
      foe->spr.anim_speed = 5;
      break;
    case 1:
      foe->spr.pow_of_dest = (Sint16) (4 + (num_level >> 1));
      foe->y_speed = 0.6f + (float) ((float) num_level / 30.0);
      foe->spr.anim_speed = 4;
      break;
    case 2:
      foe->spr.pow_of_dest = (Sint16) (2 + (num_level >> 1));
      foe->y_speed = 0.5f + (float) ((float) num_level / 35.0);
      foe->spr.anim_speed = 3;
      break;
    }
  /* set level of energy (zero correspond to destruction of the metor) */
  foe->spr.energy_level =
    (Sint16) ((ship->type << 1) + (foe->spr.pow_of_dest << 3) / 5);
  /* set number of images of the metor sprite */
  foe->spr.numof_images = METEOR_NUMOF_IMAGES;
  /* set current image */
  foe->spr.current_image = (Sint16) (rand () % METEOR_NUMOF_IMAGES);
  /* clear counter delay before next image */
  foe->spr.anim_count = 0;
  /* set addresses of the images buffer */
  for (k = 0; k < foe->spr.numof_images; k++)
    {
      foe->spr.img[k] = (image *) & meteor_images[meteor_size][k];
    }

  /* set shot time-frequency: meteor has never shot */
  foe->fire_rate = 10000;
  /* set counter of delay between two shots */
  foe->fire_rate_count = foe->fire_rate;
  /* set type of displacement: 
   * meteors are identified like lonely foe */
  foe->displacement = DISPLACEMENT_LONELY_FOE;
  /* set x and y coordinates of the meteor  */
  foe->spr.xcoord =
    offscreen_startx +
    (float) (rand () % (offscreen_width_visible - foe->spr.img[0]->w));
  foe->spr.ycoord = (float) (offscreen_starty - 64);
  /* clear horizontal speed of the displacement */
  foe->x_speed = 0.0;
  /* set type of meteor */
  foe->type = BIGMETEOR + meteor_size;
  /* set delay time before destruction */
  foe->timelife = 210;

  /* set animation direction */
  if (rand () % 2)
    {
      /* reverse animation direction */
      foe->sens_anim = -1;
    }
  else
    {
      /* forward animation direction */
      foe->sens_anim = 1;
    }
  num_of_meteors++;
}

/** 
 * Prepare the next phase: curve phase or guardian phase
 * this function is also called to begin a new game 
 */
bool
meteors_finished (void)
{
  /* meteors phase currently in progress? */
  if (!meteor_activity)
    {
      /* meteors phase not enable */
      return TRUE;
    }

  /* next phase */
  if (num_level != -1)
    {
      /* number of meteors reached? */
      if (num_of_meteors >= (40 + (num_level << 1)))
        {
          switch (num_level)
            {
            case 3:
              guardian_new (1);
              break;
            case 7:
              guardian_new (2);
              break;
            case 11:
              guardian_new (3);
              break;
            case 15:
              guardian_new (4);
              break;
            case 19:
              guardian_new (5);
              break;
            case 23:
              guardian_new (6);
              break;
            case 27:
              guardian_new (7);
              break;
            case 31:
              guardian_new (8);
              break;
            case 35:
              guardian_new (9);
              break;
            case 39:
              guardian_new (10);
              break;
            case 41:
              guardian_new (11);
              break;
            default:
              if (!next_level_without_guardian ())
                {
                  return FALSE;
                }
              break;
            }
        }
    }

  /* 
   * start a new game: first level of the game 
   */
  else
    {
      if (num_of_meteors >= (40 + (num_level << 1)))
        {
          num_level++;
          if (num_level > MAX_NUM_OF_LEVELS)
            {
              num_level = 0;
            }
          /* load first guardian */
          if (!guardian_load (1))
            {
              return FALSE;
            }
          /* load grid phase */
          if (!grid_load (num_level))
            {
              return FALSE;
            }
          if (!curve_load_level (num_level))
            {
              return FALSE;
            }
          if (!meteors_load (num_level))
            {
              return FALSE;
            }
          /* enable the curve phase */
          curve_enable_level ();
          courbe.activity = TRUE;
          grid.is_enable = FALSE;
          meteor_activity = FALSE;
          guardian->number = 0;
        }
    }
  return TRUE;
}

/**
 * Release meteors sprite data
 */
void
meteors_images_free (void)
{
  Sint32 i, j;
  for (i = 0; i < (METEOR_MAXOF_TYPES); i++)
    {
      for (j = 0; j < METEOR_NUMOF_IMAGES; j++)
        {
          if (meteor_images[i][j].img != NULL)
            {
              free_memory (meteor_images[i][j].img);
              meteor_images[i][j].img = NULL;
            }
          if (meteor_images[i][j].compress != NULL)
            {
              free_memory (meteor_images[i][j].compress);
              meteor_images[i][j].compress = NULL;
            }
        }
    }
}

/**
 * Load meteors sprite data
 * @param num_meteor Meteors level number from 0 to 42
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
meteors_load (Sint32 num_meteor)
{
  char *file, *data;
  meteors_images_free ();
  if (num_meteor > MAX_NUM_OF_LEVELS || num_meteor < 0)
    {
      num_meteor = 0;
    }
  file =
    loadfile_num ("graphics/sprites/meteors/meteor_%02d.spr", num_meteor);
  if (file == NULL)
    {
      return FALSE;
    }
  data =
    images_read (&meteor_images[0][0], METEOR_MAXOF_TYPES,
                 METEOR_NUMOF_IMAGES, file, METEOR_NUMOF_IMAGES);
  free_memory (file);
  if (data == NULL)
    {
      return FALSE;
    }
  return TRUE;
}


/**
 * Convert meteors from data image to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
meteors_extract (void)
{
  Uint32 i, type, frame;
  const char *model =
    EXPORT_DIR "/meteors/level-%02d/type-%01d/meteor-%01d.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/meteors"))
    {
      return FALSE;
    }

  for (i = 0; i < MAX_NUM_OF_LEVELS; i++)
    {
      if (!meteors_load (i))
        {
          free_memory (filename);
          return FALSE;
        }
      sprintf (filename, EXPORT_DIR "/meteors/level-%02d", i);
      if (!create_dir (filename))
        {
          return FALSE;
        }
      for (type = 0; type < METEOR_MAXOF_TYPES; type++)
        {
          sprintf (filename, EXPORT_DIR "/meteors/level-%02d/type-%01d", i,
                   type + 1);
          if (!create_dir (filename))
            {
              return FALSE;
            }
          for (frame = 0; frame < METEOR_NUMOF_IMAGES; frame++)
            {
              sprintf (filename,
                       EXPORT_DIR
                       "/meteors/level-%02d/type-%01d/meteor-%02d.png", i,
                       type + 1, frame);
              if (!image_to_png (&meteor_images[type][frame], filename))
                {
                  free_memory (filename);
                  return FALSE;
                }
            }
        }

    }
  free_memory (filename);
  return TRUE;
}
#endif

/** 
 * Jump to next level without loading a guardian 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
next_level_without_guardian (void)
{
  bool is_finished;
  spaceship_disappears = TRUE;
  is_finished = text_level_move (num_level);
  if (starfield_speed == 0.0 && is_finished)
    {
      /* next level */
      num_level++;
      LOG_INF ("level number: %i", num_level);
      if (num_level > MAX_NUM_OF_LEVELS)
        {
          num_level = 0;
        }
      /* load grid phase */
      if (!grid_load (num_level))
        {
          return FALSE;
        }
      /* load curve phase level file (little skirmish) */
      if (!curve_load_level (num_level))
        {
          return FALSE;
        }
      if (!meteors_load (num_level))
        {
          return FALSE;
        }
      /* enable the curve level file loaded previously */
      curve_enable_level ();
      /* grid phase enable */
      courbe.activity = TRUE;
      grid.is_enable = FALSE;
      meteor_activity = FALSE;
      spaceship_show ();
    }
  return TRUE;
}
