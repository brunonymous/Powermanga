/**
 * @file images.h 
 * @brief Extract *.spr file into 'image' and 'bitmap' structures 
 * @created 2006-12-13
 * @date 2011-02-26 
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: images.h,v 1.25 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __IMAGES__
#define __IMAGES__

#ifdef __cplusplus
extern "C"
{
#endif

/** Max. origins of shots (location of the cannons) per sprite */
#define MAX_OF_CANNONS 12
/** Max number of points of collision */
#define MAX_OF_COLLISION_POINTS 12
/** Max number of zones of collision  */
#define MAX_OF_COLLISION_ZONES 6
/** Max number of imgaes per sprite */
#define IMAGES_MAXOF 40

#define EXPORT_DIR "sprites"

  typedef struct _compress
  {
    Uint32 offset;
    Uint16 r1;
    Uint16 r2;
  }
  _compress;

  /** Width and height of collision zone */
  typedef enum
  {
    IMAGE_WIDTH,
    IMAGE_HEIGHT
  } IMAGE_ENUM;

  /** Bitmap structure used for fontes, TLK's logo, main menu text, 
   * scrolltext, and right options box */
  typedef struct bitmap
  {
    /** Number of pixels */
    Sint32 numof_pixels;
    /** Pixels buffer */
    char *img;
    /** Size of offsets buffer */
    Sint32 nbr_data_comp;
    /** Offsets and repeat values buffer */
    char *compress;
  } bitmap;

  /** Image structure used for all sprites */
  typedef struct image
  {
    /** X-center of gravity of the image */
    Sint16 x_gc;
    /** Y-center of gravity of the image */
    Sint16 y_gc;
    /** Image width */
    Sint16 w;
    /** Image height */
    Sint16 h;
    /** Number of collision coordinates  */
    Sint16 numof_collisions_points;
    /** List of the collision coordinates */
    Sint16 collisions_points[MAX_OF_COLLISION_POINTS][2];
    /** Number of collision areas */
    Sint16 numof_collisions_zones;
    /** List of the collision zones coordinates */
    Sint16 collisions_coords[MAX_OF_COLLISION_ZONES][2];
    /** List of the collision zones sizes (width/height) */
    Sint16 collisions_sizes[MAX_OF_COLLISION_ZONES][2];
    /** Number of cannons coordinates */
    Sint16 numof_cannons;
    /** Lists of the cannons coordinates */
    Sint16 cannons_coords[MAX_OF_CANNONS][2];
    /** Lists of the shots angles of the canons */
    Sint16 cannons_angles[MAX_OF_CANNONS];
    /** Nmber of pixels of the image */
    Sint32 numof_pixels;
    /** Pixel data */
    char *img;
    /** Size of offsets and repeats values table */
    Sint32 nbr_data_comp;
    /** Offsets and repeat values table */
    char *compress;
  }
  image;

  typedef struct sprite
  {
    /** 0=friend sprite / 1=ennemy sprite */
    Uint32 type;
    /** Trajectory: 0=linear/1=compute/2=curve */
    Sint16 trajectory;
    /** Power of destruction (if collision) */
    Sint16 pow_of_dest;
    /** Energy level  (<= 0 destroyed sprite) */
    Sint16 energy_level;
    /** Initial energy level */
    Sint16 max_energy_level;
    /** Number of images of the sprite */
    Sint16 numof_images;
    /** Current image index */
    Sint16 current_image;
    /** Time delay before next image (animation speed) */
    Sint16 anim_speed;
    /** Time delay counter before ne image */
    Sint16 anim_count;
    /** Images data, points of collision, and
     * location of the cannons of the sprite,*/
    image *img[IMAGES_MAXOF];
    /** X coordinate */
    float xcoord;
    /** Y coordinate */
    float ycoord;
    /** Speed of the sprite */
    float speed;
  } sprite;
  bool image_load (const char *fname, image * img, Uint32 num_of_sprites,
                   Uint32 num_of_images);
  bool image_load_num (const char *fname, Sint32 num, image * img,
                       Uint32 num_of_sprites, Uint32 num_of_anims);
  bool image_load_single (const char *fname, image * img);
  bool bitmap_load (const char *fname, bitmap * fonte, Uint32 num_of_obj,
                    Uint32 num_of_images);
  char *images_read (image * img, Uint32 num_of_sprites, Uint32 num_of_images,
                     char *addr, Uint32 max_of_anims);
  void images_free (image * first_image, Uint32 num_of_sprites,
                    Uint32 num_of_anims, Uint32 max_of_anims);
  void bitmap_free (bitmap * first_bitmap, Uint32 num_of_bitmap,
                    Uint32 num_of_anims, Uint32 max_of_anims);
#ifdef PNG_EXPORT_ENABLE
  bool image_to_png (image * img, const char *filename);
  bool bitmap_to_png (bitmap * bmp, const char *filename, Uint32 width,
                      Uint32 height, Uint32 size_of_line);
#endif
#ifdef __cplusplus
}
#endif
#endif
