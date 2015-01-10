/** 
 * @file explosions.h
 * @brief handle explosions and explosions fragments 
 * @created 2006-12-12 
 * @date 2012-08-26
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: explosions.h,v 1.14 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __EXPLOSIONS__
#define __EXPLOSIONS__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    EXPLOSION_MEDIUM,
    EXPLOSION_BIG,
    EXPLOSION_SMALL
  }
  EXPLOSION_SIZE;

  bool explosions_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool explosions_extract (void);
#endif
  void explosions_free (void);
  void explosions_handle (void);
  void explosions_add_serie (enemy * foe);
  void explosion_add (float coordx, float coordy, float speed, Sint32 type,
                      Sint32 delay);
  void explosion_guardian_add (float coordx, float coordy);
  void explosions_fragments_add (float coordx, float coordy, float speed,
                                 Sint32 numof, Sint32 delay,
                                 Sint16 anim_speed);

#ifdef __cplusplus
}
#endif
#endif
