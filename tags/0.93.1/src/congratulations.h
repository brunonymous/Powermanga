/** 
 * @file congratulations.h 
 * @brief Handle the congratulations 
 * @created 2007-06-01
 * @date 2012-08-26 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: congratulations.h,v 1.8 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __CONGRATULATIONS__
#define __CONGRATULATIONS__
#ifdef __cplusplus
extern "C"
{
#endif
/** If TRUE display congratulations */
  extern bool is_congratulations_enabled;

  void congratulations_initialize (void);
  void congratulations (void);
#ifdef __cplusplus
}
#endif
#endif
