/** 
 * @file scrolltext.c
 * @brief Handle scroll-text of the main menu 
 * @date 2012-08-25
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: scrolltext.c,v 1.24 2012/08/25 13:58:37 gurumeditation Exp $
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
#include "images.h"
#include "display.h"
#include "electrical_shock.h"
#include "gfx_wrapper.h"
#include "log_recorder.h"
#include "scrolltext.h"
#include "sprites_string.h"

/** Maximum number of chars on the screen */
#define SCROLLTEXT_MAXOF_CHARS 400
static const Sint32 NUMOF_FRAMES_BEFORE_NEW_CHAR = 16;
static const Sint32 SCROLLTEXT_EMPTY_LENGTH = 67;
bitmap fnt_scroll[FONT_SCROLLTEXT_MAXOF_GLYPHS];

/* pointer to strings of the scrolltext */
static char *scrolltext_menu = NULL;
static char *scrolltext_current = NULL;
static char *scrolltext_emtpy = NULL;

static bool scrolltext_direction_x = FALSE;
static bool scrolltext_direction_y = FALSE;
static bool scrolltext_enable = FALSE;
static Sint32 scrolltext_length = 0;
static Sint32 scrolltext_str_index = 0;
static Sint32 scrolltext_frame_count = 0;
static Sint32 scrolltext_numof_chars = 0;
static Sint32 scrolltext_coord_x = 0;
static Sint32 scrolltext_coord_y = 0;
static Uint32 scrolltext_angle = 0;
static Sint16 scrolltext_type = 0;

/** Fonte element structure */
typedef struct fntscroll
{
  /** Sprite structure */
  sprite spr;
  /** Image number */
  Sint16 num_image;
  /** Type of scrolltext */
  Sint16 typ_of_scroll;
  /** TRUE if flicker the sprite */
  bool blink;
  /** TRUE if x displacement */
  bool dir_x;
  /** TRUE if y displacement */
  bool dir_y;
  Sint32 coor_x;
  Sint32 coor_y;
  Sint32 speed;
  struct fntscroll *previous;
  struct fntscroll *next;
  bool is_enabled;

} fntscroll;

/** Table of all chars elements */
static fntscroll *scrolltext_chars = NULL;
static fntscroll *schar_first = NULL;
static fntscroll *schar_last = NULL;
/** Index of table of fonts elements */
static void scrolltext_del_schar (fntscroll * schar);
static fntscroll *scrolltext_get_schar (void);
static void scrolltext_get_char (void);
static void scrolltext_move_and_display (void);

/** 
 * Load text data, font bitmap and clear list and structure chars 
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
scrolltext_once_init (void)
{
  Sint32 i;
  fntscroll *schar;

  scrolltext_free ();

  if (!scrolltext_load ())
    {
      return FALSE;
    }

  /* extract scrolltext font bitmap (12,143 bytes) */
  if (!bitmap_load
      ("graphics/bitmap/fonts/font_scrolltext.spr", &fnt_scroll[0], 1,
       FONT_SCROLLTEXT_MAXOF_GLYPHS))
    {
      return FALSE;
    }

  /* allocate empty string */
  if (scrolltext_emtpy == NULL)
    {
      scrolltext_emtpy = memory_allocation (SCROLLTEXT_EMPTY_LENGTH + 1);
      if (scrolltext_emtpy == NULL)
        {
          LOG_ERR ("scrolltext_emtpy out of memory");
          return FALSE;
        }
      for (i = 0; i < SCROLLTEXT_EMPTY_LENGTH; i++)
        {
          scrolltext_emtpy[i] = ' ';
        }
      scrolltext_emtpy[i] = '\0';
    }

  /* allocate chars elements */
  if (scrolltext_chars == NULL)
    {
      scrolltext_chars =
        (fntscroll *) memory_allocation (SCROLLTEXT_MAXOF_CHARS *
                                         sizeof (fntscroll));
      if (scrolltext_chars == NULL)
        {
          LOG_ERR ("scrolltext_chars out of memory");
          return FALSE;
        }
    }

  /* clear list and chars elements */
  for (i = 0; i < SCROLLTEXT_MAXOF_CHARS; i++)
    {
      schar = &scrolltext_chars[i];
      schar->is_enabled = FALSE;
    }
  scrolltext_numof_chars = 0;
  scrolltext_init ();
  return TRUE;
}

