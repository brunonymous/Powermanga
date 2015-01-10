/**
 * @file script_page.c
 * @brief Handle the displaying of the pages order in Windows shareware
 *        version 
 * @created 2007-04-18 
 * @date 2012-08-26 
 * @author Laurent Cance 
 * @author Patrice Duhamel 
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: script_page.c,v 1.15 2012/08/26 19:22:39 gurumeditation Exp $
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
#include "log_recorder.h"
#ifdef SHAREWARE_VERSION
#ifdef WIN32
#include <objbase.h>
#include <basetsd.h>
#define   sprintf   _snwprintf
#else
#include <wchar.h>
#endif
#include <stdarg.h>
#include "script_page.h"
static Sint32 window_width = 1024;
static Sint32 window_height = 768;
static char *script_dirname = ".";
static bool is_16bits_dithering = FALSE;
static char sname[512];

/**
 * Locate a file under one of the data directories
 * param filename The single filename
 * @return Pointer to a malloc'd buffer containing the full pathname
 */
static char *
script_locate_file (const char const *filename)
{
  char *name, *pathname;
  name = memory_allocation (strlen (script_dirname) + strlen (filename) + 1);
  if (name == NULL)
    {
      TRACE_ERR ("not enough memory!");
      return NULL;
    }
  sprintf (name, "%s%s", script_dirname, filename);
  pathname = locate_data_file (name);
  if (pathname == NULL)
    {
      TRACE_ERR ("'%s' file not found!", name);
      return NULL;
    }
  free_memory (name);
  return pathname;
}

/**
 * Initialize some values
 * @param width Width of the main window
 * @param height Hieght of the main windows
 * @param dirname Name of directory which contains the files
 */
void
script_initialize (Sint32 width, Sint32 height, char *dirname)
{
  window_width = width;
  window_height = height;
  script_dirname = dirname;
}

/**
 * Enable the dithering
 */
void
script_set_dithering (void)
{
  is_16bits_dithering = TRUE;
}

/**
 * Load file for use as a font
 * @param name
 * @param fonte
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
script_open_font (char *name, struct Fonte *fonte)
{
  char *pathname;
  pathname = script_locate_file (name);
  if (pathname == NULL)
    {
      return NULL;
    }
  fonte->font = TTF_OpenFont (pathname, fonte->size);
  if (fonte->font == NULL)
    {
      LOG_ERR ("TTF_OpenFont(%s) return %s", pathname, TTF_GetError ());
      free_memory (pathname);
      return FALSE;
    }
  LOG_INF ("TTF_OpenFont(%s) successful!", pathname);
  free_memory (pathname);
  /* set a rendering style of the loaded font if required */
  if (fonte->style == TTF_STYLE_ITALIC)
    {
      TTF_SetFontStyle (fonte->font, TTF_STYLE_ITALIC);
    }
  if (fonte->style == TTF_STYLE_BOLD)
    {
      TTF_SetFontStyle (fonte->font, TTF_STYLE_BOLD);
    }
  return TRUE;
}

/**
 * @param str
 * @return
 */
static bool
isBEGIN (char *str)
{
  bool result = FALSE;
  Sint32 n, nn;
  nn = strlen (str);
  n = 0;
  while ((n < nn) && (!result))
    {
      if (str[n] == (char) '{')
        {
          result = TRUE;
        }
      n++;
    }
  return result;
}

/**
 * @param str
 * @return
 */
static bool
isEND (char *str)
{
  Sint32 n, nn, res;
  nn = strlen (str);
  res = 0;
  n = 0;
  while ((n < nn) && (res == 0))
    {
      if (str[n] == (char) '}')
        res = 1;
      n++;
    }

  return (res == 1);
}

/**
 * @param str
 * @param m
 * @return
 */
static bool
match (char *str, char *m)
{
  Sint32 res;
  res = 0;
  Uint32 n = 0;
  while ((n < strlen (m)) && (res == 0))
    {
      if (str[n] != m[n])
        res = 1;
      n++;
    }

  return (res == 0);
}


/**
 *
 * @param str
 * @param c 
 * @return
 */
static Sint32
car (char *str, char c)
{
  Sint32 n;
  n = 0;
  while (str[n] != c)
    {
      n++;
    }
  return n;
}

/**
 * @param str
 * @param m
 * @return
 */
static bool
script_match_strings (char *str, char *m)
{
  Uint32 n;
  Sint32 res;
  n = 0;
  res = 0;
  while ((n < strlen (str)) && (res == 0))
    {
      if (match (&str[n], m))
        {
          res = 1;
        }
      n++;
    }
  return (res == 1);
}

/**
 * Read a string delimited by quotes
 * @input str The string to check
 * @return The string delimited by quotes
 */
static char *
script_get_string (char *str)
{
  Sint32 n, nn;
  n = 0;
  while (str[n] != (char) '"')
    {
      n++;
    }
  n++;
  nn = 0;
  while (str[n] != (char) '"')
    {
      sname[nn] = str[n];
      n++;
      nn++;
    }
  sname[nn] = '\0';
  return sname;
}

/**
 * Parse the "FONTES" section
 * @param f File structure pointer that identifies the stream to read 
 * @param fontename
 * @param page
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
script_add_font_section (FILE * f, char *fontename, struct Page *page)
{
  Sint32 nfonte;
  char *fname;
  char str[256];
  Sint32 res;
  nfonte = page->nFONTES;
  snprintf (page->FONTES[nfonte].name, 256, "%s", fontename);
  page->FONTES[nfonte].OMBRAGE = 0;
  page->FONTES[nfonte].DEGRADE = 0;
  page->FONTES[nfonte].OUTLINE = 0;
  page->FONTES[nfonte].font = NULL;
  page->FONTES[nfonte].style = TTF_STYLE_NORMAL;
  fgets (str, 256, f);
  while (!isBEGIN (str))
    {
      fgets (str, 256, f);
    }
  res = 0;
  while (!isEND (str))
    {
      fgets (str, 256, f);
      if (script_match_strings (str, "FICHIER"))
        {
          fname = script_get_string (str);
          res = 1;
        }
      if (script_match_strings (str, "OMBRAGE"))
        {
          page->FONTES[nfonte].OMBRAGE = 1;
          sscanf (&str[car (str, '=') + 1], "(%d,%d)",
                  &page->FONTES[nfonte].Ombrex, &page->FONTES[nfonte].Ombrey);

        }
      if (script_match_strings (str, "SIZE"))
        {
          sscanf (&str[car (str, (char) '=') + 1], "%d",
                  &page->FONTES[nfonte].size);
        }
      if (script_match_strings (str, "OUTLINE"))
        {
          sscanf (&str[car (str, (char) '=') + 1], "%d",
                  &page->FONTES[nfonte].OUTLINE);
        }
      if (script_match_strings (str, "DEGRADE"))
        {
          page->FONTES[nfonte].DEGRADE = 1;
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d,%d)/(%d,%d,%d)",
                  &page->FONTES[nfonte].DEGRADE_HAUT.r,
                  &page->FONTES[nfonte].DEGRADE_HAUT.g,
                  &page->FONTES[nfonte].DEGRADE_HAUT.b,
                  &page->FONTES[nfonte].DEGRADE_BAS.r,
                  &page->FONTES[nfonte].DEGRADE_BAS.g,
                  &page->FONTES[nfonte].DEGRADE_BAS.b);
        }
      if (script_match_strings (str, "SINUS"))
        {
          page->FONTES[nfonte].DEGRADE = 2;
        }
      if (script_match_strings (str, "ITALIQUE"))
        {
          page->FONTES[nfonte].style = TTF_STYLE_ITALIC;
        }
      if (script_match_strings (str, "GRAS"))
        {
          page->FONTES[nfonte].style = TTF_STYLE_BOLD;
        }
    }
  if ((page->FONTES[nfonte].size != 0) && (res == 1))
    {
      sprintf (page->FONTES[nfonte].strfile, "%s", fname);
      if (!script_open_font (fname, &page->FONTES[nfonte]))
        {
          LOG_ERR ("script_open_font(%s) failed!", fname);
          return FALSE;
        }
      page->nFONTES++;
    }
  return TRUE;
}

/**
 * @param ptr
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param sizex
 * @param sizey
 */
