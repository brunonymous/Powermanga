/**
 * @file counter_shareware.c 
 * @brief Handle the displaying of the pages order in Windows shareware
 *        version 
 * @created 2007-04-18 
 * @date 2012-08-26 
 * @author Patrice Duhamel 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: counter_shareware.c,v 1.10 2012/08/26 19:22:39 gurumeditation Exp $
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
#include "config.h"
#include "powermanga.h"
#include "tools.h"
#include "config_file.h"
#ifdef SHAREWARE_VERSION
#include "counter_shareware.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
/** Key encryption */
const char Codecounter_cryptage[] = "TLK";
/** Games list in the counter file */
static TLKGameInfo *GameCount = NULL;
/** Number of game found in the counter file */
static Uint32 num_of_games = 0;

#ifndef WIN32
#define DWORD Uint32
#define HKEY Uint32
#define LPBYTE char*
#define ERROR_SUCCESS 0
#define ERROR_UNKNOWN 99999
#define HKEY_CURRENT_USER ((HKEY) 0x80000001)
#define KEY_QUERY_VALUE 0x00000001
#define REG_BINARY 3
#define KEY_WRITE 0x20006
#define REG_BINARY 3
#define LPSECURITY_ATTRIBUTES void *
#define REG_OPTION_NON_VOLATILE 0

/**
 * Opens the specified registry key
 * @param h_key handle to an open registry key
 * @param lpSubKey name of the registry subkey to be opened
 * @param ulOptions reserved, set to zero
 * @param samDesired desired access rights to the key
 * @param phkResult pointer to variable that receives a handle to the opened key
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegOpenKeyEx (HKEY h_key, const char *lpSubKey, DWORD ulOptions,
              Sint32 samDesired, Uint32 * phkResult)
{
  return ERROR_UNKNOWN;
  return ERROR_SUCCESS;
}

/**
 * Retrieves the type and data for the specified value name associated with an
 * open registry key
 * @param h_key handle to an open registry key
 * @param lpValueName name of the value to be set
 * @param lpReserved must be NULL
 * @param lpType pointer to variable that receives data type code
 * @param lpData pointer to a buffer that receives the value's data
 * @param lpcbData pointer to a variable that specifies the size of the buffer 
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegQueryValueEx (HKEY h_key, const char *lpValueName, Uint32 * lpReserved,
                 Uint32 * lpType, LPBYTE lpData, Uint32 * lpcbData)
{
  return ERROR_UNKNOWN;
  return ERROR_SUCCESS;
}

/**
 * Closes a handle to the specified registry key.
 * @param h_key A handle to the open key to be closed.
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegCloseKey (HKEY h_key)
{
  return ERROR_UNKNOWN;
}

/**
 * Creates the specified registry key
 * @param h_key A handle to an open registry key
 * @param lpSubKey subkey name  that this function opens or creates
 * @param Reserved reserved, set to zero
 * @param lpClass he class (object type) of this key
 * @param dwOptions option value
 * @param samDesired mask that specifies the access rights
 * @lpSecurityAttributes pointer to a security attribute
 * phkResult pointer to a variable that receives handle 
 * lpdwDisposition variable that receives REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY 
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegCreateKeyEx (HKEY h_key, const char *lpSubKey, DWORD Reserved,
                char *lpClass, DWORD dwOptions, Sint32 samDesired,
                LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                Uint32 * phkResult, Uint32 * lpdwDisposition)
{
  return ERROR_UNKNOWN;
}

/**
 * Sets the data and type of a specified value under a registry key
 * @param h_key handle to an open registry key
 * @param lpValueName name of the value to be set
 * @param Reserved must be zero
 * @param dwType type of data pointed to by the lpData parameter
 * @param lpData The data to be stored
 * @param cbData size of the information in bytes
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegSetValueEx (HKEY h_key, const char *lpValueName, DWORD Reserved,
               DWORD dwType, const char *lpData, DWORD cbData)
{
  return ERROR_UNKNOWN;
}

/**
 * Deletes a subkey and its values
 * @param h_key handle to an open registry key
 * @param lpSubKey subkey name  that this function opens or creates
 * @return ERROR_SUCCESS if it completed successfully
 */
static Sint32
RegDeleteKey (HKEY hKey, const char *lpSubKey)
{
  return ERROR_UNKNOWN;
}
#endif

/**
 * Encrypt a string
 * @param src
 * @param size
 * @param dst
 * @param code
 */
