/**
 * @file sdl_mixer.c
 * @brief Handle sounds and musics
 * @created 2003-06-27
 * @date 2012-08-26 
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: sdl_mixer.c,v 1.24 2012/08/26 19:22:39 gurumeditation Exp $
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
#include "display.h"
#include "log_recorder.h"
#include "menu.h"
#include "menu_sections.h"
#ifdef USE_SDLMIXER
#include "sdl_mixer.h"

static Mix_Music *music_chunk = NULL;
/** List of flags of requested sounds */
bool sounds_play[SOUND_NUMOF];
static bool music_enabled = TRUE;
/** Module number loaded in memory */
static Sint32 module_num_loaded = -1;
/** Module number requested */
static Sint32 module_num_selected = 0;
/** If TRUE enable or disable the music */
static bool start_stop_music = FALSE;
static const Uint32 VOLUME_INC = MIX_MAX_VOLUME / 16;
static Uint32 music_volume_selected = 0;
static Uint32 music_volume = 0;
/** Size in bytes of the waves samples */
Uint32 sound_samples_len = 0;
static bool volume_down_pressed = FALSE;
static bool volume_up_pressed = FALSE;


/** Filenames of the waves sounds */
static const char *sounds_filenames[SOUND_NUMOF] = {
  "sounds/sound_upgrade_spaceship.wav",
  "sounds/sound_downgrade_spaceship.wav",
  "sounds/sound_select_option.wav",
  "sounds/sound_select_closed_option.wav",
  "sounds/sound_purple_gem.wav",
  "sounds/sound_yellow_gem.wav",
  "sounds/sound_green_gem.wav",
  "sounds/sound_red_gem.wav",
  "sounds/sound_lonely_foe.wav",
  "sounds/sound_circular_shock.wav",
  "sounds/sound_spaceship_fire.wav",
  "sounds/sound_guardian_fire_2.wav",
  "sounds/sound_big_explosion_1.wav",
  "sounds/sound_big_explosion_2.wav",
  "sounds/sound_big_explosion_3.wav",
  "sounds/sound_big_explosion_4.wav",
  "sounds/sound_medium_explosion_1.wav",
  "sounds/sound_medium_explosion_2.wav",
  "sounds/sound_medium_explosion_3.wav",
  "sounds/sound_medium_explosion_4.wav",
  "sounds/sound_small_explosion_1.wav",
  "sounds/sound_small_explosion_2.wav",
  "sounds/sound_small_explosion_3.wav",
  "sounds/sound_small_explosion_4.wav",
  "sounds/sound_enemy_fire_1.wav",
  "sounds/sound_enemy_fire_2.wav",
  "sounds/sound_guardian_fire_1.wav",
  "sounds/sound_guardian_fire_3.wav"
};

/** Filenames of the musics modules */
#ifdef POWERMANGA_HANDHELD_CONSOLE
static const char *musics_filenames[] = {
  "sounds/handheld_console/music_menu.ogg",
  "sounds/handheld_console/music_game.ogg",
  "sounds/handheld_console/music_congratulations.ogg"
};
#else
static const char *musics_filenames[] = {
  "sounds/music_menu.zik",
  "sounds/music_game.zik",
  "sounds/music_congratulations.zik"
};
#endif

/** Internal format of the waves sounds */
static Mix_Chunk *sounds_chunck[SOUND_NUMOF];
static bool sound_load_module (Sint32 module_num);
/** 
 * First initializations of SDL_mixer and load waves sounds files 
 * @return TRUE if successful
 */