static void
script_recurs (char *ptr, Sint32 x1, Sint32 y1, Sint32 x2, Sint32 y2,
               Sint32 sizex, Sint32 sizey)
{
  Sint32 x, y, dx, dy, d;
  Sint32 c, c1, c2, c3, c4;
  dx = 1 + x2 - x1;
  dy = 1 + y2 - y1;
  if (dx > dy)
    {
      d = dx;
    }
  else
    {
      d = dy;
    }
  if ((dx > 1) && (dy > 1))
    {
      x = (x1 + x2) / 2;
      y = y1;
      c1 = (ptr[x1 + y1 * sizex] & 255);
      c2 = (ptr[x2 + y1 * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dx - dx / 2;
      if (c < 0)
        {
          c = 0;
        }
      if (c > 255)
        {
          c = 255;
        }
      if (ptr[x + y * sizex] == 0)
        {
          ptr[x + y * sizex] = c;
        }
      x = (x1 + x2) / 2;
      y = y2;
      c1 = (ptr[x1 + y2 * sizex] & 255);
      c2 = (ptr[x2 + y2 * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dx - dx / 2;
      if (c < 0)
        c = 0;
      if (c > 255)
        {
          c = 255;
        }
      if (ptr[x + y * sizex] == 0)
        {
          ptr[x + y * sizex] = c;
        }
      x = x1;
      y = (y1 + y2) / 2;
      c1 = (ptr[x1 + y1 * sizex] & 255);
      c2 = (ptr[x1 + y2 * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dy - dy / 2;
      if (c < 0)
        c = 0;
      if (c > 255)
        c = 255;
      if (ptr[x + y * sizex] == 0)
        ptr[x + y * sizex] = c;
      x = x2;
      y = (y1 + y2) / 2;
      c1 = (ptr[x2 + y1 * sizex] & 255);
      c2 = (ptr[x2 + y2 * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dy - dy / 2;
      if (c < 0)
        {
          c = 0;
        }
      if (c > 255)
        {
          c = 255;
        }
      if (ptr[x + y * sizex] == 0)
        {
          ptr[x + y * sizex] = c;
        }
      x = (x1 + x2) / 2;
      y = (y1 + y2) / 2;
      c1 = (ptr[x1 + y1 * sizex] & 255);
      c2 = (ptr[x2 + y1 * sizex] & 255);
      c3 = (ptr[x1 + y2 * sizex] & 255);
      c4 = (ptr[x2 + y2 * sizex] & 255);
      c = (c1 + c2 + c3 + c4) / 4 + rand () % d - d / 2;
      if (c < 0)
        {
          c = 0;
        }
      if (c > 255)
        {
          c = 255;
        }
      if (ptr[x + y * sizex] == 0)
        {
          ptr[x + y * sizex] = c;
        }
      if ((dx > 2) && (dy > 2))
        {
          script_recurs (ptr, x1, y1, (x1 + x2) / 2, (y1 + y2) / 2, sizex,
                         sizey);
          script_recurs (ptr, (x1 + x2) / 2, y1, x2, (y1 + y2) / 2, sizex,
                         sizey);
          script_recurs (ptr, x1, (y1 + y2) / 2, (x1 + x2) / 2, y2, sizex,
                         sizey);
          script_recurs (ptr, (x1 + x2) / 2, (y1 + y2) / 2, x2, y2, sizex,
                         sizey);
        }
    }
}


/**
 *
 * @param prt
 * @param x1
 * @param x2
 * @param y
 * @param sizex
 * @param sizey
 */
static void
script_recursx (char *ptr, Sint32 x1, Sint32 x2, Sint32 y, Sint32 sizex,
                Sint32 sizey)
{
  Sint32 x, dx;
  Sint32 c, c1, c2;
  dx = 1 + x2 - x1;
  if (dx > 1)
    {
      x = (x1 + x2) / 2;
      c1 = (ptr[x1 + y * sizex] & 255);
      c2 = (ptr[x2 + y * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dx - dx / 2;
      if (c < 0)
        {
          c = 0;
        }
      if (c > 255)
        {
          c = 255;
        }
      if (ptr[x + y * sizex] == 0)
        {
          ptr[x + y * sizex] = c;
        }
      if (dx > 2)
        {
          script_recursx (ptr, x1, (x1 + x2) / 2, y, sizex, sizey);
          script_recursx (ptr, (x1 + x2) / 2, x2, y, sizex, sizey);
        }
    }
}

/**
 *
 * @param ptr
 * @param y1
 * @param y2
 * @param x
 * @param sizex
 * @param sizey
 */
static void
script_recursy (char *ptr, Sint32 y1, Sint32 y2, Sint32 x, Sint32 sizex,
                Sint32 sizey)
{
  Sint32 y, dy;
  Sint32 c, c1, c2;

  dy = 1 + y2 - y1;

  if (dy > 1)
    {
      y = (y1 + y2) / 2;
      c1 = (ptr[x + y1 * sizex] & 255);
      c2 = (ptr[x + y2 * sizex] & 255);
      c = (c1 + c2) / 2 + rand () % dy - dy / 2;
      if (c < 0)
        c = 0;
      if (c > 255)
        c = 255;
      if (ptr[x + y * sizex] == 0)
        ptr[x + y * sizex] = c;

      if (dy > 2)
        {
          script_recursy (ptr, y1, (y1 + y2) / 2, x, sizex, sizey);
          script_recursy (ptr, (y1 + y2) / 2, y2, x, sizex, sizey);
        }
    }
}

/**
 * Parse a 'BITMAPS' section
 * @param f File structure pointer that identifies the stream to read 
 * @param name
 * @param page
 * @return TRUE if it completed successfully or FALSE otherwise
 */
static bool
script_add_bitmap_section (FILE * f, char *name, struct Page *page)
{
  SDL_PixelFormat SDL_FormatRGB32A;
  SDL_Surface *surface_tmp;
  Sint32 nBM;
  char *fname, *pathname;
  char str[256];
  Sint32 res;
  Sint32 n1, n2;
  Sint32 r, g, b;
  char *ptr, *ptr2;
  Sint32 lx, ly;
  Sint32 c;
  nBM = page->nBITMAPS;
  snprintf (page->BITMAPS[nBM].name, 256, "%s", name);
  memset (page->BITMAPS[nBM].strfile, 0, 256);
  page->BITMAPS[nBM].OMBRAGE = 0;
  page->BITMAPS[nBM].OUTLINE = 0;
  page->BITMAPS[nBM].ptr = NULL;
  page->BITMAPS[nBM].Surface = NULL;
  page->BITMAPS[nBM].CLOUDS = 0;
  fgets (str, 256, f);
  while (!isBEGIN (str))
    {
      fgets (str, 256, f);
    }
  res = 0;
  fgets (str, 256, f);
  while (!isEND (str))
    {
      if (script_match_strings (str, "FICHIER"))
        {
          fname = script_get_string (str);
          res = 1;
        }

      if (script_match_strings (str, "CLOUDS"))
        {
          page->BITMAPS[nBM].CLOUDS = 1;
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d,%d,%d,%d)", &r,
                  &g, &b, &lx, &ly);

          page->BITMAPS[nBM].cloud[0] = r;
          page->BITMAPS[nBM].cloud[1] = g;
          page->BITMAPS[nBM].cloud[2] = b;
          page->BITMAPS[nBM].cloud[3] = lx;
          page->BITMAPS[nBM].cloud[4] = ly;

          res = 2;
        }
      if (script_match_strings (str, "OMBRAGE"))
        {
          page->BITMAPS[nBM].OMBRAGE = 1;
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d)",
                  &page->BITMAPS[nBM].Ombrex, &page->BITMAPS[nBM].Ombrey);

        }
      if (script_match_strings (str, "OUTLINE"))
        {
          sscanf (&str[car (str, (char) '=') + 1], "%d",
                  &page->BITMAPS[nBM].OUTLINE);
        }
      fgets (str, 256, f);
    }
  if (res == 1)
    {
      pathname = script_locate_file (fname);
      if (pathname == NULL)
        {
          return FALSE;
        }
      SDL_FormatRGB32A =
      {
      NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0x000000FF, 0x0000FF00,
          0x00FF0000, 0xFF000000, 0, 0};
      /* load bitmap and converted it in 32 bpp */
      surface_tmp = IMG_Load (pathname);
      if (surface_tmp == NULL)
        {
          LOG_ERR ("IMG_Load(%s) return %s", pathname, SDL_GetError ());
          free_memory (pathname);
          return FALSE;
        }
      free_memory (pathname);
      page->BITMAPS[nBM].Surface =
        SDL_ConvertSurface (surface_tmp, &SDL_FormatRGB32A, SDL_SWSURFACE);
      page->BITMAPS[nBM].x = page->BITMAPS[nBM].Surface->w;
      page->BITMAPS[nBM].y = page->BITMAPS[nBM].Surface->h;
      page->BITMAPS[nBM].ptr = (char *) page->BITMAPS[nBM].Surface->pixels;
      page->nBITMAPS++;
    }
  if (res == 2)
    {
      ptr2 = (char *) memory_allocation (lx * ly * 4);
      if (ptr2 == NULL)
        {
          LOG_ERR ("not enough memory to alloc %i bytes!", lx * ly * 4);
          return FALSE;
        }
      ptr = (char *) memory_allocation ((lx + 1) * (ly + 1));
      if (ptr == NULL)
        {
          LOG_ERR ("not enough memory to alloc %i bytes!",
                   (lx + 1) * (ly + 1));
          return FALSE;
        }
      for (n1 = 0; n1 <= lx; n1++)
        {
          for (n2 = 0; n2 <= ly; n2++)
            {
              ptr[n1 + n2 * (lx + 1)] = 0;
            }
        }
      ptr[0] = 128 + rand () % 127;
      ptr[lx] = 128 + rand () % 127;
      ptr[(lx + 1) * (ly) + 0] = ptr[lx];
      script_recursx (ptr, 0, lx, 0, lx + 1, ly + 1);
      script_recursy (ptr, 0, ly, 0, lx + 1, ly + 1);
      for (n1 = 0; n1 <= lx; n1++)
        {
          ptr[n1 + (ly) * (lx + 1)] = ptr[n1];
        }
      for (n1 = 0; n1 <= ly; n1++)
        {
          ptr[lx + n1 * (lx + 1)] = ptr[(lx + 1) * n1];
        }
      script_recurs (ptr, 0, 0, lx, ly, lx + 1, ly + 1);
      for (n1 = 0; n1 < lx; n1++)
        {
          for (n2 = 0; n2 < ly; n2++)
            {
              c = ptr[n1 + n2 * (lx + 1)] & 255;
              ptr2[4 * (n1 + n2 * lx) + 0] = (r * c) / 255;
              ptr2[4 * (n1 + n2 * lx) + 1] = (g * c) / 255;
              ptr2[4 * (n1 + n2 * lx) + 2] = (b * c) / 255;
            }
        }
      page->BITMAPS[nBM].x = lx;
      page->BITMAPS[nBM].y = ly;
      page->BITMAPS[nBM].ptr = ptr2;
      page->nBITMAPS++;
      free_memory (ptr);
    }
  return TRUE;
}

/**
 * Search a font from 'Page' structure
 * @param font_name The font name (ie "FONTE0")
 * @param page Pointer to a 'Page' structure
 * @return Font index from 0 to n, otherwise -1 if font was not found 
 */
static Sint32
script_search_font (char *font_name, struct Page *page)
{
  Sint32 n, index;
  index = -1;
  n = 0;
  while ((n < page->nFONTES) && (index == -1))
    {
      if (strcmp (font_name, page->FONTES[n].name) == 0)
        {
          index = n;
        }
      n++;
    }
  if (index == -1)
    {
      LOG_ERR ("\"%s\" font not found!", font_name);
    }
  return index;
}

/**
 * Search a bitmap from 'Page' structure
 * @param bmp_name The bitmap name (ie "SCREENSHOT1")
 * @param page Pointer to a 'Page' structure
 * @return Bitmap index from 0 to n, otherwise -1 if bitmap was not found 
 */
static Sint32
script_search_bitmap (char *bmp_name, struct Page *page)
{
  Sint32 n, index;
  index = -1;
  LOG_DBG ("bitmap name: %s", bmp_name);
  n = 0;
  while ((n < page->nBITMAPS) && (index == -1))
    {
      if (strcmp (bmp_name, page->BITMAPS[n].name) == 0)
        {
          index = n;
        }
      n++;
    }
  if (index == -1)
    {
      LOG_ERR ("\"$s\" bitmap not found!", bmp_name);
    }
  return index;
}

/**
 * Check if a string contains a quote char
 * @param str The sting to check
 * @return TRUE if the string contains a quote char, otherwise FALSE
 */
static bool
script_check_quote (char *str)
{
  Sint32 index, g, length;
  index = 0;
  g = 0;
  length = strlen (str);
  while (index < length)
    {
      if (str[index] == (char) '"')
        {
          g++;
        }
      index++;
    }
  return (g == 1);
}

/**
 * Read a string from a 'TEXTE' section
 * @param f File structure pointer that identifies the stream to read 
 * @return String pointer to string or NULL if alloc failed
 */
static char *
script_read_string (FILE * f)
{
  char ss[512];
  char *sss;
  Sint32 nn, nnn, nn2;
  sss = (char *) memory_allocation (512);
  if (sss == NULL)
    {
      return NULL;
    }
  fscanf (f, "%s", ss);
  nn = 0;
  nnn = strlen (ss);
  while (nn < nnn - 1)
    {
      sss[nn] = ss[nn + 1];
      nn++;
    }
  sss[nn] = (char) ' ';
  nn++;
  if (script_check_quote (ss))
    {
      fscanf (f, "%s", ss);
      nnn = strlen (ss);
      nn2 = 0;
      while (nn2 < nnn)
        {
          sss[nn] = ss[nn2];
          nn++;
          nn2++;
        }
      sss[nn] = (char) ' ';
      nn++;
      while (!script_check_quote (ss))
        {
          fscanf (f, "%s", ss);
          nnn = strlen (ss);
          nn2 = 0;
          while (nn2 < nnn)
            {
              sss[nn] = ss[nn2];
              nn++;
              nn2++;
            }
          sss[nn] = (char) ' ';
          nn++;
        }
    }
  nn = 0;
  while (sss[nn] != (char) '"')
    {
      nn++;
    }
  sss[nn] = (char) '\0';
  return sss;
}

/**
 * Read "COMPOSITION" section
 * @param f File structure pointer that identifies the stream to read 
 */
static bool
script_read_composition_section (FILE * f, struct Page *page)
{
  char str[256];
  Sint32 n;
  fscanf (f, "%s", str);
  while (!isBEGIN (str))
    {
      fscanf (f, "%s", str);
    }
  n = 0;
  fscanf (f, "%s", str);
  while (!isEND (str))
    {
      if (script_match_strings (str, "MOSAIC"))
        {
          page->OPS[n].op = MOSAIQUE;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      if (script_match_strings (str, "ETIRER"))
        {
          page->OPS[n].op = ETIRER;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      if (script_match_strings (str, "TEXTE"))
        {
          page->OPS[n].op = TEXTE;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          page->OPS[n].str = script_read_string (f);
          if (page->OPS[n].str == NULL)
            {
              return FALSE;
            }
          n++;
        }
      /* coordinates of counter string */
      if (script_match_strings (str, "COMPTEUR"))
        {
          page->OPS[n].op = COMPTEUR;
          memset (page->OPS[n].name, 0, 512);
          char tmp[256];
          sscanf (&str[car (str, (char) '=') + 1], "%s", tmp);
          sscanf (tmp, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      /* coordinates of sting of number of
       * times the game has been launched */
      if (script_match_strings (str, "NBLAUNCH"))
        {
          page->OPS[n].op = NBLAUNCH;
          /* intiatlize number of times */
          page->OPS[n].amp = 0;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      if (script_match_strings (str, "FBITMAP"))
        {
          page->OPS[n].op = BITMAPFADE;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      else if (script_match_strings (str, "BITMAP"))
        {
          page->OPS[n].op = BITMAP;
          sscanf (&str[car (str, (char) '=') + 1], "%s", page->OPS[n].name);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      else if (script_match_strings (str, "ONDEOMBREE"))
        {
          page->OPS[n].op = ONDES2;
          memset (page->OPS[n].name, 0, 512);
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d)",
                  &page->OPS[n].amp, &page->OPS[n].rayon);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          n++;
        }
      else if (script_match_strings (str, "ONDE"))
        {
          page->OPS[n].op = ONDES;
          memset (page->OPS[n].name, 0, 512);
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d)",
                  &page->OPS[n].amp, &page->OPS[n].rayon);
          n++;
        }
      else if (script_match_strings (str, "EMBOSS"))
        {
          page->OPS[n].op = EMBOSS;
          memset (page->OPS[n].name, 0, 512);
          sscanf (&str[car (str, (char) '=') + 1], "(%d,%d,%d)",
                  &page->OPS[n].x, &page->OPS[n].y, &page->OPS[n].rayon);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d,%d)", &page->OPS[n].rgb.r,
                  &page->OPS[n].rgb.g, &page->OPS[n].rgb.b);
          n++;
        }
      if (script_match_strings (str, "CARREDEGRADE"))
        {
          page->OPS[n].op = CARREDEGRADE;
          memset (page->OPS[n].name, 0, 512);
          sscanf (&str[car (str, '=') + 1], "COULEUR(%d,%d,%d)",
                  &page->OPS[n].rgb.r, &page->OPS[n].rgb.g,
                  &page->OPS[n].rgb.b);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].x, &page->OPS[n].y);
          fscanf (f, "%s", str);
          sscanf (str, "(%d,%d)", &page->OPS[n].lx, &page->OPS[n].ly);
          n++;
        }
      fscanf (f, "%s", str);
    }
  page->nOPS = n;
  return TRUE;
}

/**
 * Read the script file
 * @param fname The filename specified by path
 * @return
 */
struct Page *
script_read_file (char *fname)
{
  bool read_file = TRUE;
  char *pathname;
  struct Page *PAGE;
  FILE *f;
  char str[512];
  PAGE = (struct Page *) memory_allocation (sizeof (struct Page));
  if (PAGE == NULL)
    {
      LOG_ERR ("not enough memory to allocate 'Page' structure");
      return NULL;
    }
  PAGE->nBITMAPS = 0;
  PAGE->nFONTES = 0;
  PAGE->nOPS = 0;
  pathname = script_locate_file (fname);
  if (pathname == NULL)
    {
      return NULL;
    }
  LOG_DBG ("filename = %s", fname);
  f = fopen (pathname, "rb");
  free_memory (pathname);
  if (f == NULL)
    {
      return NULL;
    }
  while (read_file)
    {
      fscanf (f, "%s", str);
      if (strcmp (str, "FONTES") == 0)
        {
          fscanf (f, "%s", str);
          while (!isBEGIN (str))
            {
              fscanf (f, "%s", str);
            }

          fscanf (f, "%s", str);
          while (!isEND (str))
            {
              if (!script_add_font_section (f, str, PAGE))
                {
                  return FALSE;
                }
              fscanf (f, "%s", str);
            }
        }
      if (strcmp (str, "BITMAPS") == 0)
        {
          fscanf (f, "%s", str);
          while (!isBEGIN (str))
            {
              fscanf (f, "%s", str);
            }
          fscanf (f, "%s", str);
          while (!isEND (str))
            {
              if (!script_add_bitmap_section (f, str, PAGE))
                {
                  return NULL;
                }
              fscanf (f, "%s", str);
            }
        }
      if (strcmp (str, "COMPOSITION") == 0)
        {
          if (!script_read_composition_section (f, PAGE))
            {
              return NULL;
            }
          read_file = FALSE;
        }
      if (strcmp (str, "END") == 0)
        {
          read_file = FALSE;
        }
      if (feof (f))
        {
          read_file = FALSE;
        }
    }
  fclose (f);
  return PAGE;
}

/**
 *
 * @param prt
 * @param sizex
 * @param sizey
 * @return 
 */
static char *
script_outline (char *ptr, Sint32 sizex, Sint32 sizey)
{
  char *pp;
  char c;
  Sint32 n1, n2, n;
  pp = (char *) memory_allocation (sizex * sizey);
  if (pp == NULL)
    {
      return NULL;
    }
  for (n = 0; n < sizex * sizey; n++)
    {
      pp[n] = 0;
    }
  for (n1 = 1; n1 < sizex - 1; n1++)
    for (n2 = 1; n2 < sizey - 1; n2++)
      {
        if (ptr[n1 + n2 * sizex] != (char) 255)
          {
            c = 0;
            if (ptr[n1 + 1 + (n2 + 0) * sizex] != 0)
              {
                c = 1;
              }
            if (ptr[n1 - 1 + (n2 + 0) * sizex] != 0)
              {
                c = 1;
              }
            if (ptr[n1 + 0 + (n2 + 1) * sizex] != 0)
              {
                c = 1;
              }
            if (ptr[n1 + 0 + (n2 - 1) * sizex] != 0)
              {
                c = 1;
              }
            if (c == 0)
              {
                pp[n1 + n2 * sizex] = 0;
              }
            else
              {
                pp[n1 + n2 * sizex] = 128;
              }
          }
        else
          {
            pp[n1 + n2 * sizex] = 255;
          }
      }
  free_memory (ptr);
  return pp;
}

/**
 * Draw a string
 * @param prt
 * @param str
 * @param x
 * @param y
 * @param font
 * @param f
 * @return
 */
Sint32
script_draw_string (char *ptr, char *str, Sint32 x, Sint32 y,
                    struct Fonte * font, Sint32 f)
{
  Sint32 n;
  SDL_Surface *stext;
  char *tmp;
  Sint32 sizex, sizey;
  Sint32 px, py;
  char *psrc;
  Sint32 n1, n2;
  Sint32 c[4];
  Sint32 r[4];
  Sint32 g[4];
  Sint32 b[4];
  Sint32 r0, g0, b0;
  Sint32 adr, adr2;
  char *ptr2;
  Sint32 nn2;
  SDL_Color white = { 255, 255, 255, 0 };
  if (font->font == NULL)
    {
      LOG_ERR ("TTF_Font pointer is NULL!");
      return 0;
    }
  stext = TTF_RenderUTF8_Blended (font->font, str, white);
  if (stext == NULL)
    {
      LOG_ERR ("TTF_RenderUTF8_Blended(%s) return %s", ptr, SDL_GetError ());
      return 0;
    }
  sizex = stext->w + font->OUTLINE * 2;
  sizey = stext->h + font->OUTLINE * 2 * 4;
  tmp = (char *) memory_allocation (sizex * sizey);
  if (tmp == NULL)
    {
      return 0;
    }
  memset (tmp, 0, sizex * sizey);
  for (n = 0; n < sizex * sizey; n++)
    {
      tmp[n] = 0;
    }
  psrc = (char *) stext->pixels;
  for (py = 0; py < stext->h; py++)
    {
      for (px = 0; px < stext->w; px++)
        {

          tmp[(py + font->OUTLINE) * sizex + (px + font->OUTLINE)] =
            psrc[py * stext->pitch + px * 4 + 3];
        }
    }
  for (n = 0; n < font->OUTLINE; n++)
    {
      tmp = script_outline (tmp, sizex, sizey);
    }
  SDL_FreeSurface (stext);
  ptr2 = (char *) memory_allocation ((sizey / 2) * (sizex / 2) * 4);
  if (ptr2 == NULL)
    {
      return 0;
    }

  for (n2 = 0; n2 < sizey / 2; n2++)
    {
      adr2 = 4 * ((sizex / 2) * n2);
      adr = 4 * (x + window_width * (y + n2));

      for (n1 = 0; n1 < sizex / 2; n1++)
        {
          r0 = ptr[adr + 0] & 255;
          g0 = ptr[adr + 1] & 255;
          b0 = ptr[adr + 2] & 255;
          ptr2[adr2 + 0] = r0;
          ptr2[adr2 + 1] = g0;
          ptr2[adr2 + 2] = b0;
          adr2 += 4;
          adr += 4;
        }
    }
  for (n2 = 0; n2 < sizey / 2; n2++)
    {
      if ((y + n2 >= 0) && (y + n2 < window_height))
        for (n1 = 0; n1 < sizex / 2; n1++)
          {
            if ((x + n1 >= 0) && (x + n1 < window_width))
              {
                adr = n1 * 2 + 0 + sizex * (n2 * 2 + 0);
                c[0] = tmp[adr] & 255;
                c[1] = tmp[adr + 1] & 255;
                c[2] = tmp[adr + sizex] & 255;
                c[3] = tmp[adr + sizex + 1] & 255;
                adr2 = 4 * (n1 + (sizex / 2) * (n2));
                for (n = 0; n < 4; n++)
                  {
                    if (c[n] == 0)
                      {
                        r[n] = ptr2[adr2 + 0] & 255;
                        g[n] = ptr2[adr2 + 1] & 255;
                        b[n] = ptr2[adr2 + 2] & 255;
                      }
                    else if (c[n] == 128)
                      {
                        if (f == 1)
                          {
                            r[n] = (ptr2[adr2 + 0] & 255) >> 1;
                            g[n] = (ptr2[adr2 + 1] & 255) >> 1;
                            b[n] = (ptr2[adr2 + 2] & 255) >> 1;
                          }
                        else
                          {
                            /* outline */
                            r[n] = 0;
                            g[n] = 0;
                            b[n] = 0;
                          }
                      }
                    else
                      {
                        if (f == 1)
                          {
                            r[n] = (ptr2[adr2 + 0] & 255) >> 1;
                            g[n] = (ptr2[adr2 + 1] & 255) >> 1;
                            b[n] = (ptr2[adr2 + 2] & 255) >> 1;
                          }
                        else
                          {

                            if (font->DEGRADE == 2)
                              {
                                nn2 =
                                  (Sint32) (n2 + 7 * sin (n1 * 3.14159 / 90));
                                if (nn2 > sizey / 2)
                                  {
                                    nn2 = sizey / 2;
                                  }
                                if (nn2 < 0)
                                  {
                                    nn2 = 0;
                                  }
                              }
                            else
                              {
                                nn2 = n2;
                              }
                            r[n] =
                              font->DEGRADE_HAUT.r +
                              nn2 * (font->DEGRADE_BAS.r -
                                     font->DEGRADE_HAUT.r) / ((sizey /
                                                               2) + 1);
                            g[n] =
                              font->DEGRADE_HAUT.g +
                              nn2 * (font->DEGRADE_BAS.g -
                                     font->DEGRADE_HAUT.g) / ((sizey /
                                                               2) + 1);
                            b[n] =
                              font->DEGRADE_HAUT.b +
                              nn2 * (font->DEGRADE_BAS.b -
                                     font->DEGRADE_HAUT.b) / ((sizey /
                                                               2) + 1);
                          }

                      }
                  }

                r0 = (r[0] + r[1] + r[2] + r[3]) / 4;
                g0 = (g[0] + g[1] + g[2] + g[3]) / 4;
                b0 = (b[0] + b[1] + b[2] + b[3]) / 4;

                ptr[4 * (x + n1 + window_width * (y + n2)) + 0] = (char) r0;
                ptr[4 * (x + n1 + window_width * (y + n2)) + 1] = (char) g0;
                ptr[4 * (x + n1 + window_width * (y + n2)) + 2] = (char) b0;

              }
          }
    }
  free_memory (ptr2);
  free_memory (tmp);
  return (sizex / 2);
}

/**
 * Draw rectangle
 * @param ptr
 * @param x
 * @param y
 * @param lx
 * @param ly
 * @param r
 * @param g
 * @param b
 */
void
script_draw_rectangle (char *ptr, Sint32 x, Sint32 y, Sint32 lx, Sint32 ly,
                       Sint32 r, Sint32 g, Sint32 b)
{
  Sint32 n1, n2;
  float s;
  Sint32 r0, g0, b0;
  Sint32 top, bottom, left, right;
  Sint32 ss;
  ss = 24;
  for (n1 = 0; n1 < ly; n1++)
    {
      if ((y + n1 >= 0) && (y + n1 < window_height))
        for (n2 = 0; n2 < lx; n2++)
          {

            if ((x + n2 >= 0) && (x + n2 < window_width))
              {

                top = n1 - ss;
                bottom = n1 + ss;
                left = n2 - ss;
                right = n2 + ss;

                if (right >= lx)
                  right = lx - 1;
                if (left < 0)
                  left = 0;
                if (top < 0)
                  top = 0;
                if (bottom >= ly)
                  bottom = ly - 1;

                s =
                  ((float) ((right - left) * (bottom - top)) -
                   ss * ss * 2) / (ss * ss * 2);
                if (s > 1)
                  s = 1;
                if (s < 0)
                  s = 0;
                s = s * s;

                r0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 0] & 255;
                g0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 1] & 255;
                b0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 2] & 255;

                r0 = (Sint32) (r * s + (1 - s) * r0);
                g0 = (Sint32) (g * s + (1 - s) * g0);
                b0 = (Sint32) (b * s + (1 - s) * b0);


                ptr[4 * (x + n2 + (y + n1) * window_width) + 0] = r0;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 1] = g0;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 2] = b0;

              }
          }
    }
}

