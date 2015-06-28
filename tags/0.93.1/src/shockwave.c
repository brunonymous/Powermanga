/** 
 * @file shockwave.c
 * @brief  handle powerful circular shock wave propagated by spaceship 
 * @created 1999-09-05 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Etienne Sobole
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: shockwave.c,v 1.21 2012/08/25 19:18:32 gurumeditation Exp $
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
#include "assembler.h"
#include "images.h"
#include "config_file.h"
#include "display.h"
#include "enemies.h"
#include "bonus.h"
#include "log_recorder.h"
#include "shots.h"
#include "sdl_mixer.h"
#include "shockwave.h"
#include "spaceship.h"

/** Number of colors used by shockwave */
#define NUMOF_COLORS_SHOCKWAVE 25
/** Maximum number of shockwaves at the same time*/
#define MAX_NUMOF_SHOCKWAVES 20
/** Maximum number of rings that comprise a shockwave */
const Sint32 NUMOF_RINGS_SHOCKWAVE = 55;
/** Maximum number of points that comprise a shockwave */
const Sint32 NUMOF_POINTS_SHOCKWAVE = 129;
/** Circular shockwave structure */
typedef struct shockwave_struct
{
  /** Circular shock wave number */
  Sint32 ring_index;
  /** X-coordinate of circular shock wave center */
  Sint32 center_x;
  /** Y-coordinate of circular shock wave center */
  Sint32 center_y;
  /** Color palette index */
  Sint32 color_index;
  /** Previous element of the chained list */
  struct shockwave_struct *previous;
  /** Next element of the chained list */
  struct shockwave_struct *next;
  /** TRUE if the element is already chained */
  bool is_enabled;
} shockwave_struct;
static shockwave_struct *shockwave_first = NULL;
static shockwave_struct *shockwave_last = NULL;

/** Data structure of the shockwaves */
static shockwave_struct *shockwave = NULL;
/** List of indexes on the shockwaves data structure */

/** Current number of shockwaves */
static Sint32 num_of_shockwaves;
/** Colors table of shockwave */
unsigned char shockwave_colors[NUMOF_COLORS_SHOCKWAVE + 1];
/** Coordinates of a polygon */
static Sint32 coords[10];
/** Buffer to draw a polygon */
static Sint32 *shockwave_left_buffer = NULL;
static Sint32 *shockwave_right_buffer = NULL;
/** 55 rings precalculated */
static Sint16 *shockwave_ring_x = NULL;
static Sint16 *shockwave_ring_y = NULL;

static shockwave_struct *shockwave_get (void);
static void shockwave_del (shockwave_struct * shock);
static void draw_polygon (char *, Sint32 *, Sint32 numofpts, Uint32);

/**
 * Allocate buffers and precalcule the rings
 * @return TRUE if it completed successfully or 0 otherwise
 */