bool
sound_once_init (void)
{
  Uint32 i;
  Sint32 audio_rate, audio_buffers;
  Uint16 audio_format;
  const char *filename;
  char *pathname;
  Mix_Chunk *sample;

  /* force no sound */
  if (power_conf->nosound)
    {
      LOG_INF ("sound has been disabled");
      return TRUE;
    }
  music_volume_selected = MIX_MAX_VOLUME;
  music_volume = MIX_MAX_VOLUME;
  start_stop_music = FALSE;
  music_enabled = TRUE;
  module_num_loaded = -1;
  module_num_selected = MUSIC_INTRO;
  if (SDL_Init (SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0)
    {
      LOG_ERR ("SDL_Init() failed: %s", SDL_GetError ());
      power_conf->nosound = 1;
      return 1;
    }
#ifdef POWERMANGA_GPX2
  /* we need a reduced audio rate for the GP2X to make sure sound
   * doesn't lag */
  audio_rate = 22050;
  audio_buffers = 64;
#else
  audio_rate = 44100;
  audio_buffers = 4096;
#endif
  audio_format = AUDIO_S16;
  if (Mix_OpenAudio (audio_rate, audio_format, 2, audio_buffers))
    {
      LOG_ERR ("Mix_OpenAudio() return %s", SDL_GetError ());
      power_conf->nosound = 1;
      SDL_Quit ();
      return TRUE;
    }
  Mix_AllocateChannels (MAX_OF_CHANNELS);
  if (!sound_music_play (MUSIC_INTRO))
    {
      return FALSE;
    }

  /*
   * load waves sounds files 
   */
  sound_samples_len = 0;
  for (i = 0; i < SOUND_NUMOF; i++)
    {
      filename = sounds_filenames[i];
      pathname = locate_data_file (filename);
      if (pathname == NULL)
        {
          LOG_ERR ("error locating data \"%s\" file", filename);
          return FALSE;
        }
      sample = Mix_LoadWAV (pathname);
      if (sample == NULL)
        {
          free_memory (pathname);
          LOG_ERR ("Mix_LoadWAV() return: %s", Mix_GetError ());
          return FALSE;
        }
      free_memory (pathname);
      sounds_chunck[i] = sample;
      /* calculate the size in bytes of the waves samples */
      sound_samples_len += sample->alen;
    }
  LOG_INF ("sound has been successfully initialized");
  return TRUE;
}

static void
sound_free_current_music (void)
{
  if (module_num_loaded >= 0 && music_chunk != NULL)
    {
      LOG_INF ("Free music %i, module_num_loaded");
      Mix_HaltMusic ();
      Mix_FreeMusic (music_chunk);
      module_num_loaded = -1;
      music_chunk = NULL;
    }
}

/**
 * Load a music module
 * @param module_num Music module number
 * @return TRUE if success
 */
static bool
sound_load_module (Sint32 module_num)
{
  const char *filename;
  char *pathname;
  if (power_conf->nosound)
    {
      return TRUE;
    }
  if (!music_enabled)
    {
      return TRUE;
    }
  sound_free_current_music ();
  filename = musics_filenames[module_num];
  pathname = locate_data_file (filename);
  if (pathname == NULL)
    {
      LOG_ERR ("error locating data \"%s\" file", filename);
      return FALSE;
    }
  LOG_DBG ("try to load \"%s\" file", filename);
  music_chunk = Mix_LoadMUS (pathname);
  if (NULL == music_chunk)
    {
      LOG_ERR ("Mix_LoadMUS(%s) return: %s", pathname, SDL_GetError ());
      free_memory (pathname);
      return FALSE;
    }
  free_memory (pathname);
  LOG_DBG ("\"%s\" module has been loaded", filename);
  module_num_loaded = module_num;
  return TRUE;
}

/**
 * Load and play a music module
 * @param module_num Music module number
 * @return TRUE if success
 */
bool
sound_music_play (Sint32 module_num)
{
  if (power_conf->nosound)
    {
      return TRUE;
    }
  module_num_selected = module_num;
  if (!music_enabled)
    {
      return TRUE;
    }
  if (!sound_load_module (module_num))
    {
      return FALSE;
    }
  if (Mix_PlayMusic (music_chunk, -1) == -1)
    {
      LOG_ERR ("Mix_PlayMusic() return %s", SDL_GetError ());
    }
  return TRUE;
}

/**
 * Volume control
 */
static void
sound_volume_ctrl (void)
{
  Uint32 volume = music_volume_selected;

  /* volume up */
  if (keys_down[K_PAGEUP] && !volume_down_pressed)
    {
      music_volume_selected =
        (music_volume_selected + VOLUME_INC >
         MIX_MAX_VOLUME) ? MIX_MAX_VOLUME : music_volume_selected +
        VOLUME_INC;
    }
  volume_down_pressed = keys_down[K_PAGEUP];

  /* volume down */
  if (keys_down[K_PAGEDOWN] && !volume_up_pressed)
    {
      music_volume_selected = (music_volume_selected <= VOLUME_INC)
        ? 0 : music_volume_selected - VOLUME_INC;
    }
  volume_up_pressed = keys_down[K_PAGEDOWN];

  if (volume == music_volume_selected)
    {
      return;
    }

  /* set the volume of all channels */
  Mix_Volume (-1, music_volume_selected);
}

/**
 * Play music and sounds
 */
void
sound_handle (void)
{
  Uint32 i;
  if (power_conf->nosound)
    {
      return;
    }
  /* volume adjustment */
  sound_volume_ctrl ();

  /* current play? */
  if (!gameover_enable)
    {
      if (music_volume != music_volume_selected / 2)
        {
          music_volume = music_volume_selected / 2;
          Mix_VolumeMusic (music_volume);
        }
    }
  else
    {
      if (music_volume != music_volume_selected)
        {
          music_volume = music_volume_selected;
          Mix_VolumeMusic (music_volume);
        }
    }
  for (i = 0; i < SOUND_NUMOF; i++)
    {
      if (sounds_play[i])
        {
          if (Mix_PlayChannel (-1, sounds_chunck[i], 0) == -1)
            {
              /*
                 LOG_DBG ("Mix_PlayChannel return %s", Mix_GetError ());
               */
            }
          sounds_play[i] = FALSE;
        }
    }

  /* [CTRL] + [S] released */
  if (start_stop_music && !keys_down[K_CTRL] && !keys_down[K_S])
    {
      start_stop_music = FALSE;
      if (music_enabled)
        {
          /* disable the music, sound only! */
          music_enabled = FALSE;
          if (!sound_load_module (module_num_selected))
            {
              quit_game = TRUE;
            }
          Mix_VolumeMusic (0);
        }
      else
        {
          /* enable the music, sound and music ! */
          music_enabled = TRUE;
          if (!sound_load_module (module_num_selected))
            {
              quit_game = TRUE;
            }
          Mix_PlayMusic (music_chunk, -1);
          Mix_VolumeMusic (music_volume_selected);
        }
    }
  else
    {
      if (keys_down[K_CTRL] && keys_down[K_S])
        {
          start_stop_music = TRUE;
        }
    }
}

/**
 * Release waves sounds and close SDL_mixer
 */
void
sound_free (void)
{
  Uint32 i;
  if (power_conf->nosound)
    {
      return;
    }
  sound_free_current_music ();
  for (i = 0; i < SOUND_NUMOF; i++)
    {
      /* sound was loaded? */
      if (sounds_chunck[i] != NULL)
        {
          Mix_FreeChunk (sounds_chunck[i]);
          sounds_chunck[i] = NULL;
        }
    }
  Mix_CloseAudio ();
  SDL_Quit ();
}

/**
 * Request to play sound effect
 * @param sound_nu Sound number
 */
void
sound_play (Uint32 sound_nu)
{
  if (!gameover_enable && !player_pause && menu_section == NO_SECTION_SELECTED
      && menu_status == MENU_OFF)
    {
      sounds_play[sound_nu] = TRUE;
    }
}
#endif
