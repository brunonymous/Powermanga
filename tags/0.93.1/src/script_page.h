/**
 * @file script_page.h
 * @brief Handle the displaying of the pages order in Windows shareware
 *        version 
 * @created 2007-04-17
 * @date 2012-08-26 
 * @author Patrice Duhamel 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: script_page.h,v 1.9 2012/08/26 19:22:39 gurumeditation Exp $
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
#ifndef __SCRIPT_PAGE_ORDER__
#define __SCRIPT_PAGE_ORDER__

#ifdef __cplusplus
extern "C"
{
#endif

#include "powermanga.h"
#include "config_file.h"
#ifdef SHAREWARE_VERSION
/* compile function to save script if necessary */
/* #define USE_WRITE_SCRIPT */
#define MOSAIQUE 1
#define TEXTE 2
#define	BITMAP 3
#define	ETIRER 4
#define CARREDEGRADE 5
#define BITMAPFADE 7
#define ONDES 8
#define ONDES2 9
#define EMBOSS 10
#define COMPTEUR 100
#define NBLAUNCH 101
  struct RGB
  {
    Sint32 r, g, b;
  };
  struct Fonte
  {
    char strfile[256];
    char name[256];
    TTF_Font *font;
    Sint32 size;
    Sint32 style;
    Sint32 OMBRAGE, Ombrex, Ombrey;
    Sint32 OUTLINE;
    Sint32 DEGRADE;
    struct RGB DEGRADE_HAUT;
    struct RGB DEGRADE_BAS;
  };
  struct Image
  {
    char strfile[256];
    char name[256];
    Sint32 CLOUDS;
    Sint32 cloud[5];
    Sint32 x, y;
    char *ptr;
    SDL_Surface *Surface;
    Sint32 OMBRAGE, Ombrex, Ombrey;
    Sint32 OUTLINE;
  };
  struct OP
  {
    Sint32 op;
    Sint32 x, y;
    char name[512];
    char *str;
    Sint32 lx, ly;
    struct RGB rgb;
    Sint32 x1, y1, x2, y2;
    Sint32 amp, rayon;
  };
  typedef struct Page
  {
    Sint32 nBITMAPS;
    struct Image BITMAPS[256];
    Sint32 nFONTES;
    struct Fonte FONTES[256];
    Sint32 nOPS;
    struct OP OPS[256];
  } Page;
  struct Page *script_read_file (char *fname);
  void script_free (Page * page);
  SDL_Surface *script_generate_page (struct Page *page);
  void script_initialize (Sint32 width, Sint32 height, char *dirname);
  void script_set_dithering (void);
  bool script_get_counter_pos (Page * page, Sint32 * x, Sint32 * y);
  void script_set_number_of_launch (Page * page, Sint32 count);
#endif
#ifdef __cplusplus
}
#endif
#endif
