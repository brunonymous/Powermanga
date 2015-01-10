/** 
 * @file movie.h
 * @brief play start and congratulations animations files
 * ("movie_congratulation.gca" and "movie_introduction.gca")
 * @created 2007-01-01 
 * @date 2012-08-26 
 * @author Etienne Sobole
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: movie.h,v 1.11 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __MOVIE__
#define __MOVIE__
#ifdef __cplusplus
extern "C"
{
#endif
  typedef enum
  {
    MOVIE_NOT_PLAYED,
    MOVIE_INTRODUCTION,
    MOVIE_CONGRATULATIONS,
    MOVIE_PLAYED_CURRENTLY
  } MOVIE_ENUM;

  bool movie_player (void);
  void movie_free (void);
  extern Uint32 movie_playing_switch;
  /** Pointer to the buffer for the current animation movie */
  extern unsigned char *movie_buffer;
#ifdef __cplusplus
}
#endif
#endif
