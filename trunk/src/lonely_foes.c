/**
 * @file lonely_foes.c
 * @brief Handle lonely foes (come out randomly as penality) 
 * @created 1999-09-09
 * @date 2010-01-01
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: lonely_foes.c,v 1.30 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "curve_phase.h"
#include "display.h"
#include "enemies.h"
#include "shots.h"
#include "lonely_foes.h"
#include "spaceship.h"
#include "sdl_mixer.h"

/** Current number of lonely foes */
static Uint32 lonely_foes_count = 0;

/**
 * Return a new enemy element
 * @param power Power of destruction of this new enemy
 * @param energy Energy level of this new enemey
 * @param current_image Current image index 
 * @param type Identifier of this enemy
 * @param shot_delay Delay value before two shots 
 * @return Pointer to the new enemy element
 */
static enemy *
lonely_foe_new (Sint16 power, Sint16 energy, Sint16 current_image,
                Sint32 type, Sint32 shot_delay)
{
  Sint32 i;
  enemy *foe;
  foe = enemy_get ();
  if (foe == NULL)
    {
      return NULL;
    }
  foe->spr.pow_of_dest = power;
  foe->spr.energy_level = energy;
  foe->spr.numof_images = 32;
  foe->spr.current_image = current_image;
  foe->spr.anim_count = 0;
  for (i = 0; i < foe->spr.numof_images; i++)
    {
      foe->spr.img[i] = (image *) & enemi[type][i];
    }
  foe->fire_rate = shot_delay;
  foe->fire_rate_count = foe->fire_rate;
  foe->displacement = DISPLACEMENT_LONELY_FOE;
  foe->type = type;
  foe->dead = FALSE;
  foe->visible = TRUE;
  return foe;
}

/**
 * Create a new enemy
 * @param delay_next Animation speed
 * @param type Identifier of this enemy
 * @param shot_delay Delay value before two shots 
 * @param speed Speed of the displacement  
 * @return Pointer to the new enemy element
 */
static enemy *
lonely_foe_create (Sint16 delay_next, Sint32 type, Sint32 shot_delay,
                   float speed)
{
  spaceship_struct *ship = spaceship_get ();
  Sint16 power = (Sint16) ((ship->type << 1) + type - 40);
  Sint16 energy = (Sint16) ((ship->type << 2) + (power << 3) / 3 + 10);
  enemy *foe = lonely_foe_new (power, energy, 0, type, shot_delay);
  if (foe == NULL)
    {
      return NULL;
    }
  foe->spr.anim_speed = delay_next;
  foe->spr.speed = speed;
  return foe;
}

/**
 * Add a Gozuky foe droped by a Naggys foe in the guardian phase
 * @param foe Pointer to a Naggys foe 
 */
void
lonely_foe_add_gozuky (enemy * foe, Uint32 cannon_pos)
{
  Sint32 xcoord, ycoord;
  Sint16 power, current_image;
  spaceship_struct *ship = spaceship_get ();
  sprite *spr = &foe->spr;
  xcoord =
    (Sint32) (spr->xcoord +
              spr->img[spr->
                       current_image]->cannons_coords[cannon_pos][XCOORD]);
  /* check if the sprite is visible or not  */
  if (xcoord <= offscreen_startx ||
      xcoord >= (offscreen_startx + offscreen_width_visible - 1))
    {
      return;
    }
  ycoord =
    (Sint32) (spr->ycoord +
              spr->img[spr->
                       current_image]->cannons_coords[cannon_pos][YCOORD]);
  power = (Sint16) ((ship->type << 1) + 10);
  current_image = (Sint16) (rand () % ENEMIES_SPECIAL_NUM_OF_IMAGES);
  foe = lonely_foe_new (power, power, current_image, GOZUKY, 6000);
  if (foe == NULL)
    {
      return;
    }
  spr = &foe->spr;
  spr->anim_speed = 1;
  spr->speed = 0.2f;
  spr->xcoord = (float) (xcoord - spr->img[0]->x_gc);
  spr->ycoord = (float) (ycoord - spr->img[0]->y_gc);
}

/**
 * Create a new enemy element which follow a curve
 * @param type Identifier of this enemy
 * @param shot_delay Delay value before two shots 
 */