/**
 * 
 * @param prt
 * @param x
 * @param y
 * @param lx
 * @param ly
 * @param im
 */
void
script_draw_rectangleBitmap (char *ptr, Sint32 x, Sint32 y, Sint32 lx,
                             Sint32 ly, char *im)
{
  Sint32 n1, n2;
  float s;
  Sint32 r0, g0, b0;
  Sint32 top, bottom, left, right;
  Sint32 r, g, b;
  Sint32 ss;

  ss = (Sint32) (sqrt (lx * lx + ly * ly) / 10);


  for (n1 = 0; n1 < ly; n1++)
    {
      if ((y + n1 >= 0) && (y + n1 < window_height))
        for (n2 = 0; n2 < lx; n2++)
          {

            if ((x + n2 >= 0) && (x + n2 < window_width))
              {

                top = n1 - ss;
                bottom = n1 + ss;
                left = n2 - ss;
                right = n2 + ss;

                if (right >= lx)
                  right = lx - 1;
                if (left < 0)
                  left = 0;
                if (top < 0)
                  top = 0;
                if (bottom >= ly)
                  bottom = ly - 1;

                s =
                  ((float) ((right - left) * (bottom - top)) -
                   ss * ss * 2) / (ss * ss * 2);
                if (s > 1)
                  s = 1;
                if (s < 0)
                  s = 0;
                s = s * s;

                r0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 0] & 255;
                g0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 1] & 255;
                b0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 2] & 255;

                r = im[4 * (n1 * lx + n2) + 0] & 255;
                g = im[4 * (n1 * lx + n2) + 1] & 255;
                b = im[4 * (n1 * lx + n2) + 2] & 255;

                r0 = (Sint32) (r * s + (1 - s) * r0);
                g0 = (Sint32) (g * s + (1 - s) * g0);
                b0 = (Sint32) (b * s + (1 - s) * b0);


                ptr[4 * (x + n2 + (y + n1) * window_width) + 0] = r0;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 1] = g0;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 2] = b0;

              }

          }
    }
}