bool
shockwave_once_init (void)
{
  double a, step, pi;
  Sint32 i, j, n, r;
  Sint16 x, y;


  /* allocate shockwaves data structure */
  if (shockwave == NULL)
    {
      shockwave =
        (shockwave_struct *) memory_allocation (MAX_NUMOF_SHOCKWAVES *
                                                sizeof (shockwave_struct));
      if (shockwave == NULL)
        {
          LOG_ERR ("shockwave out of memory");
          return FALSE;
        }
    }
  if (shockwave_right_buffer == NULL)
    {
      shockwave_right_buffer =
        (Sint32 *) memory_allocation (offscreen_height * sizeof (Sint32));
      if (shockwave_right_buffer == NULL)
        {
          LOG_ERR ("shockwave_right_buffer out of memory");
          return FALSE;
        }

    }
  if (shockwave_left_buffer == NULL)
    {
      shockwave_left_buffer =
        (Sint32 *) memory_allocation (offscreen_height * sizeof (Sint32));
      if (shockwave_left_buffer == NULL)
        {
          LOG_ERR ("shockwave_left_buffer out of memory");
          return FALSE;
        }

    }

  if (shockwave_ring_x == NULL)
    {
      shockwave_ring_x =
        (Sint16 *) memory_allocation (NUMOF_RINGS_SHOCKWAVE *
                                      NUMOF_POINTS_SHOCKWAVE * 2 *
                                      sizeof (Sint16));
      if (shockwave_ring_x == NULL)
        {
          LOG_ERR ("'shockwave_ring_x' out of memory");
          return FALSE;
        }
      shockwave_ring_y =
        shockwave_ring_x + NUMOF_RINGS_SHOCKWAVE * NUMOF_POINTS_SHOCKWAVE;
    }

  /* set colors of the shockwaves */
  shockwave_colors[0] = search_color (255, 255, 0);
  shockwave_colors[1] = search_color (248, 248, 0);
  shockwave_colors[2] = search_color (241, 241, 0);
  shockwave_colors[3] = search_color (234, 234, 0);
  shockwave_colors[4] = search_color (227, 227, 0);
  shockwave_colors[5] = search_color (220, 220, 0);
  shockwave_colors[6] = search_color (213, 213, 0);
  shockwave_colors[7] = search_color (206, 206, 0);
  shockwave_colors[8] = search_color (199, 199, 0);
  shockwave_colors[9] = search_color (192, 192, 0);
  shockwave_colors[10] = search_color (185, 185, 0);
  shockwave_colors[11] = search_color (178, 178, 0);
  shockwave_colors[12] = search_color (171, 171, 0);
  shockwave_colors[13] = search_color (164, 164, 0);
  shockwave_colors[14] = search_color (157, 157, 0);
  shockwave_colors[15] = search_color (150, 150, 0);
  shockwave_colors[16] = search_color (143, 143, 0);
  shockwave_colors[17] = search_color (136, 136, 0);
  shockwave_colors[18] = search_color (129, 129, 0);
  shockwave_colors[19] = search_color (122, 122, 0);
  shockwave_colors[20] = search_color (115, 115, 0);
  shockwave_colors[21] = search_color (108, 108, 0);
  shockwave_colors[22] = search_color (101, 101, 0);
  shockwave_colors[23] = search_color (94, 94, 0);
  shockwave_colors[24] = search_color (87, 87, 0);

  /* 
   * precalculate the 55 rings (rings are circles) 
   */
  n = NUMOF_POINTS_SHOCKWAVE;
  pi = 4 * atan (1.0);
  /* ring radius: 30 to 300 pixels */
  r = 30 * pixel_size;
  step = (pi * 2) / (NUMOF_POINTS_SHOCKWAVE - 1);
  for (i = 0; i < NUMOF_RINGS_SHOCKWAVE; i++, r += 5)
    {
      /* clear angle value */
      a = 0.0;
      for (j = 0; j < (NUMOF_POINTS_SHOCKWAVE - 1); j++)
        {
          x = (Sint16) (cos (a) * r);
          y = (Sint16) (sin (a) * r);
          a = a + step;
          shockwave_ring_x[i * n + j] = x;
          shockwave_ring_y[i * n + j] = y;
        }
      /* the first and last point are equal to get a closer ring */
      shockwave_ring_x[i * n + j] = shockwave_ring_x[i * n];
      shockwave_ring_y[i * n + j] = shockwave_ring_y[i * n];
    }
  shockwave_init ();
  return TRUE;
}

/**
 * Release memory used for the shockwaves
 */
void
shockwave_free (void)
{
  LOG_DBG ("deallocates the memory used by the shockwaves");
  if (shockwave != NULL)
    {
      free_memory ((char *) shockwave);
      shockwave = NULL;
    }
  if (shockwave_right_buffer != NULL)
    {
      free_memory ((char *) shockwave_right_buffer);
      shockwave_right_buffer = NULL;
    }
  if (shockwave_left_buffer != NULL)
    {
      free_memory ((char *) shockwave_left_buffer);
      shockwave_left_buffer = NULL;
    }
  if (shockwave_ring_x != NULL)
    {
      free_memory ((char *) shockwave_ring_x);
      shockwave_ring_x = NULL;
      shockwave_ring_y = NULL;
    }
}

/**
 * Draw powerful circular shockwave propagated by the player spaceship
 */
