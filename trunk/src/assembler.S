/** 
 * @file assembler.S 
 * @brief Optimized routines of displaying 
 * @created 1999-08-22
 * @date 2007-09-02
 * @author Emmanuel Founaud
 * @author Etienne Sobole
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: assembler.S,v 1.8 2009/01/08 20:54:08 gurumeditation Exp $
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
.text

#------------------------------------------------------------------------------
# function "flag" determine ASM routines (no C routines)
#------------------------------------------------------------------------------
# void _type_routine_gfx(long *adresse)
.global _type_routine_gfx
.type _type_routine_gfx@function
_type_routine_gfx:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx                                              #save ebx, esi ,edi registers, x86 GCC convention
    pushl   %esi
    pushl   %edi
    movl    8(%ebp),%edi
    movl    $10, %eax
    movl    %eax, (%edi)  
    jge     bbcl2
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
   
## @method void draw_rectangle_8(char *oscreen, Sint32 xcoord,
#              Sint32 ycoord, Sint32 color, Sint32 width, Sint32 height)
# Draw an empty box
# @param oscreen Offscreen destination
# @param xcoord X-coordinate
# @param ycoord Y-coordinate
# @param color The color of the rectangle 
# @param width The width of the rectangle, in pixels
# @param height The height of the rectangle, in pixels
.global draw_rectangle_8
.type draw_rectangle_8@function
draw_rectangle_8:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx                                              #save ebx, esi ,edi registers, x86 GCC convention                            
    pushl   %esi
    pushl   %edi
    movl    8(%ebp),%edi                                      #edi=adresse destination (_pSource)
    movl    16(%ebp),%eax                                     #eax=_iY
    shll    $9,%eax                                           #_iY*512
    addl    12(%ebp),%eax                                     #_iY*512+_iX
    addl    %eax,%edi
    movl    20(%ebp),%eax                                     #eax=_iCouleur
    movl    28(%ebp),%esi                                     #esi=_iHaute
    shll    $9,%esi                                           #_iHaute*512
    addl    %edi,%esi
    movl    24(%ebp),%ecx                                     #ecx=_iLarge
    movl    %edi,%edx
    lea     -1(%edx, %ecx), %ebx
    decl    %ecx
bbcl1:
    movb    %al,(%ecx, %edi)
    movb    %al,(%ecx, %esi)
    decl    %ecx
    jge     bbcl1
    movl    28(%ebp),%ecx                                     #_iHaute
    shll    $9,%ecx
bbcl2:
    movb    %al,(%ecx, %edx)
    movb    %al,(%ecx, %ebx)
    subl    $512, %ecx
    jge     bbcl2
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(16-bit)#####################################################################
.global draw_rectangle_16
.type draw_rectangle_16@function  #8=adr / 12=x / 16=y / 20=coul / 24=large / 28=haute
draw_rectangle_16:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl   %esi
    pushl   %edi
    movl    8(%ebp),%edi                                       #edi=adresse destination (_pSource)
    movl    16(%ebp),%eax                                      #eax=_iY
    shll    $10,%eax                                           #_iY*1024
    movl    12(%ebp),%ebx                                      #ebx=_iX
    addl    %ebx,%ebx
    addl    %ebx,%eax                                          #_iY*1024+_iX
    addl    %eax,%edi
    movl    20(%ebp),%eax                                      #eax=_iCouleur
    movl    28(%ebp),%esi                                      #esi=_iHaute
    shll    $10,%esi                                           #_iHaute*1024
    addl    %edi,%esi
    movl    24(%ebp),%ecx                                      #ecx=_iLarge
    addl    %ecx,%ecx    
    movl    %edi,%edx
    lea     (%edx, %ecx), %ebx
    subl    $2, %ecx
bbbcl1:
    movw    %ax,(%ecx, %edi)
    movw    %ax,(%ecx, %esi)
    subl    $2, %ecx
    jge     bbbcl1
    movl    28(%ebp),%ecx                                      #_iHaute
    shll    $10,%ecx
bbbcl2:
    movw    %ax,(%ecx, %edx)                                   #vertical 
    movw    %ax,(%ecx, %ebx)
    subl    $512, %ecx
    jge     bbbcl2
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(24-bit)#####################################################################
.global draw_rectangle_24
.type draw_rectangle_24@function  #8=adr / 12=x / 16=y / 20=coul / 24=large / 28=haute
draw_rectangle_24:
    pushl   %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                    
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    movl    8(%ebp),%edi                                       #edi=adresse destination (_pSource)
    movl    16(%ebp),%eax                                      #eax=_iY
    imul    $1536, %eax
    movl    12(%ebp),%ebx                                      #ebx=_iX
    addl    %ebx,%eax
    addl    %ebx,%ebx
    addl    %ebx,%eax                                          #_iY*1024+_iX
    addl    %eax,%edi
    movw    20(%ebp),%ax                                       #eax=_iCouleur #0=BB 1=GG 2=RR 3=xx
    movb    22(%ebp),%cl                                       #ecx=_iCouleur
    movl    28(%ebp),%esi                                      #esi=_iHaute
    pushl   %esi
    imul    $1536, %esi
    addl    %edi,%esi
    movl    24(%ebp),%ebx                                      #ebx=_iLarge
    movl    %ebx,%ebp
    addl    %ebx,%ebp
    addl    %ebx,%ebp
    movl    %edi,%edx
    lea     (%edx, %ebp), %ebx
    subl    $3, %ebp
bbbcl124:
    movw    %ax,(%ebp, %edi)
    movb    %cl,2(%ebp, %edi)
    movw    %ax,(%ebp, %esi)
    movb    %cl,2(%ebp, %esi)
    subl    $3, %ebp
    jge     bbbcl124
    popl    %ebp                                               #_iHaute
    imul    $1536, %ebp
bbbcl224:
    movw    %ax,(%ebp, %edx)
    movb    %cl,2(%ebp, %edx)
    movw    %ax,(%ebp, %ebx)
    movb    %cl,2(%ebp, %ebx)
    subl    $1536, %ebp
    jge     bbbcl224
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

#(32-bit)#####################################################################
.global draw_rectangle_32
.type draw_rectangle_32@function  #8=adr / 12=x / 16=y / 20=coul / 24=large / 28=haute
draw_rectangle_32:
pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl   %esi
    pushl   %edi
    movl    8(%ebp),%edi                                       #edi=adresse destination (_pSource)
    movl    16(%ebp),%eax                                      #eax=_iY
    shll    $11,%eax                                           #_iY*2048
    movl    12(%ebp),%ebx                                      #ebx=_iX
    addl    %ebx,%ebx
    addl    %ebx,%ebx
    addl    %ebx,%eax                                          #_iY*1024+_iX
    addl    %eax,%edi
    movl    20(%ebp),%eax                                      #eax=_iCouleur
    movl    28(%ebp),%esi                                      #esi=_iHaute
    shll    $11,%esi                                           #_iHaute*2048
    addl    %edi,%esi
    movl    24(%ebp),%ecx                                      #ecx=_iLarge
    addl    %ecx,%ecx    
    addl    %ecx,%ecx    
    movl    %edi,%edx
    lea     (%edx, %ecx), %ebx
    subl    $4, %ecx
bbbcl132:
    movl    %eax,(%ecx, %edi)
    movl    %eax,(%ecx, %esi)
    subl    $4, %ecx
    jge     bbbcl132
    movl    28(%ebp),%ecx                                      #_iHaute
    shll    $11,%ecx
bbbcl232:
    movl    %eax,(%ecx, %edx)                                  #vertical 
    movl    %eax,(%ecx, %ebx)
    subl    $2048, %ecx
    jge     bbbcl232
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
    ret

## void draw_bitmap_char_8 (src, dest, offset_s, offset_d) 
# Draw a bitmap char
# @param src Char pixels source
# @param dest Desintation where to copy to
# @param width Width of the char in pixels
# @param height Height of the char in pixels
# @param offset_s x offset into source bitmap to copy from
# @param offset_d x offset into destination offscreen to copy to 
#(8-bit)######################################################################
.global draw_bitmap_char_8
.type draw_bitmap_char_8@function
draw_bitmap_char_8:
    pushl    %ebp
    movl     %esp, %ebp
    pushl    %ebx          #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%esi  #esi = src
    movl     12(%ebp),%edi #edi = dest
    movl     24(%ebp),%ebx #ebx = offset_s
    movl     28(%ebp),%edx #edx = offset_d
    xorl     %eax,%eax
118:
    movl     16(%ebp),%ecx #ecx=width
98:
    movb     (%esi),%al
    orl      %eax,%eax      #zero ?
    je       108f
    movb     %al,(%edi)
108:
    addl     $0x1,%esi
    addl     $0x1,%edi
    decl     %ecx
    jnz      98b
    lea      (%esi, %ebx), %esi
    lea      (%edi, %edx), %edi
    decl     20(%ebp)
    jnz      118b
    popl     %edi
    popl     %esi
    popl     %ebx
    leave
    ret
#(16-bit)#####################################################################
.global draw_bitmap_char_16
.type draw_bitmap_char_16@function
draw_bitmap_char_16:
    pushl    %ebp
    movl     %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%esi            #esi=adresse source (_pSource)
    movl     12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl     24(%ebp),%ebx            #ebx=offset source
    movl     28(%ebp),%edx            #edx=offset destination
    xorl     %eax,%eax
11:
    movl     16(%ebp),%ecx            #ecx=largeur en pixels
9:
    movw     (%esi),%ax
    orl      %eax,%eax                    #zero ?
    je       10f
    movw     %ax,(%edi)
10:
    addl     $0x2,%esi
    addl     $0x2,%edi
    decl     %ecx
    jnz      9b
    lea      (%esi, %ebx), %esi
    lea      (%edi, %edx), %edi
    decl     20(%ebp)
    jnz      11b
    popl     %edi
    popl     %esi
    popl     %ebx
    leave
    ret
#(24-bit)#####################################################################
.global draw_bitmap_char_24
.type draw_bitmap_char_24@function
draw_bitmap_char_24:
    pushl    %ebp
    movl     %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%esi            #esi=adresse source (_pSource)
    movl     12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl     24(%ebp),%ebx            #ebx=offset source
    movl     28(%ebp),%edx            #edx=offset destination
    xorl     %eax,%eax
1132:
    movl     16(%ebp),%ecx            #ecx=largeur en pixels
932:
    movl     (%esi),%eax
    orl      %eax,%eax                    #zero ?
    je       1032f
    movl     %eax,(%edi)
1032:
    addl     $0x3,%esi
    addl     $0x3,%edi
    decl     %ecx
    jnz      932b
    lea      (%esi, %ebx), %esi
    lea      (%edi, %edx), %edi
    decl     20(%ebp)
    jnz      1132b
    popl     %edi
    popl     %esi
    popl     %ebx
    leave 
    ret
#(32-bit)#####################################################################
.global draw_bitmap_char_32
.type draw_bitmap_char_32@function
draw_bitmap_char_32:
    pushl    %ebp
    movl     %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%esi            #esi=adresse source (_pSource)
    movl     12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl     24(%ebp),%ebx            #ebx=offset source
    movl     28(%ebp),%edx            #edx=offset destination
    xorl     %eax,%eax
1132:
    movl     16(%ebp),%ecx            #ecx=largeur en pixels
932:
    movl     (%esi),%eax
    orl      %eax,%eax                    #zero ?
    je       1032f
    movl     %eax,(%edi)
1032:
    addl     $0x4,%esi
    addl     $0x4,%edi
    decl     %ecx
    jnz      932b
    lea      (%esi, %ebx), %esi
    lea      (%edi, %edx), %edi
    decl     20(%ebp)
    jnz      1132b
    popl     %edi
    popl     %esi
    popl     %ebx
    leave
    ret

## @method void put_sprite_8 (char *oscreen, char* dest, char *repeats, Uint32 size)
# Draw a sprite (from a bitmap or an image structure)
# @param pixels Pointer to the pixels of the sprite 
# @param oscreen Pointer to the offscreen destination
# @param repeats Pointer to the table of offsets and repeat values 
# @param size Size of the table of offsets and repeat values
.global put_sprite_8
.type put_sprite_8, @function
put_sprite_8 :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%esi  #esi = pixels
    movl    12(%ebp),%edi #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%edx #edx=taille en pixels
    xorl    %ecx, %ecx
8:
    movl    (%ebx), %eax  #deplacement
    addl    %eax, %edi    #offset
    movw    4(%ebx),%cx   #nombre de donnee en mots 32-bit (4 octets)
    rep;movsl
    movw    6(%ebx),%cx   #nombre de donnee en octets 8-bit
    addl    $0x8,%ebx
    rep;movsb
    decl    %edx
    jnz        8b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(16-bit)######################################################################
.global put_sprite_16
.type        put_sprite_16, @function
put_sprite_16:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%esi            #esi = pixels
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%edx         #edx=taille en pixels
     xorl    %ecx, %ecx
7:
    movl    (%ebx), %eax            #deplacement
    addl    %eax, %edi                #offset
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;movsl
    movw    6(%ebx),%cx                #nombre de donnee en mots 16-bit (2 octets)
    addl    $0x8,%ebx
    rep;movsw
    decl    %edx
    jnz        7b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(24-bit)######################################################################
.global put_sprite_24
.type        put_sprite_24, @function
put_sprite_24:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%esi            #esi = pixels
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%edx         #edx=taille en pixels
     xorl    %ecx, %ecx
724:
    movl    (%ebx), %eax            #deplacement
    addl    %eax, %edi                #offset
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;movsl
    movw    6(%ebx),%cx                #nombre de donnee en mots 8-bit (1 octet)
    addl    $0x8,%ebx
    rep;movsb
    decl    %edx
    jnz        724b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(32-bit)######################################################################
.global put_sprite_32
.type        put_sprite_32, @function
put_sprite_32:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%esi            #esi = pixels
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%edx         #edx=taille en pixels
    xorl    %ecx, %ecx
8:
    movl    (%ebx), %eax            #deplacement
    addl    %eax, %edi                #offset
    xorl    %ecx, %ecx
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;movsl
    movw    6(%ebx),%cx                #nombre de donnee en octets 8-bit
    rep;movsl
    addl    $0x8,%ebx
    decl    %edx
    jnz        8b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

## @method void put_sprite_mask_8bits (Uint32 color, char *oscreen,
#               char* repeats, Uint32 size)
# Draw a sprite mask
# @param color Color of the mask
# @param oscreen Pointer to the offscreen destination
# @param repeats Pointer to the table of offsets and repeat values 
# @param size Size of the table of offsets and repeat values
#(8-bit)######################################################################
.global put_sprite_mask_8bits
.type put_sprite_mask_8bits, @function
put_sprite_mask_8bits:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%ebx            #ebx=couleur en 8-bit
    movb    %bl,%bh
    movl    %ebx,%eax
    shll    $16,%eax
    movw    %bx,%ax
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%ecx         #edx=taille en pixels
    xorl    %ecx, %ecx
628:
    movl    (%ebx), %edx            #edx=offset
    addl    %edx, %edi                #additonne le deplacement
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;stosl
    movw    6(%ebx),%cx                #nombre de donnee en mots 8-bit (1 octets)
    addl    $0x8,%ebx
    rep;stosb
    decl    20(%ebp)
    jnz        628b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(16-bit)#####################################################################
.global put_sprite_mask_16bits
.type        put_sprite_mask_16bits, @function
put_sprite_mask_16bits:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%ebx            #ebx=couleur en 16-bit
    movl    %ebx,%eax
    shll    $16,%eax
    movw    %bx,%ax
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%ecx         #edx=taille en pixels
    xorl    %ecx, %ecx
62:
    movl    (%ebx), %edx            #edx=offset
    addl    %edx, %edi                #additonne le deplacement
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;stosl
    movw    6(%ebx),%cx                #nombre de donnee en mots 16-bit (2 octets)
    addl    $0x8,%ebx
    rep;stosw
    decl    20(%ebp)
    jnz        62b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(24-bit)#####################################################################
.global put_sprite_mask_24bits
.type        put_sprite_mask_24bits, @function
put_sprite_mask_24bits:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%ebx            #ebx=couleur en 16-bit
    movl    %ebx,%eax
    shll    $16,%eax
    movw    %bx,%ax
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%ecx         #edx=taille en pixels
    xorl    %ecx, %ecx
6224:
    movl    (%ebx), %edx            #edx=offset
    addl    %edx, %edi                #additonne le deplacement
    movw    4(%ebx),%cx                #nombre de donnee en mots 32-bit (4 octets)
    rep;stosl
    movw    6(%ebx),%cx                #nombre de donnee en mots 8-bit (1 octet)
    addl    $0x8,%ebx
    rep;stosb
    decl    20(%ebp)
    jnz        6224b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

#(32-bit)#####################################################################
.global put_sprite_mask_32bits
.type        put_sprite_mask_32bits, @function
put_sprite_mask_32bits:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    cld
    movl    8(%ebp),%eax            #eax=couleur en 8-bit
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ebx         #ebx=adresse table 'compress' (offset et repetition)
    movl    20(%ebp),%ecx         #edx=taille en pixels
    xorl    %ecx, %ecx
62832:
    movl    (%ebx), %edx            #edx=offset
    addl    %edx, %edi                #additonne le deplacement
    movw    4(%ebx),%cx                #nombre de fois 4 pixel bits (4 octets)
    rep;stosl
    movb    6(%ebx),%cl                #nombre de fois 1 pixel 8-bit (1 octets)
    addl    $0x8,%ebx
    rep;stosl
    decl    20(%ebp)
    jnz        62832b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

#------------------------------------------------------------------------------
# convertir une image codage 8-bit en codage 16/24/32-bit
# entree     => _pSource             : adresse image en 256 couleurs
#                 => _pDestination    :    adresse du buffer destination
#                    => _pPalette            : pointeur sur la palette 256 couleurs 16-bit
#                    => _iTaille                : nombre de pixels a traiter
#------------------------------------------------------------------------------
#(16-bit)#####################################################################
.global conv8_16
.type        conv8_16, @function
conv8_16 :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ecx         #ecx=adresse palette 16-bit(_pPalette)
    movl    20(%ebp),%edx         #edx=taille en octets de l image source     (_iTaille)
5:
    xorl    %eax, %eax
    movb    (%esi),%al                #al=index sur la palette
    addl    $0x01,%esi                #pixel suivant
    addl    %eax, %eax                #*2
    lea        (%ecx, %eax), %eax
    movw    (%eax), %bx                #code couleur 16-bit
    movw    %bx,(%edi)
    addl    $0x2,%edi
    decl    %edx
    jnz        5b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(24-bit)#####################################################################
.global conv8_24
.type        conv8_24, @function
conv8_24 :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ecx         #ecx=adresse palette 24-bit(_pPalette)
    movl    20(%ebp),%edx         #edx=taille en octets de l image source     (_iTaille)
5824:
    xorl    %eax, %eax
    movb    (%esi),%al                #al=index sur la palette
    addl    $0x01,%esi                #next 8-bit pixel
    addl    %eax, %eax                #*2
    addl    %eax, %eax                #*4
    lea        (%ecx, %eax), %eax
    movl    (%eax), %ebx
    movl    %ebx,(%edi)
    movw    (%eax),%bx                #read blue & green
    movw    %bx,(%eax)
    movb    2(%eax), %bl            #read red
    movb    %bl,2(%edi)
    addl    $0x3,%edi
    decl    %edx
    jnz        5824b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
#(32-bit)#####################################################################
.global conv8_32
.type        conv8_32, @function
conv8_32 :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ecx         #ecx=adresse palette 32-bit(_pPalette)
    movl    20(%ebp),%edx         #edx=taille en octets de l image source     (_iTaille)
5:
    xorl    %eax, %eax
    movb    (%esi),%al                #al=index sur la palette
    addl    $0x01,%esi                #pixel suivant
    addl    %eax, %eax                #*2
    addl    %eax, %eax                #*4
    lea        (%ecx, %eax), %eax
    movl    (%eax), %ebx
    movl    %ebx,(%edi)
    addl    $0x4,%edi
    decl    %edx
    jnz        5b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

## @method void convert_palette_24_to_16(unsigned char *src, unsigned char *dst)
# Convert a 24-bit RGB palette to a 16-bit RGB palette 
# @param src 24-bit RGB palette source
# @param dest 16-bit RGB palette destination 
.global convert_palette_24_to_16
.type convert_palette_24_to_16,@function
convert_palette_24_to_16:
    pushl %ebp
    movl    %esp, %ebp
    pushl   %ebx          #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl   %esi
    pushl   %edi
    movl    8(%ebp),%esi  #src 24-bit RGB palette
    movl    12(%ebp),%edi #dest 16-bit RGB palette
    movl    $256,%edx     #number of colors
    xorl    %eax, %eax
20:
    movb    0(%esi), %bl     #red hue                        00000000RRRRRRRR
    andl    $0x000000f8,%ebx #keep 5 bit upper               00000000RRRRR000        
    shll    $0x05,%ebx       #                               000RRRRR00000000
    movb    1(%esi), %bl     #green hue                      000RRRRRGGGGGGGG
    andl    $0xfffffffc,%ebx #                               000RRRRRGGGGGG00
    shll    $0x03,%ebx       #                               RRRRRGGGGGG00000
    movb    2(%esi), %al     #blue hue                       00000000BBBBBBBB
    andl    $0x000000f8,%eax #keep 5 bit upper               00000000BBBBB000        
    shrl    $0x03,%eax       #                               00000000000BBBBB
    addl    %ebx, %eax       #16-bit color                   RRRRRGGGGGGBBBBB
    movw    %ax,(%edi)
    addl    $0x3,%esi
    addl    $0x2,%edi
    decl    %edx
    jnz     20b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

## @method void convert_palette_24_to_15(unsigned char *src, unsigned char *dst)
# Convert a 24-bit RGB palette to a 15-bit RGB palette 
# @param src 24-bit RGB palette source
# @param dest 15-bit RGB palette destination 
.global convert_palette_24_to_15
.type convert_palette_24_to_15,@function
convert_palette_24_to_15:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl    8(%ebp),%esi            #esi=adresse palette 24-bit (_pSource)
    movl    12(%ebp),%edi            #edi=adresse palette 16-bit (_pDestination)
    movl    $256,%edx                     #edx=nombre de couleurs
    xorl    %eax, %eax
2015:
    movb    0(%esi), %bl            #code couleur rouge                                 #00000000RRRRRRRR
    andl    $0x000000f8,%ebx    #on garde les 5 bits superieurs            #00000000RRRRR000        
    shll    $0x05,%ebx                                                                                        #000RRRRR00000000
    movb    1(%esi), %bl            #code couleur vert                                     #000RRRRRGGGGGGGG
    andl    $0xfffffff8,%ebx                                                                            #000RRRRRGGGGG000
    shll    $0x02,%ebx                                                                                        #0RRRRRGGGGG00000
    movb    2(%esi), %al            #code couleur bleu                                     #00000000BBBBBBBB
    andl    $0x000000f8,%eax    #on garde les 5 bits superieurs            #00000000BBBBB000        
    shrl    $0x03,%eax                                                                                        #00000000000BBBBB
    addl        %ebx, %eax            #code couleur sur 16-bit                        #0RRRRRGGGGGBBBBB
    movw    %ax,(%edi)
    addl    $0x3,%esi
    addl    $0x2,%edi
    decl    %edx
    jnz        2015b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret

#------------------------------------------------------------------------------
#entree => _pSource                :    adresse source
#                => _pDestination     : adresse destination
#                => _iLargeur            :    largeur d une ligne en mot de 64 bits (8 octets)
#                => _iHauteur            :    hauteur de l image en lignes
#                => _iOffset                : offset pour atteindre la prochaine ligne source en octets
#                => _iOffset                : offset pour atteindre la prochaine ligne destination en octets
#------------------------------------------------------------------------------
.global copie4octets
.type     copie4octets,@function
copie4octets:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %esi
    pushl    %edi
    pushl    %ebx
    cld
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    20(%ebp),%ebx            #ecx=hauteur ecran
    movl    24(%ebp),%eax            #eax=offset sur la prochaine ligne source
    movl    28(%ebp),%edx            #edx=offset sur la prochaine ligne destination
    movl    16(%ebp),%ecx            #ecx=largeur ecran en mot de 32-bit
    pushl    %ebp
    movl    %ecx,%ebp
33:
    movl    %ebp,%ecx                    #ecx=largeur ecran en mot de 32-bit
    rep;movsl
    lea        (%esi, %eax), %esi
    lea        (%edi, %edx), %edi
    decl    %ebx
    jnz        33b
    popl    %ebp
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret

#------------------------------------------------------------------------------
#entree => _pSource                :    adresse source
#                => _pDestination     : adresse destination
#                => _iLargeur            :    largeur d une ligne en mot de 64 bits (8 octets)
#                => _iHauteur            :    hauteur de l image en lignes
#                => _iOffset                : offset pour atteindre la prochaine ligne source en octets
#                => _iOffset                : offset pour atteindre la prochaine ligne destination en octets
#------------------------------------------------------------------------------
#( 8-bit)#####################################################################
.global _COPY2X8BITS
.type     _COPY2X8BITS,@function
_COPY2X8BITS:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %esi
    pushl    %edi
    pushl    %ebx
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    24(%ebp),%ebx            #ebx=offset sur la prochaine ligne source
    movl    28(%ebp),%edx            #edx=offset sur la prochaine ligne destination
440:
    movl    16(%ebp), %ecx
441:
    movb    (%esi), %al
    addl    $0x1,%esi
    movb    %al,(%edi)
    movb    %al,1(%edi)
    addl    $0x02,%edi
    dec        %ecx
    jnz        441b
    lea        (%esi, %ebx),%esi
    lea        (%edi, %edx),%edi
    decl    20(%ebp)
    jnz        440b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
#(16-bit)#####################################################################
.global _COPY2X16BITS
.type     _COPY2X16BITS,@function
_COPY2X16BITS:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %esi
    pushl    %edi
    pushl    %ebx
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    24(%ebp),%ebx            #ebx=offset sur la prochaine ligne source
    movl    28(%ebp),%edx            #edx=offset sur la prochaine ligne destination
440:
    movl    16(%ebp), %ecx
441:
    movw    (%esi), %ax
    addl    $0x2,%esi
    movw    %ax,(%edi)
    movw    %ax,2(%edi)
    addl    $0x04,%edi
    dec        %ecx
    jnz        441b
    lea        (%esi, %ebx),%esi
    lea        (%edi, %edx),%edi
    decl    20(%ebp)
    jnz        440b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
#(24-bit)#####################################################################
.global _COPY2X24BITS
.type     _COPY2X24BITS,@function
_COPY2X24BITS:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %esi
    pushl    %edi
    pushl    %ebx
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    24(%ebp),%ebx            #ebx=offset sur la prochaine ligne source
    movl    28(%ebp),%edx            #edx=offset sur la prochaine ligne destination
    pushl    %ebp
    movl    20(%ebp),%ecx            #hauteur
    pushl    %ecx
    movl    16(%ebp),%ecx            #largeur
    pushl    %ecx
    movl    $6,%ecx
44024:
    movl    (%esp), %ebp             #largeur
44124:
    movw    (%esi), %ax
    movw    %ax,(%edi)
    movw    %ax,3(%edi)
    movb    2(%esi), %al
    addl    $0x3,%esi
    movb    %al,2(%edi)
    movb    %al,5(%edi)
    lea        (%edi, %ecx),%edi
    decl    %ebp
    jnz        44124b
    lea        (%esi, %ebx),%esi
    lea        (%edi, %edx),%edi
    decl    4(%esp)
    jnz        44024b
    popl    %ecx
    popl    %ecx
    popl    %ebp
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
#(32-bit)#####################################################################
.global _COPY2X32BITS
.type     _COPY2X32BITS,@function
_COPY2X32BITS:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %esi
    pushl    %edi
    pushl    %ebx
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi           #edi=adresse destination (_pDestination)
    movl    24(%ebp),%ebx           #ebx=offset sur la prochaine ligne source
    movl    28(%ebp),%edx           #edx=offset sur la prochaine ligne destination
440:
    movl    16(%ebp), %ecx
441:
    movl    (%esi), %eax
    addl    $0x4,%esi
    movl    %eax,(%edi)
    movl    %eax,4(%edi)
    addl    $0x08,%edi
    dec        %ecx
    jnz        441b
    lea        (%esi, %ebx),%esi
    lea        (%edi, %edx),%edi
    decl    20(%ebp)
    jnz        440b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret

## @method void clear_offscreen (char *oscreen, Uint32 width, Uint32 height, Uint32 offset)
# Clear a offscreen
# @param oscreen Pointer to a offscreen destination
# @param with Width of region to clear in 32-bit long words 
# @param height Height of region to clear in lines
# @param offset Offet of the next line
.global clear_offscreen    
.type clear_offscreen,@function
clear_offscreen:
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx
    pushl    %ecx
    pushl    %edx
    pushl    %edi
    movl    8(%ebp),%edi      #offscreen destination
    movl    16(%ebp),%edx     #height in lines
    movl    20(%ebp),%ebx     #offset to next line
    xorl    %eax, %eax
    cld
    jmp        1f
.align 4
1:
    movl    12(%ebp),%ecx    #with in long words of regtion to clear 
    rep;stosl
    addl    %ebx,%edi
    decl    %edx
    jnz     1b
    popl    %edi
    popl    %edx
    popl    %ecx
    popl    %ebx
    leave
    ret

#------------------------------------------------------------------------------
# convertit un pixel 8-bit sur 16-bit
# entree     => _iValeur  : index sur la palette (0 a 255)
#                    => _pPalette : pointeur sur la palette de couleurs 16-bit
#    sortie    <= pixel code sur 16-bit (unsigned int)
#------------------------------------------------------------------------------
.global pixel8_16
.type pixel8_16,@function
pixel8_16:
    pushl %ebp
    movl    %esp, %ebp
    movl    8(%ebp), %eax            #pixel sur 8-bit
    movl    12(%ebp),%ecx         #ecx=adresse palette (_pPalette)
    addl    %eax, %eax                #*2
    lea        (%ecx, %eax), %eax
    movw    (%eax), %ax                #eax=code couleur 16-bit
    leave
    ret

#------------------------------------------------------------------------------
# convertir une image codage 8-bit en codage 24-bit
# entree     => _pSource             : adresse image en 256 couleurs
#                 => _pDestination    :    adresse du buffer destination
#                    => _pPalette            : pointeur sur la palette 256 couleurs
#                    => _iTaille                : nombre de pixels a traiter
#------------------------------------------------------------------------------
.global convertit8_24
.type        convertit8_24,@function
convertit8_24 :
    pushl %ebp
    movl    %esp, %ebp
    leave
    ret
    movl    %eax, %ebx
    addl    %eax, %eax
    addl    %ebx, %eax                    #*3
    lea        (%ecx, %eax), %eax
    movb    (%eax), %bl                #code couleur rouge                                 #00000000RRRRRRRR
    shll    $0x08,%ebx                                                                                        #RRRRRRRR00000000
    movb    1(%eax), %bl            #code couleur vert                                     #RRRRRRRGGGGGGGGG
    movb    2(%eax), %al            #code couleur bleu                                     #00000000BBBBBBBB
    movw    %bx,(%edi)
    movb    %al,1(%edi)
    addl    $0x3,%edi
    leave
    ret

#------------------------------------------------------------------------------
# convertir une image codage 8-bit en codage 16-bit
# entree     => _pSource             : adresse image en 256 couleurs
#                 => _pDestination    :    adresse du buffer destination
#                    => _pPalette            : pointeur sur la palette 256 couleurs
#                    => _iTaille                : nombre de pixels a traiter
#------------------------------------------------------------------------------
.global convertit8_16
.type        convertit8_16, @function
convertit8_16 :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl    8(%ebp),%esi            #esi=adresse source (_pSource)
    movl    12(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    16(%ebp),%ecx         #ecx=adresse palette (_pPalette)
    movl    20(%ebp),%edx         #edx=taille en octets de l image source     (_iTaille)
    xorl    %eax, %eax
2:
    xorl    %eax, %eax
    movb    (%esi),%al                #al=index sur la palette
    addl    $0x01,%esi            
    movl    %eax, %ebx
    addl    %eax, %eax
    addl    %ebx, %eax                #*3
    lea        (%ecx, %eax), %eax
    movb    (%eax), %bl                #code couleur rouge                                 #00000000RRRRRRRR
    andl    $0x000000f8,%ebx    #on garde les 5 bits superieurs            #00000000RRRRR000        
    shll    $0x05,%ebx                                                                                        #000RRRRR00000000
    movb    1(%eax), %bl            #code couleur vert                                     #000RRRRRGGGGGGGG
    andl    $0xfffffffa,%ebx                                                                            #000RRRRRGGGGGG00
    shll    $0x03,%ebx                                                                                        #RRRRRGGGGGG00000
    movb    2(%eax), %al            #code couleur bleu                                     #00000000BBBBBBBB
    andl    $0x000000f8,%eax    #on garde les 5 bits superieurs            #00000000BBBBB000        
    shrl    $0x03,%eax                                                                                        #00000000000BBBBB
    orl        %ebx, %eax                #code couleur sur 16-bit                        #RRRRRGGGGGGBBBBB
    movw    %ax,(%edi)
    addl    $0x2,%edi
    decl    %edx
    jnz        2b
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret



#########################################################################################################

.global poke
.type poke, @function
poke:
    pushl %ebp
    pushl    %edi
    movl    %esp, %ebp
    movl    8(%ebp),%edi
    movl    12(%ebp),%eax
    movb    %al,(%edi)
    popl     %ebp
    popl    %edi
    leave
    ret




#########################################################################################################


#------------------------------------------------------------------------------
# affichage polygone
# entree 
#                 => _pDestination    :    adresse du buffer destination
#                    => _iPixels                : nombre de pixels
#                    => _iCouleur            : nombre de pixels a traiter
#------------------------------------------------------------------------------
#(8-bit)######################################################################
.global poly8bits
.type        poly8bits, @function
poly8bits :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    12(%ebp),%edx         #edx=nombre de pixels
    movl    16(%ebp),%ecx         #ecx=code couleur
    cmpl    $0x0,%edx
    jle        4038f
4048:
    movb    %cl,(%edi)
    addl    $0x1,%edi
    decl    %edx
    jnz        4048b
4038:
    popl    %edi
    leave
    ret
#(16-bit)######################################################################
.global poly16bits
.type        poly16bits, @function
poly16bits :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    12(%ebp),%edx         #edx=nombre de pixels
    movl    16(%ebp),%ecx         #ecx=code couleur
    cmpl    $0x0,%edx
    jle        403f
404:
    movw    %cx,(%edi)
    addl    $0x2,%edi
    decl    %edx
    jnz        404b
403:
    popl    %edi
    leave
    ret
#(24-bit)######################################################################
.global poly24bits
.type        poly24bits, @function
poly24bits :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    12(%ebp),%edx         #edx=nombre de pixels
    movw    16(%ebp),%cx             #ecx=code couleur
    movb    18(%ebp),%bl             #bl=code couleur
    cmpl    $0x0,%edx
    jle        40324f
40424:
    movw    %cx,(%edi)
    movb    %bl,2(%edi)
    addl    $0x3,%edi
    decl    %edx
    jnz        40424b
40324:
    popl    %edi
    leave
    ret
#(32-bit)######################################################################
.global poly32bits
.type        poly32bits, @function
poly32bits :
    pushl %ebp
    movl    %esp, %ebp
    pushl    %ebx         #save ebx, esi ,edi registers (x86 GCC convention)                            
    pushl    %esi
    pushl    %edi
    movl     8(%ebp),%edi            #edi=adresse destination (_pDestination)
    movl    12(%ebp),%edx         #edx=nombre de pixels
    movl    16(%ebp),%ecx         #ecx=code couleur
    cmpl    $0x0,%edx
    jle        40332f
40432:
    movl    %ecx,(%edi)
    addl    $0x4,%edi
    decl    %edx
    jnz        40432b
40332:
    popl    %edi
    leave
    ret

#------------------------------------------------------------------------------
# display lightning (8-bit 256 colors) screen width 512 bytes
# entree     => _pDestination     : pointer to the screen
#                 => _pEclair                :    pointer to the structure "eclair"
#                    => _iTaille                : nimber of iterations
#------------------------------------------------------------------------------
.global draw_electrical_shock_8
.type    draw_electrical_shock_8, @function
draw_electrical_shock_8:
    pushl     %ebp
    movl        %esp, %ebp
    pushl        %ebx                 #save registers ebx, esi ,edi (gcc convention)                            
    pushl        %esi
    pushl        %edi
    pushl        %ebp
    movl        8(%ebp),%eax          #pointer in the game offscreen
    movl        12(%ebp),%edx         #edx=eclair1 (pointer to lightning structure)
    movl        16(%ebp),%ebx         #ebx=recval=4 (recurcive value)
    movl        %eax,(_et_adr)        #save screen address
    movl      16(%edx),%eax           #eax=eclair1->col1
    movl        20(%edx),%ecx         #ecx=eclair1->col2
    movb      %al,(_et_col1)          #save color 1
    movb        %cl,(_et_col2)        #save color 2
    movl        24(%edx),%eax         #eax=eclair1->r1 (random value)
    movl        28(%edx),%ecx         #ecx=eclair1->r2 (random value)
    movl        32(%edx),%ebp         #ecx=eclair1->r3 (random value)
    movl        %eax,(_et_r1)
    movl        %ecx,(_et_r2)
    movl        %ebp,(_et_r3)
    movl        12(%edx),%eax         #eax=eclair1->dy
    movl        8(%edx),%ebp          #ebp=eclair1->dx
    movl        4(%edx),%ecx          #ecx=eclair1->sy
    movl        0(%edx),%edx          #edx=eclair1->sx
    pushl        %eax
    pushl        %ebp
    pushl        %ecx
    pushl        %edx
    pushl        %ebx                 #niveau de recurence
    call        rec_line8
    addl        $20,%esp
101:
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
# ---------- ---------- ---------- ---------- #
rec_line8:    
    pushal                                        # %eax, %ecx, %edx, %ebx, %esp ,%ebp, %esi, %edi    (%esp=%esp-40)
    movl        4+32(%esp),%ebp        #_niveau
    cmp            $0, %ebp
    je            final8
    movl        8+32(%esp),%eax        #_px1
    movl        12+32(%esp),%ebx    #_py1
    movl        16+32(%esp),%ecx    #_px2
    movl        20+32(%esp),%edx    #_py2
    subl        %eax,%ecx                    #dx
    subl        %ebx,%edx                    #dy
    addl        16+32(%esp),%eax    #_px2
    addl        20+32(%esp),%ebx    #_py2
    sarl        $1,%eax                        #x moyen
    sarl        $1,%ebx                        #y moyen
    call        rnd
    sarl        $24,%ebp                    #entre -64 et +64
    imul        %ebp,%ecx
    imul        %ebp,%edx
    sarl        $9,%ecx                        #/512
    sarl        $9,%edx                        #/512
    addl        %edx,%eax                    #new x
    subl        %ecx,%ebx                    #new y
    movl        8+32(%esp),%ecx
    movl        12+32(%esp),%edx
    movl        4+32(%esp),%ebp        #_niveau
    pushl        %ebx    
    pushl        %eax    
    subl        $1,%ebp
    pushl        %edx
    pushl        %ecx
    pushl        %ebp
    call        rec_line8      
    addl        $20,%esp
    movl        20+32(%esp),%edx        #_py2
    movl        16+32(%esp),%ecx        #_px2
    pushl        %edx
    pushl        %ecx
    pushl        %ebx    
    pushl        %eax    
    pushl        %ebp
    call    rec_line8      
    addl        $20,%esp
    popal
    ret
# ---------- ---------- ---------- ---------- #
final8:
    movl    8+32(%esp),%eax                #_px1
    movl    12+32(%esp),%ebx            #_py1
    movl    16+32(%esp),%ecx            #_px2
    movl    20+32(%esp),%edx            #_py2
    movl    %eax,(_et_x1)
    movl    %ebx,(_et_y1)
    movl    %ecx,(_et_x2)
    movl    %edx,(_et_y2)
    call    line8
    popal
    ret
# ---------- ---------- ---------- ---------- #
line8:
    pushal
    movl    (_et_y1),%eax
    movl    (_et_x1),%ebx
    sall    $9,%eax                                #*512
    movl    (_et_adr),%edi                #edi=screen address
    addl    %ebx,%eax
    #addl    %ebx,%eax
    addl    %eax,%edi
    movl    (_et_x1),%eax
    movl    (_et_x2),%ebx
    movl    $1,%ecx
    subl    %eax,%ebx                            #dx
    jg        noch18
    movl    $-1,%ecx
    negl    %ebx
noch18:
    movl        %ebx,(_et_dx)
    movl        (_et_y1),%eax
    movl        (_et_y2),%ebx
    movl        $512,%edx
    subl        %eax,%ebx                         #dy
    jg        noch28
    movl        $-512,%edx
    negl        %ebx
noch28:
    movl        %ebx,(_et_dy)
    movb        (_et_col1),%al
    movb        (_et_col2),%ah
    cmpl        (_et_dx),%ebx
    jg            other_line8
    movl        (_et_dx),%ebx
    movl        (_et_dx),%ebp
    shrl        $1,%ebx
bcl18:
    addl        (_et_dy),%ebx
    addl        %ecx,%edi
    cmpl        (_et_dx),%ebx
    jle            nomodif18
    subl        (_et_dx),%ebx
    addl        %edx,%edi
nomodif18:
    movb        %ah,-512(%edi)
    movb        %al,(%edi)
    movb        %ah,512(%edi)
    decl        %ebp
    jg            bcl18
    popal
    ret
# ---------- ---------- ---------- ---------- #
other_line8:
    movl    (_et_dy),%ebx
    movl    (_et_dy),%ebp
    shrl    $1,%ebx
bcl28:
    addl        (_et_dx),%ebx
    addl        %edx,%edi
    cmpl        (_et_dy),%ebx
    jle            nomodif28
    subl        (_et_dy),%ebx
    addl        %ecx,%edi
nomodif28:
    movb    %ah,-1(%edi)
    movb    %al,(%edi)
    movb    %ah,1(%edi)
    decl    %ebp
    jg        bcl28
    popal
    ret
#------------------------------------------------------------------------------
# display lightning
# entree     => _pDestination     : pointer to the screen
#                 => _pEclair                :    pointer to the structure "eclair"
#                    => _iTaille                : nimber of iterations
#------------------------------------------------------------------------------
.global draw_electrical_shock_16
.type    draw_electrical_shock_16, @function
draw_electrical_shock_16:
    pushl     %ebp
    movl        %esp, %ebp
    pushl        %ebx                 #save registers ebx, esi ,edi (gcc convention)                            
    pushl        %esi
    pushl        %edi
    pushl        %ebp
    movl        8(%ebp),%eax          #pointer in the game offscreen
    movl        12(%ebp),%edx         #edx=eclair1 (pointer to lightning structure)
    movl        16(%ebp),%ebx         #ebx=recval=4 (recurcive value)
    movl        %eax,(_et_adr)        #save screen address
    movl      16(%edx),%eax           #eax=eclair1->col1
    movl        20(%edx),%ecx         #ecx=eclair1->col2
    movl      %eax,(_et_col1)         #save color 1
    movl        %ecx,(_et_col2)       #save color 2
    movl        24(%edx),%eax         #eax=eclair1->r1 (random value)
    movl        28(%edx),%ecx         #ecx=eclair1->r2 (random value)
    movl        32(%edx),%ebp         #ecx=eclair1->r3 (random value)
    movl        %eax,(_et_r1)
    movl        %ecx,(_et_r2)
    movl        %ebp,(_et_r3)
    movl        12(%edx),%eax         #eax=eclair1->dy
    movl        8(%edx),%ebp          #ebp=eclair1->dx
    movl        4(%edx),%ecx          #ecx=eclair1->sy
    movl        0(%edx),%edx          #edx=eclair1->sx
    pushl        %eax
    pushl        %ebp
    pushl        %ecx
    pushl        %edx
    pushl        %ebx                 #niveau de recurence
    call        rec_line
    addl        $20,%esp
101:
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
# ---------- ---------- ---------- ---------- # 
rec_line:    
    pushal                                        # %eax, %ecx, %edx, %ebx, %esp ,%ebp, %esi, %edi    (%esp=%esp-32)
    movl        4+32(%esp),%ebp        #_niveau
    cmp            $0, %ebp
    je            final
    movl        8+32(%esp),%eax        #_px1
    movl        12+32(%esp),%ebx    #_py1
    movl        16+32(%esp),%ecx    #_px2
    movl        20+32(%esp),%edx    #_py2
    subl        %eax,%ecx                    #dx
    subl        %ebx,%edx                    #dy
    addl        16+32(%esp),%eax    #_px2
    addl        20+32(%esp),%ebx    #_py2
    sarl        $1,%eax                        #x moyen
    sarl        $1,%ebx                        #y moyen
    call        rnd
    sarl        $24,%ebp                    #entre -64 et +64
    imul        %ebp,%ecx
    imul        %ebp,%edx
    sarl        $9,%ecx                        #/512
    sarl        $9,%edx                        #/512
    addl        %edx,%eax                    #new x
    subl        %ecx,%ebx                    #new y
    movl        8+32(%esp),%ecx
    movl        12+32(%esp),%edx
    movl        4+32(%esp),%ebp        #_niveau
    pushl        %ebx    
    pushl        %eax    
    subl        $1,%ebp
    pushl        %edx
    pushl        %ecx
    pushl        %ebp
    call        rec_line      
    addl        $20,%esp
    movl        20+32(%esp),%edx        #_py2
    movl        16+32(%esp),%ecx        #_px2
    pushl        %edx
    pushl        %ecx
    pushl        %ebx    
    pushl        %eax    
    pushl        %ebp
    call    rec_line      
    addl        $20,%esp
    popal
    ret
# ---------- ---------- ---------- ---------- #
final:
    movl    8+32(%esp),%eax                #_px1
    movl    12+32(%esp),%ebx            #_py1
    movl    16+32(%esp),%ecx            #_px2
    movl    20+32(%esp),%edx            #_py2
    movl    %eax,(_et_x1)
    movl    %ebx,(_et_y1)
    movl    %ecx,(_et_x2)
    movl    %edx,(_et_y2)
    call    line
    popal
    ret
# ---------- ---------- ---------- ---------- #
line:
    pushal
    movl        (_et_y1),%eax
    movl        (_et_x1),%ebx
    sall        $10,%eax                            #*512*2
    movl        (_et_adr),%edi                #edi=screen address
    addl        %ebx,%eax
    addl        %ebx,%eax
    addl        %eax,%edi
    movl        (_et_x1),%eax
    movl        (_et_x2),%ebx
    movl        $2,%ecx
    subl        %eax,%ebx                            #dx
    jg            noch1
    movl        $-2,%ecx
    negl        %ebx
noch1:
    movl        %ebx,(_et_dx)
    movl        (_et_y1),%eax
    movl        (_et_y2),%ebx
    movl        $1024,%edx
    subl        %eax,%ebx     #dy
    jg            noch2
    movl        $-1024,%edx
    negl        %ebx
noch2:
    movl        %ebx,(_et_dy)
    movl        (_et_col1),%eax
    movl        (_et_col2),%esi
    cmpl        (_et_dx),%ebx
    jg            other_line
    movl        (_et_dx),%ebx
    movl        (_et_dx),%ebp
    shrl        $1,%ebx
bcl1:
    addl        (_et_dy),%ebx
    addl        %ecx,%edi
    cmpl        (_et_dx),%ebx
    jle            nomodif1
    subl        (_et_dx),%ebx
    addl        %edx,%edi
nomodif1:
    movw        %si,-1024(%edi)
    movw        %ax,(%edi)                                    #put pixel
    movw        %si,1024(%edi)                            #put pixel
    decl        %ebp
    jg            bcl1
    popal
    ret
# ---------- ---------- ---------- ---------- #
other_line:
    movl    (_et_dy),%ebx
    movl    (_et_dy),%ebp
    shrl    $1,%ebx
bcl2:
    addl        (_et_dx),%ebx
    addl        %edx,%edi
    cmpl        (_et_dy),%ebx
    jle            nomodif2
    subl        (_et_dy),%ebx
    addl        %ecx,%edi
nomodif2:
    movw    %si,-2(%edi)                                #put pixel
    movw    %ax,(%edi)                                    #put pixel
    movw    %si,2(%edi)                                    #put pixel
    decl    %ebp
    jg        bcl2
    popal
    ret
# ---------- ---------- ---------- ---------- #
rnd:
    pushl        %eax
    pushl        %ebx
    movl        (_et_r1),%eax
    movl        (_et_r2),%ebx
    movl        (_et_r3),%ebp
    roll        $13,%eax
    xorl        %ebp,%eax
    xorl        %ebp,%ebx
    rorl        $7,%ebx
    addl        $27,%ebp
    xorl        %eax,%ebp
    xorl        %ebx,%ebp
    movl        %ebp,(_et_r1)
    movl        %eax,(_et_r2)
    movl        %ebx,(_et_r3)
    popl        %ebx
    popl        %eax
     ret

    
##################################################################
.global draw_electrical_shock_24
.type    draw_electrical_shock_24, @function
draw_electrical_shock_24:
    pushl     %ebp
    movl        %esp, %ebp
    pushl        %ebx                 #save registers ebx, esi ,edi (gcc convention)                            
    pushl        %esi
    pushl        %edi
    pushl        %ebp
    movl        8(%ebp),%eax          #pointer in the game offscreen
    movl        12(%ebp),%edx         #edx=eclair1 (pointer to lightning structure)
    movl        16(%ebp),%ebx         #ebx=recval=4 (recurcive value)
    movl        %eax,(_et_adr)        #save screen address
    movl      16(%edx),%eax           #eax=eclair1->col1
    movl        20(%edx),%ecx         #ecx=eclair1->col2
    movl      %eax,(_et_col1)         #save color 1
    movl        %ecx,(_et_col2)       #save color 2
    movl        24(%edx),%eax         #eax=eclair1->r1 (random value)
    movl        28(%edx),%ecx         #ecx=eclair1->r2 (random value)
    movl        32(%edx),%ebp         #ecx=eclair1->r3 (random value)
    movl        %eax,(_et_r1)
    movl        %ecx,(_et_r2)
    movl        %ebp,(_et_r3)
    movl        12(%edx),%eax          #eax=eclair1->dy
    movl        8(%edx),%ebp           #ebp=eclair1->dx
    movl        4(%edx),%ecx           #ecx=eclair1->sy
    movl        0(%edx),%edx           #edx=eclair1->sx
    pushl        %eax
    pushl        %ebp
    pushl        %ecx
    pushl        %edx
    pushl        %ebx                  #niveau de recurence
    call        rec_line24
    addl        $20,%esp
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
# ---------- ---------- ---------- ---------- #
rec_line24:    
    pushal                                        # %eax, %ecx, %edx, %ebx, %esp ,%ebp, %esi, %edi    (%esp=%esp-40)
    movl        4+32(%esp),%ebp        #_niveau
    cmp            $0, %ebp
    je            final24
    movl        8+32(%esp),%eax        #_px1
    movl        12+32(%esp),%ebx    #_py1
    movl        16+32(%esp),%ecx    #_px2
    movl        20+32(%esp),%edx    #_py2
    subl        %eax,%ecx                    #dx
    subl        %ebx,%edx                    #dy
    addl        16+32(%esp),%eax    #_px2
    addl        20+32(%esp),%ebx    #_py2
    sarl        $1,%eax                        #x moyen
    sarl        $1,%ebx                        #y moyen
    call        rnd32
    sarl        $24,%ebp                    #entre -64 et +64
    imul        %ebp,%ecx
    imul        %ebp,%edx
    sarl        $9,%ecx                        #/512
    sarl        $9,%edx                        #/512
    addl        %edx,%eax                    #new x
    subl        %ecx,%ebx                    #new y
    movl        8+32(%esp),%ecx
    movl        12+32(%esp),%edx
    movl        4+32(%esp),%ebp        #_niveau
    pushl        %ebx    
    pushl        %eax    
    subl        $1,%ebp
    pushl        %edx
    pushl        %ecx
    pushl        %ebp
    call        rec_line24      
    addl        $20,%esp
    movl        20+32(%esp),%edx        #_py2
    movl        16+32(%esp),%ecx        #_px2
    pushl        %edx
    pushl        %ecx
    pushl        %ebx    
    pushl        %eax    
    pushl        %ebp
    call    rec_line24
    addl        $20,%esp
    popal
    ret
# ---------- ---------- ---------- ---------- #
final24:
    movl    8+32(%esp),%eax                #_px1
    movl    12+32(%esp),%ebx            #_py1
    movl    16+32(%esp),%ecx            #_px2
    movl    20+32(%esp),%edx            #_py2
    movl    %eax,(_et_x1)
    movl    %ebx,(_et_y1)
    movl    %ecx,(_et_x2)
    movl    %edx,(_et_y2)
    call    line24
    popal
    ret
# ---------- ---------- ---------- ---------- #
line24:
    pushal
    movl    (_et_y1),%eax
    movl    (_et_x1),%ebx
    imul    $1536, %eax                    #(!)*1536 (24-bit depth)
    movl    (_et_adr),%edi            #edi=screen address
    addl    %ebx,%eax                        #(!)
    addl    %ebx,%eax                        #(!)*3 (3 bytes peer pixels)
    addl    %ebx,%eax                        #(!)
    addl    %eax,%edi
    movl    (_et_x1),%eax
    movl    (_et_x2),%ebx
    movl    $3,%ecx                            #(!)size of one pixel in 24-bit (inc)
    subl    %eax,%ebx                        #Dx
    jg        noch124
    movl    $-3,%ecx                        #(!)size of one pixel in 24-bit (dec)
    negl    %ebx
noch124:
    movl    %ebx,(_et_dx)
    movl        (_et_y1),%eax
    movl        (_et_y2),%ebx
    movl        $1536,%edx                #(!)size of line (512 pixels in 24-bit)
    subl        %eax,%ebx                     #Dy
    jg        noch224
    movl        $-1536,%edx                #(!)size of line (512 pixels in 24-bit)
    negl        %ebx
noch224:
    movl        %ebx,(_et_dy)
    #movl        (_et_col1),%eax        #color 1
    #movl        (_et_col2),%esi        #color 2
    movw    (_et_col2),%si
    movb    (_et_col2+2),%ah    
    bswap    %esi
    movw    (_et_col1),%si    
    movb    (_et_col1+2),%al    
    cmpl        (_et_dx),%ebx
    jg            other_line24
    movl        (_et_dx),%ebx
    movl        (_et_dx),%ebp
    shrl        $1,%ebx
bcl124:
    addl        (_et_dy),%ebx
    addl        %ecx,%edi
    cmpl        (_et_dx),%ebx
    jle            nomodif124
    subl        (_et_dx),%ebx
    addl        %edx,%edi
nomodif124:
    movw        %si,(%edi)
    movb        %al,2(%edi)
    bswap        %esi
    movw        %si,-1536(%edi)            #(!)put one pixel
    movb        %ah,-1538(%edi)            #(!)put one pixel
    movw        %si, 1536(%edi)            #(!)put one pixel
    movb        %ah, 1538(%edi)            #(!)put one pixel
    bswap        %esi
  #movl        %esi,-1536(%edi)        #(!)put one pixel
    #movl        %eax,(%edi)                    #(!)put one pixel
    #movl        %esi,1536(%edi)            #(!)put one pixel
    decl        %ebp
    jg            bcl124
    popal
    ret
# ---------- ---------- ---------- ---------- #
other_line24:
    movl    (_et_dy),%ebx
    movl    (_et_dy),%ebp
    shrl    $1,%ebx
bcl224:
    addl        (_et_dx),%ebx
    addl        %edx,%edi
    cmpl        (_et_dy),%ebx
    jle            nomodif224
    subl        (_et_dy),%ebx
    addl        %ecx,%edi
nomodif224:
    movw        %si,(%edi)
    movb        %al,2(%edi)
    bswap        %esi
    movw        %si,-1536(%edi)            #(!)put one pixel
    movb        %ah,-1538(%edi)            #(!)put one pixel
    movw        %si, 1536(%edi)            #(!)put one pixel
    movb        %ah, 1538(%edi)            #(!)put one pixel
    bswap        %esi
  #movl    %esi,-3(%edi)            #(!)put one pixel
    #movl    %eax,(%edi)                #(!)put one pixel
    #movl    %esi,3(%edi)            #(!)put one pixel
    decl    %ebp
    jg        bcl224
    popal
    ret
# ---------- ---------- ---------- ---------- #
rnd24:
    pushl        %eax
    pushl        %ebx
    movl        (_et_r1),%eax
    movl        (_et_r2),%ebx
    movl        (_et_r3),%ebp
    roll        $13,%eax
    xorl        %ebp,%eax
    xorl        %ebp,%ebx
    rorl        $7,%ebx
    addl        $27,%ebp
    xorl        %eax,%ebp
    xorl        %ebx,%ebp
    movl        %ebp,(_et_r1)
    movl        %eax,(_et_r2)
    movl        %ebx,(_et_r3)
    popl        %ebx
    popl        %eax
     ret
     
#------------------------------------------------------------------------------
# display lightning
# entree     => _pDestination     : pointer to the screen
#                 => _pEclair                :    pointer to the structure "eclair"
#                    => _iTaille                : nimber of iterations
#------------------------------------------------------------------------------
.global draw_electrical_shock_32
.type    draw_electrical_shock_32, @function
draw_electrical_shock_32:
    pushl     %ebp
    movl        %esp, %ebp
    pushl        %ebx                            #save registers ebx, esi ,edi (gcc convention)                            
    pushl        %esi
    pushl        %edi
    pushl        %ebp
    movl        8(%ebp),%eax            #edi=ecran_ram (pointer to screen)
    movl        12(%ebp),%edx         #edx=eclair1 (pointer to lightning structure)
    movl        16(%ebp),%ebx         #ebx=recval=4 (recurcive value)
    movl        %eax,(_et_adr)        #save screen address
    movl      16(%edx),%eax            #eax=eclair1->col1
    movl        20(%edx),%ecx            #ecx=eclair1->col2
    movl      %eax,(_et_col1)        #save color 1
    movl        %ecx,(_et_col2)        #save color 2
    movl        24(%edx),%eax            #eax=eclair1->r1 (random value)
    movl        28(%edx),%ecx            #ecx=eclair1->r2 (random value)
    movl        32(%edx),%ebp            #ecx=eclair1->r3 (random value)
    movl        %eax,(_et_r1)
    movl        %ecx,(_et_r2)
    movl        %ebp,(_et_r3)
    movl        12(%edx),%eax            #eax=eclair1->dy
    movl        8(%edx),%ebp            #ebp=eclair1->dx
    movl        4(%edx),%ecx            #ecx=eclair1->sy
    movl        0(%edx),%edx            #edx=eclair1->sx
    pushl        %eax
    pushl        %ebp
    pushl        %ecx
    pushl        %edx
    pushl        %ebx                            #niveau de recurence
    call        rec_line32
    addl        $20,%esp
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    leave
    ret
# ---------- ---------- ---------- ---------- #
rec_line32:    
    pushal                                        # %eax, %ecx, %edx, %ebx, %esp ,%ebp, %esi, %edi    (%esp=%esp-40)
    movl        4+32(%esp),%ebp        #_niveau
    cmp            $0, %ebp
    je            final32
    movl        8+32(%esp),%eax        #_px1
    movl        12+32(%esp),%ebx    #_py1
    movl        16+32(%esp),%ecx    #_px2
    movl        20+32(%esp),%edx    #_py2
    subl        %eax,%ecx                    #dx
    subl        %ebx,%edx                    #dy
    addl        16+32(%esp),%eax    #_px2
    addl        20+32(%esp),%ebx    #_py2
    sarl        $1,%eax                        #x moyen
    sarl        $1,%ebx                        #y moyen
    call        rnd32
    sarl        $24,%ebp                    #entre -64 et +64
    imul        %ebp,%ecx
    imul        %ebp,%edx
    sarl        $9,%ecx                        #/512
    sarl        $9,%edx                        #/512
    addl        %edx,%eax                    #new x
    subl        %ecx,%ebx                    #new y
    movl        8+32(%esp),%ecx
    movl        12+32(%esp),%edx
    movl        4+32(%esp),%ebp        #_niveau
    pushl        %ebx    
    pushl        %eax    
    subl        $1,%ebp
    pushl        %edx
    pushl        %ecx
    pushl        %ebp
    call        rec_line32      
    addl        $20,%esp
    movl        20+32(%esp),%edx        #_py2
    movl        16+32(%esp),%ecx        #_px2
    pushl        %edx
    pushl        %ecx
    pushl        %ebx    
    pushl        %eax    
    pushl        %ebp
    call    rec_line32
    addl        $20,%esp
    popal
    ret
# ---------- ---------- ---------- ---------- #
final32:
    movl    8+32(%esp),%eax                #_px1
    movl    12+32(%esp),%ebx            #_py1
    movl    16+32(%esp),%ecx            #_px2
    movl    20+32(%esp),%edx            #_py2
    movl    %eax,(_et_x1)
    movl    %ebx,(_et_y1)
    movl    %ecx,(_et_x2)
    movl    %edx,(_et_y2)
    call    line32
    popal
    ret
# ---------- ---------- ---------- ---------- #
line32:
    pushal
    movl     (_et_y1),%eax
    movl     (_et_x1),%ebx
    sall     $11,%eax                        #(!)*2048 (32-bit depth)
    movl     (_et_adr),%edi            #edi=screen address
    addl     %ebx,%ebx                        #(!)
    addl     %ebx,%ebx                        #(!)*4 (4 bytes peer pixels)
    addl     %ebx,%eax
    addl     %eax,%edi
    movl     (_et_x1),%eax
    movl     (_et_x2),%ebx
    movl     $4,%ecx                            #(!)size of one pixel in 32-bit (inc)
    subl     %eax,%ebx                        #Dx
    jg       noch132
    movl     $-4,%ecx                        #(!)size of one pixel in 32-bit (dec)
    negl     %ebx
noch132:
    movl     %ebx,(_et_dx)
    movl     (_et_y1),%eax
    movl     (_et_y2),%ebx
    movl     $2048,%edx                #(!)size of line (512 pixels in 32-bit)
    subl     %eax,%ebx                     #Dy
    jg       noch232
    movl     $-2048,%edx                #(!)size of line (512 pixels in 32-bit)
    negl     %ebx
noch232:
    movl     %ebx,(_et_dy)
    movl     (_et_col1),%eax        #color 1
    movl     (_et_col2),%esi        #color 2
    cmpl     (_et_dx),%ebx
    jg       other_line32
    movl     (_et_dx),%ebx
    movl     (_et_dx),%ebp
    shrl     $1,%ebx
bcl132:
    addl     (_et_dy),%ebx
    addl     %ecx,%edi
    cmpl     (_et_dx),%ebx
    jle      nomodif132
    subl     (_et_dx),%ebx
    addl     %edx,%edi
nomodif132:
    movl     %esi,-2048(%edi)        #(!)put one pixel
    movl     %eax,(%edi)                    #(!)put one pixel
    movl     %esi,2048(%edi)                         #(!)put one pixel
    decl     %ebp
    jg       bcl132
    popal
    ret
# ---------- ---------- ---------- ---------- #
other_line32:
    movl     (_et_dy),%ebx
    movl     (_et_dy),%ebp
    shrl     $1,%ebx
bcl232:
    addl     (_et_dx),%ebx
    addl     %edx,%edi
    cmpl     (_et_dy),%ebx
    jle      nomodif232
    subl     (_et_dy),%ebx
    addl     %ecx,%edi
nomodif232:
    movl     %esi,-4(%edi)                                     #(!)put one pixel
    movl     %eax,(%edi)                                       #(!)put one pixel
    movl     %esi,4(%edi)                                      #(!)put one pixel
    decl     %ebp
    jg       bcl232
    popal
    ret
# ---------- ---------- ---------- ---------- #
rnd32:
    pushl    %eax
    pushl    %ebx
    movl     (_et_r1),%eax
    movl     (_et_r2),%ebx
    movl     (_et_r3),%ebp
    roll     $13,%eax
    xorl     %ebp,%eax
    xorl     %ebp,%ebx
    rorl     $7,%ebx
    addl     $27,%ebp
    xorl     %eax,%ebp
    xorl     %ebx,%ebp
    movl     %ebp,(_et_r1)
    movl     %eax,(_et_r2)
    movl     %ebx,(_et_r3)
    popl     %ebx
    popl     %eax
    ret

##################################################################
#data for draw_electrical_shock
.data
_et_adr:        .int  0
_et_x1:         .int 50
_et_y1:         .int 50
_et_x2:         .int 150
_et_y2:         .int 150
_et_dx:         .int 0
_et_dy:         .int 0
_et_r1:         .int 0
_et_r2:         .int 0
_et_r3:         .int 0
_et_col1:       .int 0    #11
_et_col2:       .int 0    #13
#_et_col1:       .byte 11
#_et_col2:       .byte 13
