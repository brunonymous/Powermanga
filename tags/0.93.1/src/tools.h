/**
 * @file tools.h
 * @brief handle memory allocation and file access
 * @created 1999-08-17
 * @date 2012-08-26 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: tools.h,v 1.33 2012/08/26 15:44:26 gurumeditation Exp $
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
#ifndef __TOOLS__
#define __TOOLS__

#ifdef __cplusplus
extern "C"
{
#endif

#define TRUE                   1
#define FALSE                  0

  /** Data structure of a graphic image */
  typedef struct
  {
    /** Image width */
    Uint32 width;
    /** Image height */
    Uint32 height;
    /** Number of bits per pixels */
    Uint32 depth;
    /** Size of the bitmap graphic in bytes */
    Uint32 size;
    /** Pointer to the image data */
    char *pixel;
    /** Palette of colors */
    unsigned char palette[768];
  } bitmap_desc;

#if defined (USE_MALLOC_WRAPPER)
  bool memory_init (Uint32 numofzones);
#endif
  char *memory_allocation (Uint32 size);
  void free_memory (char *addr);
#if defined (USE_MALLOC_WRAPPER)
  void memory_releases_all (void);
#endif
  bitmap_desc *load_pcx (const char *);
  Sint16 little_endian_to_short (Sint16 * addr);
  Sint32 little_endian_to_int (Sint32 * addr);
  void int_to_little_endian (Sint32 value, Sint32 * addr);
  void convert32bits_2bigendian (unsigned char *memory);
  void integer_to_ascii (Sint32 value, Uint32 padding, char *str);
#if defined(_WIN32_WCE)
  void free_wince_module_pathname (void);
#endif
  char *locate_data_file (const char *const name);
  char *load_file (const char *const filename);
  char *loadfile_with_lang (const char *const filename, Uint32 * const fsize);
  char *loadfile_num (const char *const filename, Sint32 num);
  char *loadfile (const char *const filename, Uint32 * const size);
  size_t get_file_size (FILE * fstream);
  char *load_absolute_file (const char *const filename, Uint32 * const fsize);
  bool loadfile_into_buffer (const char *const filename, char *const buffer);
  bool file_write (const char *filename, const char *filedata,
                   const size_t filesize);
  void fps_init (void);
  void fps_print (void);
  Sint32 wait_next_frame (Sint32 delay, Sint32 max);
  Sint32 get_time_difference (void);
  Sint16 sign (float);
  float calc_target_angle (Sint16 pxs, Sint16 pys, Sint16 pxd, Sint16 pyd);
  float get_new_angle (float old_angle, float new_angle, float agilite);
  bool create_dir (const char *dirname);
#if defined(_WIN32_WCE)
  void display_windows_ce_infos (void);
#endif
  char *string_duplicate (register const char *str);
  bool alloc_precalulate_sinus (void);
  void free_precalulate_sinus (void);
  FILE *fopen_data (const char *fname, const char *fmode);
  extern float *precalc_sin;
  extern float *precalc_cos;
  extern float *precalc_sin128;
  extern float *precalc_cos128;

  extern float depix[13][32];
  extern float depiy[13][32];
#if defined (USE_MALLOC_WRAPPER)
  extern Uint32 mem_numof_zones;
  extern Uint32 mem_total_size;
#endif

#ifdef __cplusplus
}
#endif
#endif
