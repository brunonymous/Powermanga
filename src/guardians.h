/** 
 * @file guardians.h 
 * @brief handle the guardians 
 * @created 2006-12-30 
 * @date 2012-08-26 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: guardians.h,v 1.39 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __GUARDIANS__
#define __GUARDIANS__

#ifdef __cplusplus
extern "C"
{
#endif

/** Number maximum of displacement peer guardian */
#define GUARDIANS_MAX_OF_DISPLACEMENTS 100
/** Number maximum of images peer guardian */
#define GUARDIAN_MAX_OF_ANIMS 5
/** Maximum number of sprites which compose a guardian */
#define GUARDIAN_MAX_ELEMENTS 2

  typedef enum
  {
    GUARD_MOVEMENT_TOWARD_RIGHT = 0,
    GUARD_MOVEMENT_TOWARD_BOTTOM = 8,
    GUARD_MOVEMENT_TOWARD_LEFT = 16,
    GUARD_MOVEMENT_TOWARD_TOP = 24,
    GUARD_IMMOBILE = 32
  } GUARD_MOVEMENT;

  typedef struct guardian_struct
  {
    /** If TRUE guardian appearing in progress */
    bool is_appearing;
    /** If FALSE the guardian is not display during his appearing */
    bool is_blinking;
    /** Current guardian number from 1 to 14 */
    Uint32 number;
    /** True if the guardian is enabled */
    bool is_enabled;
    enemy *foe[GUARDIAN_MAX_ELEMENTS];
    /** Time delay between two moving */
    Sint32 move_time_delay;
    /** Current move index */
    Uint32 move_current;
    /** Number of displacements for the current guardian */
    Uint32 move_max;
    Uint32 move_direction[GUARDIANS_MAX_OF_DISPLACEMENTS];
    Sint32 move_delay[GUARDIANS_MAX_OF_DISPLACEMENTS];
    Uint32 move_speed[GUARDIANS_MAX_OF_DISPLACEMENTS];
    /** Time delay before next image (animation speed) */
    Uint32 anim_delay_count;
    /** TRUE if the guardian follow a vertical trajectory */
    bool is_vertical_trajectory;
    /** TRUE if the direction of the guardian has changed */
    bool has_changed_direction;
    /** Current set of images */
    Uint32 current_images_set;
    /** Delay counter before next lonely foe */
    Sint32 lonely_foe_delay;
    /** Delay counter before next Soukee */
    Sint32 soukee_delay;
    /** Delay counter before next Quibouly */
    Sint32 quibouly_delay;
    /** Delay counter before next Shury */
    Sint32 shuriky_delay;
    /** Delay counter before next Tournadee */
    Sint32 tournadee_delay;
    /** Indicate on which side the next tornado will be launched */
    bool is_tournadee_left_pos;
    /***  Delay counter before next missile */
    Sint32 missile_delay;
    /** Delay counter before next Sapouch */
    Sint32 sapouch_delay;
    /** Delay counter before next Perturbians */
    Sint32 perturbians_delay;
    /** Delay counter before next Saakamin */
    Sint32 saakamin_delay;
    bool devilians_enable;
    Sint32 devilians_counter;
    Sint32 devilians_delay;
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float y_inc;
  } guardian_struct;

  bool guardians_once_init (void);
  void guardians_free (void);
  void guardian_handle (enemy * guard);
  bool guardian_new (Uint32 guard_num);
  bool guardian_load (Sint32 guardian_num);
#ifdef PNG_EXPORT_ENABLE
  bool guardians_extract (void);
#endif
  bool guardian_finished (void);
  extern guardian_struct *guardian;

#ifdef __cplusplus
}
#endif
#endif
