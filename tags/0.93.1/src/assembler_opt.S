/** 
 * @file assembler_opt.S 
 * @brief Optimized routines of displaying 
 * @created 1999-09-06 
 * @date 2007-07-09
 * @author Jean-Michel Martin de Santero
 * @author Bruno Ethvignot
 */
/* 
 * copyright (c) 1998-2015 TLK Games all rights reserved
 * $Id: assembler_opt.S,v 1.2 2007/07/09 20:21:36 gurumeditation Exp $
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

#------------------------------------------------------------------------------
# copie une zone de 256*184 d un ecran de 512*440 dans un ecran de 640*400 
# en doublant les pixels horizontalement et en sautant une ligne sur deux
#entree => _pSource                :    adresse source
#                => _pDestination     : adresse destination
#                => _iHauteur            : 184 ligne
#------------------------------------------------------------------------------
#(8 bits)######################################################################
.global _COPY2X8BITS_512x440
.type     _COPY2X8BITS_512x440,@function
_COPY2X8BITS_512x440:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    movl    8(%ebp),%esi                                      #esi=adresse source (_pSource)
    movl    12(%ebp),%edi                                     #edi=adresse destination (_pDestination)
    movl    16(%ebp),%edx
    xorl    %ebx, %ebx
xbcl2:
    movl    $128,%ecx                                         #128*4=512
xbcl1:
    movw    (%esi),%ax
    addl    $2,%esi
    movb    %al,%bl
    movb    %ah,%al
    shll    $16,%eax
    movb    %bl,%bh
    addl    %ebx,%eax
    addl    $4,%edi
    movl    %eax,-4(%edi)
    decl    %ecx
    jnz     xbcl1
    lea     256(%esi) , %esi
    lea     128+640(%edi), %edi
    decl    %edx
    jnz     xbcl2
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
    
#(16 bits)#####################################################################
.global _COPY2X16BITS_512x440
.type     _COPY2X16BITS_512x440,@function
_COPY2X16BITS_512x440:
    pushl %ebp
    movl    %esp, %ebp
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    movl    8(%ebp),%esi                                      #esi=adresse source (_pSource)
    movl    12(%ebp),%edi                                     #edi=adresse destination (_pDestination)
    movl    $(512*2)-(256*2),%ebx                             #ebx=offset sur la prochaine ligne source
    movl    $(640*2)-(512*2)+(640*2),%edx                     #edx=offset sur la prochaine ligne destination
440:
    movl    $8, %ecx                                          #8*32=256 pixels (largeur d une ligne)
441:

    movw    (%esi), %ax
    movw    %ax,(%edi)
    movw    %ax,2(%edi)
    movw    2(%esi), %ax
    movw    %ax,4(%edi)
    movw    %ax,6(%edi)
    movw    4(%esi), %ax
    movw    %ax,8(%edi)
    movw    %ax,10(%edi)
    movw    6(%esi), %ax
    movw    %ax,12(%edi)
    movw    %ax,14(%edi)
    movw    8(%esi), %ax
    movw    %ax,16(%edi)
    movw    %ax,18(%edi)
    movw    10(%esi), %ax
    movw    %ax,20(%edi)
    movw    %ax,22(%edi)
    movw    12(%esi), %ax
    movw    %ax,24(%edi)
    movw    %ax,26(%edi)
    movw    14(%esi), %ax
    movw    %ax,28(%edi)
    movw    %ax,30(%edi)

    movw    16(%esi), %ax
    movw    %ax,32(%edi)
    movw    %ax,34(%edi)
    movw    18(%esi), %ax
    movw    %ax,36(%edi)
    movw    %ax,38(%edi)
    movw    20(%esi), %ax
    movw    %ax,40(%edi)
    movw    %ax,42(%edi)
    movw    22(%esi), %ax
    movw    %ax,44(%edi)
    movw    %ax,46(%edi)
    movw    24(%esi), %ax
    movw    %ax,48(%edi)
    movw    %ax,50(%edi)
    movw    26(%esi), %ax
    movw    %ax,52(%edi)
    movw    %ax,54(%edi)
    movw    28(%esi), %ax
    movw    %ax,56(%edi)
    movw    %ax,58(%edi)
    movw    30(%esi), %ax
    movw    %ax,60(%edi)
    movw    %ax,62(%edi)

    movw    32(%esi), %ax
    movw    %ax,64(%edi)
    movw    %ax,66(%edi)
    movw    34(%esi), %ax
    movw    %ax,68(%edi)
    movw    %ax,70(%edi)
    movw    36(%esi), %ax
    movw    %ax,72(%edi)
    movw    %ax,74(%edi)
    movw    38(%esi), %ax
    movw    %ax,76(%edi)
    movw    %ax,78(%edi)
    movw    40(%esi), %ax
    movw    %ax,80(%edi)
    movw    %ax,82(%edi)
    movw    42(%esi), %ax
    movw    %ax,84(%edi)
    movw    %ax,86(%edi)
    movw    44(%esi), %ax
    movw    %ax,88(%edi)
    movw    %ax,90(%edi)
    movw    46(%esi), %ax
    movw    %ax,92(%edi)
    movw    %ax,94(%edi)

    movw    48(%esi), %ax
    movw    %ax,96(%edi)
    movw    %ax,98(%edi)
    movw    50(%esi), %ax
    movw    %ax,100(%edi)
    movw    %ax,102(%edi)
    movw    52(%esi), %ax
    movw    %ax,104(%edi)
    movw    %ax,106(%edi)
    movw    54(%esi), %ax
    movw    %ax,108(%edi)
    movw    %ax,110(%edi)
    movw    56(%esi), %ax
    movw    %ax,112(%edi)
    movw    %ax,114(%edi)
    movw    58(%esi), %ax
    movw    %ax,116(%edi)
    movw    %ax,118(%edi)
    movw    60(%esi), %ax
    movw    %ax,120(%edi)
    movw    %ax,122(%edi)
    movw    62(%esi), %ax
    movw    %ax,124(%edi)
    movw    %ax,126(%edi)

    addl    $64,%esi
    addl    $128,%edi
    dec     %ecx
    jnz     441b
    lea     (%esi, %ebx),%esi
    lea     (%edi, %edx),%edi
    decl    16(%ebp)
    jnz     440b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
#(24 bits)#####################################################################
.global _COPY2X24BITS_512x440
.type     _COPY2X24BITS_512x440,@function
_COPY2X24BITS_512x440:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    movl    8(%ebp),%esi                                      #esi=adresse source (_pSource)
    movl    12(%ebp),%edi                                     #edi=adresse destination (_pDestination)
    movl    $(512*3)-(256*3),%ebx                             #ebx=offset sur la prochaine ligne source
    movl    $(640*3)-(512*3)+(640*3),%edx                     #edx=offset sur la prochaine ligne destination
44024:
    movl    $32, %ecx                                         #32*8=256 pixels (largeur d une ligne)
44124:
    movw    (%esi),%ax                                        #
    movw    %ax,(%edi) 
    movw    %ax,3(%edi) 
    movb    2(%esi),%al
    movb    %al,2(%edi) 
    movb    %al,5(%edi) 
    movw    3(%esi),%ax                                       #
    movw    %ax,6(%edi) 
    movw    %ax,9(%edi) 
    movb    5(%esi),%al
    movb    %al,8(%edi) 
    movb    %al,11(%edi) 
    movw    6(%esi),%ax                                       #
    movw    %ax,12(%edi) 
    movw    %ax,15(%edi) 
    movb    8(%esi),%al
    movb    %al,14(%edi) 
    movb    %al,17(%edi) 
    movw    9(%esi),%ax                                       #
    movw    %ax,18(%edi) 
    movw    %ax,21(%edi) 
    movb    11(%esi),%al
    movb    %al,20(%edi) 
    movb    %al,23(%edi) 
    movw    12(%esi),%ax                                      #
    movw    %ax,24(%edi) 
    movw    %ax,27(%edi) 
    movb    14(%esi),%al
    movb    %al,26(%edi) 
    movb    %al,29(%edi) 
    movw    15(%esi),%ax                                      #
    movw    %ax,30(%edi) 
    movw    %ax,33(%edi) 
    movb    17(%esi),%al
    movb    %al,32(%edi) 
    movb    %al,35(%edi) 
    movw    18(%esi),%ax                                      #
    movw    %ax,36(%edi) 
    movw    %ax,39(%edi) 
    movb    20(%esi),%al
    movb    %al,38(%edi) 
    movb    %al,41(%edi) 
    movw    21(%esi),%ax                                      #
    movw    %ax,42(%edi) 
    movw    %ax,45(%edi) 
    movb    23(%esi),%al
    movb    %al,44(%edi) 
    movb    %al,47(%edi) 
    addl    $3*8,%esi
    addl    $6*8,%edi
    decl    %ecx
    jnz     44124b
    lea     (%esi, %ebx),%esi
    lea     (%edi, %edx),%edi
    decl    16(%ebp)
    jnz     44024b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret
#(32 bits)#####################################################################
.global _COPY2X32BITS_512x440
.type     _COPY2X32BITS_512x440,@function
_COPY2X32BITS_512x440:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    movl    8(%ebp),%esi                                      #esi=adresse source (_pSource)
    movl    12(%ebp),%edi                                     #edi=adresse destination (_pDestination)
    movl    $(512*4)-(256*4),%ebx                             #ebx=offset sur la prochaine ligne source
    movl    $(640*4)-(512*4)+(640*4),%edx                     #edx=offset sur la prochaine ligne destination
440:
    movl    $8, %ecx                                          #8*32=256 pixels (largeur d une ligne)
441:
    movl    (%esi), %eax
    movl    %eax,(%edi)
    movl    %eax,4(%edi)
    movl    4(%esi), %eax
    movl    %eax,8(%edi)
    movl    %eax,12(%edi)
    movl    8(%esi), %eax
    movl    %eax,16(%edi)
    movl    %eax,20(%edi)
    movl    12(%esi), %eax
    movl    %eax,24(%edi)
    movl    %eax,28(%edi)
    movl    16(%esi), %eax
    movl    %eax,32(%edi)
    movl    %eax,36(%edi)
    movl    20(%esi), %eax
    movl    %eax,40(%edi)
    movl    %eax,44(%edi)
    movl    24(%esi), %eax
    movl    %eax,48(%edi)
    movl    %eax,52(%edi)
    movl    28(%esi), %eax
    movl    %eax,56(%edi)
    movl    %eax,60(%edi)

    movl    32(%esi), %eax
    movl    %eax,64(%edi)
    movl    %eax,68(%edi)
    movl    36(%esi), %eax
    movl    %eax,72(%edi)
    movl    %eax,76(%edi)
    movl    40(%esi), %eax
    movl    %eax,80(%edi)
    movl    %eax,84(%edi)
    movl    44(%esi), %eax
    movl    %eax,88(%edi)
    movl    %eax,92(%edi)
    movl    48(%esi), %eax
    movl    %eax,96(%edi)
    movl    %eax,100(%edi)
    movl    52(%esi), %eax
    movl    %eax,104(%edi)
    movl    %eax,108(%edi)
    movl    56(%esi), %eax
    movl    %eax,112(%edi)
    movl    %eax,116(%edi)
    movl    60(%esi), %eax
    movl    %eax,120(%edi)
    movl    %eax,124(%edi)

    movl    64(%esi), %eax
    movl    %eax,128(%edi)
    movl    %eax,132(%edi)
    movl    68(%esi), %eax
    movl    %eax,136(%edi)
    movl    %eax,140(%edi)
    movl    72(%esi), %eax
    movl    %eax,144(%edi)
    movl    %eax,148(%edi)
    movl    76(%esi), %eax
    movl    %eax,152(%edi)
    movl    %eax,156(%edi)
    movl    80(%esi), %eax
    movl    %eax,160(%edi)
    movl    %eax,164(%edi)
    movl    84(%esi), %eax
    movl    %eax,168(%edi)
    movl    %eax,172(%edi)
    movl    88(%esi), %eax
    movl    %eax,176(%edi)
    movl    %eax,180(%edi)
    movl    92(%esi), %eax
    movl    %eax,184(%edi)
    movl    %eax,188(%edi)

    movl    96(%esi), %eax
    movl    %eax,192(%edi)
    movl    %eax,196(%edi)
    movl    100(%esi), %eax
    movl    %eax,200(%edi)
    movl    %eax,204(%edi)
    movl    104(%esi), %eax
    movl    %eax,208(%edi)
    movl    %eax,212(%edi)
    movl    108(%esi), %eax
    movl    %eax,216(%edi)
    movl    %eax,220(%edi)
    movl    112(%esi), %eax
    movl    %eax,224(%edi)
    movl    %eax,228(%edi)
    movl    116(%esi), %eax
    movl    %eax,232(%edi)
    movl    %eax,236(%edi)
    movl    120(%esi), %eax
    movl    %eax,240(%edi)
    movl    %eax,244(%edi)
    movl    124(%esi), %eax
    movl    %eax,248(%edi)
    movl    %eax,252(%edi)
    
    addl    $128,%esi
    addl    $256,%edi
    dec     %ecx
    jnz     441b
    lea     (%esi, %ebx),%esi
    lea     (%edi, %edx),%edi
    decl    16(%ebp)
    jnz     440b
    popl    %ebx
    popl    %edi
    popl    %esi
    leave
    ret