static void
counter_crypt (char *src, int size, char *dst, char *code)
{
  unsigned char c;
  Sint32 pos = 0, pc = 0, cl;
  cl = strlen (code);
  while (pos < size)
    {
      c = src[pos];
      c = 255 - (c ^ code[pc]);
      dst[pos] = c;
      pos++;
      pc = (pc + 1) % cl;
    }
  dst[pos] = 0;
}

/**
 * Decrypt a string
 * @param src
 * @param size
 * @param dst
 * @param code
 */
static void
counter_uncrypt (char *src, Sint32 size, char *dst, char *code)
{
  unsigned char c;
  Sint32 pos = 0, pc = 0, cl;
  cl = strlen (code);
  while (pos < size)
    {
      c = src[pos];
      c = 255 - (c ^ code[pc]);
      dst[pos] = c;
      pos++;
      pc = (pc + 1) % cl;
    }
  dst[pos] = 0;
}

/**
 * Load games list in the counter file
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
counter_load_file (void)
{
  Sint32 ver;
  Uint32 n, nb, chk, pos;
  DWORD size;
  char *buffer = NULL;
  HKEY hk;
  Uint32 regerr;
  size = 0;
  regerr = RegOpenKeyEx (HKEY_CURRENT_USER,
                         "Software\\TLK Games\\Data",
                         0, KEY_QUERY_VALUE, &hk);
  if (regerr == ERROR_SUCCESS)
    {
      DWORD regtype = REG_BINARY;
      regerr = RegQueryValueEx (hk,
                                "Count",
                                NULL, &regtype, (LPBYTE) buffer, &size);
      buffer = (char *) memory_allocation (size + 1);
      if (NULL == buffer)
        {
          return FALSE;
        }
      regerr = RegQueryValueEx (hk,
                                "Count", 0, &regtype, (LPBYTE) buffer, &size);

      if (regerr != ERROR_SUCCESS)
        {
          RegCloseKey (hk);
          free_memory (buffer);
          return FALSE;
        }
      RegCloseKey (hk);
    }
  else
    {
      return FALSE;
    }


  /* checksum */
  chk = 0;
  for (n = 4; n < size; n++)
    {
      chk += (unsigned char) buffer[n];
    }
  if (size > 0)
    {
      n = *((Uint32 *) buffer);
    }
  else
    {
      n = 1;
    }
  if (n != chk)
    {
      /* bad checksum: delete file! */
      regerr = RegOpenKeyEx (HKEY_CURRENT_USER,
                             "Software\\TLK Games\\Data", 0, KEY_WRITE, &hk);
      if (regerr == ERROR_SUCCESS)
        {
          regerr = RegDeleteKey (hk, "Count");
        }
      RegCloseKey (hk);
      free_memory (buffer);
      return FALSE;
    }
  /* uncrypt the buffer */
  counter_uncrypt (buffer, size, buffer, (char *) Codecounter_cryptage);
  pos = 4;
  /* version */
  ver = *((Uint32 *) (buffer + pos));
  pos += 4;
  /* number of input */
  nb = *((Uint32 *) (buffer + pos));
  pos += 4;
  num_of_games = nb;
  GameCount =
    (TLKGameInfo *) memory_allocation (sizeof (TLKGameInfo) *
                                       (num_of_games + 1));
  if (GameCount == NULL)
    {
      return FALSE;
    }
  for (n = 0; n < num_of_games; n++)
    {
      GameCount[n].count = *((Uint32 *) (buffer + pos));
      pos += 4;
      GameCount[n].time = *((Uint32 *) (buffer + pos));
      pos += 4;
      GameCount[n].version = *((Uint32 *) (buffer + pos));
      pos += 4;
      nb = *((Uint32 *) (buffer + pos));
      pos += 4;
      memcpy (GameCount[n].name, buffer + pos, nb);
      pos += nb;
    }
  if (buffer != NULL)
    {
      free_memory (buffer);
    }
  return TRUE;
}

/**
 * Save the counter file
 * @return
 */
