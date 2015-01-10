/** 
 * @file energy_gauge.h 
 * @brief Handle displaying of the energy gauge the top score panel
 *        for the player's spaceship and the current guardian
 * @created 2007-01-06
 * @date 2012-08-25
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: energy_gauge.h,v 1.11 2012/08/25 15:55:00 gurumeditation Exp $
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

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef __ENERGY_GAUGE__
#define __ENERGY_GAUGE__
  bool energy_gauge_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool energy_gauge_extract ();
#endif
  void energy_gauge_free (void);
  void energy_gauge_init (void);
  void energy_gauge_spaceship_update (void);
  void energy_gauge_guardian_update (void);
  extern bool energy_gauge_spaceship_is_update;
  extern bool energy_gauge_guard_is_update;
#ifdef __cplusplus
}
#endif
#endif