static char dither16[5][2][2] =
  { {{1, 1}, {1, 1}}, {{1, 1}, {0, 1}}, {{1, 0}, {0, 1}}, {{1, 0}, {0, 0}},
  {{0, 0}, {0, 0}} };
/**
 *
 * @param ptr
 * @param x
 * @param y
 * @param lx
 * @param ly
 * @param ss
 */
static void
script_draw_shadow_rectangle (char *ptr, Sint32 x, Sint32 y, Sint32 lx,
                              Sint32 ly, Sint32 ss)
{
  Sint32 n1, n2;
  float s;
  Sint32 r0, g0, b0;
  Sint32 r, g, b;
  Sint32 top, bottom, left, right;
  for (n1 = 0; n1 < ly; n1++)
    {
      if ((y + n1 >= 0) && (y + n1 < window_height))
        for (n2 = 0; n2 < lx; n2++)
          {
            if ((x + n2 >= 0) && (x + n2 < window_width))
              {
                top = n1 - ss;
                bottom = n1 + ss;
                left = n2 - ss;
                right = n2 + ss;
                if (right >= lx)
                  {
                    right = lx - 1;
                  }
                if (left < 0)
                  {
                    left = 0;
                  }
                if (top < 0)
                  {
                    top = 0;
                  }
                if (bottom >= ly)
                  {
                    bottom = ly - 1;
                  }
                s =
                  ((float) ((right - left) * (bottom - top)) -
                   ss * ss * 2) / (ss * ss * 2);
                if (s > 1)
                  {
                    s = 1;
                  }
                if (s < 0)
                  {
                    s = 0;
                  }
                s = s * s;
                r0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 0] & 255;
                g0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 1] & 255;
                b0 = ptr[4 * (x + n2 + (y + n1) * window_width) + 2] & 255;
                r = (Sint32) (1 - s / 2) * r0;
                g = (Sint32) (1 - s / 2) * g0;
                b = (Sint32) (1 - s / 2) * b0;
                if (is_16bits_dithering)
                  {
                    r0 = r & 7;
                    if (r0 > 4)
                      r0 = 4;
                    r0 = dither16[4 - r0][x & 1][y & 1];
                    r = ((r >> 3) + r0) << 3;
                    if (r > 255)
                      r = 255;
                    g0 = g & 3;
                    g0 = dither16[4 - g0][x & 1][y & 1];
                    g = ((g >> 2) + g0) << 2;
                    if (g > 255)
                      g = 255;
                    b0 = b & 7;
                    if (b0 > 4)
                      b0 = 4;
                    b0 = dither16[4 - b0][x & 1][y & 1];
                    b = ((b >> 3) + b0) << 3;
                    if (b > 255)
                      b = 255;
                  }
                ptr[4 * (x + n2 + (y + n1) * window_width) + 0] = r;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 1] = g;
                ptr[4 * (x + n2 + (y + n1) * window_width) + 2] = b;
              }
          }
    }
}

