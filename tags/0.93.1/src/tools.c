/**
 * @file tools.c
 * @brief handle memory allocation and file access
 * @created 1999-08-19
 * @date 2011-02-17
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: tools.c,v 1.53 2012/08/26 15:44:26 gurumeditation Exp $
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
#include "log_recorder.h"
#include "tools.h"
#include "config_file.h"
#include <stdio.h>

#if defined (USE_MALLOC_WRAPPER)
/**
 * memory-block description structure
 */
typedef struct
{
  /** Pointer to memory zone */
  char *addr;
  /** Size of memory zone in bytes */
  Sint32 size;
}
mem_struct;
mem_struct *memory_list_base = NULL;
mem_struct *memory_list = NULL;
/** Size of memory table */
static Uint32 memory_list_size;
/** Maximum number of memory zones being able to be allocated */
static Uint32 mem_maxnumof_zones;
/** Number of currently memory zones */
Uint32 mem_numof_zones;
/** Total memory size currently allocated */
Uint32 mem_total_size;
/** Maximum number of memory zones reached */
static Uint32 mem_maxreached_zones;
#endif
Uint32 loops_counter;
#ifdef POWERMANGA_SDL
static Uint32 time_begin;
static Uint32 ticks_previous;
#else
static struct timeval time_begin;
static struct timeval ticks_previous;
#endif
static Uint16 little_endian_to_ushort (Uint16 * _pMem);
/** Prefixe where the data files are localised */
static const char prefix_dir[] = PREFIX;
float *precalc_sin = NULL;
float *precalc_cos = NULL;
float *precalc_sin128 = NULL;
float *precalc_cos128 = NULL;
/** Table used for displacement of the guided missile */
float depix[13][32];
/** Table used for displacement of the guided missile */
float depiy[13][32];
#if defined(_WIN32_WCE)
/** The fully-qualified path for the directory that contains
 * the Powermanga executable on Windows Mobile */
static char *wince_module_pathname = NULL;
#endif

/**
 * Initialize the memory table for our malloc() wrapper
 * @param numofzones Maximum number of memory zones
 * @return Boolean value on success or failure
 */
#if defined (USE_MALLOC_WRAPPER)
bool
memory_init (Uint32 numofzones)
{
  Uint32 i;
  mem_struct *memlst = memory_list_base;
  /* clear number of memory zones reserved */
  mem_numof_zones = 0;
  /* maximum number of memory zones being able to be allocated */
  mem_maxnumof_zones = numofzones;
  mem_maxreached_zones = 0;
  memory_list_size = mem_maxnumof_zones * sizeof (mem_struct);
  mem_total_size = memory_list_size;
  memory_list_base = (mem_struct *) malloc (memory_list_size);
  if (memory_list_base == NULL)
    {
      LOG_ERR ("malloc() failed");
      return FALSE;
    }
  memory_list = memory_list_base;

  /* clear memory table */
  memlst = memory_list_base;
  for (i = 0; i < mem_maxnumof_zones; i++, memlst++)
    {
      memlst->addr = NULL;
      memlst->size = 0;
    }
  return TRUE;
}
#endif

/**
 * Allocate memory, malloc() wrapper
 * @param memsize Size in bytes to alloc
 * @return Pointer to the allocated memory or NULL if an error occurred
 */
char *
memory_allocation (Uint32 memsize)
{
  char *addr = NULL;
#if defined (USE_MALLOC_WRAPPER)
  if (mem_numof_zones >= mem_maxnumof_zones)
    {
      LOG_ERR (" table overflow; size request %i bytes;"
               " total allocate: %i in %i zones",
               memsize, mem_total_size, mem_numof_zones);
      return NULL;
    }
#endif
  addr = (char *) malloc (memsize);
  if (addr == NULL)
    {
#if defined (USE_MALLOC_WRAPPER)
      LOG_ERR ("malloc() return NULL; size request %i bytes;"
               " total allocate: %i in %i zones",
               memsize, mem_total_size, mem_numof_zones);
#else
      LOG_ERR ("malloc() return NULL; size request %i bytes", memsize);
#endif
      return NULL;
    }
  memset (addr, 0, sizeof (char) * memsize);
#if defined (USE_MALLOC_WRAPPER)
  mem_total_size += memsize;
  memory_list->addr = addr;
  memory_list->size = memsize;
  memory_list += 1;
  mem_numof_zones++;
  if (mem_numof_zones > mem_maxreached_zones)
    {
      mem_maxreached_zones = mem_numof_zones;
    }
#endif
  return addr;
}

/**
 * Deallocates the memory, free() wrapper
 * @param addr Pointer to memory
 */
