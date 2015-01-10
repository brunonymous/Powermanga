/** 
 * @file enemies.h 
 * @brief handle enemies 
 * @created 2006-11-19
 * @date 2009-01-11
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: enemies.h,v 1.40 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __ENEMIES__
#define __ENEMIES__

#ifdef __cplusplus
extern "C"
{
#endif

/** Maximum of enemies active at any time */
#define MAX_OF_ENEMIES 180
/** Maxinum number of types of small enemy */
#define ENEMIES_MAX_SMALL_TYPES 21
/** Maxium number of types of bloodsucker enemy */
#define ENEMIES_MAX_BLOODSUCKER_TYPES 21
/** Maximum number of types of lonely foes */
#define LONELY_FOES_MAX_OF 40
/* Number of images of bloodsucker */
#define ENEMIES_MAX_SPECIAL_TYPES 8
#define ENEMIES_MAX_OF_TYPES ENEMIES_MAX_SMALL_TYPES + ENEMIES_MAX_BLOODSUCKER_TYPES + LONELY_FOES_MAX_OF + ENEMIES_MAX_SPECIAL_TYPES
/* Number of images of bloodsucker */
#define BLOODSUCKER_NUM_OF_IMAGES 8
/* Number of animation images of a lonely foe, guardian or special ennemy */
#define ENEMIES_SPECIAL_NUM_OF_IMAGES 32

  extern Sint32 num_of_enemies;

  /** Defined the types of displacements that exist */
  typedef enum
  { DISPLACEMENT_CURVE,
    DISPLACEMENT_GRID,
    DISPLACEMENT_LONELY_FOE,
    DISPLACEMENT_GUARDIAN
  }
  DISPLACEMENTS;

  /** Type of enemies vessels */
  enum TYPE_VAIS_E
  {
    /* standards */
    BOONIES, MOOARCKS, ACHEES, EVILIANS, RAGOONS, FLABBIES, BOWANS,
    HIJACKERS, DISTASIANS, BRITTERS, SICKIES, ARIANS, STINCKIES, BZEURCKS,
    WEAKIES,
    MASKIES, SCRAPIES, SCROLLIANS, SHAMISTEES, FOOLINIANS, GLAMOOSH,
    BIGBOONIES, BIGMOOARCKS, BIGACHEES, BIGEVILIANS, BIGRAGOONS,
    BIGFLABBIES, BIGBOWANS, BIGHIJACKERS, BIGDISTASIANS, BIGBRITTERS,
    BIGSICKIES, BIGARIANS, BIGSTINCKIES, BIGBZEURCKS, BIGWEAKIES,
    BIGMASKIES, BIGSCRAPIES, BIGSCROLLIANS, BIGSHAMISTEES, BIGFOOLINIANS,
    BIGGLAMOOSH,
    /* lonely foe */
    SUBJUGANEERS, MILLOUZ, SWORDINIANS, TOUBOUG, DISGOOSTEES, EARTHINIANS,
    BIRIANSTEES, BELCHOUTIES, VIONIEES, HOCKYS, TODHAIRIES, DEFECTINIANS,
    BLAVIRTHE, SOONIEES, ANGOUFF, GAFFIES, BITTERIANS, BLEUERCKS, ARCHINIANS,
    CLOWNIES, DEMONIANS, TOUTIES, FIDGETINIANS, EFFIES, DIMITINIANS,
    PAINIANS, ENSLAVEERS, FEABILIANS, DIVERTIZERS, SAPOUCH, HORRIBIANS,
    CARRYONIANS, DEVILIANS, ROUGHLEERS, ABASCUSIANS, ROTIES, STENCHIES,
    PERTURBIANS, MADIRIANS, BAINIES,
    /* special */
    SHURIKY, NAGGYS, GOZUKY, SOUKEE, QUIBOULY, TOURNADEE, SAAKEY, SAAKAMIN,
    BIGMETEOR, NORMALMETEOR, SMALLMETEOR,
    /* guards */
    THANIKEE, BARYBOOG, PIKKIOU, NEGDEIS, FLASHY, MEECKY, TYPYBOON, MATHYDEE,
    OVYDOON, GATLEENY, NAUTEE, KAMEAMEA, SUPRALIS, GHOTTEN
  };

  /** Data structure of the images's enemy */
  extern image enemi[ENEMIES_MAX_SMALL_TYPES + ENEMIES_MAX_BLOODSUCKER_TYPES +
                     LONELY_FOES_MAX_OF +
                     ENEMIES_MAX_SPECIAL_TYPES][IMAGES_MAXOF];
  typedef struct enemy
  {
    /** Sprite structure */
    sprite spr;
    /** TRUE if ennemy is dead */
    bool dead;
    /** TRUE if enemy is still visible but inactive */
    bool visible_dead;
    /** Index of the mask color table (fade-out effect) */
    Sint16 dead_color_index;
    /** TRUE if display ennemy's white mask */
    bool is_white_mask_displayed;
    /** TRUE if display the sprite enemy */
    bool visible;
    /** Time during which vessel's invincible */
    Sint16 invincible;
    bool retournement;
    /* TRUE if enemy change of direction */
    bool change_dir;
    /** Delay value before next shot */
    Sint32 fire_rate_count;
    /** Counter of delay between two shots */
    Sint32 fire_rate;
    /** Type of displacement: 0 = curve, 1 = grid, 
     * 3 = lonely foe, 2 = guardian */
    Sint16 displacement;
    /** Curve or grid index */
    Sint16 pos_vaiss[2];
    /** Curve number used */
    Sint16 num_courbe;
    /** Type of enemy: 0 = 16x16 size, 1 = 32*32 size, 2 = lonely foes */
    Sint32 type;
    /** Anim index offset (0 = increase, 1 = decreasing) */
    Sint32 sens_anim;
    /** Horizontal speed */
    float x_speed;
    /** Vertical speed */
    float y_speed;
    /** Timelife of the metor, delay counter before destruction */
    Sint32 timelife;
    /** Angle displacement (ex. SOUKEE into guardian phase) */
    float angle_tir;
    Sint16 img_angle;
    Sint16 img_old_angle;
    /** Angle offset */
    float agilite;
    struct enemy *previous;
    struct enemy *next;
    bool is_enabled;
    Uint32 id;
  } enemy;

  bool enemies_once_init (void);
  void enemies_init (void);
#ifdef PNG_EXPORT_ENABLE
  bool enemies_extract (void);
#endif
  void enemies_kill (void);
  void enemies_free (void);
  void enemies_handle (void);
  void enemy_set_fadeout (enemy * foe);
  enemy *enemy_get (void);
  enemy *enemy_get_first (void);
  void enemy_draw (enemy * foe);
  void enemy_guns_collisions (enemy * foe);
  void enemy_satellites_collisions (enemy * foe);
  void enemy_spaceship_collision (enemy * foe);
#ifdef __cplusplus
}
#endif
#endif
