# custom, minimal startup for C code
# (c)2011 Mariusz Buras (all), Pawel Goralski (modifications for C)

REDIRECT_OUTPUT_TO_SERIAL   = 0    /*0-output to console,1-output to serial port*/

        #.globl    _exit
        .globl    _memcpy
        .globl    _strlen
        .globl    _strstr
        .globl    ___mulsi3
        .globl    ___udivsi3
        .globl    ___umodsi3
        .globl    ___modsi3
        .globl    ___divsi3

 /* --------------------------------------------------------------*/
_memcpy:
        move.l  d2,-(sp)
        move.l  4+4(sp),a0       /*a0=source*/
        move.l  4+8(sp),a1       /*a1=dest*/
        move.l  4+12(sp),d2      /*d2=length*/

        cmp.l   #16,d2
        blo.b   .inf16

        move.l  a0,d0
        move.l  a1,d1
        and.b   #1,d0
        and.b   #1,d1
        cmp.b   d0,d1
        bne.b   .oddcp

        move.l  a0,d0
        tst.b   d1
        beq.b   .alig
             /*both addr are odd*/
        move.b  (a1)+,(a0)+
        subq.l  #1,d2
.alig:
        moveq   #3,d1
        and.l   d2,d1
        sub.l   d1,d2
 /*=barrier*/
.c:
        move.l  (a1)+,(a0)+
        subq.l  #4,d2
        bne.b   .c
        move.w  d1,d2
        bra.b   .remai

.oddcp: cmp.l   #0x10000,d2
        blo.b   .inf16
        move.l  a0,d0
.c2:
        move.b  (a1)+,(a0)+
        subq.l  #1,d2
        bne.b   .c2
        bra.b   .end

.inf16:
        move.l  a0,d0
.remai:
        subq.w  #1,d2
        blo.b   .end
.c3:
        move.b  (a1)+,(a0)+
        dbf     d2,.c3

.end:
        move.l  (sp)+,d2
        rts

 /* -------------------------------------------------------------- */

 _strcpy:
         move.l    4(sp), a0
         move.l    8(sp), a1

 l1:     move.b    (a1)+, (a0)+
         bne.s     l1
         rts

 /* --------------------------------------------------------------  */

_strlen:
        move.l    4(sp), a0
        moveq     #0,d0
        bra.s     .l1
.l2:    addq.l    #1,d0
.l1:    tst.b     (a0)+
        bne.s     .l2
        rts

 /* -------------------------------------------------------------- */

_strstr:
 /* this code is 52 bytes bigger than the c implementation. But it is way faster. */
 /*from http: cristi.indefero.net/p/uClibc-cristi/source/tree/ff250619f58caa6b10c951911c43fbb8a34fda8d/libc/string/strstr.c */
            movem.l 4(sp),a0-a1
            movem.l d2/a2-a3,-(sp)
 /*a0: aystack */
 /*a1: needle (>=1 char string) */

            move.b  (a1),d1              /* d1: neddle0 */
            beq.s   .ret1
            subq.l  #1,a0
.do1:       addq.l  #1,a0
            move.b  (a0),d2              /* d2: haystack0 */
            beq.s   .ret0
            cmp.b   d2,d1
            bne.s   .do1

            addq.l  #1,a1
            move.b  (a1)+,d2             /*d2: needle1 */
            beq.s   .ret1
            bra.s   .jin

.for:
.do2:       addq.l  #1,a0
            move.b  (a0),d0              /*d0: haystack1 */
            beq.s   .ret0
            cmp.b   d0,d1
            beq.s   .do2break
            addq.l  #1,a0
            move.b  (a0),d0
            beq.s   .ret0
.shloop:    cmp.b   d0,d1
            bne.s   .do2

.do2break:
.jin:       addq.l  #1,a0
            move.b  (a0),d0
            beq.s   .ret0
            cmp.b   d0,d2
            bne.s   .shloop

            move.l  a0,a2                /*a2: rhaystack */
            addq.l  #1,a2
            subq.l  #1,a0
            move.l  a1,a3                /*a3: rneedle */
            move.b  (a3),d0

            cmp.b   (a2),d0
            bne.s   .endif
.do3:       tst.b   d0
            beq.s   .ret1
            addq.l  #1,a1
            move.b  (a1),d0
            addq.l  #1,a2
            cmp.b   (a2),d0
            bne.s   .do3break
            tst.b   d0
            beq.s   .ret1
            addq.l  #1,a1
            move.b  (a1),d0
            addq.l  #1,a2
            cmp.b   (a2),d0
            beq.s   .do3
.do3break:
.endif:      move.l  a3,a1
            tst.b   d0
            bne.s   .for

.ret1:       move.l   a0,d0
            bra.s   .ret
.ret0:       moveq   #0,d0
.ret:        movem.l (sp)+,d2/a2-a3
            rts

 /* -------------------------------------------------------------- */