void
free_memory (char *addr)
{
#if defined (USE_MALLOC_WRAPPER)
  mem_struct *memlist;
  mem_struct *memlist_src;
  Uint32 i;
#endif
  if (addr == NULL)
    {
      LOG_ERR ("try to release a null address!");
      return;
    }
#if defined (USE_MALLOC_WRAPPER)
  memlist = memory_list_base;
  for (i = 0; i < mem_numof_zones; i++, memlist++)
    {
      /* search address */
      if (memlist->addr == addr)
        {
          free (addr);
          memlist_src = memlist + 1;
          mem_total_size -= memlist->size;
          mem_numof_zones--;
          memory_list--;
          while (i < mem_numof_zones)
            {
              memlist->addr = memlist_src->addr;
              memlist->size = memlist_src->size;
              i++;
              memlist++;
              memlist_src++;
            }
          memlist->addr = NULL;
          memlist->size = 0;
          addr = NULL;
          break;
        }
    }
  if (addr != NULL)
    {
      LOG_ERR ("can't release the address %p", addr);
    }
#else
  free (addr);
#endif
}

/**
 * Releases all memory allocated
 * @param verbose Verbose level
 */
#if defined (USE_MALLOC_WRAPPER)
void
memory_releases_all (void)
{
  Uint32 i;
  char *addr;
  mem_struct *memlist = memory_list_base;
  LOG_INF ("maximum of memory which were allocated during the game: %i",
           mem_maxreached_zones);
  if (mem_numof_zones > 0)
    {
      LOG_WARN ("%i zones were not released", mem_numof_zones);
      for (i = 0; i < mem_numof_zones; i++, memlist++)
        {
          addr = memlist->addr;
          if (addr != NULL)
            {
              LOG_WARN ("-> free(%p); size=%i", memlist->addr, memlist->size);
              free (addr);
              memlist->addr = NULL;
              memlist->size = 0;
            }
        }
    }
  if (memory_list_base != NULL)
    {
      free (memory_list_base);
      memory_list_base = NULL;
    }
  mem_numof_zones = 0;
}
#endif

/**
 * Load and decompress a PCX file
 * @param filename Filename specified by path
 * @return Pointer to a bitmap_desc structure or null if an error occurred 
 */
bitmap_desc *
load_pcx (const char *filename)
{
  Uint32 width, height, depth, size, ptr;
  Uint16 *ptr16;
  unsigned char numof_bytes;
  unsigned char val;
  Uint32 i, j, total;
  unsigned char *filedata, *pixel;
  bitmap_desc *bmp;
  filedata = (unsigned char *) loadfile (filename, &size);
  if (filedata == NULL)
    {
      return NULL;
    }
  ptr16 = (Uint16 *) filedata;
  width = (little_endian_to_ushort (ptr16 + 4)
           - little_endian_to_ushort (ptr16 + 2)) + 1;
  height = (little_endian_to_ushort (ptr16 + 5)
            - little_endian_to_ushort (ptr16 + 3)) + 1;
  /* bits per pixel */
  depth = filedata[3];

  /* allocate bitmap description structure memory */
  bmp = (bitmap_desc *) memory_allocation (sizeof (bitmap_desc));
  if (bmp == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'bmp'");
      return NULL;
    }
  bmp->width = width;
  bmp->height = height;
  bmp->depth = depth;
  bmp->size = width * height * (depth >> 3);
  /* allocate bitmap memory */
  bmp->pixel = memory_allocation (bmp->size);
  if (bmp->pixel == NULL)
    {
      LOG_ERR ("height=%i / width=%i", filename, width, height);
      LOG_ERR ("not enough memory to allocate %i bytes", bmp->size);
      free_memory ((char *) bmp);
      return NULL;
    }
  /* decompress rle */
  pixel = (unsigned char *) bmp->pixel;
  total = 0;
  i = size - 768;
  ptr = 128;
  while (ptr < i)
    {
      if ((filedata[ptr] & 0xC0) == 0xC0)
        {
          numof_bytes = filedata[ptr] & 0x3F;
          ptr++;
        }
      else
        {
          numof_bytes = 1;
        }
      val = filedata[ptr];
      /* bug fixed by Samuel Hocevar */
      total += numof_bytes;
      if (total >= bmp->size)
        {
          break;
        }
      for (j = 0; j < numof_bytes; j++)
        {
          *pixel = val;
          pixel++;
        }
      ptr++;
    }
  free_memory ((char *) filedata);
  LOG_DBG ("filename: \"%s\"; height:%i; width:%i; size:%i bytes", filename,
           width, height, bmp->size);
  return bmp;
}

/**
 * Read a little-endian 16-bit signed short
 * @param pointer to a 16-bit signed short
 * @return the big or little endian 16-bit signed short value
 */
Sint16
little_endian_to_short (Sint16 * addr)
{
  /* value be extracted byte by byte,
   * do not cast odd addresses to a pointer.
   * Long word read on odd address is illegal on ARM architecture */
  Sint16 value;
  unsigned char *mem = (unsigned char *) addr;
  value = mem[1];
  value <<= 8;
  value = (Sint16) (value | mem[0]);
  return value;
}

/**
 * Read a little-endian 16-bit unsigned short
 * @param pointer to a 16-bit unsigned short
 * @return the big or little endian 16-bit unsigned short value
 */
static Uint16
little_endian_to_ushort (Uint16 * addr)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Sint16 val = 0;
  unsigned char *ptr = (unsigned char *) addr;
  val = ptr[1];
  val <<= 8;
  val += ptr[0];
  return val;
#else
  return *addr;
#endif
}