/**
 * Convert  from data bitmaps to PNG file
 * @return TRUE if successful
 */
#ifdef PNG_EXPORT_ENABLE
bool
scrolltext_extract (void)
{
  Uint32 i;
  const char *model = EXPORT_DIR "/scrolltext/char-xx.png";
  char *filename = memory_allocation (strlen (model) + 1);
  if (filename == NULL)
    {
      LOG_ERR ("not enough memory to allocate %i bytes\n",
               (Uint32) (strlen (model) + 1));
      return FALSE;
    }
  if (!create_dir (EXPORT_DIR "/scrolltext"))
    {
      free_memory (filename);
      return FALSE;
    }
  for (i = 0; i < FONT_SCROLLTEXT_MAXOF_GLYPHS; i++)
    {
      sprintf (filename, EXPORT_DIR "/scrolltext/char-%02d.png", i);
      if (!bitmap_to_png (&fnt_scroll[i], filename, 20, 21, offscreen_pitch))
        {
          free_memory (filename);
          return FALSE;
        }
    }
  free_memory (filename);
  return TRUE;
}
#endif



/** 
 * Load scrolltext ascii data
 * @return TRUE if it completed successfully or FALSE otherwise
 */
bool
scrolltext_load (void)
{
  Uint32 filesize, i, j;
  char *filedata;
  filedata = loadfile_with_lang ("texts/scroll_%s.txt", &filesize);
  if (filedata == NULL)
    {
      return FALSE;
    }

  scrolltext_menu = memory_allocation (filesize);
  if (scrolltext_menu == NULL)
    {
      free_memory (filedata);
      return FALSE;
    }

  j = 0;
  for (i = 0; i < filesize; i++)
    {
      if (filedata[i] >= ' ' && filedata[i] != '"')
        {
          scrolltext_menu[j++] = filedata[i];
        }
    }
  free_memory (filedata);
  return TRUE;
}

/** 
 * Release scrolltext data
 */
void
scrolltext_free (void)
{
  bitmap_free (&fnt_scroll[0], 1, FONT_SCROLLTEXT_MAXOF_GLYPHS, 16);
  if (scrolltext_menu != NULL)
    {
      free_memory (scrolltext_menu);
      scrolltext_menu = NULL;
    }
  if (scrolltext_chars != NULL)
    {
      free_memory ((char *) scrolltext_chars);
      scrolltext_chars = NULL;
    }
  if (scrolltext_emtpy != NULL)
    {
      free_memory (scrolltext_emtpy);
      scrolltext_emtpy = NULL;
    }
}

/** 
 * Initialize the scrolltext
 */
void
scrolltext_init (void)
{
  scrolltext_enable = TRUE;
  scrolltext_current = scrolltext_menu;
  scrolltext_length = (Sint32) strlen (scrolltext_current);
  scrolltext_str_index = 0;
  /* frame counter, determine the appearance of the next character */
  scrolltext_frame_count = 0;
  scrolltext_coord_x = offscreen_startx + offscreen_width_visible;
#ifdef UNDER_DEVELOPMENT
  scrolltext_coord_y = 271;
#else
  scrolltext_coord_y = 291;
#endif
  /* type of scrolltext to use */
  scrolltext_type = SCROLL_PRESENT;
  /* enable the horizontal displacement of the chars */
  scrolltext_direction_x = TRUE;
  /* disable the vertical displacement of the chars */
  scrolltext_direction_y = 0;
  schar_first = NULL;
  schar_last = NULL;
}

/** 
 * Disable the scrolltext
 * @param scroll_2_destruct
 */
