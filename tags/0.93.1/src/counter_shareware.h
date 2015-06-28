/**
 * @file counter_shareware.h 
 * @brief Handle the displaying of the pages order in Windows shareware
 *        version 
 * @created 1999-08-16
 * @date 2012-08-26 
 * @author Patrice Duhamel 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: counter_shareware.h,v 1.8 2012/08/26 17:09:14 gurumeditation Exp $
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
#include "config_file.h"
#ifdef SHAREWARE_VERSION
#ifndef __TLK_COMPTEUR_H__
#define __TLK_COMPTEUR_H__


#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Counter file:
 * save infomation for each game in the registry
 * 'HKEY_CURRENT_USER\Software\TLK Games\Data'
 * save:
 * - game name and version
 * - number of times the game has been launched   
 * - clear the counter after a game upgrade    
 *
 * Use:
 * # game_name version 1.1
 * int CounterLaunch = counter_shareware_update("game_name", 1, 1);
 * if (CounterLaunch >= 8)
 *   {
 *     # display page order 1    
 *   }
 *
 * # leave game
 * counter_shareware_free();
 */

/**
 * Stucture of a decrypted input in counter file
 */
  typedef struct
  {
  /** Number of times the game has been launched */
    Uint32 count;
  /** Number of seconds since the game has been launched, unused */
    Uint32 time;
  /** Game version (= vmaj << 16 + vmin) */
    Uint32 version;
  /** Game name */
    char name[256];
  } TLKGameInfo;

/** Load games list and update launch counter */
  Sint32 counter_shareware_update (char *gamename, Sint32 vmaj, Sint32 vmin);
/** Remove games list */
  void counter_shareware_free (void);
#endif

#ifdef __cplusplus
}
#endif
#endif