/**
 * Read a little-endian 32-bit signed long
 * @param pointer to a 32-bit signed long
 * @return the big or little endian 32-bit signed long value
 */
Sint32
little_endian_to_int (Sint32 * addr)
{
  /* value be extracted byte by byte,
   * do not cast odd addresses to a pointer.
   * Long word read on odd address is illegal on ARM architecture */
  Sint32 value;
  unsigned char *mem = (unsigned char *) addr;
  value = mem[3];
  value <<= 8;
  value += mem[2];
  value <<= 8;
  value += mem[1];
  value <<= 8;
  value += mem[0];
  return value;
}

/**
 * Write a little-endian 32-bit signed long
 * @param pointer to a 32-bit signed long
 * @return the big or little endian 32-bit signed long value
 */
void
int_to_little_endian (Sint32 value, Sint32 * addr)
{
  /* value be writed byte by byte,
   * do not cast odd addresses to a pointer.
   * Long word read on odd address is illegal on ARM architecture */
  unsigned char *mem = (unsigned char *) addr;
  mem[0] = (unsigned char) (value & 0xff);
  value >>= 8;
  mem[1] = (unsigned char) (value & 0xff);
  value >>= 8;
  mem[2] = (unsigned char) (value & 0xff);
  value >>= 8;
  mem[3] = (unsigned char) (value & 0xff);
}

/**
 * Convert a 32-bit zone memory
 * @param pointer to a 4 bytes in memory
 */
void
convert32bits_2bigendian (unsigned char *memory)
{
  unsigned char b0, b1, b2, b3;
  b0 = memory[1];
  b1 = memory[0];
  b2 = memory[3];
  b3 = memory[2];
  memory[0] = b2;
  memory[1] = b3;
  memory[2] = b0;
  memory[3] = b1;
}

/**
 * Creates a string representing an integer number
 * @param value the integer value to be converted
 * @param padding length of the string
 * @param str the string representation of the number
 */
void
integer_to_ascii (Sint32 value, Uint32 padding, char *str)
{
  char *ptr = str + padding - 1;
  bool neg = (value < 0);
  if (neg)
    {
      value = -value;
      --padding;
    }
  do
    {
      *ptr-- = (char) (value % 10) + '0';
      value /= 10;
      --padding;
    }
  while (value && padding > 0);
  for (; padding > 0; --padding)
    {
      *ptr-- = '0';
    }
  if (neg)
    {
      *ptr-- = '-';
    }
}

/**
 * Frees the memory used for the full pathname of Powermagane directory
 */
#if defined(_WIN32_WCE)
void
free_wince_module_pathname (void)
{
  if (wince_module_pathname != NULL)
    {
      free_memory (wince_module_pathname);
      wince_module_pathname = NULL;
    }
}

/**
 * Convert a wide-character string to a new character string
 * @param source Pointer to the wide-chars string to be converted  
 * @param length The number of wide-chars in the source string
 * @param code_page The code page used to perform the conversion
 * @return Pointer to the buffer to receive the translated string or
 *         NULL upon failure. This buffer must be released once it is 
 *         not needed anymore
 */
char *
wide_char_to_bytes (wchar_t * source, Uint32 length, Uint32 code_page)
{
  Sint32 size;
  char *dest;
  if (source == NULL)
    {
      return NULL;
    }
  if (length == 0)
    {
      length = wcslen (source) + 1;
    }
  size = WideCharToMultiByte (code_page, 0, source, length,
                              NULL, 0, NULL, NULL);
  if (size == 0)
    {
      LOG_ERR ("WideCharToMultiByte() failed!");
      return NULL;
    }
  dest = memory_allocation (size);
  if (dest == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes", size);
    }
  size = WideCharToMultiByte (code_page, 0, source, length,
                              dest, size, NULL, NULL);
  if (size == 0)
    {
      LOG_ERR ("WideCharToMultiByte() failed!");
      free_memory (dest);
      return NULL;
    }
  dest[size] = 0;
  return dest;
}
#endif

/** Directory list to locate a file */
static const char *data_directories[] = {
  /* normally unused, except when running from the source directory... */
  ".",
  /* special value meaning "$(PREFIX)/share/games/powermanga/" */
  /* also marks end of list */
  0
};

/**
 * Locate a file under one of the data directories
 * @param name Name of file relative to data directory
 * @return Pointer to a malloc'd buffer containing the name under which the
 * file was found (free()-ing the buffer is the responsibility of the caller.)
 * or NULL if could not locate file (not found, or not enough memory, or the
 * name given was absolute)
 * @author Andre Majorel
 */
