/**
 * @file lispreader.c 
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
 * $Id: lispreader.c,v 1.19 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "lispreader.h"
#include "log_recorder.h"

typedef enum
{
  LISP_PATTERN_ANY = 1,
  LISP_PATTERN_SYMBOL,
  LISP_PATTERN_STRING,
  LISP_PATTERN_INTEGER,
  LISP_PATTERN_REAL,
  LISP_PATTERN_BOOLEAN,
  LISP_PATTERN_LIST,
  LISP_PATTERN_OR
} LISP_PATTERN_ENUM;

typedef enum
{
  LISP_STREAM_FILE = 1,
  LISP_STREAM_STRING,
  LISP_STREAM_ANY
} LISP_STREAM_ENUM;

typedef enum
{
  TOKEN_ERROR = -1,
  TOKEN_EOF = 0,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_SYMBOL,
  TOKEN_STRING,
  TOKEN_INTEGER,
  TOKEN_REAL,
  TOKEN_PATTERN_OPEN_PAREN,
  TOKEN_DOT,
  TOKEN_TRUE,
  TOKEN_FALSE
} TOKEN_ENUM;

#define MAX_TOKEN_LENGTH           1024
#define lisp_nil()           ((lisp_object_t*)0)
#define lisp_nil_p(obj)      (obj == 0)
#define lisp_integer_p(obj)  (lisp_type((obj)) == LISP_TYPE_INTEGER)
#define lisp_symbol_p(obj)   (lisp_type((obj)) == LISP_TYPE_SYMBOL)
#define lisp_string_p(obj)   (lisp_type((obj)) == LISP_TYPE_STRING)
#define lisp_cons_p(obj)     (lisp_type((obj)) == LISP_TYPE_CONS)
#define lisp_boolean_p(obj)  (lisp_type((obj)) == LISP_TYPE_BOOLEAN)
static char token_string[MAX_TOKEN_LENGTH + 1] = "";
static Sint32 token_length = 0;
static lisp_object_t end_marker = { LISP_TYPE_EOF, {{0, 0}} };
static lisp_object_t error_object = { LISP_TYPE_PARSE_ERROR, {{0, 0}} };
static lisp_object_t close_paren_marker = { LISP_TYPE_PARSE_ERROR, {{0, 0}} };
static lisp_object_t dot_marker = { LISP_TYPE_PARSE_ERROR, {{0, 0}} };

static char *lisp_string (lisp_object_t * obj);
static Sint32 lisp_integer (lisp_object_t * obj);
static float lisp_real (lisp_object_t * obj);
static Sint32 lisp_type (lisp_object_t * obj);
static void lisp_dump (lisp_object_t * obj, FILE * out);

/**
 * Clear the current string
 */
static void
_token_clear (void)
{
  token_string[0] = '\0';
  token_length = 0;
}

/**
 *  Copy current string
 * @param c A character code of string read currently
 */
static void
_token_append (char c)
{
  assert (token_length < MAX_TOKEN_LENGTH);
  token_string[token_length++] = c;
  token_string[token_length] = '\0';
}

/**
 * Return the next char of the configuratoin file
 * @param stream Pointer to a lisp_stream_t
 * @return Code of the char
 */
static Sint32
_next_char (lisp_stream_t * stream)
{
  char c;
  switch (stream->type)
    {
    case LISP_STREAM_FILE:
      return getc (stream->v.file);
    case LISP_STREAM_STRING:
      {
        c = stream->v.string.buf[stream->v.string.pos];
        if (c == 0)
          {
            return EOF;
          }
        ++stream->v.string.pos;
        return c;
      }
    case LISP_STREAM_ANY:
      return stream->v.any.next_char (stream->v.any.data);
    }
  assert (0);
  return EOF;
}

/**
 *
 * @param c
 * @param stream
 */
static void
_unget_char (char c, lisp_stream_t * stream)
{
  switch (stream->type)
    {
    case LISP_STREAM_FILE:
      ungetc (c, stream->v.file);
      break;
    case LISP_STREAM_STRING:
      --stream->v.string.pos;
      break;
    case LISP_STREAM_ANY:
      stream->v.any.unget_char (c, stream->v.any.data);
      break;
    default:
      assert (0);
    }
}

/**
 * Scan the configuration file 
 * @input stream A pointer to a lisp_stream_t struture
 * @return A return code
 */