___mulsi3:
 /* from http: upx-ucl-3.08/src/stub/src/m68k-atari.tos.S */
     /* compute high-word */
        lea     4(sp),a0             /*                    8 */
        move.w  (a0)+,d1             /*                    8 */
        move.w  (a0)+,d0             /*                    8 */
        mulu.w  (a0)+,d0             /*                   74 */
        mulu.w  (a0),d1              /*                   74 */
        add.w   d1,d0                /*                    4 */
        swap    d0                   /*                    4 */
        clr.w   d0                   /*                    4 */
     /*add low-word  */
        move.w  6(sp),d1             /*                   12 */
        mulu.w  (a0),d1              /*                   74 */
        add.l   d1,d0                /*                    6 */
        rts                          /*                  274 */


 /* -------------------------------------------------------------- */

 /*from http: svn.opentom.org/opentom/trunk/linux-2.6/arch/m68knommu/lib/umodsi3.S */
___umodsi3:
    move.l  8(sp),d1         /*d1 = divisor */
    move.l  4(sp),d0         /*d0 = dividend */
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    bsr.s   ___udivsi3
                             /*delayed popping: 8 */
    move.l  8+8(sp),-(sp)    /* d1 = divisor */
    move.l  d0,-(sp)
    bsr.s   ___mulsi3        /* d0 = (a/b)*b */
    lea     16(sp),sp        /* delayed popping: 0 */
    move.l  4(sp),d1         /* d1 = dividend */
    sub.l   d0,d1            /* d1 = a - (a/b)*b */
    move.l  d1,d0
    rts

 /* -------------------------------------------------------------- */

 /*from http: svn.opentom.org/opentom/trunk/linux-2.6/arch/m68knommu/lib/modsi3.S */
 /*(modified) */
___modsi3:
    move.l  8(sp),d1         /* d1 = divisor */
    move.l  4(sp),d0         /* d0 = dividend */
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    bsr.s   ___divsi3
                             /* delayed popping: 8 */
    move.l  8+8(sp),-(sp)    /* divisor */
    move.l  d0,-(sp)
    bsr.s   ___mulsi3        /* d0 = (a/b)*b */
    lea     16(sp),sp        /* delayed popping: 0 */
    move.l  4(sp), d1        /* d1 = dividend */
    sub.l   d0,d1            /* d1 = a - (a/b)*b */
    move.l  d1,d0
    rts

 /* -------------------------------------------------------------- */

 /*from http: www.koders.com/noncode/fid355C9167E5496B5F863EAEB5758B4236711466D2.aspx */
___udivsi3:
 /*#if !(defined(__mcf5200__) || defined(__mcoldfire__)) */
    move.l  d2,-(sp)
    move.l  4+8(sp),d1       /* d1 = divisor */
    move.l  4+4(sp),d0       /* d0 = dividend */

    cmp.l   #0x10000,d1       /* divisor >= 2 ^ 16 ?   */
    bcc.s   L3               /* then try next algorithm */
    move.l  d0,d2
    clr.w   d2
    swap    d2
    divu    d1,d2            /* high quotient in lower word */
    move.w  d2,d0            /* save high quotient */
    swap    d0
    move.w  4+6(sp),d2       /* get low dividend + high rest */
    divu    d1,d2            /* low quotient */
    move.w  d2,d0
    bra.s   L6

L3: move.l  d1,d2            /* use d2 as divisor backup */
L4: lsr.l   #1,d1            /* shift divisor */
    lsr.l   #1,d0            /* shift dividend */
    cmp.l   #0x10000,d1       /* still divisor >= 2 ^ 16 ?  */
    bcc.s   L4
    divu    d1,d0            /* now we have 16 bit divisor */
    and.l   #0xffff,d0        /* mask out divisor, ignore remainder */

 /* Multiply the 16 bit tentative quotient with the 32 bit divisor.  Because of */
 /*   the operand ranges, this might give a 33 bit product.  If this product is */
 /*   greater than the dividend, the tentative quotient was too large. */
    move.l  d2,d1
    mulu    d0,d1            /* low part, 32 bits */
    swap    d2
    mulu    d0,d2            /* high part, at most 17 bits */
    swap    d2               /* align high part with low part */
    tst.w   d2               /* high part 17 bits? */
    bne.s   L5               /* if 17 bits, quotient was too large */
    add.l   d2,d1            /* add parts */
    bcs.s   L5               /* if sum is 33 bits, quotient was too large */
    cmp.l   4+4(sp),d1       /* compare the sum with the dividend */
    bls.s   L6               /* if sum > dividend, quotient was too large */
L5: subq.l  #1,d0            /* adjust quotient */

L6: move.l  (sp)+,d2
    rts

 /* -------------------------------------------------------------- */

 /* from http: svn.opentom.org/opentom/trunk/linux-2.6/arch/m68knommu/lib/divsi3.S */
___divsi3:
    move.l  d2,-(sp)

    moveq   #1,d2            /* sign of result stored in d2 (=1 or =-1) */
    move.l  4+8(sp),d1       /* d1 = divisor */
    bpl.s   L1
    neg.l   d1
    neg.b   d2               /* change sign because divisor <0  */
L1: move.l  4+4(sp),d0       /* d0 = dividend */
    bpl.s   l2
    neg.l   d0
    neg.b   d2

l2:
    move.l  d1, -(sp)
    move.l  d0, -(sp)
    bsr.s   ___udivsi3       /* divide abs(dividend) by abs(divisor) */
    addq.l  #8,sp

    tst.b   d2
    bpl.s   l3
    neg.l   d0

l3: move.l  (sp)+,d2
    rts
