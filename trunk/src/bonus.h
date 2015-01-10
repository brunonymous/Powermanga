/** 
 * @file bonus.h 
 * @brief Handle gems, bonus and penality 
 * @created 2006-11-19 
 * @date 2014-10-11 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: bonus.h,v 1.18 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __BONUS__
#define __BONUS__

#ifdef __cplusplus
extern "C"
{
#endif

/** Number of different bonus 
 * (the last type is never used)  */
#define GEM_NUMOF_TYPES 6
/** Maximum number of images peer bonus */
#define GEM_NUMOF_IMAGES 32

  bool bonus_once_init (void);
  void bonus_init (void);
  void bonus_free (void);
  void bonus_disable_all (void);
  void bonus_handle (void);
  void bonus_add (const enemy * const pve);
  void bonus_meteor_add (const enemy * const pve);
  extern image bonus[GEM_NUMOF_TYPES][GEM_NUMOF_IMAGES];
#ifdef PNG_EXPORT_ENABLE
  bool bonus_extract (void);
#endif

#ifdef __cplusplus
}
#endif
#endif
