/**
 * @file log_recorder.c 
 * @brief Records log messages  
 * @created 2009-02-21 
 * @date 2014-10-12
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: log_recorder.c,v 1.11 2012/08/26 19:16:07 gurumeditation Exp $
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
#include "log_recorder.h"
#include <time.h>

#if defined(POWERMANGA_LOG_ENABLED)
#if defined(UNDER_DEVELOPMENT)
#define ENABLE_LOG_FILE
#endif

const Uint32 LOG_MESSAGE_SIZE = 1536;
const Uint32 LOG_OUTPUT_SIZE = 2048;
#if defined(ENABLE_LOG_FILE)
static FILE *log_fstream = NULL;
#endif
static char *buffer_message = NULL;
static char *output_message = NULL;
static LOG_LEVELS verbose_level = LOG_NOTHING;
#if !defined(_WIN32_WCE)
static struct tm *cur_time = NULL;
#endif

static const char *log_levels[LOG_NUMOF] = {
  "(--)",
  "(EE)",
  "(II)",
  "(WW)",
  "(DD)"
};

/**
 * Change level of debug
 * @param verbose Level of debug
 */
void
log_set_level (LOG_LEVELS verbose)
{
  if (verbose > LOG_NUMOF - 1)
    {
      verbose = (LOG_LEVELS) (LOG_NUMOF - 1);
    }
  verbose_level = verbose;
}

/**
 * Initialize log proccess, allocate buffer and open a file
 * @param verbose Level of debug
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
log_initialize (LOG_LEVELS verbose)
{
#if defined(_WIN32_WCE)
  char *pathname;
#endif
#if defined(ENABLE_LOG_FILE)
  const char *filename;
#endif
  log_set_level (verbose);
#ifndef WIN32
  if (cur_time == NULL)
    {
      cur_time = (struct tm *) memory_allocation (sizeof (struct tm));
      if (cur_time == NULL)
        {
          fprintf (stderr, "log_recorder.c/log_initialize()"
                   "not enough memory to allocate %i bytes\n",
                   LOG_MESSAGE_SIZE);
          return FALSE;
        }
    }
#endif
  if (buffer_message == NULL)
    {
      buffer_message = memory_allocation (LOG_MESSAGE_SIZE);
      if (buffer_message == NULL)
        {
          fprintf (stderr, "log_recorder.c/log_initialize()"
                   "not enough memory to allocate %i bytes\n",
                   LOG_MESSAGE_SIZE);
          return FALSE;
        }
    }
  if (output_message == NULL)
    {
      output_message = memory_allocation (LOG_OUTPUT_SIZE);
      if (output_message == NULL)
        {
          fprintf (stderr, "log_recorder.c/log_initialize()"
                   "not enough memory to allocate %i bytes\n",
                   LOG_OUTPUT_SIZE);
          return FALSE;
        }
    }

#if defined(ENABLE_LOG_FILE)
#if !defined(_WIN32)
  filename = "/tmp/powermanga-log.txt";
#else
  filename = "powermanga-log.txt";
#endif
#if defined(_WIN32_WCE)
  pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      fprintf (stderr, "log_recorder.c/log_initialize()"
               "can't locate file: %s", filename);
      return FALSE;
    }
  filename = pathname;
#endif
  log_fstream = fopen (filename, "a");
  if (log_fstream == NULL)
    {
#if defined(_WIN32_WCE)
      fprintf (stderr, "log_recorder.c/log_initialize()"
               "fopen(%s) failed\n", filename);
      free_memory (pathname);
#else
      fprintf (stderr, "log_recorder.c/log_initialize()"
               "fopen(%s) failed (%s)\n", filename, strerror (errno));
#endif
      return FALSE;
    }
#if defined(_WIN32_WCE)
  free_memory (pathname);
#endif
#endif
  return TRUE;
}

/**
 * Close file log and release memories
 */
void
log_close (void)
{
#if defined(ENABLE_LOG_FILE)
  if (log_fstream != NULL)
    {
      fclose (log_fstream);
      log_fstream = NULL;
    }
#endif
  if (buffer_message != NULL)
    {
      free_memory (buffer_message);
      buffer_message = NULL;
    }
  if (output_message != NULL)
    {
      free_memory (output_message);
      output_message = NULL;
    }
#ifndef WIN32
  if (cur_time != NULL)
    {
      free_memory ((char *) cur_time);
      cur_time = NULL;
    }
#endif
}

/**
 * Log message to a message 
 * @param level The level of this message 
 * @param filename The filename in which this function is called
 * @param line_num The line number on which this function is called
 * @param function The function name in which this function is called
 * @param format The format string to be appended to the log
 * @param ... The arguments to use to fill out format 
 */