static Sint32
_scan (lisp_stream_t * stream)
{
  static const char *delims = "\"();";
  Sint32 c;
  Sint32 have_nondigits;
  Sint32 have_digits;
  Sint32 have_floating_point;
  bool search = TRUE;
  _token_clear ();
  do
    {
      c = _next_char (stream);
      if (c == EOF)
        {
          return TOKEN_EOF;
        }
      /* comment start : all comments are ignored */
      else if (c == ';')
        {
          while (search)
            {
              c = _next_char (stream);
              if (c == EOF)
                {
                  return TOKEN_EOF;
                }
              else if (c == '\n')
                {
                  break;
                }
            }
        }
    }
  while (isspace (c));

  switch (c)
    {
    case '(':
      return TOKEN_OPEN_PAREN;

    case ')':
      return TOKEN_CLOSE_PAREN;

    case '"':
      while (search)
        {
          c = _next_char (stream);
          if (c == EOF)
            {
              return TOKEN_ERROR;
            }
          if (c == '"')
            {
              break;
            }
          if (c == '\\')
            {
              c = _next_char (stream);
              switch (c)
                {
                case EOF:
                  return TOKEN_ERROR;

                case 'n':
                  c = '\n';
                  break;

                case 't':
                  c = '\t';
                  break;
                }
            }
          _token_append ((char) c);
        }
      return TOKEN_STRING;

    case '#':
      c = _next_char (stream);
      if (c == EOF)
        {
          return TOKEN_ERROR;
        }
      switch (c)
        {
        case 't':
          return TOKEN_TRUE;
        case 'f':
          return TOKEN_FALSE;
        case '?':
          c = _next_char (stream);
          if (c == EOF)
            {
              return TOKEN_ERROR;
            }
          if (c == '(')
            {
              return TOKEN_PATTERN_OPEN_PAREN;
            }
          else
            {
              return TOKEN_ERROR;
            }
        }
      return TOKEN_ERROR;

    default:
      if (isdigit (c) || c == '-')
        {
          have_nondigits = 0;
          have_digits = 0;
          have_floating_point = 0;
          do
            {
              if (isdigit (c))
                {
                  have_digits = 1;
                }
              else if (c == '.')
                {
                  have_floating_point++;
                }
              _token_append ((char) c);
              c = _next_char (stream);
              if (c != EOF && !isdigit (c) && !isspace (c) && c != '.'
                  && !strchr (delims, c))
                {
                  have_nondigits = 1;
                }
            }
          while (c != EOF && !isspace (c) && !strchr (delims, c));
          if (c != EOF)
            {
              _unget_char ((char) c, stream);
            }
          if (have_nondigits || !have_digits || have_floating_point > 1)
            {
              return TOKEN_SYMBOL;
            }
          else if (have_floating_point == 1)
            {
              return TOKEN_REAL;
            }
          else
            {
              return TOKEN_INTEGER;
            }
        }
      else
        {
          if (c == '.')
            {
              c = _next_char (stream);
              if (c != EOF && !isspace (c) && !strchr (delims, c))
                {
                  _token_append ('.');
                }
              else
                {
                  _unget_char ((char) c, stream);
                  return TOKEN_DOT;
                }
            }
          do
            {
              _token_append ((char) c);
              c = _next_char (stream);
            }
          while (c != EOF && !isspace (c) && !strchr (delims, c));
          if (c != EOF)
            {
              _unget_char ((char) c, stream);
            }
          return TOKEN_SYMBOL;
        }
    }
}

/**
 * Release a object type
 * @param obj A pointer to a lisp_object_t structure
 */
void
lisp_free (lisp_object_t * obj)
{
  if (obj == NULL)
    {
      return;
    }
  switch (obj->type)
    {
    case LISP_TYPE_INTERNAL:
    case LISP_TYPE_PARSE_ERROR:
    case LISP_TYPE_EOF:
      return;
    case LISP_TYPE_SYMBOL:
    case LISP_TYPE_STRING:
      free_memory ((char *) obj->v.string);
      break;
    case LISP_TYPE_CONS:
    case LISP_TYPE_PATTERN_CONS:
      lisp_free (obj->v.cons.car);
      lisp_free (obj->v.cons.cdr);
      break;
    case LISP_TYPE_PATTERN_VAR:
      lisp_free (obj->v.pattern.sub);
      break;
    }
  free_memory ((char *) obj);
}

/**
 * Create a lisp_object_t structure
 * @param type A object type code
 * @return A pointer to a lisp_object_t
 */
