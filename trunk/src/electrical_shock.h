/** 
 * @file electrical_shock.h 
 * @brief Handle powerful electrical shocks
 * @created 2006-12-03 
 * @date 2007-06-01
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: electrical_shock.h,v 1.9 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __ELECTRICAL_SHOCK__
#define __ELECTRICAL_SHOCK__

#ifdef __cplusplus
extern "C"
{
#endif

  bool electrical_shock_once_init (void);
  void electrical_shock (void);

  typedef struct Eclair
  {
    Sint32 sx, sy;
    Sint32 dx, dy;
    Sint32 col1, col2;
    Sint32 r1, r2, r3;
  } Eclair;

  extern Eclair eclair1;
  extern bool electrical_shock_enable;

#ifdef __cplusplus
}
#endif
#endif
