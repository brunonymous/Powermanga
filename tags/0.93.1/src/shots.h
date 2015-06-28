/**
 * @file shots.h 
 * @brief Handle all shots elements 
 * @created 2006-11-20 
 * @date 2012-08-26
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: shots.h,v 1.30 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __SHOTS__
#define __SHOTS__
#include "enemies.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SHOT_NUMOF_IMAGES 32
#define MAX_OF_SHOTS 400

  typedef enum
  {
    FRIEND,
    ENEMY
  }
  SHOT_TYPEOF;

  typedef struct shot_struct
  {
    /** Sprite structure */
    sprite spr;
    /** Index of progression */
    Sint32 curve_index;
    /** Curve number used */
    Sint16 curve_num;
    /** Time of life */
    Sint16 timelife;
    /** Angle of the projectile */
    float angle;
    /**  Rotation speed */
    float velocity;
    Sint16 img_angle;
    Sint16 img_old_angle;
    /** Flicker the sprite */
    bool is_blinking;
    /** Previous element of the chained list */
    struct shot_struct *previous;
    /** Next element of the chained list */
    struct shot_struct *next;
    /** TRUE if the element is already chained */
    bool is_enabled;
  } shot_struct;

  bool shots_once_init (void);
  void shots_free (void);
  void shots_init (void);
  void shots_handle (void);
  shot_struct *shot_get (void);
  void shot_enemy_add (const enemy * const ev, Sint32 k);
  shot_struct *shot_guardian_add (const enemy * const guard, Uint32 cannon,
                                  Sint16 power, float speed);
  void shot_satellite_add (Sint32 xcoord, Sint32 ycoord, Sint16 img_angle);
  void shot_linear_spaceship_add (Sint16 damage, Sint16 anim_speed,
                                  Sint32 image_num, Sint16 angle,
                                  Sint32 cannon_pos, float speed);
  void shot_calulated_spaceship_add (Sint16 damage, Sint32 image_num,
                                     Sint16 angle, Sint32 cannon_pos,
                                     float shot_angle);
  void shot_curve_spaceship_add (Sint16 damage, Sint16 anim_speed,
                                 Sint32 image_num, Sint16 angle,
                                 Sint32 cannon_pos, Sint16 curve_num);
  float shot_x_move (float angle, float speed, float xcoord);
  float shot_y_move (float angle, float speed, float ycoord);
#ifdef PNG_EXPORT_ENABLE
  bool shots_extract (void);
#endif

  extern Sint32 num_of_shots;

/* 
 * index of the fires images in the table of the fires (fire[][]) 
 */
  typedef enum
  {
    /* vertical shot force 2 power 1 from spaceship */
    SP2MIJ,
    /* shot force 1 power 1 from spaceship */
    TIR1P1J,
    /* shot power 1 from enemies */
    TIR1P1E,
    /* shot power 2 from enemies */
    TIR1P2E,
    /* shot power 3 from enemies */
    TIR1P3E,
    /* vertical shot force 2 power 1 from spaceship */
    SP1V1J,
    /* vertical shot force 2 power 2 from spaceship */
    SP1V2J,
    /* vertical shot force 2 power 3 from spaceship */
    SP1V3J,
    /* vertical shot force 2 power 4 from spaceship */
    SP1V4J,
    /* vertical shot force 2 power 5 from spaceship */
    SP1V5J,
    /* horizontal shot force 2 power 1 from spaceship */
    SP1H1J,
    /* horizontal shot force 2 power 2 from spaceship */
    SP1H2J,
    /* horizontal shot force 2 power 3 from spaceship */
    SP1H3J,
    /* horizontal shot force 2 power 4 from spaceship */
    SP1H4J,
    /* horizontal shot force 2 power 5 from spaceship */
    SP1H5J,
    /* shot force 1 (2) power 1 from spaceship */
    T2NP1J,
    /* shot force 1 (2) power 1 from spaceship */
    T2NP2J,
    /* front shot force 1 (1) from spaceship */
    V1TN1,
    /* front shot force 1 (2) from spaceship */
    V1TN2,
    /* front shot force 1 (3) from spaceship */
    V1TN3,
    /* left shot force 1 (1) from spaceship 1 */
    V1TGN1,
    /* left shot force 1 (2) from spaceship 1 */
    V1TGN2,
    /* left shot force 1 (3) from spaceship 1 */
    V1TGN3,
    /* left shot force 1 (1) from spaceship 1 */
    V1TDN1,
    /* left shot force 1 (2) from spaceship 1 */
    V1TDN2,
    /* left shot force 1 (3) from spaceship 1 */
    V1TDN3,
    /* shot force 1 (1) power 1 from spaceship 1 */
    V2TN1,
    /* shot force 1 (1) power 1 from spaceship 1 */
    V2TN2,
    /* shot force 1 (3) power 1 from spaceship 1 */
    V2TN3,
    /* left shot force 1 (1) from spaceship 1 */
    V2TGN1,
    /* left shot force 1 (2) from spaceship 1 */
    V2TGN2,
    /* left shot force 1 (3) from spaceship 1 */
    V2TGN3,
    /* left shot force 1 (1) from spaceship 1 */
    V2TDN1,
    /* left shot force 1 (2) from spaceship 1 */
    V2TDN2,
    /* left shot force 1 (3) from spaceship 1 */
    V2TDN3,
    V2THN1,
    V2THN2,
    V2THN3,
    V3THN1,
    V3THN2,
    V3THN3,
    /* missile x0 */
    MISSx0,
    /* missile x1 */
    MISSx1,
    /* missile x2 */
    MISSx2,
    /* missile x3 */
    MISSx3,
    /* missile x4 from guard 6 */
    MISSx4,
    /* missile x5 */
    MISSx5,

    V3TBA,
    V3TBB,
    V3TBC,
    V3TDA,
    V3TDB,
    V3TDC,
    V3TGA,
    V3TGB,
    V3TGC,
    V3TSA,
    V3TSB,
    V3TSC,
    V3TSD,
    V3TSE,

    V4THA,
    V4THB,
    V4THC,
    V4TBA,
    V4TBB,
    V4TBC,
    V4TDA,
    V4TDB,
    V4TDC,
    V4TGA,
    V4TGB,
    V4TGC,
    V4TSA,
    V4TSB,
    V4TSC,
    V4TSD,
    V4TSE,

  /** Maximum number of differents shots (78) */
    SHOT_MAX_OF_TYPE
  }
  IMAGES_SHOTS_OFFSETS;

  extern image fire[SHOT_MAX_OF_TYPE][SHOT_NUMOF_IMAGES];


#ifdef __cplusplus
}
#endif

#endif