void
shockwave_draw (void)
{
  /* at least a polygon is visibile */
  bool visible;
  shockwave_struct *shock;
  Sint32 ring, centerx, centery;
  Sint32 i, j, n;
  n = NUMOF_POINTS_SHOCKWAVE;

  shock = shockwave_first;
  if (shock == NULL)
    {
      return;
    }

  /* process each shockwave */
  for (i = 0; i < num_of_shockwaves; i++, shock = shock->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (shock == NULL && i < (num_of_shockwaves - 1))
        {
          LOG_ERR ("shock->next is null %i/%i", i, num_of_shockwaves);
          break;
        }
#endif
      /* no polygon is visible */
      visible = FALSE;
      /* there remain rings? */
      if ((shock->ring_index + 2) <= (NUMOF_RINGS_SHOCKWAVE - 2))
        {
          ring = shock->ring_index;
          centerx = shock->center_x;
          centery = shock->center_y;

          /* each polygon of the shockwave is processed */
          for (j = 0; j < (NUMOF_POINTS_SHOCKWAVE - 1); j++)
            {
              /* fill coordinates table */
              coords[2] = shockwave_ring_x[ring * n + j] + centerx;
              coords[3] = shockwave_ring_y[ring * n + j] + centery;
              /* test if the polygon is visible or not */
              if (coords[2] >= (offscreen_startx - 5)
                  && coords[2] <=
                  (offscreen_startx + offscreen_width_visible + 5)
                  && coords[3] >= (offscreen_starty - 5)
                  && coords[3] <=
                  (offscreen_starty + offscreen_height_visible + 5))
                {
                  coords[0] = shockwave_ring_x[(ring + 2) * n + j] + centerx;
                  coords[1] = shockwave_ring_y[(ring + 2) * n + j] + centery;
                  coords[4] = shockwave_ring_x[ring * n + j + 1] + centerx;
                  coords[5] = shockwave_ring_y[ring * n + j + 1] + centery;
                  coords[6] =
                    shockwave_ring_x[(ring + 2) * n + j + 1] + centerx;
                  coords[7] =
                    shockwave_ring_y[(ring + 2) * n + j + 1] + centery;
                  coords[8] = coords[0];
                  coords[9] = coords[1];

                  /* draw the polygon */
                  draw_polygon (game_offscreen, coords, 4,
                                shockwave_colors[shock->color_index]);
                  /* at least a polygon is visibile */
                  visible = TRUE;
                }
            }

          /* is a polygon at least visible? */
          if (!visible)
            {
              /* remove a shockwave element from list */
              shockwave_del (shock);
            }
        }
      /* more no ring is available, the shockwave must be removed */
      else
        {
          /* remove a shockwave element from list */
          shockwave_del (shock);
        }
      if (!player_pause)
        {
          /* proceed to next shockwave's ring */
          shock->ring_index++;
          /* proceed to next color */
          shock->color_index++;
          if (shock->color_index >= (NUMOF_COLORS_SHOCKWAVE - 1))
            {
              shock->color_index = NUMOF_COLORS_SHOCKWAVE - 1;
            }
        }
    }
}

/** 
 * Add a circular shockwave from the player's spaceship 
 */
void
shockwave_add (void)
{
  shockwave_struct *shock;
  spaceship_struct *ship = spaceship_get ();
  shock = shockwave_get ();
  if (shock == NULL)
    {
      return;
    }
  shock->ring_index = 0;
  shock->color_index = 0;
  shock->center_x =
    (Sint32) (ship->spr.xcoord +
              ship->spr.img[ship->spr.current_image]->x_gc);
  shock->center_y =
    (Sint32) (ship->spr.ycoord +
              ship->spr.img[ship->spr.current_image]->y_gc);
#ifdef USE_SDLMIXER
  sound_play (SOUND_CIRCULAR_SHOCK);
#endif
}