/**
 *
 * @param n1
 * @param n2
 * @param amp
 * @param rayon
 * @param rmax
 * @return 
 */
static float
script_wave_heights (Sint32 n1, Sint32 n2, Sint32 amp, Sint32 rayon,
                     float rmax)
{
  float r, a, aa;
  r =
    (float) sqrt ((n1 - (window_width / 2)) * (n1 - (window_width / 2)) +
                  (n2 - (window_height / 2)) * (n2 -
                                                (window_height / 2))) / rmax;
  if (r > 1)
    {
      r = 1;
    }
  if (r < 0)
    {
      r = 0;
    }
  a = amp * (1 - r);
  aa = ((float) sin (3.14159 * rayon * r) + 1) / 2;
  return a * aa;
}


/**
 * Generate the page
 * @param page Pointer to a 'Page' structure
 * @return Pointer to a SDL_Surface structure
 */
SDL_Surface *
script_generate_page (struct Page * page)
{
  Sint32 n, nn, x, y, lx, ly, x0, y0;
  char *ptr, *ptr2, *ptr3;
  char *bm;
  Sint32 len;
  float f;
  SDL_Surface *Surface;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Surface =
    SDL_CreateRGBSurface (SDL_SWSURFACE, window_width, window_height, 32,
                          0x000000FF, 0x0000FF00, 0x00FF0000, 0x000000FF);
#else
  Surface =
    SDL_CreateRGBSurface (SDL_SWSURFACE, window_width, window_height, 32,
                          0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
#endif
  if (Surface == NULL)
    {
      LOG_ERR ("SDL_CreateRGBSurface() return %s", SDL_GetError ());
      return NULL;
    }
  ptr = (char *) Surface->pixels;
  for (n = 0; n < page->nOPS; n++)
    {
      if (page->OPS[n].op == MOSAIQUE)
        {
          nn = script_search_bitmap (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_bitmap() failed!");
              return NULL;
            }
          bm = page->BITMAPS[nn].ptr;
          lx = page->BITMAPS[nn].x;
          ly = page->BITMAPS[nn].y;
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          for (x = 0; x < window_width; x++)
            for (y = 0; y < window_height; y++)
              {
                ptr[4 * (x + y * window_width) + 0] =
                  bm[4 * (((x0 + x) % lx) + ((y0 + y) % ly) * lx) + 0];
                ptr[4 * (x + y * window_width) + 1] =
                  bm[4 * (((x0 + x) % lx) + ((y0 + y) % ly) * lx) + 1];
                ptr[4 * (x + y * window_width) + 2] =
                  bm[4 * (((x0 + x) % lx) + ((y0 + y) % ly) * lx) + 2];

              }
        }

      if (page->OPS[n].op == ETIRER)
        {
          nn = script_search_bitmap (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_bitmap() failed!");
              return NULL;
            }
          bm = page->BITMAPS[nn].ptr;
          lx = page->BITMAPS[nn].x;
          ly = page->BITMAPS[nn].y;
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          float fx, fy, xx, yy;
          Sint32 r0, g0, b0;
          Sint32 r1, g1, b1;
          Sint32 r2, g2, b2;
          Sint32 r3, g3, b3;
          Sint32 r00, g00, b00;
          Sint32 r01, g01, b01;
          Sint32 r, g, b;
          float incx, incy;

          incx = (float) (lx - 1) / window_width;
          incy = (float) (ly - 1) / window_height;

          fy = 0;
          for (y = 0; y < window_height; y++)
            {
              fx = 0;
              for (x = 0; x < window_width; x++)
                {
                  xx = fx - (int) fx;
                  yy = fy - (int) fy;

                  r0 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + (int) fy) * lx) + 0] &
                     255);
                  g0 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + (int) fy) * lx) + 1] &
                     255);
                  b0 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + (int) fy) * lx) + 2] &
                     255);

                  r1 =
                    (bm[4 * ((x0 + 1 + (int) fx) + (y0 + (int) fy) * lx) + 0]
                     & 255);
                  g1 =
                    (bm[4 * ((x0 + 1 + (int) fx) + (y0 + (int) fy) * lx) + 1]
                     & 255);
                  b1 =
                    (bm[4 * ((x0 + 1 + (int) fx) + (y0 + (int) fy) * lx) + 2]
                     & 255);

                  r2 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + 1 + (int) fy) * lx) + 0]
                     & 255);
                  g2 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + 1 + (int) fy) * lx) + 1]
                     & 255);
                  b2 =
                    (bm[4 * ((x0 + (int) fx) + (y0 + 1 + (int) fy) * lx) + 2]
                     & 255);

                  r3 =
                    (bm
                     [4 * ((x0 + 1 + (int) fx) + (y0 + 1 + (int) fy) * lx) +
                      0] & 255);
                  g3 =
                    (bm
                     [4 * ((x0 + 1 + (int) fx) + (y0 + 1 + (int) fy) * lx) +
                      1] & 255);
                  b3 =
                    (bm
                     [4 * ((x0 + 1 + (int) fx) + (y0 + 1 + (int) fy) * lx) +
                      2] & 255);

                  r00 = (Sint32) (r0 + xx * (r1 - r0));
                  g00 = (Sint32) (g0 + xx * (g1 - g0));
                  b00 = (Sint32) (b0 + xx * (b1 - b0));

                  r01 = (Sint32) (r2 + xx * (r3 - r2));
                  g01 = (Sint32) (g2 + xx * (g3 - g2));
                  b01 = (Sint32) (b2 + xx * (b3 - b2));

                  r = (Sint32) (r00 + yy * (r01 - r00));
                  g = (Sint32) (g00 + yy * (g01 - g00));
                  b = (Sint32) (b00 + yy * (b01 - b00));

                  if (is_16bits_dithering)
                    {
                      r0 = r & 7;
                      if (r0 > 4)
                        r0 = 4;
                      r0 = dither16[4 - r0][x & 1][y & 1];
                      r = ((r >> 3) + r0) << 3;
                      if (r > 255)
                        r = 255;
                      g0 = g & 3;
                      g0 = dither16[4 - g0][x & 1][y & 1];
                      g = ((g >> 2) + g0) << 2;
                      if (g > 255)
                        g = 255;
                      b0 = b & 7;
                      if (b0 > 4)
                        b0 = 4;
                      b0 = dither16[4 - b0][x & 1][y & 1];
                      b = ((b >> 3) + b0) << 3;
                      if (b > 255)
                        b = 255;
                    }

                  ptr[4 * (x + y * window_width) + 0] = r;
                  ptr[4 * (x + y * window_width) + 1] = g;
                  ptr[4 * (x + y * window_width) + 2] = b;

                  fx += incx;
                }
              fy += incy;
            }
        }
      if (page->OPS[n].op == BITMAP)
        {
          nn = script_search_bitmap (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_bitmap() failed!");
              return NULL;
            }
          bm = page->BITMAPS[nn].ptr;
          lx = page->BITMAPS[nn].x;
          ly = page->BITMAPS[nn].y;
          if (page->BITMAPS[nn].OMBRAGE == 1)
            {
              x0 = page->OPS[n].x + page->BITMAPS[nn].Ombrex * 3;
              y0 = page->OPS[n].y + page->BITMAPS[nn].Ombrey * 3;
              script_draw_shadow_rectangle (ptr, x0, y0, lx, ly, 6);
            }
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          Sint32 pr, pv, pb;
          float pa;
          for (x = 0; x < lx; x++)
            for (y = 0; y < ly; y++)
              {
                pa = (unsigned char) bm[4 * (x + y * lx) + 3] / 255.0f;

                pr =
                  ((unsigned char)
                   ptr[4 * (x0 + x + (y0 + y) * window_width) +
                       0]) * (Sint32) (1.0f - pa) +
                  ((unsigned char) bm[4 * (x + y * lx) + 0]) * (Sint32) pa;
                pv =
                  ((unsigned char)
                   ptr[4 * (x0 + x + (y0 + y) * window_width) +
                       1]) * (Sint32) (1.0f - pa) +
                  ((unsigned char) bm[4 * (x + y * lx) + 1]) * (Sint32) pa;
                pb =
                  ((unsigned char)
                   ptr[4 * (x0 + x + (y0 + y) * window_width) +
                       2]) * (Sint32) (1.0f - pa) +
                  ((unsigned char) bm[4 * (x + y * lx) + 2]) * (Sint32) pa;

                ptr[4 * (x0 + x + (y0 + y) * window_width) + 0] = pr;
                ptr[4 * (x0 + x + (y0 + y) * window_width) + 1] = pv;
                ptr[4 * (x0 + x + (y0 + y) * window_width) + 2] = pb;
              }

          if (page->BITMAPS[nn].OUTLINE == 1)
            {
              x0 = page->OPS[n].x;
              y0 = page->OPS[n].y;
              for (x = -1; x <= lx; x++)
                {
                  y = -1;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 0] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 1] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 2] = 255;
                  y = ly;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 0] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 1] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 2] = 255;
                }
              for (y = 0; y <= ly; y++)
                {
                  x = -1;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 0] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 1] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 2] = 255;
                  x = lx;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 0] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 1] = 255;
                  ptr[4 * (x0 + x + (y0 + y) * window_width) + 2] = 255;

                }
            }
          page->OPS[n].x1 = x0;
          page->OPS[n].y1 = y0;
          page->OPS[n].x2 = x0 + lx;
          page->OPS[n].y2 = y0 + ly;
        }

      if (page->OPS[n].op == BITMAPFADE)
        {
          nn = script_search_bitmap (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_bitmap() failed!");
              return NULL;
            }
          bm = page->BITMAPS[nn].ptr;
          lx = page->BITMAPS[nn].x;
          ly = page->BITMAPS[nn].y;
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          script_draw_rectangleBitmap (ptr, x0, y0, lx, ly, bm);
          page->OPS[n].x1 = x0;
          page->OPS[n].y1 = y0;
          page->OPS[n].x2 = x0 + lx;
          page->OPS[n].y2 = y0 + ly;
        }

      if (page->OPS[n].op == TEXTE)
        {
          nn = script_search_font (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_font() failed!");
              return NULL;
            }
          if (page->FONTES[nn].OMBRAGE == 1)
            {
              x0 = page->OPS[n].x + page->FONTES[nn].Ombrex;
              y0 = page->OPS[n].y + page->FONTES[nn].Ombrey;
              script_draw_string (ptr, page->OPS[n].str, x0, y0,
                                  &page->FONTES[nn], 1);

            }
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          len =
            script_draw_string (ptr, page->OPS[n].str, x0, y0,
                                &page->FONTES[nn], 0);

          page->OPS[n].x1 = x0;
          page->OPS[n].y1 = y0;
          page->OPS[n].x2 = x0 + len;
          page->OPS[n].y2 = y0 + page->FONTES[nn].size / 2;

        }


      /* display number of times the game has been launched */
      if (page->OPS[n].op == NBLAUNCH)
        {
          nn = script_search_font (page->OPS[n].name, page);
          if (nn == -1)
            {
              LOG_ERR ("script_search_font() failed!");
              return NULL;
            }
          snprintf (sname, 512, "%d", page->OPS[n].amp);
          if (page->FONTES[nn].OMBRAGE == 1)
            {
              x0 = page->OPS[n].x + page->FONTES[nn].Ombrex;
              y0 = page->OPS[n].y + page->FONTES[nn].Ombrey;
              script_draw_string (ptr, sname, x0, y0, &page->FONTES[nn], 1);

            }
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          len = script_draw_string (ptr, sname, x0, y0, &page->FONTES[nn], 0);
          page->OPS[n].x1 = x0;
          page->OPS[n].y1 = y0;
          page->OPS[n].x2 = x0 + len;
          page->OPS[n].y2 = y0 + page->FONTES[nn].size / 2;
        }

      if (page->OPS[n].op == CARREDEGRADE)
        {
          x0 = page->OPS[n].x;
          y0 = page->OPS[n].y;
          script_draw_shadow_rectangle (ptr, x0 + 16, y0 + 16,
                                        page->OPS[n].lx, page->OPS[n].ly, 24);

          script_draw_rectangle (ptr, x0, y0, page->OPS[n].lx,
                                 page->OPS[n].ly, page->OPS[n].rgb.r,
                                 page->OPS[n].rgb.g, page->OPS[n].rgb.b);
          page->OPS[n].x1 = x0;
          page->OPS[n].y1 = y0;
          page->OPS[n].x2 = x0 + page->OPS[n].lx;
          page->OPS[n].y2 = y0 + page->OPS[n].ly;
        }

      if (page->OPS[n].op == ONDES)
        {
          ptr2 =
            (char *) memory_allocation (window_width * window_height * 4);
          if (ptr2 == NULL)
            {
              return NULL;
            }
          memcpy (ptr2, ptr, window_width * window_height * 4);
          float ax1, ay1, ax2, ay2, rmax;
          Sint32 ax, ay;
          Sint32 n1, n2;
          Sint32 adr, adr2;
          rmax =
            (float) sqrt ((window_width / 2) * (window_width / 2) +
                          (window_height / 2) * (window_height / 2));
          adr2 = 0;
          for (n2 = 0; n2 < window_height; n2++)
            {
              for (n1 = 0; n1 < window_width; n1++)
                {
                  ax1 =
                    script_wave_heights (n1 - 1, n2, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ay1 =
                    script_wave_heights (n1, n2 - 1, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ax2 =
                    script_wave_heights (n1 + 1, n2, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ay2 =
                    script_wave_heights (n1, n2 + 1, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);

                  ax = n1 + (Sint32) (ax2 - ax1);
                  ay = n2 + (Sint32) (ay2 - ay1);

                  if (!
                      ((ax >= 0) && (ax < window_width) && (ay >= 0)
                       && (ay < window_height)))
                    {
                      if (ax < 0)
                        ax = 0;
                      if (ay < 0)
                        ay = 0;
                      if (ax >= window_width)
                        ax = window_width - 1;
                      if (ay >= window_height)
                        ay = window_height - 1;

                    }
                  adr = 4 * (ax + (ay) * window_width);
                  ptr[adr2 + 0] = ptr2[adr + 0];
                  ptr[adr2 + 1] = ptr2[adr + 1];
                  ptr[adr2 + 2] = ptr2[adr + 2];
                  adr2 += 4;
                }
            }
          free_memory (ptr2);
        }


      if (page->OPS[n].op == ONDES2)
        {
          ptr2 =
            (char *) memory_allocation (window_width * window_height * 4);
          if (ptr2 == NULL)
            {
              return NULL;
            }
          memcpy (ptr2, ptr, window_width * window_height * 4);
          float ax1, ay1, ax2, ay2, rmax;
          Sint32 ax, ay;
          Sint32 n1, n2;
          Sint32 r, g, b;
          Sint32 adr, adr2;
          rmax =
            (float) sqrt ((window_width / 2) * (window_width / 2) +
                          (window_height / 2) * (window_height / 2));
          adr2 = 0;
          for (n2 = 0; n2 < window_height; n2++)
            {
              for (n1 = 0; n1 < window_width; n1++)
                {
                  ax1 =
                    script_wave_heights (n1 - 1, n2, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ay1 =
                    script_wave_heights (n1, n2 - 1, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ax2 =
                    script_wave_heights (n1 + 1, n2, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);
                  ay2 =
                    script_wave_heights (n1, n2 + 1, page->OPS[n].amp,
                                         page->OPS[n].rayon, rmax);

                  ax =
                    ((Sint32) (ax2 - ax1) * page->OPS[n].x +
                     (Sint32) (ay2 - ay1) * page->OPS[n].y);

                  f = 1 + (float) ax / (16 * 128);
                  ax = n1 + (Sint32) (ax2 - ax1);
                  ay = n2 + (Sint32) (ay2 - ay1);
                  if (!
                      ((ax >= 0) && (ax < window_width) && (ay >= 0)
                       && (ay < window_height)))
                    {
                      if (ax < 0)
                        ax = 0;
                      if (ay < 0)
                        ay = 0;
                      if (ax >= window_width)
                        ax = window_width - 1;
                      if (ay >= window_height)
                        ay = window_height - 1;

                    }

                  adr = 4 * (ax + (ay) * window_width);
                  r = ptr2[adr + 0] & 255;
                  g = ptr2[adr + 1] & 255;
                  b = ptr2[adr + 2] & 255;
                  r *= (Sint32) f;
                  g *= (Sint32) f;
                  b *= (Sint32) f;
                  if (r > 255)
                    {
                      r = 255;
                    }
                  if (g > 255)
                    {
                      g = 255;
                    }
                  if (b > 255)
                    {
                      b = 255;
                    }
                  if (r < 0)
                    {
                      r = 0;
                    }
                  if (g < 0)
                    {
                      g = 0;
                    }
                  if (b < 0)
                    {
                      b = 0;
                    }
                  ptr[adr2 + 0] = r;
                  ptr[adr2 + 1] = g;
                  ptr[adr2 + 2] = b;
                  adr2 += 4;
                }
            }
          free_memory (ptr2);
        }
      if (page->OPS[n].op == EMBOSS)
        {
          Sint32 h00, h01, h10;
          Sint32 n1, n2;
          Sint32 r, g, b;
          Sint32 adr;
          ptr2 =
            (char *) memory_allocation (window_width * window_height * 4);
          if (ptr2 == NULL)
            {
              return NULL;
            }
          memcpy (ptr2, ptr, window_width * window_height * 4);
          ptr3 =
            (char *) memory_allocation (window_width * window_height * 4);
          if (ptr3 == NULL)
            {
              return NULL;
            }
          for (n2 = 0; n2 < window_height * 2; n2++)
            {
              for (n1 = 0; n1 < window_width * 2; n1++)
                {
                  adr = (n1 + n2 * window_width * 2);
                  x = page->OPS[n].x - n1 + window_width / 2;
                  y = page->OPS[n].y - n2 + window_height / 2;
                  f = sqrt (x * x + y * y) / page->OPS[n].rayon;
                  if (f > 1)
                    {
                      f = 1;
                    }
                  if (f < 0)
                    {
                      f = 0;
                    }
                  f = 1 - f;
                  f = 300 * f;
                  if (f > 255)
                    {
                      f = 255;
                    }
                  ptr3[adr] = (int) f;
                }
            }
          for (n2 = 0; n2 < window_height; n2++)
            {
              for (n1 = 0; n1 < window_width; n1++)
                {
                  adr = 4 * (n1 + n2 * window_width);
                  r = ptr2[adr + 0] & 255;
                  g = ptr2[adr + 1] & 255;
                  b = ptr2[adr + 2] & 255;
                  h00 =
                    r * page->OPS[n].rgb.r + g * page->OPS[n].rgb.g +
                    b * page->OPS[n].rgb.b;

                  if (n1 < window_width - 1)
                    {
                      r = ptr2[adr + 4 + 0] & 255;
                      g = ptr2[adr + 4 + 1] & 255;
                      b = ptr2[adr + 4 + 2] & 255;
                    }
                  h10 =
                    r * page->OPS[n].rgb.r + g * page->OPS[n].rgb.g +
                    b * page->OPS[n].rgb.b;
                  if (n2 < window_height - 1)
                    {
                      r = ptr2[adr + 4 * window_width + 0] & 255;
                      g = ptr2[adr + 4 * window_width + 1] & 255;
                      b = ptr2[adr + 4 * window_width + 2] & 255;
                    }
                  h01 =
                    r * page->OPS[n].rgb.r + g * page->OPS[n].rgb.g +
                    b * page->OPS[n].rgb.b;
                  r = ptr2[adr + 0] & 255;
                  g = ptr2[adr + 1] & 255;
                  b = ptr2[adr + 2] & 255;
                  x = n1 + (h10 - h00) + window_width / 2;
                  y = n2 + (h01 - h00) + window_height / 2;
                  if (x < 0)
                    {
                      x = 0;
                    }
                  if (y < 0)
                    {
                      y = 0;
                    }
                  if (x >= window_width * 2)
                    {
                      x = window_width * 2 - 1;
                    }
                  if (y >= window_height * 2)
                    {
                      y = window_height * 2 - 1;
                    }
                  nn = ptr3[x + y * (window_width * 2)] & 255;
                  f = (float) nn / 255;
                  r = (Sint32) (2 * r * f);
                  g = (Sint32) (2 * g * f);
                  b = (Sint32) (2 * b * f);
                  if (r > 255)
                    {
                      r = 255;
                    }
                  if (g > 255)
                    {
                      g = 255;
                    }
                  if (b > 255)
                    {
                      b = 255;
                    }
                  if (r < 0)
                    {
                      r = 0;
                    }
                  if (g < 0)
                    {
                      g = 0;
                    }
                  if (b < 0)
                    {
                      b = 0;
                    }
                  ptr[adr + 0] = r;
                  ptr[adr + 1] = g;
                  ptr[adr + 2] = b;
                }
            }
          free_memory (ptr3);
          free_memory (ptr2);
        }
    }

  /* convert */
  for (x = 0; x < window_width; x++)
    for (y = 0; y < window_height; y++)
      {
        unsigned char c = ptr[4 * (x + y * window_width) + 0];
        ptr[4 * (x + y * window_width) + 0] =
          ptr[4 * (x + y * window_width) + 2];
        ptr[4 * (x + y * window_width) + 2] = c;
        ptr[4 * (x + y * window_width) + 3] = 255;
      }
  return Surface;
}


/**
 * Free memory allocated for the script
 * @param page a pointer to a page structure
 */
void
script_free (Page * page)
{
  Uint32 i;
  Sint32 n;
  if (page == NULL)
    {
      return;
    }
  for (i = 0; i < 256; i++)
    {
      if (page->OPS[i].str != NULL)
        {
          free_memory ((char *) page->OPS[i].str);
          page->OPS[i].str = NULL;
        }
    }
  /* free images */
  for (n = 0; n < page->nBITMAPS; n++)
    {
      if (page->BITMAPS[n].Surface != NULL)
        {
          SDL_FreeSurface (page->BITMAPS[n].Surface);
          page->BITMAPS[n].Surface = NULL;
          page->BITMAPS[n].ptr = NULL;
        }
      if (page->BITMAPS[n].ptr != NULL)
        {
          free_memory ((char *) page->BITMAPS[n].ptr);
          page->BITMAPS[n].ptr = NULL;
        }
    }

  /* remove fontes */
  for (n = 0; n < page->nFONTES; n++)
    {
      if (page->FONTES[n].font != NULL)
        {
          TTF_CloseFont (page->FONTES[n].font);
          page->FONTES[n].font = NULL;
        }
    }
  free_memory ((char *) page);
}

/**
 * Return counter position if exists
 * @param page Pointer to a 'Page' structure
 * @param x 
 * @param y
 * @returni TRUE
 */
bool
script_get_counter_pos (Page * page, Sint32 * x, Sint32 * y)
{
  Sint32 n;
  for (n = 0; n < page->nOPS; n++)
    {
      if (page->OPS[n].op == COMPTEUR)
        {
          *x = page->OPS[n].x;
          *y = page->OPS[n].y;
          return TRUE;
        }
    }
  return FALSE;
}

/**
 * Modify  number of times the game has been launched
 * in OPS->amp for op NBLAUNCH 
 * to call before script_generate_page() function
 */
void
script_set_number_of_launch (Page * page, Sint32 count)
{
  Sint32 n;
  for (n = 0; n < page->nOPS; n++)
    {
      if (page->OPS[n].op == NBLAUNCH)
        {

          page->OPS[n].amp = count;
        }
    }
}


#endif
