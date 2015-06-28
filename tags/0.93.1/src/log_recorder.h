/**
 * @file log_recorder.h 
 * @brief Records log messages  
 * @created 2009-02-21 
 * @date 2015-06-28 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: log_recorder.h,v 1.10 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __LOG_RECORD__
#define __LOG_RECORD__

#if defined(__GNUC__) && !defined(__clang__) 
#define POWERMANGA_LOG_ENABLED
#endif

#if !defined(POWERMANGA_LOG_ENABLED)

#define LOG_DBG(...)  ( (void)0 )
#define LOG_INF(...)  ( (void)0 )
#define LOG_WARN(...)  ( (void)0 )
#define LOG_ERR(...)  ( (void)0 )

#else

#include <time.h>

typedef enum
{
  LOG_NOTHING,
  LOG_ERROR,
  LOG_INFO,
  LOG_WARNING,
  LOG_DEBUG,
  LOG_NUMOF
} LOG_LEVELS;

void log_message (LOG_LEVELS level, const char *filename, Sint32 line_num,
                  const char *function, ...);
void log_set_level (LOG_LEVELS verbose);
bool log_initialize (LOG_LEVELS verbose);
void log_close (void);

#define LOG_ERR(...)  log_message \
  (LOG_ERROR, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INF(...)  log_message \
  (LOG_INFO, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARN(...)  log_message \
  (LOG_WARNING, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_DBG(...)  log_message \
  (LOG_DEBUG, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif
#endif