#if defined(ENABLE_LOG_FILE)
static void
log_write (LOG_LEVELS level, const char *filename, Sint32 line_num,
           const char *function, const char *message)
{
  size_t msg_len;
#if !defined(_WIN32_WCE)
  time_t now;
  if (log_fstream == NULL)
    {
      return;
    }
  now = time (NULL);
  if (now == (time_t) - 1)
    {
      fprintf (stderr, "log_recorder.c/log_write()"
               "Can't log line: time() failed.\n");
      return;
    }

  /* Get the current time */
#ifndef WIN32
  localtime_r (&now, cur_time);
#else
  cur_time = localtime (&now);
#endif
  if (cur_time == NULL)
    {
      fprintf (stderr, "log_recorder.c/log_write()"
               "localtime(_r)() failed.\n");
      return;
    }
  msg_len = snprintf (output_message, LOG_OUTPUT_SIZE,
                      "%04u-%02u-%02u %02u:%02u:%02u %s "
                      "[File: %s][Line: %d][Function: %s] %s\n",
                      cur_time->tm_year + 1900,
                      cur_time->tm_mon + 1,
                      cur_time->tm_mday,
                      cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec,
                      log_levels[level], filename, line_num, function,
                      message);
#else
  SYSTEMTIME cur_time;
  GetLocalTime (&cur_time);
  msg_len = _snprintf (output_message, LOG_OUTPUT_SIZE,
                       "%04u-%02u-%02u %02u:%02u:%02u %s "
                       "[File: %s][Line: %d][Function: %s] %s\n",
                       cur_time.wYear,
                       cur_time.wMonth,
                       cur_time.wDay,
                       cur_time.wHour, cur_time.wMinute, cur_time.wSecond,
                       log_levels[level], filename, line_num, function,
                       message);
#endif
  if (fwrite (output_message, sizeof (char), msg_len, log_fstream) != msg_len)
    {
      fprintf (stderr, "log_recorder.c/log_write()" "fwrite() failed!\n");
    }
}
#endif

/**
 * Log messages to the screen
 * @param level The level of this message 
 * @param filename The filename in which this function is called
 * @param line_num The line number on which this function is called
 * @param function The function name in which this function is called
 * @param format The format string to be appended to the log
 * @param ... The arguments to use to fill out format 
 */
static void
log_put (LOG_LEVELS level, const char *filename, Sint32 line_num,
         const char *function, const char *message)
{
#if defined (_WIN32)
  _snprintf (output_message, LOG_OUTPUT_SIZE, "%s %s [%s:%d, %s]\n",
             log_levels[level], message, filename, line_num, function);
  /* OutputDebugString(output_message); */
#else
  snprintf (output_message, LOG_OUTPUT_SIZE, "%s %s [%s:%d, %s]\n",
            log_levels[level], message, filename, line_num, function);
#endif
  if (level == LOG_ERROR)
    {
      fprintf (stderr, "%s", output_message);
    }
  else
    {
      fprintf (stdout, "%s", output_message);
    }
}

/**
 * Log messages to the screen and/or a file.
 * @param level The level of this message 
 * @param filename The filename in which this function is called
 * @param line_num The line number on which this function is called
 * @param function The function name in which this function is called
 * @param format The format string to be appended to the log
 * @param ... The arguments to use to fill out format 
 */
static void
write_log (LOG_LEVELS level, const char *filename,
           Sint32 line_num, const char *function,
           const char *format, va_list args)
{
  Sint32 msg_len;
  if (buffer_message == NULL)
    {
      return;
    }
#if defined (_WIN32)
  msg_len = _vsnprintf (buffer_message, LOG_MESSAGE_SIZE, format, args);
#else
  msg_len = vsnprintf (buffer_message, LOG_MESSAGE_SIZE, format, args);
#endif
  if (msg_len < 1)
    {
      return;
    }
  buffer_message[msg_len] = 0;
#if defined(ENABLE_LOG_FILE)
  log_write (level, filename, line_num, function, buffer_message);
#endif
  /* put the message in the console */
  log_put (level, filename, line_num, function, buffer_message);
}

/**
 * Log messages to the screen and/or a file.
 * @param level The level of this message 
 * @param filename The filename in which this function is called
 * @param line_num The line number on which this function is called
 * @param function The function name in which this function is called
 * @param ... The arguments to use to fill out format 
 */
void
log_message (LOG_LEVELS level, const char *filename, Sint32 line_num,
             const char *function, ...)
{
  va_list args;
  const char *format;
  if (level > verbose_level)
    {
      return;
    }
  va_start (args, function);
  format = va_arg (args, const char *);
  write_log (level, filename, line_num, function, format, args);
  va_end (args);
}
#endif
