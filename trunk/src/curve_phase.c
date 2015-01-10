/** 
 * @file curve_phase.c
 * @brief Handle the curve phase 
 * @date 2010-01-01
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: curve_phase.c,v 1.22 2012/06/03 17:06:14 gurumeditation Exp $
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
#include "electrical_shock.h"
#include "enemies.h"
#include "shots.h"
#include "log_recorder.h"
#include "meteors_phase.h"
#include "gfx_wrapper.h"
#include "grid_phase.h"
#include "guardians.h"
#include "spaceship.h"

/** Maximum number of bezier curves loaded at startup */
const Sint32 CURVES_BEZIER_NUMOF = 122;
/** All bezier curves loaded at startup */
curve *initial_curve = NULL;
/** Current curve phase level data structure */
curve_level courbe;
#ifdef DEVELOPPEMENT
/* curve editor: current curve number */
static Sint16 curv_number = 0;
/* curve editor: [l] key pressed  */
static bool curve_l_key_down;
/* curve editor: [s] key pressed  */
static bool curve_s_key_down;
short ge_act_pos_x = 0, ge_act_pos_y = 0;
short ce_vais_act = 0;
#endif
static bool curves_load_all (void);
static void curve_find_numof_enemies (void);

/**
 * Allocate buffers and initialize structure of the curve phase 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
curve_once_init (void)
{
  curve_free ();

  /* allocate curve data structure */
  if (initial_curve == NULL)
    {
      initial_curve =
        (curve *) memory_allocation (CURVES_BEZIER_NUMOF * sizeof (curve));
      if (initial_curve == NULL)
        {
          LOG_ERR ("'initial_curve' out of memory");
          return FALSE;
        }
    }

  if (!curves_load_all ())
    {
      return FALSE;
    }
  return TRUE;
}

/**
 * Release memory used for the curve 
 */
void
curve_free (void)
{
  if (initial_curve != NULL)
    {
      free_memory ((char *) initial_curve);
      initial_curve = NULL;
    }
}

/**
 * Phase 1: handle curves (little skirmish)
 */
void
curve_phase (void)
{
  Sint32 i, k;
  enemy *foe;
  spaceship_struct *ship = spaceship_get ();
  if (!courbe.activity)
    {
      return;
    }

  /* we process each curve one by one */
  for (i = 0; i < courbe.total_numof_curves; i++)
    {
      /* new enemy vessel appear on the curve? */
      courbe.count_before_next[i]++;
      if (courbe.current_numof_enemies[i] >= courbe.total_numof_enemies[i]
          || courbe.count_before_next[i] <= courbe.delay_before_next[i])
        {
          continue;
        }

      /* add new a enemy sprite to the list */
      foe = enemy_get ();
      if (foe == NULL)
        {
          return;
        }

      /* set power of destruction */
      foe->spr.pow_of_dest =
        (Sint16) (1 +
                  courbe.num_vaisseau[i][courbe.current_numof_enemies[i]]);
      /* set energy level */
      foe->spr.energy_level =
        (Sint16) ((ship->type << 1) + foe->spr.pow_of_dest);
      /* set number of images for animation */
      foe->spr.numof_images = (Sint16) 32;
      /* set current image */
      foe->spr.current_image = initial_curve[courbe.num_courbe[i]].angle[0];
      /* set addresses of the images buffer */
      for (k = 0; k < foe->spr.numof_images; k++)
        {
          foe->spr.img[k] =
            (image *) &
            enemi[courbe.num_vaisseau[i][courbe.current_numof_enemies[i]]][k];
        }
      /* set shot time-frequency */
      foe->fire_rate = courbe.freq_tir[i][courbe.current_numof_enemies[i]];
      /* set counter of delay between two shots */
      foe->fire_rate_count = foe->fire_rate;

      /* set type of displacement */
      foe->displacement = DISPLACEMENT_CURVE;

      /* set x and y coordinates */
      foe->spr.xcoord =
        (float) (initial_curve[courbe.num_courbe[i]].pos_x + 128 - 32);
      foe->spr.ycoord =
        (float) (initial_curve[courbe.num_courbe[i]].pos_y + 128 - 32);

      /* clear index on the precalculated bezier curve  */
      foe->pos_vaiss[POS_CURVE] = 0;
      /* set curve number used */
      foe->num_courbe = courbe.num_courbe[i];

      /* set size of the sprite as 16x16 pixels */
      foe->type = 0;

      /* increase number of the enemies on the curve */
      courbe.current_numof_enemies[i]++;
      /* clear counter delay before appearance of the next enemy  */
      courbe.count_before_next[i] = 0;
    }
}