static lisp_object_t *
lisp_object_alloc (Sint32 type)
{
  lisp_object_t *obj =
    (lisp_object_t *) memory_allocation (sizeof (lisp_object_t));
  obj->type = type;
  return obj;
}

/**
 * Initialize string of steam
 * @param stream A pointer to a lisp_stream_t structure
 * @param buf Pointer to string
 * @return A pointer to a lisp_stream_t structure
 */
lisp_stream_t *
lisp_stream_init_string (lisp_stream_t * stream, char *buf)
{
  stream->type = LISP_STREAM_STRING;
  stream->v.string.buf = buf;
  stream->v.string.pos = 0;
  return stream;
}

/**
 * Create a integer type
 * @param value A interger value
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_integer (Sint32 value)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_INTEGER);
  obj->v.integer = value;
  return obj;
}

/**
 * Create a real type
 * @param value A real value
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_real (float value)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_REAL);
  obj->v.real = value;
  return obj;
}

/**
 * Create a symbol type
 * @param value
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_symbol (const char *value)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_SYMBOL);
  obj->v.string = string_duplicate (value);
  return obj;
}

/**
 * Create a string type
 * @param value
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_string (const char *value)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_STRING);
  obj->v.string = string_duplicate (value);
  return obj;
}

/**
 * Create a "cons" element 
 * @param car Contents of Address register (first element)
 * @param value cdr (Contents of Decrement register) 
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_cons (lisp_object_t * car, lisp_object_t * cdr)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_CONS);
  obj->v.cons.car = car;
  obj->v.cons.cdr = cdr;
  return obj;
}

/**
 * Create a boolean type
 * @param value
 * @return A pointer to a lisp_object_t structure 
 */
lisp_object_t *
lisp_make_boolean (Sint32 value)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_BOOLEAN);
  obj->v.integer = value ? 1 : 0;
  return obj;
}

/**
 * @param car
 * @param cdr
 * @return A pointer to a lisp_object_t structure
 */
static lisp_object_t *
lisp_make_pattern_cons (lisp_object_t * car, lisp_object_t * cdr)
{
  lisp_object_t *obj = lisp_object_alloc (LISP_TYPE_PATTERN_CONS);
  obj->v.cons.car = car;
  obj->v.cons.cdr = cdr;
  return obj;
}

/**
 * Parse the configuration file
 * @param in A pointer to a lisp_stream_t structure
 * @return A pointer to a lisp_object_t structure
 */
lisp_object_t *
lisp_read (lisp_stream_t * in)
{
  lisp_object_t *last, *car;
  Sint32 token = _scan (in);
  lisp_object_t *obj = lisp_nil ();
  if (token == TOKEN_EOF)
    {
      return &end_marker;
    }
  switch (token)
    {
    case TOKEN_ERROR:
      return &error_object;

    case TOKEN_EOF:
      return &end_marker;
    case TOKEN_OPEN_PAREN:
    case TOKEN_PATTERN_OPEN_PAREN:
      {
        last = lisp_nil ();
        do
          {
            car = lisp_read (in);
            if (car == &error_object || car == &end_marker)
              {
                lisp_free (obj);
                return &error_object;
              }
            else if (car == &dot_marker)
              {
                if (lisp_nil_p (last))
                  {
                    lisp_free (obj);
                    return &error_object;
                  }

                car = lisp_read (in);
                if (car == &error_object || car == &end_marker)
                  {
                    lisp_free (obj);
                    return car;
                  }
                else
                  {
                    last->v.cons.cdr = car;

                    if (_scan (in) != TOKEN_CLOSE_PAREN)
                      {
                        lisp_free (obj);
                        return &error_object;
                      }

                    car = &close_paren_marker;
                  }
              }
            else if (car != &close_paren_marker)
              {
                if (lisp_nil_p (last))
                  {
                    obj = last =
                      (token ==
                       TOKEN_OPEN_PAREN ? lisp_make_cons (car,
                                                          lisp_nil ()) :
                       lisp_make_pattern_cons (car, lisp_nil ()));

                  }
                else
                  {
                    last = last->v.cons.cdr =
                      lisp_make_cons (car, lisp_nil ());
                  }
              }
          }
        while (car != &close_paren_marker);
      }
      return obj;

    case TOKEN_CLOSE_PAREN:
      return &close_paren_marker;

    case TOKEN_SYMBOL:
      return lisp_make_symbol (token_string);

    case TOKEN_STRING:
      return lisp_make_string (token_string);

    case TOKEN_INTEGER:
      return lisp_make_integer (atoi (token_string));

    case TOKEN_REAL:
      return lisp_make_real ((float) atof (token_string));

    case TOKEN_DOT:
      return &dot_marker;

    case TOKEN_TRUE:
      return lisp_make_boolean (1);

    case TOKEN_FALSE:
      return lisp_make_boolean (0);
    }

  assert (0);
  return &error_object;
}


