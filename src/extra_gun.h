/** 
 * @file extra_gun.h
 * @brief Handle extra guns positioned on the sides of the spaceship
 * @created 2006-11-25 
 * @date 2010-10-23 
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: extra_gun.h,v 1.14 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __EXTRA_GUN__
#define __EXTRA_GUN__
#ifdef __cplusplus
extern "C"
{
#endif

  bool guns_once_init (void);
  void guns_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool guns_extract (void);
#endif
  void guns_free (void);
  bool gun_add (void);
  void guns_handle (void);
  bool guns_enemy_collisions (enemy * foe, Sint32 num_of_fragments);
  bool guns_shot_collisions (Sint32 x1, Sint32 y1, shot_struct * bullet);

#ifdef __cplusplus
}
#endif
#endif
