/** 
 * @file scrolltext.h
 * @brief Handle scroll-text of the main menu 
 * @created 2006-11-11
 * @date 2012-0825-
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: scrolltext.h,v 1.9 2012/08/25 13:58:37 gurumeditation Exp $
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
#ifndef __SCROLLTEXT__
#define __SCROLLTEXT__

#ifdef __cplusplus
extern "C"
{
#endif

/** Max of characters */
#define FONT_SCROLLTEXT_MAXOF_GLYPHS 58
#define SCROLL_PRESENT 1

  void scrolltext_handle (void);
  void scrolltext_free (void);
  bool scrolltext_once_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool scrolltext_extract (void);
#endif
  bool scrolltext_load (void);
  void scrolltext_init (void);
  void scrolltext_disable (Sint32 d);

  extern bitmap fnt_scroll[FONT_SCROLLTEXT_MAXOF_GLYPHS];

#ifdef __cplusplus
}
#endif
#endif