/** 
 * Test if curve phase is finished 
 */
void
curve_finished (void)
{
  Sint32 i;
  /* TRUE = all enemies are dead */
  bool alldead;

  /* curve phase enable? */
  if (courbe.activity)
    {
      alldead = TRUE;
      for (i = 0; i < courbe.total_numof_curves; i++)
        {
          if (courbe.current_numof_enemies[i] < courbe.total_numof_enemies[i])
            {
              /* there is still at least enemy */
              alldead = FALSE;
            }
        }

      /* move to next phase of the game */
      if (alldead)              /* no enemy is present? */
        {
          /* initialize the grid phase */
          grid_start ();
          courbe.activity = FALSE;
          meteor_activity = FALSE;
          guardian->number = 0;
          /* grid phase enable */
          grid.is_enable = TRUE;
        }
    }
}

/*
 * curve editor
 */
#ifdef DEVELOPPEMENT
void
courbe_editeur ()
{
  Sint32 i, j, tmp_tsts_x, tmp_tsts_y, handle;
  if (!courbe.total_numof_curves)
    {
      courbe.total_numof_curves = 1;
      for (i = 0; i < CURVES_MAX_OF; i++)
        {
          courbe.delay_before_next[i] = 10;
        }
    }

  /* draw grid lines */
  for (i = 128; i <= (128 + LARG_GRILLE * 15); i += 16)
    {
      if (courbe.total_numof_curves)
        line_v (game_offscreen + ((128 * offscreen_width) + i), 512,
                (Sint16) (16 * courbe.total_numof_curves), coulor[GRIS]);
    }
  for (i = 128; i <= (128 + courbe.total_numof_curves * 16); i += 16)
    {
      line_h (game_offscreen + ((i * offscreen_width) + 128), 256,
              coulor[GRIS]);
    }

  /* set mouse cursor x and y coordinates */
  mouse_x -= 128;
  mouse_y -= 128;

  if (!keys_down[K_F] && !keys_down[K_T] && !keys_down[K_N] && !keys_down[K_B]
      && !keys_down[K_V] && !keys_down[K_A])
    {
      /* set grid cursor x and y coordinates */
      ge_act_pos_x = (Sint16) (mouse_x >> 4);
      ge_act_pos_y = (Sint16) (mouse_y >> 4);
    }

  /* set mouse cursor x and y coordinates */
  mouse_x += 128;
  mouse_y += 128;

  /* check minimum and maximum coordinates of the grid's cell selected */
  if (ge_act_pos_x < 0)
    {
      ge_act_pos_x = 0;
    }
  if (ge_act_pos_x > (LARG_GRILLE - 1))
    {
      ge_act_pos_x = LARG_GRILLE - 1;
    }
  if (ge_act_pos_y < 0)
    {
      ge_act_pos_y = 0;
    }
  if (ge_act_pos_y > (courbe.total_numof_curves - 1))
    {
      ge_act_pos_y = (Sint16) (courbe.total_numof_curves - 1);
    }
  /* display current curve number */
  ltoa ((Sint32) curv_number, chaine, 10);
  textxy ("curv_number :", 128, 194 + 128 - 32, coulor[LIGHT_GRAY], 0,
          game_offscreen, offscreen_width);
  textxy ("c    n       ", 128, 194 + 128 - 32, coulor[RED], -1,
          game_offscreen, offscreen_width);
  textxy (chaine, 86 + 128 - 32, 194 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display current enemy vessel number */
  textxy ("ce_vais_act :", 128, 201 + 128 - 32, coulor[LIGHT_GRAY], 0,
          game_offscreen, offscreen_width);
  textxy (" e v         ", 128, 201 + 128 - 32, coulor[RED], -1,
          game_offscreen, offscreen_width);
  itoa ((Sint32) ce_vais_act, chaine, 10);
  textxy (chaine, 86 + 128 - 32, 201 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display rate of fire of the enemy */
  ltoa ((Sint32) courbe.freq_tir[ge_act_pos_y][ge_act_pos_x], chaine, 10);
  textxy ("ce_freq_tir :", 128, 208 + 128 - 32, coulor[LIGHT_GRAY], 0,
          game_offscreen, offscreen_width);
  textxy ("   f    t    ", 128, 208 + 128 - 32, coulor[RED], -1,
          game_offscreen, offscreen_width);
  textxy (chaine, 86 + 128 - 32, 208 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display current coordinates of grid cursor selected */
  textxy ("ge_act_pos :", 100 + 128 - 32, 194 + 128 - 32, coulor[LIGHT_GRAY],
          0, game_offscreen, offscreen_width);
  ltoa ((Sint32) ge_act_pos_x, chaine, 10);
  textxy (chaine, 154 + 128 - 32, 194 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  ltoa ((Sint32) ge_act_pos_y, chaine, 10);
  textxy (chaine, 162 + 128 - 32, 194 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display number of curves used for this level */
  textxy ("total_numof_curves :", 100 + 128 - 32, 201 + 128 - 32,
          coulor[LIGHT_GRAY], 0, game_offscreen, offscreen_width);
  textxy ("    t   c        ", 100 + 128 - 32, 201 + 128 - 32, coulor[RED],
          -1, game_offscreen, offscreen_width);
  ltoa ((Sint32) (courbe.total_numof_curves), chaine, 10);
  textxy (chaine, 170 + 128 - 32, 201 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display curve number used for this level */
  textxy ("num_courbe :", 100 + 128 - 32, 208 + 128 - 32, coulor[LIGHT_GRAY],
          0, game_offscreen, offscreen_width);
  textxy ("n       b   ", 100 + 128 - 32, 208 + 128 - 32, coulor[RED], -1,
          game_offscreen, offscreen_width);
  ltoa ((Sint32) (courbe.num_courbe[ge_act_pos_y]), chaine, 10);
  textxy (chaine, 154 + 128 - 32, 208 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display current coordinates of grid cursor selected */
  textxy ("total_numof_enemies :", 178 + 128 - 32, 194 + 128 - 32,
          coulor[LIGHT_GRAY], 0, game_offscreen, offscreen_width);
  ltoa ((Sint32) courbe.total_numof_enemies[ge_act_pos_y], chaine, 10);
  textxy (chaine, 258 + 128 - 32, 194 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display time delay before next enemy */
  textxy ("delay_before_next :", 178 + 128 - 32, 201 + 128 - 32,
          coulor[LIGHT_GRAY], 0, game_offscreen, offscreen_width);
  textxy ("    v   a           ", 178 + 128 - 32, 201 + 128 - 32,
          coulor[RED], -1, game_offscreen, offscreen_width);
  ltoa ((Sint32) (courbe.delay_before_next[ge_act_pos_y]), chaine, 10);
  textxy (chaine, 262 + 128 - 32, 201 + 128 - 32, coulor[WHITE], 0,
          game_offscreen, offscreen_width);
  /* display current coordinates of grid cursor selected */
  line_v (game_offscreen +
          ((((ge_act_pos_y << 4) + 128) * offscreen_width) +
           ((ge_act_pos_x << 4) + 128)), offscreen_width, 16, coulor[RED]);
  line_v (game_offscreen +
          ((((ge_act_pos_y << 4) + 128) * offscreen_width) +
           ((ge_act_pos_x << 4) + 128 + 16)), offscreen_width, 16,
          coulor[RED]);
  line_h (game_offscreen +
          ((((ge_act_pos_y << 4) + 128) * offscreen_width) +
           ((ge_act_pos_x << 4) + 128)), 16, coulor[RED]);
  line_h (game_offscreen +
          ((((ge_act_pos_y << 4) + 128 + 16) * offscreen_width) +
           ((ge_act_pos_x << 4) + 128)), 16, coulor[RED]);
  /* find total number of enemies on the current curve */
  curve_find_numof_enemies ();
  /* draw all enemies on the curve */
  for (i = 0; i < courbe.total_numof_curves; i++)
    {
      for (j = 0; j < courbe.total_numof_enemies[i]; j++)
        {
          draw_sprite (enemi[courbe.num_vaisseau[i][j]][15].img,
                       game_offscreen + ((128 + i * 16) * offscreen_width) +
                       (128 + 16 * j),
                       enemi[courbe.num_vaisseau[i][j]][15].compress,
                       (Sint16) (enemi[courbe.num_vaisseau[i][j]]
                                 [15].nbr_data_comp >> 2),
                       "enemi[courbe.num_vaisseau[i][j]][15].img");
        }
    }

  /* draw the cureen curve */
  tmp_tsts_x =
    initial_curve[courbe.num_courbe[ge_act_pos_y]].pos_x + 128 - 32;
  tmp_tsts_y =
    initial_curve[courbe.num_courbe[ge_act_pos_y]].pos_y + 128 - 32;
  for (i = 0;
       i < initial_curve[courbe.num_courbe[ge_act_pos_y]].nbr_pnt_curve; i++)
    {
      if (tmp_tsts_x >= 0 && tmp_tsts_x < offscreen_width && tmp_tsts_y >= 0
          && tmp_tsts_y < offscreen_height)
        put_pixel (game_offscreen, tmp_tsts_x, tmp_tsts_y, coulor[GREEN]);
      tmp_tsts_x += initial_curve[courbe.num_courbe[ge_act_pos_y]].delta_x[i];
      tmp_tsts_y += initial_curve[courbe.num_courbe[ge_act_pos_y]].delta_y[i];
    }
  /* save a level curve */
  if (keys_down[K_S] && keys_down[K_A] && !curve_s_key_down)
    {
      strcpy (str_file, "data/levels/curves_phase/curves_");
      itoa ((Sint32) curv_number, str_tmp, 10);
      strcat (str_file, str_tmp);
      _fmode = O_BINARY;
      if ((handle = _lcreat (str_file, NULL)) != -1)
        {
          _lwrite (handle, (const char *) &courbe.total_numof_curves,
                   sizeof (Sint16));
          _lwrite (handle, (const char *) courbe.num_courbe,
                   sizeof (Sint16) * CURVES_MAX_OF);
          _lwrite (handle, (const char *) courbe.total_numof_enemies,
                   sizeof (Sint16) * CURVES_MAX_OF);
          _lwrite (handle, (const char *) courbe.delay_before_next,
                   sizeof (Sint16) * CURVES_MAX_OF);
          _lwrite (handle, (const char *) courbe.num_vaisseau,
                   sizeof (Sint16) * CURVES_MAX_OF * NUM_VAI_BY_CURVE);
          _lwrite (handle, (const char *) courbe.freq_tir,
                   sizeof (Sint16) * CURVES_MAX_OF * NUM_VAI_BY_CURVE);
          _lclose (handle);
        }
      else
        {
          LOG_ERR ("Cannot create curve file!");
        }
    }
  curve_s_key_down = keys_down[K_S];

  /* load and enable a curve level */
  if (keys_down[K_L] && keys_down[K_O] && !curve_l_key_down)
    {
      curve_load_level (curv_number);
      curve_enable_level ();
    }
  curve_l_key_down = keys_down[K_L];

  if (mouse_b)
    {
      /* change the curve number used for this level */
      if (keys_down[K_C] && keys_down[K_N])
        {
          curv_number = (Sint16) (mouse_y - 128 + mouse_x - 128);
          /* check range limits */
          if (curv_number < 0)
            {
              curv_number = 0;
            }
          goto fin_curv_editor;
        }

      /* select a new enemy */
      if (keys_down[K_V] && keys_down[K_E])
        {
          ce_vais_act = (Sint16) (mouse_x - 128);
          /* check range limits */
          if (ce_vais_act > (ENEMIES_MAX_SMALL_TYPES - 1))
            {
              ce_vais_act = (ENEMIES_MAX_SMALL_TYPES - 1);
            }
          if (ce_vais_act < 0)
            {
              ce_vais_act = 0;
            }
          goto fin_curv_editor;
        }
      /* modify the delay between two shots */
      if (keys_down[K_F] && keys_down[K_T])
        {
          courbe.freq_tir[ge_act_pos_y][ge_act_pos_x] =
            (Sint16) ((mouse_y - 128) * 4 + mouse_x - 128);
          /* check range limits */
          if (courbe.freq_tir[ge_act_pos_y][ge_act_pos_x] < 0)
            {
              courbe.freq_tir[ge_act_pos_y][ge_act_pos_x] = 0;
            }
          goto fin_curv_editor;
        }
      /* modify the number of curves for this level */
      if (keys_down[K_C] && keys_down[K_T])
        {
          courbe.total_numof_curves =
            (Sint16) (mouse_y - 128 + mouse_x - 128);
          /* check range limits */
          if (courbe.total_numof_curves < 1)
            {
              courbe.total_numof_curves = 1;
            }
          if (courbe.total_numof_curves > CURVES_MAX_OF)
            {
              courbe.total_numof_curves = CURVES_MAX_OF;
            }
          goto fin_curv_editor;
        }

      /* modify the curve number used for this level */
      if (keys_down[K_N] && keys_down[K_B])
        {
          courbe.num_courbe[ge_act_pos_y] =
            (Sint16) (mouse_y - 128 + mouse_x - 128);
          /* check range limits */
          if (courbe.num_courbe[ge_act_pos_y] < 0)
            {
              courbe.num_courbe[ge_act_pos_y] = 0;
            }
          if (courbe.num_courbe[ge_act_pos_y] > (CURVES_BEZIER_NUMOF - 1))
            {
              courbe.num_courbe[ge_act_pos_y] = CURVES_BEZIER_NUMOF - 1;
            }
          goto fin_curv_editor;
        }

      /* modify time delay before appearance of the next enemy */
      if (keys_down[K_V] && keys_down[K_A])
        {
          courbe.delay_before_next[ge_act_pos_y] =
            (Sint16) ((mouse_y - 128) * 4 + mouse_x - 128);
          /* check range limits */
          if (courbe.delay_before_next[ge_act_pos_y] < 10)
            {
              courbe.delay_before_next[ge_act_pos_y] = 10;
            }
          return;
        }

      /* right mouse down? */
      if (mouse_b == 2)
        {
          /* clear grid's cells */
          courbe.num_vaisseau[ge_act_pos_y][ge_act_pos_x] = -1;
          courbe.freq_tir[ge_act_pos_y][ge_act_pos_x] = 0;
        }
      else
        {
          courbe.num_vaisseau[ge_act_pos_y][ge_act_pos_x] = ce_vais_act;
          courbe.freq_tir[ge_act_pos_y][ge_act_pos_x] = 210;
        }
    }
fin_curv_editor:;
}
#endif

/**
 * Load all curves files 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
curves_load_all (void)
{
  Sint32 i, j;
  char *fcurve;
  signed char *ptr8;
  Sint16 *ptr16;
  for (i = 0; i < CURVES_BEZIER_NUMOF; i++)
    {
      fcurve = loadfile_num ("data/curves/bezier_curve_%03d.bin", i);
      if (fcurve == NULL)
        {
          return FALSE;
        }
      /* 16-bit access */
      ptr16 = (Sint16 *) fcurve;
      /* number of points of the curve */
      initial_curve[i].nbr_pnt_curve = little_endian_to_short (ptr16++);
      /* start x coordinate */
      initial_curve[i].pos_x = little_endian_to_short (ptr16++);
      /* start y coordinate */
      initial_curve[i].pos_y = little_endian_to_short (ptr16++);
      /* 8-bit access */
      ptr8 = (signed char *) ptr16;
      for (j = 0; j < initial_curve[i].nbr_pnt_curve; j++)
        {
          initial_curve[i].delta_x[j] = *(ptr8++);
          initial_curve[i].delta_y[j] = *(ptr8++);
          initial_curve[i].angle[j] = *(ptr8++);
        }
      free_memory (fcurve);
    }
  return TRUE;
}

/**
 * load a level curve file and copy data into curve structure
 * @param level_num level number from 0 to 41
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
curve_load_level (Sint32 level_num)
{
  char *level_data;
  Sint16 *source;
  Sint16 *dest;
  Sint32 i;

  /* load the file of the level curve */
  if (level_num > MAX_NUM_OF_LEVELS || level_num < 0)
    {
      level_num = 0;
    }
  level_data =
    loadfile_num ("data/levels/curves_phase/curves_%02d.bin", level_num);
  if (level_data == NULL)
    {
      return FALSE;
    }
  source = (Sint16 *) level_data;

  /* read the number of curves for this level */
  courbe.total_numof_curves = little_endian_to_short (source++);

  /* read the curves numbers used for this level */
  for (i = 0; i < CURVES_MAX_OF; i++)
    {
      courbe.num_courbe[i] = little_endian_to_short (source++);
    }

  /* read the number of enemies on each curve */
  for (i = 0; i < CURVES_MAX_OF; i++)
    {
      courbe.total_numof_enemies[i] = little_endian_to_short (source++);
    }
  /* read time delay before appearance of the next enemy  */
  for (i = 0; i < CURVES_MAX_OF; i++)
    {
      courbe.delay_before_next[i] = little_endian_to_short (source++);
    }

  /* read the numbers of enemies used on each curve */
  dest = &courbe.num_vaisseau[0][0];
  for (i = 0; i < CURVES_MAX_OF * NUM_VAI_BY_CURVE; i++)
    {
      *(dest++) = little_endian_to_short (source++);
    }

  /* read the delay between two shots */
  dest = &courbe.freq_tir[0][0];
  for (i = 0; i < CURVES_MAX_OF * NUM_VAI_BY_CURVE; i++)
    {
      *(dest++) = little_endian_to_short (source++);
    }
  free_memory (level_data);
  return TRUE;
}

/**
 * Initialize and enable a curve phase
 */
void
curve_enable_level (void)
{
  Sint32 i;
#ifdef DEVELOPPEMENT
  enemy *foe;
  Sint32 j;
  spaceship_struct *ship = spaceship_get ();
#endif

  /* search the total number of enemies */
  curve_find_numof_enemies ();
  for (i = 0; i < courbe.total_numof_curves; i++)
    {
      /* set the number of enemies */
      courbe.current_numof_enemies[i] = 0;
      /* clear counter delay before appearance of the next enemy */
      courbe.count_before_next[i] = 0;

#ifdef DEVELOPPEMENT
      if (!curve_editor_enable)
        {
          continue;
        }
      /* add new a enemy sprite to the list */
      foe = enemy_get ();
      if (foe == NULL)
        {
          return;
        }
      /* set power of destruction */
      foe->spr.pow_of_dest =
        (Sint16) (5 + 1 +
                  courbe.num_vaisseau[i][courbe.current_numof_enemies[i]]);
      /* set energy level */
      foe->spr.energy_level =
        (Sint16) ((ship->type << 1) + foe->spr.pow_of_dest - 5);
      /* set size of the sprite as 16x16 pixels */
      foe->type = 0;
      /* set number of images for animation */
      foe->spr.numof_images = 32;
      /* set current image */
      foe->spr.current_image = initial_curve[courbe.num_courbe[i]].angle[0];
      /* set addresses of the images buffer */
      for (j = 0; j < foe->spr.numof_images; j++)
        {
          foe->spr.img[j] =
            (image *) &
            enemi[courbe.num_vaisseau[i][courbe.nbr_vaisseaux_act[i]]][j];
        }
      /* set shot time-frequency */
      foe->fire_rate = courbe.freq_tir[i][courbe.current_numof_enemies[i]];
      /* set counter of delay between two shots */
      foe->fire_rate_count = foe->fire_rate;
      /* set type of displacement */
      foe->displacement = DISPLACEMENT_CURVE;
      /* set x and y coordinates */
      foe->spr.xcoord =
        (float) (initial_curve[courbe.num_courbe[i]].pos_x + 128 - 32);
      foe->spr.ycoord =
        (float) (initial_curve[courbe.num_courbe[i]].pos_y + 128 - 32);
      /* clear index on the precalculated bezier curve  */
      foe->pos_vaiss[POS_CURVE] = 0;
      /* set curve number used */
      foe->num_courbe = courbe.num_courbe[i];
      /* increase number of the enemies on the curve */
      courbe.current_numof_enemies[i]++;
#endif
    }
}

/**
 * Fin the total number of enemies on a curve 
 */
void
curve_find_numof_enemies (void)
{
  Sint32 i, j;

find_num_of_enemies:;
  for (i = 0; i < courbe.total_numof_curves; i++)
    {
      courbe.total_numof_enemies[i] = 0;
      for (j = 0; j < NUM_VAI_BY_CURVE; j++)
        {
          /* enemy exist. */
          if (courbe.num_vaisseau[i][j] != -1)
            {
              /* increase the number of enemies */
              courbe.total_numof_enemies[i]++;
              /* previous curve is empty? */
              if (j && courbe.num_vaisseau[i][j - 1] == -1)
                {
                  courbe.num_vaisseau[i][j - 1] = courbe.num_vaisseau[i][j];
                  courbe.freq_tir[i][j - 1] = courbe.freq_tir[i][j];
                  courbe.num_vaisseau[i][j] = -1;
                  courbe.freq_tir[i][j] = 0;
                  goto find_num_of_enemies;
                }
            }
        }
    }
}
