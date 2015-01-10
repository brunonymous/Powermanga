/**
 * @file shockwave.h 
 * @brief Handle powerful circular shock wave propagated by spaceship 
 * @created 2006-12-01 
 * @date 2007-08-04
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: shockwave.h,v 1.13 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __SHOCKWAVE__
#define __SHOCKWAVE__
#ifdef __cplusplus
extern "C"
{
#endif
  void shockwave_draw (void);
  void shockwave_add (void);
  void shockwave_init (void);
  bool shockwave_collision (enemy * foe);
  bool shockwave_once_init (void);
  void shockwave_free (void);
#ifdef __cplusplus
}
#endif
#endif