/**
 * Return the code of an object type
 * @param obj A pointer to a lisp_object_t structure
 * @return An object type code
 */
Sint32
lisp_type (lisp_object_t * obj)
{
  if (obj == NULL)
    {
      return LISP_TYPE_NIL;
    }
  return obj->type;
}

/**
 * Return integer value of an integer type
 * @param obj A pointer to a lisp_object_t structure
 * @return A interger value
 */
Sint32
lisp_integer (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_INTEGER);
  return obj->v.integer;
}

/** 
 * Return string of a symbol type
 * @param obj A pointer to a lisp_object_t structure
 * @return A pointer to a string
 */
char *
lisp_symbol (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_SYMBOL);
  return obj->v.string;
}

/** 
 * Return a string of a string object
 * @param obj A pointer to a lisp_object_t structure
 * @return A pointer to a string
 */
char *
lisp_string (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_STRING);
  return obj->v.string;
}

/** 
 * Return value of a boolean object
 * @param obj A pointer to a lisp_object_t structure
 * @return 0 or 1
 */
Sint32
lisp_boolean (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_BOOLEAN);
  return obj->v.integer;
}

/** 
 * Return float value of a real type
 * @param obj A pointer to a lisp_object_t structure
 * @return A float value
 */
float
lisp_real (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_REAL || obj->type == LISP_TYPE_INTEGER);
  if (obj->type == LISP_TYPE_INTEGER)
    {
      return (float) (obj->v.integer);
    }
  return obj->v.real;
}

/**
 *
 * @param obj A pointer to a lisp_object_t structure
 * @return
 */
lisp_object_t *
lisp_car (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);
  return obj->v.cons.car;
}

/**
 *
 * @param obj A pointer to a lisp_object_t structure
 * @return
 */
lisp_object_t *
lisp_cdr (lisp_object_t * obj)
{
  assert (obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);
  return obj->v.cons.cdr;
}

/** 
 * Dump a lisp_object_t structure
 * @param obj A pointer to a lisp_object_t structure
 * @param FILE A output stream
 */
void
lisp_dump (lisp_object_t * obj, FILE * out)
{
  char *p;
  if (obj == 0)
    {
      fprintf (out, "()");
      return;
    }

  switch (lisp_type (obj))
    {
    case LISP_TYPE_EOF:
      fputs ("#<eof>", out);
      break;

    case LISP_TYPE_PARSE_ERROR:
      fputs ("#<error>", out);
      break;

    case LISP_TYPE_INTEGER:
      fprintf (out, "%d", lisp_integer (obj));
      break;

    case LISP_TYPE_REAL:
      fprintf (out, "%f", lisp_real (obj));
      break;

    case LISP_TYPE_SYMBOL:
      fputs (lisp_symbol (obj), out);
      break;

    case LISP_TYPE_STRING:
      {
        fputc ('"', out);
        for (p = lisp_string (obj); *p != 0; ++p)
          {
            if (*p == '"' || *p == '\\')
              fputc ('\\', out);
            fputc (*p, out);
          }
        fputc ('"', out);
      }
      break;

    case LISP_TYPE_CONS:
    case LISP_TYPE_PATTERN_CONS:
      fputs (lisp_type (obj) == LISP_TYPE_CONS ? "(" : "#?(", out);
      while (obj != 0)
        {
          lisp_dump (lisp_car (obj), out);
          obj = lisp_cdr (obj);
          if (obj != 0)
            {
              if (lisp_type (obj) != LISP_TYPE_CONS
                  && lisp_type (obj) != LISP_TYPE_PATTERN_CONS)
                {
                  fputs (" . ", out);
                  lisp_dump (obj, out);
                  break;
                }
              else
                fputc (' ', out);
            }
        }
      fputc (')', out);
      break;

    case LISP_TYPE_BOOLEAN:
      if (lisp_boolean (obj))
        {
          fputs ("#t", out);
        }
      else
        {
          fputs ("#f", out);
        }
      break;
    default:
      assert (0);
    }
}