char *
locate_data_file (const char *const name)
{
#if defined(_WIN32_WCE)
  wchar_t *filename;
  char *pathname;
  char c;
  Uint32 i, j;
  Uint32 len;
  if (wince_module_pathname == NULL)
    {
      len = MAX_PATH * sizeof (wchar_t);
      filename = (wchar_t *) memory_allocation (len);
      if (filename == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes", len);
          return NULL;
        }
      if (GetModuleFileName (NULL, filename, MAX_PATH) == 0)
        {
          LOG_ERR ("GetModuleFileName () failed!");
          free_memory ((char *) filename);
          return NULL;
        }
      /* removes the application name in the pathname */
      i = wcslen (filename) - 1;
      while (filename[i] != L'\\')
        {
          i--;
        }
      filename[i + 1] = 0;
      wince_module_pathname = wide_char_to_bytes (filename, 0, CP_ACP);
      if (wince_module_pathname == NULL)
        {
          LOG_ERR ("wide_char_to_bytes () failed!");
          free_memory ((char *) filename);
          return NULL;
        }
      free_memory ((char *) filename);
    }
  len = strlen (wince_module_pathname) + strlen (name) + 1;
  pathname = memory_allocation (len);
  if (pathname == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes", len);
      return NULL;
    }
  strcpy (pathname, wince_module_pathname);
  j = strlen (pathname);
  for (i = 0; i < strlen (name); i++, j++)
    {
      c = name[i];
      if (c == '/')
        {
          pathname[j] = '\\';
        }
      else
        {
          pathname[j] = c;
        }
    }
  pathname[j] = 0;
  return pathname;

#else
  static const char bogus = '\0';
  static const char *home_dir;
  const char **p;
  char *pathname;
#ifdef WIN32
  struct _stat s;
#else
  struct stat s;
#endif
  const char *subdir = "/share/games/powermanga/";

  if (name == NULL)
    {
      LOG_ERR ("NULL pointer was passed as an argument!");
      return NULL;
    }
  /* if absolute path, return a pointer to a duplicate string */
  if (*name == '/')
    {
      return string_duplicate (name);
    }
  /* process each folder of the list */
  for (p = data_directories;; p++)
    {
      if (*p == 0)
        {
          pathname =
            memory_allocation (strlen (prefix_dir) + strlen (subdir) +
                               strlen (name) + 1);
          if (pathname == NULL)
            {
              fflush (stdout);
              LOG_ERR ("not enough memory");
              return NULL;
            }
          strcpy (pathname, prefix_dir);
          strcat (pathname, subdir);
          strcat (pathname, name);
        }
      /* not user anymore */
      else if (**p == '~')
        {
          home_dir = &bogus;
          if (home_dir == &bogus)
            {
              home_dir = getenv ("HOME");
            }
          if (home_dir == 0)
            {
              /* $HOME not set. Skip this directory */
              continue;
            }
          pathname = memory_allocation (strlen (home_dir)
                                        + 1 + strlen (*p + 1) + 1 +
                                        strlen (name) + 1);
          if (pathname == NULL)
            {
              fflush (stdout);
              LOG_ERR ("not enough memory");
              return NULL;
            }
          strcpy (pathname, home_dir);
          strcat (pathname, *p + 1);
          strcat (pathname, "/");
          strcat (pathname, name);
        }
      else
        {
          /* check if the file is located in current directory */
          pathname = memory_allocation (strlen (*p) + 1 + strlen (name) + 1);
          if (pathname == NULL)
            {
              fflush (stdout);
              LOG_ERR ("not enough memory");
              return NULL;
            }
          strcpy (pathname, *p);
          strcat (pathname, "/");
          strcat (pathname, name);
        }
#ifdef WIN32
      if (_stat (pathname, &s) == 0 && !(s.st_mode & _S_IFDIR))
        {
          return pathname;
        }

#else
      if (stat (pathname, &s) == 0 && !S_ISDIR (s.st_mode))
        {
          return pathname;
        }
#endif
      free_memory (pathname);
      if (*p == 0)
        {
          break;
        }
    }
  /* not found */
  return NULL;
#endif
}


/**
 * Allocate memory and load a file
 * @param filename Filename specified by path
 * @return File data buffer pointer
 */
char *
load_file (const char *const filename)
{
  Uint32 filesize;
  return loadfile (filename, &filesize);
}

/**
 * Allocate memory and load a file (filename with a language code)
 * @param filename Filename specified by path
 * @param fsize Pointer on the size of file which will be loaded
 * @return Pointer to the file data
 */
char *
loadfile_with_lang (const char *const filename, Uint32 * const fsize)
{
  const char *lang;
  char *data, *fname;
  if (filename == NULL || strlen (filename) == 0)
    {
      LOG_ERR ("filename is a NULL string");
      return NULL;
    }

  fname = memory_allocation (strlen (filename) + 1);
  if (fname == NULL)
    {
      LOG_ERR ("filename: \"%s\" not enough memory"
               " to allocate %i bytes", filename,
               (Uint32) (strlen (filename) + 1));
      return NULL;
    }
  strcpy (fname, filename);
  lang = configfile_get_lang ();
  sprintf (fname, filename, lang);
  LOG_DBG ("file \"%s\" was loaded in memory", fname);
  data = loadfile (fname, fsize);
  free_memory (fname);
  return data;
}

/**
 * Allocate memory and load a file (filename with a number)
 * @param filename Filename specified by path
 * @param num Interger to convert in string
 * @return Pointer to the file data
 */
