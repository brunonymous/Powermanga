/**
 * @file text_overlay.h
 * @brief Handle text overlay to display abouts, cheats menu and variables  
 * @date 2007-08-06
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: text_overlay.h,v 1.11 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef _TEXT_OVERLAY_
#define _TEXT_OVERLAY_

#ifdef __cplusplus
extern "C"
{
#endif
#define FONT_OVERLAY_WIDTH 448
  bool text_overlay_once_init (void);
  void text_overlay_release (void);
  void text_overlay_draw (void);
#ifdef __cplusplus
}
#endif
#endif