static bool
counter_save_file (void)
{
  Uint32 n, nb, chk, pos;
  DWORD size;
  char *buffer = 0;
  Uint32 *pint = 0;
  HKEY hk;
  Uint32 regerr;
  size = 4 * 3;
  for (n = 0; n < num_of_games; n++)
    {
      size += 4 * 4 + strlen (GameCount[n].name) + 1;
    }
  buffer = (char *) memory_allocation (size + 1);
  if (buffer == NULL)
    {
      return FALSE;
    }
  /* 0 = checksum */
  pos = 4;
  /* file version number */
  pint = (Uint32 *) (buffer + pos);
  *pint = 2;
  pos += 4;
  /* number games */
  pint = (Uint32 *) (buffer + pos);
  *pint = num_of_games;
  pos += 4;
  for (n = 0; n < num_of_games; n++)
    {
      pint = (Uint32 *) (buffer + pos);
      *pint = GameCount[n].count;
      pos += 4;
      pint = (Uint32 *) (buffer + pos);
      *pint = GameCount[n].time;
      pos += 4;
      pint = (Uint32 *) (buffer + pos);
      *pint = GameCount[n].version;
      pos += 4;
      nb = strlen (GameCount[n].name) + 1;
      pint = (Uint32 *) (buffer + pos);
      *pint = nb;
      pos += 4;
      memcpy (buffer + pos, GameCount[n].name, nb);
      pos += nb;
    }
  /* crypt the buffer */
  counter_crypt (buffer, size, buffer, (char *) Codecounter_cryptage);
  /* generate checksum */
  chk = 0;
  for (n = 4; n < size; n++)
    {
      chk += (unsigned char) buffer[n];
    }
  pint = (Uint32 *) buffer;
  *pint = chk;
  DWORD regcm;
  regerr = RegCreateKeyEx (HKEY_CURRENT_USER,
                           "Software\\TLK Games\\Data",
                           0,
                           0,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE, 0, &hk, &regcm);
  if (regerr == ERROR_SUCCESS)
    {
      regerr = RegSetValueEx (hk,
                              "Count", 0, REG_BINARY, (LPBYTE) buffer, size);
      if (regerr != ERROR_SUCCESS)
        {
          free_memory (buffer);
          return FALSE;
        }
    }
  else
    {
      free_memory (buffer);
      return FALSE;
    }
  if (buffer != NULL)
    {
      free_memory (buffer);
    }
  return TRUE;
}

/**
 * Search a game from the list
 * @param name name of the game
 * @return index in the list, -1 otherwise
 */
static Sint32
counter_search_game (char *name)
{
  Uint32 n;
  if (NULL == GameCount)
    {
      return -1;
    }
  for (n = 0; n < num_of_games; n++)
    {
      if (strcmp (GameCount[n].name, name) == 0)
        {
          return n;
        }
    }
  return -1;
}

/**
 * Load the list of the games et increase the counter of the game
 * selected game in the counter file
 * The counter is re-initialized when a new version of the game is launched
 * If the file does not exist or that it is corrupted, it is recreated
 * @param gamename The name of the game (ie "Powermanga")
 * @param vmaj Major version
 * @param vmin Minor version
 * @return Number of times the game has been launched, 0 the first time
 */
Sint32
counter_shareware_update (char *gamename, Sint32 vmaj, Sint32 vmin)
{
  Sint32 cnt;
  Uint32 ver = (vmaj << 16) + vmin;
  cnt = 0;

  /* load file */
  if (!counter_load_file ())
    {
      /* first use or non-existent file */
      GameCount = (TLKGameInfo *) memory_allocation (sizeof (TLKGameInfo));
      if (GameCount == NULL)
        {
          return FALSE;
        }
      GameCount[0].count = 1;
      GameCount[0].time = 0;
      GameCount[0].version = ver;
      memcpy (GameCount[0].name, gamename, strlen (gamename) + 1);
      num_of_games = 1;
      counter_save_file ();
    }
  else
    {
      /* search a game */
      cnt = counter_search_game (gamename);
      if (cnt == -1)
        {
          /* add game */
          GameCount[num_of_games].count = 1;
          GameCount[0].time = 0;
          GameCount[0].version = ver;
          memcpy (GameCount[num_of_games].name, gamename,
                  strlen (gamename) + 1);
          num_of_games++;
          cnt = 0;
        }
      else
        {
          /* check version */
          if (GameCount[cnt].version < ver)
            {
              /* new version available */
              GameCount[cnt].version = ver;
              GameCount[cnt].count = 1;
              GameCount[cnt].time = 0;
              cnt = 0;
            }
          else
            {
              /* increase counter */
              GameCount[cnt].count++;
              cnt = GameCount[cnt].count;
            }
        }
      counter_save_file ();
    }
  return cnt;
}

/**
 * Release list of games
 */
void
counter_shareware_free (void)
{
  if (GameCount != NULL)
    {
      free_memory ((char *) GameCount);
      GameCount = NULL;
    }
  num_of_games = 0;
}

#endif