char *
loadfile_num (const char *const filename, Sint32 num)
{
  char *data, *fname;

  if (filename == NULL || strlen (filename) == 0)
    {
      LOG_ERR ("filename is a NULL string");
      return NULL;
    }

  fname = memory_allocation (strlen (filename) + 1);
  if (fname == NULL)
    {
      LOG_ERR ("filename: \"%s\"; num: %i; "
               "not enough memory to allocate %i bytes",
               filename, num, (Uint32) (strlen (filename) + 1));
      return NULL;
    }
  sprintf (fname, filename, num);
  LOG_DBG ("file \"%s\" was loaded in memory", fname);
  data = load_file (fname);
  free_memory (fname);
  return data;
}

/**
 * Allocate memory and load a file there
 * @param filename the file which should be loaded
 * @param fsize pointer on the size of file which will be loaded
 * @return file data buffer pointer
 */
char *
loadfile (const char *const filename, Uint32 * const fsize)
{
  char *buffer;
  char *pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      LOG_ERR ("can't locate file %s", filename);
      return NULL;
    }
  buffer = load_absolute_file (pathname, fsize);
  if (buffer == NULL)
    {
      free_memory (pathname);
      return NULL;
    }
  free_memory (pathname);
  return buffer;
}


/**
 * Getting size of a file
 * @param fstream Pointer to a FILE object that identifies the stream
 * @retrun Size of file in bytes or zero if an error occurred
 */
size_t
get_file_size (FILE * fstream)
{
  size_t fsize;
#if defined(_WIN32_WCE)
  if (fseek (fstream, 0, SEEK_END) != 0)
    {
      LOG_ERR ("fseek() failed");
      return 0;
    }
  fsize = ftell (fstream);
  if (fsize == -1)
    {
      LOG_ERR ("fread() failed");
      return 0;
    }
  if (fseek (fstream, 0, SEEK_SET) != 0)
    {
      LOG_ERR ("fseek() failed");
    }
#else
  struct stat sb;
  if (fstat (fileno (fstream), &sb))
    {
      LOG_ERR ("fstat() failed: %s", strerror (errno));
      return 0;
    }
  fsize = sb.st_size;
#endif
  return fsize;
}

/**
 * Allocate memory and load a file there
 * @param filename the file which should be loaded
 * @param fsize pointer on the size of file which will be loaded
 * @return file data buffer pointer
 */
char *
load_absolute_file (const char *const filename, Uint32 * const filesize)
{
  size_t fsize;
  FILE *fstream;
  char *buffer;
#ifdef WIN32
  fstream = fopen (filename, "rb");
#else
  fstream = fopen (filename, "r");
#endif
  if (fstream == NULL)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("can't open file  %s", filename);
#else
      LOG_ERR ("can't open file  %s (%s)", filename, strerror (errno));
#endif
      return NULL;
    }
  fsize = get_file_size (fstream);
  (*filesize) = fsize;
  if (fsize == 0)
    {
      fclose (fstream);
      LOG_ERR ("file %s is empty!", filename);
      return NULL;
    }
  buffer = memory_allocation (fsize);
  if (buffer == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes!",
               filename, (Sint32) fsize);
      fclose (fstream);
      return NULL;
    }
  if (fread (buffer, sizeof (char), fsize, fstream) != fsize)
    {
      free_memory (buffer);
#if defined(_WIN32_WCE)
      LOG_ERR ("can't read file \"%s\"", filename);
#else
      LOG_ERR ("can't read file \"%s\" (%s)", filename, strerror (errno));
#endif
      fclose (fstream);
      return NULL;
    }
  fclose (fstream);
  LOG_DBG ("file \"%s\" was loaded in memory", filename);
  return buffer;
}

/**
 * Load a file in memory buffer already allocated
 * @param filename the file which should be loaded
 * @param buffer pointer to the buffer where data are stored
 * @return boolean value on success or failure
 */
bool
loadfile_into_buffer (const char *const filename, char *const buffer)
{
  size_t fsize;
  FILE *fstream;
  char *pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      LOG_ERR ("can't locate file: '%s'", filename);
      return FALSE;
    }
#ifdef WIN32
  fstream = fopen (pathname, "rb");
#else
  fstream = fopen (pathname, "r");
#endif
  if (fstream == NULL)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("can't open \"%s\" file", pathname);
#else
      LOG_ERR ("can't open \"%s\" file (%s)", pathname, strerror (errno));
#endif
      free_memory (pathname);
      return FALSE;
    }
  fsize = get_file_size (fstream);
  if (fsize == 0)
    {
      fclose (fstream);
      LOG_ERR ("file \"%s\" is empty!", filename);
      return FALSE;
    }
  if (fread (buffer, sizeof (char), fsize, fstream) != fsize)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("can't read \"%s\" file", pathname);
#else
      LOG_ERR ("can't read \"%s\" file (%s)", pathname, strerror (errno));
#endif
      fclose (fstream);
      free_memory (pathname);
      return FALSE;
    }
  fclose (fstream);
  LOG_DBG ("\"%s\" file was loaded in memory", pathname);
  free_memory (pathname);
  return TRUE;
}