/**
 * Search a attribute name
 * @param lst Pointer to a lisp_object_t
 * @param name String representing the attribute name
 * @return Pointer to a lisp_object_t object 
 */
lisp_object_t *
search_for (lisp_object_t * lst, const char *name)
{
  lisp_object_t *cur;
  lisp_object_t *cursor = lst;
  while (!lisp_nil_p (cursor))
    {
      cur = lisp_car (cursor);
      if (!lisp_cons_p (cur) || !lisp_symbol_p (lisp_car (cur)))
        {
          lisp_dump (cur, stdout);
          LOG_ERR ("Read error in search");
        }
      else
        {
          if (strcmp (lisp_symbol (lisp_car (cur)), name) == 0)
            {
              return lisp_cdr (cur);
            }
        }

      cursor = lisp_cdr (cursor);
    }
  return NULL;
}


lisp_object_t *
lisp_car_int (lisp_object_t * lst, Sint32 * i)
{
  if (!lisp_cons_p (lst) || !lisp_integer_p (lisp_car (lst)))
    {
      LOG_ERR ("LispReader expected integer at head of list");
      return NULL;
    }

  *i = lisp_integer (lisp_car (lst));
  return lisp_cdr (lst);;
}

/**
 * Read an integer
 * @param lst Pointer to a lisp_object_t
 * @param name String representing the attribute name
 * @param i Pointer to the integer which will contain the value of the
 *          required attribute 
 * @return TRUE if the attribute were correctly found and read, or FALSE
 *         otherwise
 */
bool
lisp_read_int (lisp_object_t * lst, const char *name, Sint32 * i)
{
  lisp_object_t *obj = search_for (lst, name);
  if (obj == NULL)
    {
      return FALSE;
    }
  if (!lisp_integer_p (lisp_car (obj)))
    {
      LOG_ERR ("LispReader expected type integer at token: %s", name);
      return FALSE;
    }
  *i = lisp_integer (lisp_car (obj));
  return TRUE;
}

/**
 * Read a boolean
 * @param lst Pointer to a lisp_object_t
 * @param name String representing the attribute name
 * @param i Pointer to the boolean which will contain the value of the
 *          required attribute 
 * @return TRUE if the attribute were correctly found and read, or FALSE
 *         otherwise
 */
bool
lisp_read_bool (lisp_object_t * lst, const char *name, bool * b)
{
  lisp_object_t *obj = search_for (lst, name);
  if (obj == NULL)
    {
      return FALSE;
    }

  if (!lisp_boolean_p (lisp_car (obj)))
    {
      LOG_ERR ("LispReader expected type bool at token: %s", name);
      return FALSE;
    }
  *b = lisp_boolean (lisp_car (obj));
  return TRUE;
}

/**
 * Read a string 
 * @param lst Pointer to a lisp_object_t
 * @param name String representing the attribute name
 * @param i Pointer to the string which will contain the value of the
 *          required attribute 
 * @return TRUE if the attribute were correctly found and read, or FALSE
 *         otherwise
 */
bool
lisp_read_string (lisp_object_t * lst, const char *name, char **str)
{
  lisp_object_t *obj = search_for (lst, name);
  if (obj == NULL)
    {
      return FALSE;
    }
  if (!lisp_string_p (lisp_car (obj)))
    {
      LOG_ERR ("expected type real at token: %s", name);
      return FALSE;
    }
  *str = lisp_string (lisp_car (obj));
  return TRUE;
}

/**
 * Read a configuration file
 * @param filename The filename specified by path
 * @return A pointer to a lisp_object_t
 */
lisp_object_t *
lisp_read_file (char *filename)
{
  lisp_object_t *root_obj;
  lisp_stream_t *stream;
  Uint32 filesize;
  char *filedata;

  /* read filedata */
  filedata = load_absolute_file (filename, &filesize);
  if (filedata == NULL)
    {
      return NULL;
    }
  stream = (lisp_stream_t *) memory_allocation (sizeof (lisp_stream_t));
  if (stream == NULL)
    {
      LOG_ERR ("not enough memmory to allocate 'lisp_stream_t'", filename);
      free_memory (filedata);
      return NULL;
    }
  stream->type = LISP_STREAM_STRING;
  stream->v.string.buf = filedata;
  stream->v.string.pos = 0;
  root_obj = lisp_read (stream);
  free_memory ((char *) stream);
  free_memory (filedata);
  return root_obj;
}