void
scrolltext_disable (Sint32 scroll_2_destruct)
{
  fntscroll *schar;
  Sint32 i;
  scrolltext_current = scrolltext_emtpy;
  scrolltext_length = (Sint32) strlen (scrolltext_current);
  scrolltext_str_index = 0;
  schar = schar_first;
  if (schar == NULL)
    {
      return;
    }
  /* process each fonte sprite */
  for (i = 0; i < scrolltext_numof_chars; i++, schar = schar->next)
    {
#ifdef UNDER_DEVELOPMENT
      if (schar == NULL && i < (scrolltext_numof_chars - 1))
        {
          LOG_ERR ("schar->next is null %i/%i", i, scrolltext_numof_chars);
          break;
        }
#endif
      switch (scroll_2_destruct)
        {
        case SCROLL_PRESENT:
          if (schar->typ_of_scroll == SCROLL_PRESENT)
            {
              schar->speed = 2;
              schar->dir_x = 0;
              schar->dir_y = 1;
            }
          break;
        }
    }
}


/**
 * Check validty of scrolltext.chained list
 */
#ifdef UNDER_DEVELOPMENT
static void
schar_check_chained_list (void)
{
  Uint32 i;
  fntscroll *schar;
  Sint32 count = 0;
  for (i = 0; i < SCROLLTEXT_MAXOF_CHARS; i++)
    {
      schar = &scrolltext_chars[i];
      if (schar->is_enabled)
        {
          count++;
        }
    }
  if (count != scrolltext_numof_chars)
    {
      LOG_ERR ("Counting of the enabled elements failed!"
               "count=%i, =%i", count, scrolltext_numof_chars);
    }
  count = 0;
  schar = schar_first;
  do
    {
      count++;
      schar = schar->next;
    }
  while (schar != NULL && count <= (SCROLLTEXT_MAXOF_CHARS + 1));
  if (count != scrolltext_numof_chars)
    {
      LOG_ERR ("Counting of the next elements failed!"
               "count=%i, scrolltext_numof_chars=%i", count,
               scrolltext_numof_chars);
    }
  count = 0;
  schar = schar_last;
  do
    {
      count++;
      schar = schar->previous;
    }
  while (schar != NULL && count <= (SCROLLTEXT_MAXOF_CHARS + 1));
  if (count != scrolltext_numof_chars)
    {
      LOG_ERR ("Counting of the previous elements failed!"
               "count=%i, scrolltext_numof_chars=%i", count,
               scrolltext_numof_chars);
    }
}
#endif

/** 
 * Return a free scroll char element 
 * @return Pointer to a scroll char structure, NULL if not char available 
 */
static fntscroll *
scrolltext_get_schar (void)
{
  Uint32 i;
  fntscroll *schar;
  for (i = 0; i < SCROLLTEXT_MAXOF_CHARS; i++)
    {
      schar = &scrolltext_chars[i];
      if (schar->is_enabled)
        {
          continue;
        }
      schar->is_enabled = TRUE;
      schar->next = NULL;
      if (scrolltext_numof_chars == 0)
        {
          schar_first = schar;
          schar_last = schar;
          schar_last->previous = NULL;
        }
      else
        {
          schar_last->next = schar;
          schar->previous = schar_last;
          schar_last = schar;
        }
      scrolltext_numof_chars++;
#ifdef UNDER_DEVELOPMENT
      schar_check_chained_list ();
#endif
      return schar;
    }
  LOG_ERR ("no more element char is available");
  return NULL;
}

/** 
 * Remove a char element from list
 * @param Pointer to a scroll char structure 
 */
void
scrolltext_del_schar (fntscroll * schar)
{
  schar->is_enabled = FALSE;
  scrolltext_numof_chars--;
  if (schar_first == schar)
    {
      schar_first = schar->next;
    }
  if (schar_last == schar)
    {
      schar_last = schar->previous;
    }
  if (schar->previous != NULL)
    {
      schar->previous->next = schar->next;
    }
  if (schar->next != NULL)
    {
      schar->next->previous = schar->previous;
    }
}