bool
file_write (const char *filename, const char *filedata, const size_t filesize)
{
  FILE *fstream;

  /* set umask so that files are group-writable */
#if !defined(_WIN32_WCE)
#ifdef WIN32
  _umask (0002);
#else
  umask (0002);
#endif
#endif

  fstream = fopen (filename, "wb");
  if (fstream == NULL)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("fopen (%s) failed", filename);
#else
      LOG_ERR ("fopen (%s) return: %s", filename, strerror (errno));
#endif
      return FALSE;
    }

  if (fwrite (filedata, sizeof (char), filesize, fstream) != filesize)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("fwrite (%s) failed", filename);
#else
      LOG_ERR ("fwrite (%s) return: %s", filename, strerror (errno));
#endif
      fclose (fstream);
      return FALSE;
    }
  if (fclose (fstream) != 0)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("close (%s) failed", filename);
#else
      LOG_ERR ("close (%s) return: %s", filename, strerror (errno));
#endif
      return FALSE;
    }
  return TRUE;
}

/**
 * Initialize ticks counters
 */
void
fps_init (void)
{
#ifdef POWERMANGA_SDL
  time_begin = SDL_GetTicks ();
  ticks_previous = SDL_GetTicks ();
#else
  gettimeofday (&time_begin, NULL);
  gettimeofday (&ticks_previous, NULL);
#endif
  loops_counter = 0;
}

/**
 * Draw informations on framerate and Linux system
 */
void
fps_print (void)
{
#ifdef POWERMANGA_SDL
  double fps;
  unsigned long duration;
  Sint32 time_end;
  time_end = SDL_GetTicks ();
  duration = time_end - time_begin;
  fps = (1000.0 * loops_counter) / duration;
  LOG_INF ("number of loops: %i; running time: %li; frames per seconde: %g",
           loops_counter, duration, fps);
#else
  struct utsname kernel;
  struct stat statmem;
  unsigned long duration;
  double fps;
  double mem;
  char os_name[32];
  char os_vers[32];
  char cpu[64];
  char freq[32];
  FILE *cpuinfo;
  char txt[256];
  static struct timeval time_end;

  /* total time of execution */
  gettimeofday (&time_end, NULL);
  duration =
    (time_end.tv_sec - time_begin.tv_sec) * 1000 + (time_end.tv_usec -
                                                    time_begin.tv_usec) /
    1000;
  fps = (1000.0 * loops_counter) / duration;

  /* Linux kernel version */
  if (uname (&kernel) < 0)
    {
      strcpy (os_name, "?");
      strcpy (os_vers, "");
    }
  else
    {
      strncpy (os_name, kernel.sysname, 32);
      strncpy (os_vers, kernel.release, 32);
    }
  /* CPU and physic memory */
  stat ("/proc/kcore", &statmem);
  mem = ((float) statmem.st_size) / 1024 / 1024;
  strcpy (cpu, "Unknown");
  strcpy (freq, "Unknown");
  cpuinfo = fopen ("/proc/cpuinfo", "r");
  if (cpuinfo != NULL)
    {
      while (fgets (txt, 255, cpuinfo))
        {
          if (!strncmp (txt, "model", 5))
            {
              int i = 0;
              while (txt[i] != ':')
                i++;
              i += 2;
              for (int j = 0; j < 64;)
                {
                  if (txt[i++] != '\n')
                    cpu[j++] = txt[i - 1];
                }
            }
          if (!strncmp (txt, "cpu MHz", 7))
            {
              int i = 0;
              while (txt[i] != ':')
                i++;
              i += 2;
              sprintf (freq, "%d", atoi (txt + i));
            }
        }
    }
  LOG_INF ("operating system : %s %s", os_name, os_vers);
  LOG_INF ("processor        : %s at %s Mhz with %.0f RAM", cpu, freq, mem);
  LOG_INF ("number of loops  : %i", loops_counter);
  LOG_INF ("running time     : %li", duration);
  LOG_INF ("frames per second: %g", fps);
#endif
}

/**
 * Sleep for a time interval
 * @input delay
 * @return
 */
Sint32
wait_next_frame (Sint32 delay, Sint32 max)
{
#ifdef POWERMANGA_SDL
  if (delay > max)
    {
      delay = max;
    }
  if (delay > 0)
    {
      SDL_Delay (delay);
    }
  return (delay > 0 ? delay : 0);
#else
  struct timeval temps;
  if (delay > max)
    {
      delay = max;
    }
  if (delay > 0)
    {
      temps.tv_usec = delay % (unsigned long) 1000000;
      temps.tv_sec = delay / (unsigned long) 1000000;
      select (0, NULL, NULL, NULL, &temps);
    }
  return (delay > 0 ? delay : 0);
#endif
}

#ifdef POWERMANGA_SDL
/**
 * Calculate diffence between 2 times
 * @return difference
 */
Sint32
get_time_difference ()
{
  Sint32 diff;
  Uint32 current_ticks;
  current_ticks = SDL_GetTicks ();
  diff = current_ticks - ticks_previous;
  ticks_previous = current_ticks;
  return diff;
}
#else
/**
 * Calculate diffence between 2 times
 * @return difference
 */