/**
 * Check validty of enemies chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
shockwave_check_chained_list (void)
{
  Uint32 i;
  shockwave_struct *shock;
  Sint32 count = 0;
  for (i = 0; i < MAX_NUMOF_SHOCKWAVES; i++)
    {
      shock = &shockwave[i];
      if (shock->is_enabled)
        {
          count++;
        }
    }
  if (count != num_of_shockwaves)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, num_of_shockwaves=%i", count, num_of_shockwaves);
    }
  count = 0;
  shock = shockwave_first;
  do
    {
      count++;
      shock = shock->next;
    }
  while (shock != NULL && count <= (MAX_NUMOF_SHOCKWAVES + 1));
  if (count != num_of_shockwaves)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, num_of_shockwaves=%i", count, num_of_shockwaves);
    }
  count = 0;
  shock = shockwave_last;
  do
    {
      count++;
      shock = shock->previous;
    }
  while (shock != NULL && count <= (MAX_NUMOF_SHOCKWAVES + 1));
  if (count != num_of_shockwaves)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, num_of_shockwaves=%i", count, num_of_shockwaves);
    }
}
#endif

/** 
 * Return a free shockwave element 
 * @return Pointer to a shockwave structure 
 */
static shockwave_struct *
shockwave_get (void)
{
  Uint32 i;
  shockwave_struct *shock;
  for (i = 0; i < MAX_NUMOF_SHOCKWAVES; i++)
    {
      shock = &shockwave[i];
      if (shock->is_enabled)
        {
          continue;
        }
      shock->is_enabled = TRUE;
      shock->next = NULL;
      if (num_of_shockwaves == 0)
        {
          shockwave_first = shock;
          shockwave_last = shock;
          shockwave_last->previous = NULL;
        }
      else
        {
          shockwave_last->next = shock;
          shock->previous = shockwave_last;
          shockwave_last = shock;
        }
      num_of_shockwaves++;
#ifdef UNDER_DEVELOPMENT
      shockwave_check_chained_list ();
#endif
      return shock;
    }
  LOG_WARN ("no more element shockwave is available");
  return NULL;
}


/** 
 * Remove a shockwave element from list
 * @param Pointer to a shockwave structure 
 */
static void
shockwave_del (shockwave_struct * shock)
{
  shock->is_enabled = FALSE;
  num_of_shockwaves--;
  if (shockwave_first == shock)
    {
      shockwave_first = shock->next;
    }
  if (shockwave_last == shock)
    {
      shockwave_last = shock->previous;
    }
  if (shock->previous != NULL)
    {
      shock->previous->next = shock->next;
    }
  if (shock->next != NULL)
    {
      shock->next->previous = shock->previous;
    }
}

/** 
 * Collision between shockwave and an enemy 
 * @param enemy_num enemy element index
 * @return TRUE if collision occurs, otherwise FALSE
 */
bool
shockwave_collision (enemy * foe)
{
  shockwave_struct *shock;
  Sint32 i, centerx, centery, dist;
  Sint32 dx, dy, resultat;
  shock = shockwave_first;
  if (shock == NULL)
    {
      return FALSE;
    }
  if (num_of_shockwaves == 0)
    {
      return FALSE;
    }
  centerx =
    (Sint32) (foe->spr.xcoord + foe->spr.img[foe->spr.current_image]->x_gc);
  centery =
    (Sint32) (foe->spr.ycoord + foe->spr.img[foe->spr.current_image]->y_gc);
  /* process each shockwave */
  for (i = 0; i < num_of_shockwaves; i++, shock = shock->next)
    {
      if (shock == NULL && i < (num_of_shockwaves - 1))
        {
          LOG_ERR ("shock->next is null %i/%i", i, num_of_shockwaves);
          break;
        }
      /* calculate the distance between shockwave center and enemy center */
      dx = centerx - shock->center_x;
      dy = centery - shock->center_y;
      resultat = (dx * dx) + (dy * dy);
      dist = (Sint32) sqrt (resultat);

      /* collision detected? */
      if (dist >= (27 + (shock->ring_index * 5))
          && dist <= (43 + (shock->ring_index * 5)))
        {
          /* add a bonus gem or a lonely foe */
          bonus_add (foe);
          /* collision between the enemy an a shockwave */
          return TRUE;
        }
    }
  /* no collision between the enemy an a shockwave */
  return FALSE;
}