void
lonely_foe_curve_create (Sint32 type, Sint32 shot_delay)
{
  enemy *foe;
  Sint16 curve_num;
  Sint16 current_image, power, energy;
  spaceship_struct *ship = spaceship_get ();
  curve_num = 51 + (Sint16) (rand () % 4);
  current_image = initial_curve[curve_num].angle[0];
  power = (Sint16) ((ship->type << 1) + type - 40);
  energy = (Sint16) ((ship->type << 2) + (power << 3) / 3 + 10);
  foe = lonely_foe_new (power, energy, current_image, type, shot_delay);
  if (foe == NULL)
    {
      return;
    }
  foe->num_courbe = curve_num;
  foe->spr.xcoord = (float) (initial_curve[curve_num].pos_x + 128 - 32);
  foe->spr.ycoord = (float) (initial_curve[curve_num].pos_y + 128 - 32);
  foe->pos_vaiss[POS_CURVE] = 0;
}


/**
 * Initialize, clear the lonely foe counter
 */
void
lonely_foes_init (void)
{
  lonely_foes_count = 0;
}

/**
 * Add a lonely foe to the enemies list
 * @param foe_num Foe number of -1 if foe is selected by this function
 */
void
lonely_foe_add (Sint32 foe_num)
{
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
#ifdef USE_SDLMIXER
  sound_play (SOUND_LONELY_FOE);
#endif

  /* select an foes */
  if (foe_num == -1)
    {
      lonely_foes_count++;
      /* all the lonely foes were selected once? */
      if (lonely_foes_count > LONELY_FOES_MAX_OF)
        {
          /* select a foe randomly */
          lonely_foes_count = LONELY_FOES_MAX_OF;
          foe_num = (Sint32) (((long) rand () % lonely_foes_count));
        }
      /* select one after the other all the foes available */
      else
        {
          foe_num = lonely_foes_count - 1;
          if (foe_num < 0)
            {
              foe_num = 0;
            }
        }

    }
  switch (foe_num)
    {
      /* SUBJUGANEERS */
    case LONELY_SUBJUGANEERS:
      foe = lonely_foe_create (4, SUBJUGANEERS,
                               60 + (Sint32) (((long) rand () % (50))), -0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord
        = (float) (offscreen_startx + 64 + offscreen_width_visible);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* MILLOUZ */
    case LONELY_MILLOUZ:
      foe = lonely_foe_create (3, MILLOUZ, 0, 0.4f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        (float) (offscreen_startx +
                 ((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* SWORDINIANS */
    case LONELY_SWORDINIANS:
      foe =
        lonely_foe_create (4, SWORDINIANS,
                           50 + (Sint32) ((long) rand () % (50)), -0.3f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord
        = (float) offscreen_starty + 64 + offscreen_height_visible;
      break;

      /* TOUBOUG */
    case LONELY_TOUBOUG:
      foe = lonely_foe_create (4, TOUBOUG, 0, 0.35f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        (float) (offscreen_startx +
                 ((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* DISGOOSTEES */
    case LONELY_DISGOOSTEES:
      foe =
        lonely_foe_create (4, DISGOOSTEES,
                           50 + (Sint32) ((long) rand () % (50)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* EARTHINIANS */
    case LONELY_EARTHINIANS:
      foe =
        lonely_foe_create (4, EARTHINIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* BIRIANSTEES */
    case LONELY_BIRIANSTEES:
      foe =
        lonely_foe_create (4, BIRIANSTEES,
                           50 + (Sint32) ((long) rand () % (50)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* BELCHOUTIES */
    case LONELY_BELCHOUTIES:
      foe =
        lonely_foe_create (4, BELCHOUTIES,
                           60 + (Sint32) ((long) rand () % (50)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* VIONIEES */
    case LONELY_VIONIEES:
      foe =
        lonely_foe_create (4, VIONIEES, 50 + (Sint32) ((long) rand () % (50)),
                           2.0f +
                           (float) (((long) rand () % (100))) / 100.0f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 32 - foe->spr.img[0]->h);
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
      break;

      /* HOCKYS */
    case LONELY_HOCKYS:
      foe =
        lonely_foe_create (4, HOCKYS, 50 + (Sint32) ((long) rand () % (50)),
                           -0.4f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord
        = (float) (offscreen_starty + 64 + offscreen_height_visible);
      break;

      /* TODHAIRIES */
    case LONELY_TODHAIRIES:
      foe =
        lonely_foe_create (4, TODHAIRIES,
                           60 + (Sint32) ((long) rand () % (60)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* DEFECTINIANS */
    case LONELY_DEFECTINIANS:
      foe =
        lonely_foe_create (4, DEFECTINIANS,
                           60 + (Sint32) ((long) rand () % (60)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* BLAVIRTHE */
    case LONELY_BLAVIRTHE:
      lonely_foe_curve_create (BLAVIRTHE,
                               60 + (Sint32) (((long) rand () % (60))));
      break;

      /* SOONIEES */
    case LONELY_SOONIEES:
      foe =
        lonely_foe_create (4, SOONIEES, 60 + (Sint32) ((long) rand () % (60)),
                           0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* ANGOUFF */
    case LONELY_ANGOUFF:
      foe =
        lonely_foe_create (4, ANGOUFF, 50 + (Sint32) ((long) rand () % (50)),
                           2.0f +
                           (float) (((long) rand () % (100))) / 100.0f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 32 - foe->spr.img[0]->h);
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
      break;

      /* GAFFIES */
    case LONELY_GAFFIES:
      foe =
        lonely_foe_create (6, GAFFIES, 60 + (Sint32) ((long) rand () % (60)),
                           0.2f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* BITTERIANS */
    case LONELY_BITTERIANS:
      foe =
        lonely_foe_create (4, BITTERIANS,
                           60 + (Sint32) ((long) rand () % (50)), -0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord
        = (float) (offscreen_startx + 64 + offscreen_width_visible);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* BLEUERCKS */
    case LONELY_BLEUERCKS:
      lonely_foe_curve_create (BLEUERCKS,
                               50 + (Sint32) (((long) rand () % (50))));
      break;

      /* ARCHINIANS */
    case LONELY_ARCHINIANS:
      foe =
        lonely_foe_create (4, ARCHINIANS,
                           60 + (Sint32) ((long) rand () % (50)), -0.2f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord
        = (float) (offscreen_starty + 64 + offscreen_height_visible);
      break;

      /* CLOWNIES */
    case LONELY_CLOWNIES:
      foe =
        lonely_foe_create (4, CLOWNIES, 50 + (Sint32) ((long) rand () % (50)),
                           2.5f +
                           (float) (((long) rand () % (100))) / 100.0f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
      break;

      /* DEMONIANS */
    case LONELY_DEMONIANS:
      foe =
        lonely_foe_create (4, DEMONIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* TOUTIES */
    case LONELY_TOUTIES:
      foe =
        lonely_foe_create (4, TOUTIES, 60 + (Sint32) ((long) rand () % (50)),
                           -0.35f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord
        = (float) (offscreen_startx + 64 + offscreen_width_visible);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* FIDGETINIANS */
    case LONELY_FIDGETINIANS:
      foe =
        lonely_foe_create (4, FIDGETINIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.speed = 0.5;
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* EFFIES */
    case LONELY_EFFIES:
      foe =
        lonely_foe_create (4, EFFIES, 50 + (Sint32) ((long) rand () % (50)),
                           2.5f +
                           (float) (((long) rand () % (100))) / 100.0f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
      break;

      /* DIMITINIANS */
    case LONELY_DIMITINIANS:
      foe =
        lonely_foe_create (6, DIMITINIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.3f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      foe->sens_anim = 0;
      break;

      /* PAINIANS */
    case LONELY_PAINIANS:
      foe =
        lonely_foe_create (4, PAINIANS, 60 + (Sint32) ((long) rand () % (50)),
                           0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord = (float) (offscreen_startx - 64 - foe->spr.img[0]->w);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* ENSLAVEERS */
    case LONELY_ENSLAVEERS:
      foe =
        lonely_foe_create (4, ENSLAVEERS,
                           60 + (Sint32) ((long) rand () % (50)), +0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* FEABILIANS */
    case LONELY_FEABILIANS:
      foe =
        lonely_foe_create (3, FEABILIANS,
                           60 + (Sint32) ((long) rand () % (50)), -0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord
        = (float) (offscreen_startx + 64 + offscreen_width_visible);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* DIVERTIZERS */
    case LONELY_DIVERTIZERS:
      foe =
        lonely_foe_create (3, DIVERTIZERS,
                           60 + (Sint32) ((long) rand () % (50)), +0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) ((((long) rand () %
                   (offscreen_width_visible - foe->spr.img[0]->w))));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* SAPOUCH */
    case SAPOUCH:
    case LONELY_SAPOUCH:
      foe =
        lonely_foe_create (4, SAPOUCH, 50 + (Sint32) ((long) rand () % (50)),
                           2.5f +
                           (float) (((long) rand () % (100))) / 100.0f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      foe->retournement = FALSE;
      foe->change_dir = FALSE;
      break;

      /* HORRIBIANS */
    case LONELY_HORRIBIANS:
      foe =
        lonely_foe_create (3, HORRIBIANS,
                           60 + (Sint32) ((long) rand () % (50)), 0.6f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) ((((long) rand () %
                   (offscreen_width_visible - foe->spr.img[0]->w))));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* CARRYONIANS */
    case LONELY_CARRYONIANS:
      foe =
        lonely_foe_create (5, CARRYONIANS,
                           60 + (Sint32) ((long) rand () % (50)), -0.2f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord
        = (float) (offscreen_starty + 64 + offscreen_height_visible);
      break;

      /* DEVILIANS */
    case DEVILIANS:
    case LONELY_DEVILIANS:
      foe =
        lonely_foe_create (5, DEVILIANS,
                           60 + (Sint32) ((long) rand () % (50)), +0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) ((((long) rand () %
                   (offscreen_width_visible - foe->spr.img[0]->w))));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* ROUGHLEERS */
    case LONELY_ROUGHLEERS:
      foe =
        lonely_foe_create (6, ROUGHLEERS,
                           50 + (Sint32) ((long) rand () % (50)), 0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* ABASCUSIANS */
    case LONELY_ABASCUSIANS:
      foe =
        lonely_foe_create (4, ABASCUSIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.5);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* ROTIES */
    case LONELY_ROTIES:
      lonely_foe_curve_create (ROTIES,
                               50 + (Sint32) (((long) rand () % (50))));
      break;

      /* STENCHIES */
    case LONELY_STENCHIES:
      lonely_foe_curve_create (STENCHIES,
                               50 + (Sint32) (((long) rand () % (50))));
      break;

      /* PERTURBIANS */
    case LONELY_PERTURBIANS:
      foe =
        lonely_foe_create (6, PERTURBIANS,
                           50 + (Sint32) ((long) rand () % (50)), 0.2f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord =
        offscreen_startx +
        (float) (((long) rand () %
                  (offscreen_width_visible - foe->spr.img[0]->w)));
      foe->spr.ycoord = (float) (offscreen_starty - 64 - foe->spr.img[0]->h);
      break;

      /* MADIRIANS */
    case LONELY_MADIRIANS:
      lonely_foe_curve_create (MADIRIANS,
                               50 + (Sint32) (((long) rand () % (50))));
      break;

      /* BAINIES */
    case LONELY_BAINIES:
      foe =
        lonely_foe_create (4, BAINIES, 50 + (Sint32) ((long) rand () % (40)),
                           0.4f);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.xcoord = (float) (offscreen_startx - 64 - foe->spr.img[0]->w);
      foe->spr.ycoord =
        offscreen_starty +
        (float) (((long) rand () %
                  (offscreen_height_visible - foe->spr.img[0]->h)));
      break;

      /* NAGGYS */
    case NAGGYS:
      foe =
        lonely_foe_new ((Sint32) (ship->type << 2),
                        (Sint32) ((ship->type << 2) + 20), 0, NAGGYS, 15);
      if (foe == NULL)
        {
          break;
        }
      foe->spr.anim_speed = 2;
      foe->spr.speed = 2.0;
      foe->spr.xcoord = (float) (offscreen_startx - 64 - foe->spr.img[0]->w);
      foe->spr.ycoord =
        offscreen_starty + 48 + (float) (((long) rand () % 32));
      break;
    }
}