Sint32
get_time_difference ()
{
  struct timeval current_ticks;
  Sint32 diff;
  gettimeofday (&current_ticks, NULL);
  diff =
    (1000000 * (current_ticks.tv_sec - ticks_previous.tv_sec)) +
    (current_ticks.tv_usec - ticks_previous.tv_usec);
  ticks_previous = current_ticks;
  return diff;
}
#endif

/**
 * Check if a value is null, signed or unsigned
 * @return 0 if value is null, -1 if signed, or 1 otherwise
 */
Sint16
sign (float val)
{
  if (val == 0)
    {
      return 0;
    }
  if (val < 0)
    {
      return -1;
    }
  return 1;
}

/**
 * Calculate a shot angle from two points
 * @param pxs X coordinate of the source
 * @param pys Y coordinate of the source
 * @param pxd X coordinate of the destination 
 * @param pyd Y coordinate of the destination
 * @return angle
 */
float
calc_target_angle (Sint16 pxs, Sint16 pys, Sint16 pxd, Sint16 pyd)
{
  double length, result, dx, dy, angle;
  /* calculate distance between source and destination */
  dx = pxd - pxs;
  dy = pyd - pys;
  result = (dx * dx) + (dy * dy);
  length = sqrt (result);
  if (length == 0)
    {
      /* avoid division by zero */
      return 0.0;
    }
  result = dx / length;
  angle = acos (result);
  result = dy / length;
  if (asin (result) < 0)
    {
      angle = -angle;
    }
  return (float) angle;
}

/** 
 * Calculate a new angle from another angle and some deftness
 * @param old_angle
 * @param new_angle
 * @param deftness
 * @return New angle
 */
float
get_new_angle (float old_angle, float new_angle, float deftness)
{
  float delta_angle;
  delta_angle = old_angle - new_angle;
  if (((delta_angle < 0) && (delta_angle > -3.14)) || (delta_angle > 3.14))
    {
      old_angle = old_angle + deftness;
      if (old_angle > 3.14)
        {
          old_angle = old_angle - 6.28f;
        }
    }
  if (((delta_angle > 0) && (delta_angle < 3.14)) || (delta_angle < -3.14))
    {
      old_angle = old_angle - deftness;
      if (old_angle < -3.14)
        {
          old_angle = old_angle + 6.28f;
        }
    }
  return old_angle;
}

/**
 * Allocates memory and coopies into it the string addressed
 * @param Null-terminated string to duplicate.
 * @return Pointer to a newly allocated copy of the string or NULL 
 */
char *
string_duplicate (register const char *str)
{
  register char *new_str;
  register Uint32 size;
  size = strlen (str) + 1;
  new_str = memory_allocation (size);
  if (new_str == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes", size);
      return NULL;
    }
  memcpy (new_str, str, size);
  return new_str;
}

/**
 * Allocate and precalculate sinus and cosinus curves 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
alloc_precalulate_sinus (void)
{
  Uint32 i;
  double a, step, pi;

  if (precalc_sin128 == NULL)
    {
      i = 128 * sizeof (float) * 2;
      precalc_sin128 = (float *) memory_allocation (i);
      if (precalc_sin128 == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes", i);
          return FALSE;
        }
      precalc_cos128 = precalc_sin128 + 128;
    }
  pi = 4 * atan (1.0);
  a = 0.0;
  step = (pi * 2) / 128;
  for (i = 0; i < 128; i++)
    {
      precalc_sin128[i] = (float) sin (a);
      precalc_cos128[i] = (float) cos (a);
      a += step;
    }
  if (precalc_sin == NULL)
    {
      i = 32 * sizeof (float) * 2;
      precalc_sin = (float *) memory_allocation (i);
      if (precalc_sin == NULL)
        {
          LOG_ERR ("not enough memory to allocate %i bytes", i);
          return FALSE;
        }
      precalc_cos = precalc_sin + 32;
    }

  step = (pi * 2) / 32;
  a = 0.0;
  for (i = 0; i < 32; i++)
    {
      precalc_sin[i] = (float) sin (a);
      precalc_cos[i] = (float) cos (a);
      a += step;
    }
  /* table user for guided missile */
  for (i = 0; i < 32; i++)
    {
      depix[0][i] = precalc_cos[i] * 0.5f;
      depix[1][i] = precalc_cos[i] * 1.0f;
      depix[2][i] = precalc_cos[i] * 1.5f;
      depix[3][i] = precalc_cos[i] * 2.0f;
      depix[4][i] = precalc_cos[i] * 2.5f;
      depix[5][i] = precalc_cos[i] * 3.0f;
      depix[6][i] = precalc_cos[i] * 3.5f;
      depix[7][i] = precalc_cos[i] * 4.0f;
      depix[8][i] = precalc_cos[i] * 4.5f;
      depix[9][i] = precalc_cos[i] * 5.0f;
      depix[10][i] = precalc_cos[i] * 5.5f;
      depix[11][i] = precalc_cos[i] * 6.0f;
      depix[12][i] = precalc_cos[i] * 6.5f;
      depiy[0][i] = precalc_sin[i] * 0.5f;
      depiy[1][i] = precalc_sin[i] * 1.0f;
      depiy[2][i] = precalc_sin[i] * 1.5f;
      depiy[3][i] = precalc_sin[i] * 2.0f;
      depiy[4][i] = precalc_sin[i] * 2.5f;
      depiy[5][i] = precalc_sin[i] * 3.0f;
      depiy[6][i] = precalc_sin[i] * 3.5f;
      depiy[7][i] = precalc_sin[i] * 4.0f;
      depiy[8][i] = precalc_sin[i] * 4.5f;
      depiy[9][i] = precalc_sin[i] * 5.0f;
      depiy[10][i] = precalc_sin[i] * 5.5f;
      depiy[11][i] = precalc_sin[i] * 6.0f;
      depiy[12][i] = precalc_sin[i] * 6.5f;
    }
  return TRUE;
}