/**
 * Initialize shockwaves data structure 
 */
void
shockwave_init (void)
{
  Sint32 i;
  shockwave_struct *shock;

  /* release all shockwaves */
  for (i = 0; i < MAX_NUMOF_SHOCKWAVES; i++)
    {
      shock = &shockwave[i];
      /* clear shockwave color index */
      shock->color_index = 0;
      shock->is_enabled = FALSE;
    }
  shockwave_first = NULL;
  shockwave_last = NULL;

  /* clear the number of shockwave */
  num_of_shockwaves = 0;
}

/**
 * Draw one polygon
 * @param addr pointer to the screen buffer  
 * @param coords table of coordinates
 * @param numofpts number of points
 * @param color color of the polygon
 * @author Etienne Sobole
 * @copyright EKO System
 */
/* Une super routine de polygone
 * j'ai ultra commente. comme ca tu pourras meme apprendre quelques chose.
 * j'ai pas compile. donc j'espere que toute les variables sont definies.
 * j'ai (par consequent) pas teste... si ca c'est pas de la confiance en soit!!!
 * a bientot.
 * PS : si t'oublie ma tondeuse je te casse la tete.
 * Etienne
 */
static void
draw_polygon (char *addr, Sint32 * coords, Sint32 numofpts, Uint32 color)
{
  Sint32 i, j, x;
  Sint32 x1, x2, y1, y2;
  Sint32 min, max;
  Sint32 step;
  Sint32 *left = shockwave_left_buffer;
  Sint32 *right = shockwave_right_buffer;
  Sint32 dy;
  Sint32 dx;
  Sint32 numofpixels;
  char *drawaddr;
  char *linestart;
  switch (bytes_per_pixel)
    {
    case 2:
      color = pal16[color];
      break;
    case 3:
      color = pal32[color];
      break;
    case 4:
      color = pal32[color];
      break;
    }
  /* je pense pas qu'il soit possible de depasser
   * ces valeurs avant quelques millenaires */
  min = 10000000;
  max = -10000000;
  for (i = 0; i < numofpts * 2; i += 2)
    {
      y1 = coords[i + 1];
      y2 = coords[i + 3];
      dy = y2 - y1;
      /* ignore horizontal segment */
      if (dy != 0)
        {
          if (y1 < min)
            min = y1;
          if (y1 > max)
            max = y1;
          x1 = coords[i];
          x2 = coords[i + 2];
          dx = x2 - x1;

          /* left buffer */
          if (dy > 0)
            {
              step = (dx << 16) / dy;
              /* increase precision */
              x = x1 << 16;
              /* y1 < y2 it's ok */
              for (j = y1; j <= y2; j++)
                {
                  left[j] = x;
                  x += step;
                }
            }
          /* right buffer */
          else
            {
              step = (dx << 16) / dy;
              /* increase precision */
              x = x2 << 16;
              /* y2 < y1 it's ok */
              for (j = y2; j <= y1; j++)
                {
                  right[j] = x;
                  x += step;
                }
            }
        }
    }

  /* pointer to the start of each line */
  linestart = addr + min * offscreen_pitch;
  drawaddr = linestart;
  /* une ligne de moins pour une belle jointure */
  for (i = min; i < max; i++)
    {
      /* decrease precision */
      x1 = left[i] >> 16;
      x2 = right[i] >> 16;
      /* number of pixels to draw */
      numofpixels = x2 - x1;
      /* fist pixel to draw */
      drawaddr += (x1 * bytes_per_pixel);
      switch (bytes_per_pixel)
        {
        case 1:
          poly8bits (drawaddr, numofpixels, color);
          break;
        case 2:
          poly16bits (drawaddr, numofpixels, color);
          break;
        case 3:
          poly24bits (drawaddr, numofpixels, color);
          break;
        case 4:
          poly32bits (drawaddr, numofpixels, color);
          break;
        }
      /* next line */
      linestart += offscreen_pitch;
      drawaddr = linestart;
    }
}
