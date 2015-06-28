/**
 * @file starfield.h 
 * @brief Handle the starfields 
 * @created 2006-11-26 
 * @date 2009-01-11
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: starfield.h,v 1.13 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __STARFIELD__
#define __STARFIELD__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    /** Big star of the starfield */
    STAR_BIG,
    /** Middle star of the starfield */
    STAR_MIDDLE,
    /** Little star of the starfield */
    STAR_LITTLE,
    /** Star used for player's spaceship */
    STAR_SPACESHIP,
    /** Four types of different stars */
    TYPE_OF_STARS
  } STARS_ENUM;

#define STAR_NUMOF_IMAGES 8

  bool starfield_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool starfield_extract (void);
#endif
  void starfield_free (void);
  void starfield_handle (void);

  extern image star_field[TYPE_OF_STARS][STAR_NUMOF_IMAGES];
  extern float starfield_speed;

#ifdef __cplusplus
}
#endif
#endif