/**
 * Release precalculded sinus and cosinus curves
 */
void
free_precalulate_sinus (void)
{
  if (precalc_sin != NULL)
    {
      free_memory ((char *) precalc_sin);
      precalc_sin = NULL;
      precalc_cos = NULL;
    }
  if (precalc_sin128 != NULL)
    {
      free_memory ((char *) precalc_sin128);
      precalc_sin128 = NULL;
      precalc_cos128 = NULL;
    }
}

/**
 * Open a file and return an I/O stream
 * @param fname fname The filename specified by path
 * @param fmode Mode parameter
 * @return A pointer to a FILE structure
 */
FILE *
fopen_data (const char *fname, const char *fmode)
{
  FILE *fi;
  fi = fopen (fname, fmode);
  if (fi == NULL)
    {
#if defined(_WIN32_WCE)
      LOG_ERR ("fopen (%s, %s) failed!", fname, fmode);
#else
      LOG_ERR ("fopen (%s, %s) return: %s", fname, fmode, strerror (errno));
#endif
      return NULL;
    }
  return fi;
}

/**
 * create a new directory 
 * @param dirname A direcotry name
 * @return Boolean value on success or failure
 */
bool
create_dir (const char *dirname)
{
#if defined(_WIN32_WCE)
  LOG_DBG ("create_dir(%s) not implemented!", dirname);
  return FALSE;

  /* FIXME convert char* ti wchar_t* */
/*
  bool result = TRUE;
  if (CreateDirectory (config_dir, NULL) == FALSE)
    {
      if (GetLastError () != ERROR_ALREADY_EXISTS)
        {
          LOG_ERR ("CreateDirectory(%s) failed!", config_dir);
          result = FALSE;
        }
    }
  return result;
*/
#else
#if defined(_WIN32)
  Sint32 result;
  result = _mkdir (dirname);
  if (result == 0 || result == EEXIST)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
#else
  Sint32 result;
  result = mkdir (dirname, S_IRWXU);
  if (result == 0 || result == EEXIST)
    {
      return TRUE;
    }
  else if (errno == EEXIST || errno == EISDIR)
    {
      return TRUE;
    }
  else
    {
      LOG_ERR ("mkdir(%s): %s", dirname, strerror (errno));
      return FALSE;
    }
#endif
#endif
}

#if defined(_WIN32_WCE)
#define PARAMAX_LEN 256
#ifndef SPI_GETPLATFORMMANUFACTURER
#define SPI_GETPLATFORMMANUFACTURER 262
#endif
#ifndef SPI_GETPLATFORMNAME
#define SPI_GETPLATFORMNAME 260
#endif
void
display_windows_ce_infos (void)
{
  Uint32 i;
  char *param;
  WCHAR wparam[PARAMAX_LEN + 1];
  OSVERSIONINFO vinfo;
  static const char *action_names[] = {
    "SPI_GETPLATFORMMANUFACTURER",
    "SPI_GETPLATFORMNAME",
    "SPI_GETPLATFORMTYPE"
  };
  static const Uint32 actions[] = {
    SPI_GETPLATFORMMANUFACTURER,
    SPI_GETPLATFORMNAME,
    SPI_GETPLATFORMTYPE
  };
  for (i = 0; i < sizeof (actions) / sizeof (Uint32); i++)
    {
      if (SystemParametersInfo (actions[i], PARAMAX_LEN, wparam, 0) == FALSE)
        {
          LOG_ERR ("SystemParametersInfo(%s) "
                   "failed (error %i)", action_names[i], GetLastError ());
          continue;
        }
      param = wide_char_to_bytes (wparam, 0, CP_ACP);
      if (param == NULL)
        {
          continue;
        }
      LOG_INF ("%s %s", action_names[i], param);
      free_memory (param);
    }

  if (GetVersionEx (&vinfo) == FALSE)
    {
      LOG_ERR ("GetVersionEx() " "failed (error %i)", GetLastError ());
    }
  else
    {
      if (vinfo.dwPlatformId != VER_PLATFORM_WIN32_CE)
        {
          LOG_ERR ("this system is not a Windows Embedded CE OS");
        }
      else
        {
          LOG_INF ("Windows Mobile %i.%i", vinfo.dwMajorVersion,
                   vinfo.dwMinorVersion);
        }
    }

  LOG_INF ("SM_CXSCREEN: %i; SM_CYSCREEN: %i",
           GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN));
}
#endif
