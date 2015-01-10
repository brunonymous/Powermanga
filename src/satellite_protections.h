/** 
 * @file satellite_protections.h
 * @brief handle orbital satellite protections gravitate around the 
 *        player's spaceship and protect it from hostiles shots
 *        and enemies
 * @created 2006-11-10 
 * @date 2012-08-25 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: satellite_protections.h,v 1.13 2012/08/25 15:55:00 gurumeditation Exp $
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
#ifndef __SATELLITE_PROTECTION__
#define __SATELLITE_PROTECTION__

#ifdef __cplusplus
extern "C"
{
#endif
  bool satellites_once_init (void);
  void satellites_free (void);
  void satellites_init (void);
  void satellites_handle (void);
  void satellites_add (void);
  void satellite_add (void);
  void satellites_setup (void);
  bool satellites_enemy_collisions (enemy * foe, Sint32 num_of_fragments);
  bool satellites_shot_collisions (Sint32 x1, Sint32 y1,
                                   shot_struct * projectile);
#ifdef PNG_EXPORT_ENABLE
  bool satellite_extract ();
#endif
#ifdef __cplusplus
}
#endif
#endif
