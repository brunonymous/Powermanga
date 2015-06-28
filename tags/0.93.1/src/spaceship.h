/** 
 * @file spaceship.h
 * @brief Handle the player's spaceship 
 * @created 2006-11-17
 * @date 2012-08-28
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: spaceship.h,v 1.27 2012/08/25 15:55:00 gurumeditation Exp $
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
#ifndef __SPACESHIP__
#define __SPACESHIP__

#ifdef __cplusplus
extern "C"
{
#endif

/** Spaceship invincibility time */
#define SPACESHIP_INVINCIBILITY_TIME 15
/** There are 5 options for each level */
#define SPACESHIP_MAX_OPTION_LEVELS 5

 /** The differents types of spaceships */
  typedef enum
  {
    SPACESHIP_TYPE_1,
    SPACESHIP_TYPE_2,
    SPACESHIP_TYPE_3,
    SPACESHIP_TYPE_4,
    SPACESHIP_TYPE_5,
      /** Maximum number of types spaceships */
    SPACESHIP_NUM_OF_TYPES
  }
  SPACESHIP_TYPES;

  /** Structure of spaceship's player */
  typedef struct spaceship_struct
  {
    /** Structure of the sprite images */
    sprite spr;
    /** Horizontal speed */
    float x_speed;
    /** Vertical speed */
    float y_speed;
    /** TRUE if display spaceship's white mask */
    bool is_white_mask_displayed;
    /* TRUE if display spaceship */
    bool is_visible;
    /** Invincibility time */
    Sint16 invincibility_delay;
    /** Invincibility counter time */
    Sint16 invincibility_count;
    /** Number of gems collected from 0 to 11 */
    Sint16 gems_count;
    /** Number of satellite protections from 0 to 5 */
    Sint16 num_of_satellites;
    /** Number of extra gun from 0 to 2 */
    Sint16 num_of_extraguns;
    /** Type of vessel from SPACESHIP_TYPE_1 to SPACESHIP_TYPE_5 */
    Sint16 type;
    /** Time before 2 shots */
    Sint32 fire_rate;
    /** Shot force 2 tempo counter */
    Sint32 fire_rate_enhanced;
    /* level of options, from 0 (none) to 5 (maxi) */
    /** Speed booster (0 = freezed) */
    Sint16 speed_booster;
    /** Front shot force 1 */
    Sint16 shot_front_basic;
    /** Rear shot force 1 */
    Sint16 shot_rear_basic;
    /** Right shot force 1 */
    Sint16 shot_right_basic;
    /** Left shot force 1 */
    Sint16 shot_left_basic;
    /** Front shot force 2 */
    Sint16 shot_front_enhanced;
    /** Rear shot force 2 */
    Sint16 shot_rear_enhanced;
    /** Right shot force 2 */
    Sint16 shot_right_enhanced;
    /** Left shot force 2 */
    Sint16 shot_left_enhanced;
    /** TRUE if vessel has just changed */
    bool has_just_upgraded;
  } spaceship_struct;

  bool spaceship_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool spaceship_extract (void);
#endif
  void spaceship_free (void);
  spaceship_struct *spaceship_get (void);
  void spaceship_energy_restore (void);
  void spaceship_invincibility (void);
  void spaceship_show (void);
  void spaceship_initialize (void);
  void spaceship_gameover (void);
  void spaceship_downgrading (void);
  void spaceship_set_invincibility (Sint16 invincibility);
  void spaceship_control_movements (void);
  bool spaceship_upgrading (void);
  void spaceship_most_powerfull (void);
  bool spaceship_enemy_collision (enemy * foe, float speed_x, float speed_y);
  bool spaceship_shot_collision (Sint32 x1, Sint32 y1, shot_struct * bullet);
  void spaceship_speed_control (void);
  void spaceship_draw (void);

  extern bool spaceship_disappears;
  extern Sint32 spaceship_appears_count;
  extern bool spaceship_is_dead;

#ifdef __cplusplus
}
#endif

#endif