/** 
 * Move and display the scrolltext 
 */
void
scrolltext_handle (void)
{
  scrolltext_get_char ();
  scrolltext_move_and_display ();
}

/** 
 * Get a new char from string 
 */
void
scrolltext_get_char (void)
{
  unsigned char val_fonte;
  fntscroll *schar;
  if (!scrolltext_enable)
    {
      return;
    }

  /* test if read new char */
  if (scrolltext_frame_count < NUMOF_FRAMES_BEFORE_NEW_CHAR)
    {
      scrolltext_frame_count++;
      return;
    }
  scrolltext_frame_count = 1;

  /* read character code */
  val_fonte = (unsigned char) (scrolltext_current[scrolltext_str_index] - 33);
  scrolltext_str_index++;
  if (scrolltext_str_index >= scrolltext_length)
    {
      scrolltext_str_index = 0;
    }

  /* too many fontes on the screen? */
  if (scrolltext_numof_chars >= SCROLLTEXT_MAXOF_CHARS)
    {
      return;
    }

  /* character supported? */
  if (val_fonte >= FONT_SCROLLTEXT_MAXOF_GLYPHS)
    {
      return;
    }

  schar = scrolltext_get_schar ();
  if (schar == NULL)
    {
      return;
    }
  /* fonte don't blink */
  schar->blink = FALSE;
  /* 2 = sprite trajectory follow a curve */
  schar->spr.trajectory = 2;
  /* set number of images of the sprite */
  schar->spr.numof_images = 1;
  /* set x and y coordinates */
  schar->coor_x = scrolltext_coord_x;
  schar->coor_y = scrolltext_coord_y;
  /* set speed of the displacement */
  schar->speed = -1;
  schar->num_image = val_fonte;
  schar->typ_of_scroll = scrolltext_type;
  /* set displacement of the fontes */
  schar->dir_x = scrolltext_direction_x;
  schar->dir_y = scrolltext_direction_y;
}

/** 
 * Move and display the fontes
 */
void
scrolltext_move_and_display (void)
{
  Sint32 i;
  Uint32 angle, ycoord;
  float sin_value;
  fntscroll *schar;
  schar = schar_first;
  if (schar == NULL)
    {
      return;
    }
  scrolltext_angle = (scrolltext_angle + 3) & 127;
  angle = scrolltext_angle;


  /* process each fonte sprite */
  for (i = 0; i < scrolltext_numof_chars; i++, schar = schar->next)
    {
      angle = (angle + 2) & 127;
#ifdef UNDER_DEVELOPMENT
      if (schar == NULL && i < (scrolltext_numof_chars - 1))
        {
          LOG_ERR ("schar->next is null %i/%i", i, scrolltext_numof_chars);
          break;
        }
#endif
      /* horizontal displacement */
      if (schar->dir_x)
        {
          schar->coor_x += schar->speed;
        }

      /* vertical displacement */
      if (schar->dir_y)
        {
          schar->coor_y += schar->speed;
        }

      /* disable char sprite if not visible */
      if ((Sint16) (schar->coor_x + 32) < offscreen_startx
          || (Sint16) (schar->coor_y + 32) < offscreen_starty
          || (Sint16) schar->coor_y >
          offscreen_starty + offscreen_height_visible
          || (Sint16) schar->coor_x >
          offscreen_startx + offscreen_width_visible)
        {
          scrolltext_del_schar (schar);
          continue;
        }

      sin_value = precalc_sin128[angle];
#ifdef UNDER_DEVELOPMENT
      ycoord = (Uint32) (schar->coor_y + sin_value * 24);
#else
      ycoord = (Uint32) (schar->coor_y + sin_value * 8);
#endif

      /* manage the flickering of the sprite */
      if (schar->blink)
        {
          if (++schar->spr.anim_count <= 1)
            {
              continue;
            }
          schar->spr.anim_count = 0;
        }
      draw_bitmap (&fnt_scroll[schar->num_image], schar->coor_x, ycoord);
    }
}
