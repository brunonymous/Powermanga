/**
 * @file sound.h 
 * @brief Handle sounds and musics
 * @created 2006-12-30 
 * @date 2007-08-11
 */
/* 
 * copyright (c) 1998-2006 TLK Games all rights reserved
 * $Id: sdl_mixer.h,v 1.5 2007/08/13 06:43:24 gurumeditation Exp $
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
#ifndef __SOUND__
#define __SOUND__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USE_SDLMIXER
#define MAX_OF_CHANNELS 16

  bool sound_once_init (void);
  void sound_handle (void);
  bool sound_music_play (Sint32);
  void sound_free (void);
  void sound_play (Uint32);

  /** List of musics used by the game */
  typedef enum
  {
    MUSIC_INTRO,
    MUSIC_GAME,
    MUSIC_CONGRATULATIONS
  }
  MUSIC_LIST;

  /** List of sounds used by the game */
  typedef enum
  {
    SOUND_UPGRADE_SPACESHIP,
    SOUND_DOWNGRADE_SPACESHIP,
    /* select open option in the right option panel */
    SOUND_SELECT_OPTION,
    /* select closed option in the right option panel */
    SOUND_SELECT_CLOSED_OPTION,
    SOUND_PURPLE_GEM,
    SOUND_YELLOW_GEM,
    SOUND_GREEN_GEM,
    SOUND_RED_GEM,
    SOUND_LONELY_FOE,
    /* powerful circular shock (yellow gem + energy max.) */
    SOUND_CIRCULAR_SHOCK,
    SOUND_SPACESHIP_FIRE,
    SOUND_GUARDIAN_FIRE_2,
    SOUND_BIG_EXPLOSION_1,
    SOUND_BIG_EXPLOSION_2,
    SOUND_BIG_EXPLOSION_3,
    SOUND_BIG_EXPLOSION_4,
    SOUND_MEDIUM_EXPLOSION_1,
    SOUND_MEDIUM_EXPLOSION_2,
    SOUND_MEDIUM_EXPLOSION_3,
    SOUND_MEDIUM_EXPLOSION_4,
    SOUND_SMALL_EXPLOSION_1,
    SOUND_SMALL_EXPLOSION_2,
    SOUND_SMALL_EXPLOSION_3,
    SOUND_SMALL_EXPLOSION_4,
    SOUND_ENEMY_FIRE_1,
    SOUND_ENEMY_FIRE_2,
    SOUND_GUARDIAN_FIRE_1,
    SOUND_GUARDIAN_FIRE_3,

    SOUND_NUMOF
  }
  SOUNDS_INDEXES;

  extern Uint32 sound_samples_len;
  extern bool sounds_play[SOUND_NUMOF];

#ifdef __cplusplus
}
#endif

#endif
#endif
