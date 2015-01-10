/** 
 * @file menu_sections.h
 * @brief hanlde high score table, about and order menu sections 
 * @created 1998-06-29 
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: menu_sections.h,v 1.10 2012/08/26 15:44:26 gurumeditation Exp $
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
#ifndef __MENU_SECTIONS__
#define __MENU_SECTIONS__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    NO_SECTION_SELECTED,
    SECTION_GAME_OVER,
    SECTION_HIGH_SCORE,
    SECTION_ABOUT,
    SECTION_ORDER
  } SECTIONS_ENUM;

  extern Uint32 menu_section;
  bool menu_sections_once_init (void);
  void menu_sections_free (void);
  void menu_sections_run ();
  bool menu_section_set (Uint32 section);
  bool is_playername_input ();

#ifdef __cplusplus
}
#endif
#endif
