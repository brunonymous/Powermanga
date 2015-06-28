/** 
 * @file meteors_phase.h
 * @brief Handle the meteor storm 
 * @created 2006-11-28 
 * @date 2009-01-11
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: meteors_phase.h,v 1.16 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __METEORS_PHASE__
#define __METEORS_PHASE__

#ifdef __cplusplus
extern "C"
{
#endif

  bool meteors_once_init (void);
  void meteors_free (void);
  bool meteors_load (Sint32 meteor_num);
#ifdef PNG_EXPORT_ENABLE
  bool meteors_extract (void);
#endif
  void meteors_handle (void);
  bool meteors_finished (void);
  void meteors_images_free (void);

/** Number of images of the meteors */
#define METEOR_NUMOF_IMAGES 32

  extern Sint32 num_of_meteors;
  extern bool meteor_activity;

#ifdef __cplusplus
}
#endif
#endif
