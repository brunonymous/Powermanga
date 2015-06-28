/** 
 * @file curve_phase.h
 * @brief Handle the curve phase 
 * @created 2006-11-26
 * @date 2007-08-22
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: curve_phase.h,v 1.15 2012/06/03 17:06:14 gurumeditation Exp $
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
#ifndef __CURVE_PHASE__
#define __CURVE_PHASE__

#ifdef __cplusplus
extern "C"
{
#endif

  bool curve_once_init (void);
  void curve_free (void);
  bool curve_load_level (Sint32 leveln);
  void curve_enable_level (void);
  void curve_phase (void);
  void curve_finished (void);
#ifdef DEVELOPPEMENT
  void courbe_editeur (void);
#endif

/* Maximum number of points on a curve */
#define MAX_PNT_OF_CURVE 2000
  typedef struct curve
  {
    /** Number of points on the curve */
    Sint16 nbr_pnt_curve;
    /** Curve start coordinates on the screen */
    Sint16 pos_x, pos_y;
    /** X offsets */
    signed char delta_x[MAX_PNT_OF_CURVE];
    /** Y offsets */
    signed char delta_y[MAX_PNT_OF_CURVE];
    /** Ennemies angles */
    signed char angle[MAX_PNT_OF_CURVE];
  } curve;

/*
 * curve level structure
 */
/** 5 curves maximum at any time */
#define CURVES_MAX_OF  5
/** 16 enemies peer curve maximum */
#define NUM_VAI_BY_CURVE 16
  typedef struct curve_level
  {
    /** 1 = curve phase enable */
    bool activity;
    /** Number of current enemies on each curve (16 maximum) */
    Sint16 current_numof_enemies[CURVES_MAX_OF];
    /** Counter delay before appearance of the next enemy  */
    Sint16 count_before_next[CURVES_MAX_OF];
    /** Total number of curves for this level */
    Sint16 total_numof_curves;
    /** The list of bezier curves numbers used for this level */
    Sint16 num_courbe[CURVES_MAX_OF];
    /** Number of enemies on each curve */
    Sint16 total_numof_enemies[CURVES_MAX_OF];
    /** Time delay before appearance of the next enemy  */
    Sint16 delay_before_next[CURVES_MAX_OF];
    /** Enemies numbers on each curve */
    Sint16 num_vaisseau[CURVES_MAX_OF][NUM_VAI_BY_CURVE];
    /** Delay between two shots */
    Sint16 freq_tir[CURVES_MAX_OF][NUM_VAI_BY_CURVE];
  } curve_level;

  /** Data structure of the current level */
  extern curve_level courbe;
  /** All bezier curves loaded at startup */
  extern curve *initial_curve;
#ifdef __cplusplus
}
#endif
#endif
