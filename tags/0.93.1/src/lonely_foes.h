/**
 * @file lonely_foes.h 
 * @brief Handle lonely foes (come out randomly as penality) 
 * @created 2006-12-10 
 * @date 2009-03-07
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/*
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: lonely_foes.h,v 1.12 2012/06/03 17:06:15 gurumeditation Exp $
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
#ifndef __LONELY_FOES__
#define __LONELY_FOES__
#ifdef __cplusplus
extern "C"
{
#endif
  typedef enum
  {
    LONELY_SUBJUGANEERS,
    LONELY_MILLOUZ,
    LONELY_SWORDINIANS,
    LONELY_TOUBOUG,
    LONELY_DISGOOSTEES,
    LONELY_EARTHINIANS,
    LONELY_BIRIANSTEES,
    LONELY_BELCHOUTIES,
    LONELY_VIONIEES,
    LONELY_HOCKYS,
    LONELY_TODHAIRIES,
    LONELY_DEFECTINIANS,
    LONELY_BLAVIRTHE,
    LONELY_SOONIEES,
    LONELY_ANGOUFF,
    LONELY_GAFFIES,
    LONELY_BITTERIANS,
    LONELY_BLEUERCKS,
    LONELY_ARCHINIANS,
    LONELY_CLOWNIES,
    LONELY_DEMONIANS,
    LONELY_TOUTIES,
    LONELY_FIDGETINIANS,
    LONELY_EFFIES,
    LONELY_DIMITINIANS,
    LONELY_PAINIANS,
    LONELY_ENSLAVEERS,
    LONELY_FEABILIANS,
    LONELY_DIVERTIZERS,
    LONELY_SAPOUCH,
    LONELY_HORRIBIANS,
    LONELY_CARRYONIANS,
    LONELY_DEVILIANS,
    LONELY_ROUGHLEERS,
    LONELY_ABASCUSIANS,
    LONELY_ROTIES,
    LONELY_STENCHIES,
    LONELY_PERTURBIANS,
    LONELY_MADIRIANS,
    LONELY_BAINIES
  } LONELY_FOES;
  void lonely_foes_init (void);
  void lonely_foe_add_gozuky (enemy * foe, Uint32 cannon_pos);
  void lonely_foe_add (Sint32 foe_num);
#ifdef __cplusplus
}
#endif
#endif
