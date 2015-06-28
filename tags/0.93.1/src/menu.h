/** 
 * @file menu.h 
 * @brief handle displaying and updating the graphical components of the game 
 * @created 2006-12-07 
 * @date 2014-10-12 
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: menu.h,v 1.15 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __MENU__
#define __MENU__

#ifdef __cplusplus
extern "C"
{
#endif

  bool menu_once_init (void);
  void menu_free (void);
  void menu_handle (void);
  bool menu_check_button (void);
#ifdef PNG_EXPORT_ENABLE
  bool menu_extract ();
#endif

/** The different possible status for the menu */
  typedef enum
  {
    MENU_OFF,
    MENU_ON,
    MENU_UP,
    MENU_DOWN
  }
  MENU_STATUS;

  extern Sint32 menu_status;
  extern Sint32 menu_coord_y;
  extern float starfield_speed;
#ifdef __cplusplus
}
#endif
#endif
