/** 
 * @file grid_phase.h
 * @brief Handle grid phase (enemy wave like Space Invaders)   
 * @created 2006-11-29
 * @date 2012-08-25
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: grid_phase.h,v 1.13 2012/08/25 19:18:32 gurumeditation Exp $
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
#ifndef __GRID_PHASE__
#define __GRID_PHASE__

#ifdef __cplusplus
extern "C"
{
#endif

/* grid editor */
/*
#ifdef DEVELOPPEMENT
void Grid_Edit (void);
#endif
*/

  bool grid_load (Sint32 num_grid);
  void grid_start (void);
  void grid_handle (void);
  void grid_finished (void);

/** Width of the grid, or maximum number of enemies per row */
#define GRID_WIDTH 16
/** Height of the grid, or maximum number of enemies per line */
#define GRID_HEIGHT 10

/** Data structure of a grid level */
  typedef struct grid_struct
  {
  /** Grid phase enable or not */
    bool is_enable;
  /** Appearance of the grid enable or not */
    bool is_appearing;
  /** Movement toward right or left */
    bool right_movement;
  /** Enemy number of each enemy on the grid */
    Sint16 enemy_num[GRID_WIDTH][GRID_HEIGHT];
  /** Shot time-frequency of each enemy on the grid */
    Sint16 shoot_speed[GRID_WIDTH][GRID_HEIGHT];
    float coor_x, coor_y;
    float vit_dep_x;
    float vit_dep_y;
    float speed_x;
  } grid_struct;

  extern grid_struct grid;

#ifdef __cplusplus
}
#endif
#endif
