/**
 * @file lispreader.h 
 * @brief Parse configuration file 
 * @created 2007-06-15
 * @date 2015-01-10 
 * @author Mark Probst 
 * @author Ingo Ruhnke <grumbel@gmx.de>
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2000 Mark Probst
 * copyright (c) 2002 Ingo Ruhnke <grumbel@gmx.de>
 * copyright (c) 2007-2015 TLK Games all rights reserved
 * $Id: lispreader.h,v 1.4 2012/08/26 17:09:14 gurumeditation Exp $
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
#ifndef __LISPREADER__
#define __LISPREADER__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    LISP_TYPE_INTERNAL = -3,
    LISP_TYPE_PARSE_ERROR = -2,
    LISP_TYPE_EOF = -1,
    LISP_TYPE_NIL = 0,
    LISP_TYPE_SYMBOL,
    LISP_TYPE_INTEGER,
    LISP_TYPE_STRING,
    LISP_TYPE_REAL,
    LISP_TYPE_CONS,
    LISP_TYPE_PATTERN_CONS,
    LISP_TYPE_BOOLEAN,
    LISP_TYPE_PATTERN_VAR
  } LISP_TYPE_ENUM;

  typedef struct
  {
    Sint32 type;
    union
    {
      FILE *file;
      struct
      {
        char *buf;
        Sint32 pos;
      }
      string;
      struct
      {
        void *data;
          Sint32 (*next_char) (void *data);
        void (*unget_char) (char c, void *data);
      }
      any;
    } v;
  }
  lisp_stream_t;

  typedef struct _lisp_object_t lisp_object_t;
  struct _lisp_object_t
  {
    Sint32 type;
    union
    {
      struct
      {
        struct _lisp_object_t *car;
        struct _lisp_object_t *cdr;
      }
      cons;
      char *string;
      Sint32 integer;
      float real;
      struct
      {
        Sint32 type;
        Sint32 index;
        struct _lisp_object_t *sub;
      }
      pattern;
    } v;
  };

  lisp_object_t *search_for (lisp_object_t * lst, const char *name);
  lisp_object_t *lisp_car_int (lisp_object_t * lst, Sint32 * i);
  bool lisp_read_int (lisp_object_t * lst, const char *name, Sint32 * i);
  bool lisp_read_bool (lisp_object_t * lst, const char *name, bool * b);
  bool lisp_read_string (lisp_object_t * lst, const char *name, char **str);
  lisp_object_t *lisp_read_file (char *filename);
  char *lisp_symbol (lisp_object_t * obj);
  lisp_object_t *lisp_car (lisp_object_t * obj);
  lisp_object_t *lisp_cdr (lisp_object_t * obj);
  void lisp_free (lisp_object_t * obj);

#ifdef __cplusplus
}
#endif
#endif
