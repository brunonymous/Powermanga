/** 
 * @file texts.h 
 * @brief Handle texts PAUSE, LEVEL and GAME OVER
 * @created 2006-12-19 
 * @date 2012-08-26
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: texts.h,v 1.20 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __TEXTS__
#define __TEXTS__

#ifdef __cplusplus
extern "C"
{
#endif

  bool texts_init_once (void);
  void texts_free (void);
  void text_enemy_name_init (const char *name);
  void text_enemy_name_draw (Sint32 offsetx, Sint32 offsety, bool restart);
  void text_draw_score (void);
  void text_pause_draw (void);
  void text_gameover_init (void);
  void text_level_draw (void);
  bool text_gameover_draw (void);
  void texts_init (void);
  bool text_level_move (Sint32 level_nu);

  extern bool is_player_score_displayed;

#ifdef __cplusplus
}
#endif
#endif
