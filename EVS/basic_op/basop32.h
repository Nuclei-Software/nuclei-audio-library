/*
  ===========================================================================
   File: BASOP32.H                                       v.2.3 - 30.Nov.2009
  ===========================================================================

            ITU-T STL  BASIC OPERATORS

            GLOBAL FUNCTION PROTOTYPES

   History:
   26.Jan.00   v1.0     Incorporated to the STL from updated G.723.1/G.729 
                        basic operator library (based on basic_op.h) and 
                        G.723.1's basop.h.
   05.Jul.00   v1.1     Added 32-bit shiftless mult/mac/msub operators

   03 Nov 04   v2.0     Incorporation of new 32-bit / 40-bit / control
                        operators for the ITU-T Standard Tool Library as 
                        described in Geneva, 20-30 January 2004 WP 3/16 Q10/16
                        TD 11 document and subsequent discussions on the
                        wp3audio@yahoogroups.com email reflector.
                        norm_s()      weight reduced from 15 to 1.
                        norm_l()      weight reduced from 30 to 1.
                        L_abs()       weight reduced from  2 to 1.
                        L_add()       weight reduced from  2 to 1.
                        L_negate()    weight reduced from  2 to 1.
                        L_shl()       weight reduced from  2 to 1.
                        L_shr()       weight reduced from  2 to 1.
                        L_sub()       weight reduced from  2 to 1.
                        mac_r()       weight reduced from  2 to 1.
                        msu_r()       weight reduced from  2 to 1.
                        mult_r()      weight reduced from  2 to 1.
                        L_deposit_h() weight reduced from  2 to 1.
                        L_deposit_l() weight reduced from  2 to 1.
                        L_mls() weight of 5.
                        div_l() weight of 32.
                        i_mult() weight of 3.

   30 Nov 09   v2.3     round() function is now round_fx().
                        saturate() is not referencable from outside application

   13 Mar 12            Add Overflow2 flag for additional overflow checking.
  ============================================================================
*/


#ifndef _BASIC_OP_H
#define _BASIC_OP_H

/* #define BASOP_OVERFLOW2 */

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/

#include "macro.h"
#include "typedef.h"
#include <stdint.h>

extern Flag Overflow, Overflow2;
extern Flag Carry;

#define BASOP_SATURATE_WARNING_ON
#define BASOP_SATURATE_WARNING_OFF
#define BASOP_SATURATE_ERROR_ON
#define BASOP_SATURATE_ERROR_OFF
#define BASOP_CHECK()


#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

#if defined(SUPPORT_DSP_STD)
#define STATIC_INLINE static inline
#else
#define STATIC_INLINE
#endif

// #define CHECK_OVERFLOW

#if defined(CHECK_OVERFLOW)
#define CHECK_OV()                                                             \
    do {                                                                       \
        if (__RV_RDOV()) {                                                     \
            Overflow = 1;                                                      \
            __RV_CLROV();                                                      \
        }                                                                      \
    } while (0)
#else
#define CHECK_OV()
#endif

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

STATIC_INLINE Word16 add (Word16 var1, Word16 var2);    /* Short add,           1   */
STATIC_INLINE Word16 sub (Word16 var1, Word16 var2);    /* Short sub,           1   */
STATIC_INLINE Word16 abs_s (Word16 var1);               /* Short abs,           1   */
STATIC_INLINE Word16 shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
STATIC_INLINE Word16 shr (Word16 var1, Word16 var2);    /* Short shift right,   1   */
STATIC_INLINE Word16 mult (Word16 var1, Word16 var2);   /* Short mult,          1   */
STATIC_INLINE Word32 L_mult (Word16 var1, Word16 var2); /* Long mult,           1   */
// static inline Word16 negate (Word16 var1);              /* Short negate,        1   */
// static inline Word16 extract_h (Word32 L_var1);         /* Extract high,        1   */
// static inline Word16 extract_l (Word32 L_var1);         /* Extract low,         1   */
STATIC_INLINE Word16 round_fx (Word32 L_var1);          /* Round,               1   */
STATIC_INLINE Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2);   /* Mac,  1  */
STATIC_INLINE Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2);   /* Msu,  1  */
Word32 L_macNs (Word32 L_var3, Word16 var1, Word16 var2); /* Mac without
                                                             sat, 1   */
Word32 L_msuNs (Word32 L_var3, Word16 var1, Word16 var2); /* Msu without
                                                             sat, 1   */
STATIC_INLINE Word32 L_add (Word32 L_var1, Word32 L_var2);    /* Long add,        1 */
STATIC_INLINE Word32 L_sub (Word32 L_var1, Word32 L_var2);    /* Long sub,        1 */
Word32 L_add_c (Word32 L_var1, Word32 L_var2);  /* Long add with c, 2 */
Word32 L_sub_c (Word32 L_var1, Word32 L_var2);  /* Long sub with c, 2 */
// static inline Word32 L_negate (Word32 L_var1);                /* Long negate,     1 */
STATIC_INLINE Word16 mult_r (Word16 var1, Word16 var2);       /* Mult with round, 1 */
STATIC_INLINE Word32 L_shl (Word32 L_var1, Word16 var2);      /* Long shift left, 1 */
STATIC_INLINE Word32 L_shr (Word32 L_var1, Word16 var2);      /* Long shift right, 1 */
STATIC_INLINE Word16 shr_r (Word16 var1, Word16 var2);        /* Shift right with
                                                   round, 2           */
STATIC_INLINE Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2); /* Mac with
                                                           rounding, 1 */
STATIC_INLINE Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2); /* Msu with
                                                           rounding, 1 */
// static inline Word32 L_deposit_h (Word16 var1);        /* 16 bit var1 -> MSB,     1 */
// static inline Word32 L_deposit_l (Word16 var1);        /* 16 bit var1 -> LSB,     1 */

STATIC_INLINE Word32 L_shr_r (Word32 L_var1, Word16 var2); /* Long shift right with
                                                round,             3  */
STATIC_INLINE Word32 L_abs (Word32 L_var1);            /* Long abs,              1  */
Word32 L_sat (Word32 L_var1);            /* Long saturation,       4  */
STATIC_INLINE Word16 norm_s (Word16 var1);             /* Short norm,            1  */
STATIC_INLINE Word16 div_s (Word16 var1, Word16 var2); /* Short division,       18  */
STATIC_INLINE Word16 norm_l (Word32 L_var1);           /* Long norm,             1  */


/*
 * Additional G.723.1 operators
*/
Word32 L_mls( Word32, Word16 ) ;    /* Weight FFS; currently assigned 5 */
Word16 div_l( Word32, Word16 ) ;    /* Weight FFS; currently assigned 32 */
Word16 i_mult(Word16 a, Word16 b);  /* Weight FFS; currently assigned 3 */

/*
 *  New shiftless operators, not used in G.729/G.723.1
*/
// static inline Word32 L_mult0(Word16 v1, Word16 v2); /* 32-bit Multiply w/o shift         1 */
STATIC_INLINE Word32 L_mac0(Word32 L_v3, Word16 v1, Word16 v2); /* 32-bit Mac w/o shift  1 */
STATIC_INLINE Word32 L_msu0(Word32 L_v3, Word16 v1, Word16 v2); /* 32-bit Msu w/o shift  1 */

static inline Word16 negate(Word16 var1) {
    Word16 var_out;
    var_out = (var1 == MIN_16) ? MAX_16 : -var1;

#if (WMOPS)
    multiCounter[currCounter].negate++;
#endif
    BASOP_CHECK();

    return (var_out);
}

static inline Word32 L_negate(Word32 L_var1) {
    Word32 L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;

#if (WMOPS)
    multiCounter[currCounter].L_negate++;
#endif
    BASOP_CHECK();
    return L_var_out;
}

static inline Word32 L_deposit_h(Word16 var1) {
    Word32 L_var_out;
    L_var_out = (Word32)var1 << 16;

#if (WMOPS)
    multiCounter[currCounter].L_deposit_h++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

static inline Word32 L_deposit_l(Word16 var1) {
    Word32 L_var_out;
    L_var_out = (Word32)var1;

#if (WMOPS)
    multiCounter[currCounter].L_deposit_l++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

static inline Word16 extract_h(Word32 L_var1) {
    Word16 var_out;

    var_out = (Word16) ((uint32_t)L_var1 >> 16);

#if (WMOPS)
    multiCounter[currCounter].extract_h++;
#endif
    BASOP_CHECK();

    return (var_out);
}

static inline Word16 extract_l(Word32 L_var1) {
    Word16 var_out;
    var_out = (Word16)L_var1;

#if (WMOPS)
    multiCounter[currCounter].extract_l++;
#endif
    BASOP_CHECK();

    return (var_out);
}

static inline Word32 L_mult0(Word16 var1, Word16 var2) {
    Word32 L_var_out;

    L_var_out = (Word32)var1 * (Word32)var2;

#if (WMOPS)
    multiCounter[currCounter].L_mult0++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

#if defined(SUPPORT_DSP_STD)

static inline Word16 saturate(Word32 L_var1) {
    Word16 var_out = __RV_SCLIP32((L_var1), 15);
    CHECK_OV();
    return (var_out);
}

STATIC_INLINE Word16 add(Word16 var1, Word16 var2) {
    Word16 var_out;
    var_out = __RV_KADD16(var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].add++;
#endif

    return (var_out);
}

STATIC_INLINE Word16 sub(Word16 var1, Word16 var2) {
    Word16 var_out;
    var_out = __RV_KSUB16(var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].sub++;
#endif

    return (var_out);
}

STATIC_INLINE Word16 abs_s(Word16 var1) {
    Word16 var_out;
    var_out = __RV_KABS16(var1);

#if (WMOPS)
    multiCounter[currCounter].abs_s++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 shl(Word16 var1, Word16 var2) {
    Word16 var_out;
    var2 = __RV_SCLIP32(var2, 4);
    var_out = __RV_KSLRA16(var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].shl++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 shr(Word16 var1, Word16 var2) {
    Word16 var_out;
    var2 = __RV_SCLIP32(-var2, 4);
    var_out = __RV_KSLRA16(var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].shr++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 mult(Word16 var1, Word16 var2) {
    Word16 var_out;
    var_out = __RV_KHM16(var1, var2);

#if (WMOPS)
    multiCounter[currCounter].mult++;
#endif

    return (var_out);
}

STATIC_INLINE Word32 L_mult(Word16 var1, Word16 var2) {
    Word32 L_var_out;
    L_var_out = __RV_KDMBB(var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_mult++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word16 round_fx(Word32 L_var1) {
    Word16 var_out;
	L_var1 = __RV_KSLRAW_U(L_var1, -16);
    var_out = __RV_SCLIP32(L_var1, 15);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_add--;
    multiCounter[currCounter].extract_h--;
    multiCounter[currCounter].round++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word32 L_mac(Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    L_var_out = __RV_KDMABB(L_var3, var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_mult--;
    multiCounter[currCounter].L_add--;
    multiCounter[currCounter].L_mac++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    Word32 L_product;

    L_product = L_mult (var1, var2);
    L_var_out = L_sub (L_var3, L_product);

#if (WMOPS)
    multiCounter[currCounter].L_mult--;
    multiCounter[currCounter].L_sub--;
    multiCounter[currCounter].L_msu++;
#endif

    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word32 L_add(Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;
    L_var_out = __RV_KADDW(L_var1, L_var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_add++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word32 L_sub(Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;
    L_var_out = __RV_KSUBW(L_var1, L_var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_sub++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word16 mult_r(Word16 var1, Word16 var2) {
    // KHMBB shift right without rouding, so not capable to replace
    Word16 var_out;
    Word32 a = (Word32)var1 * (Word32)var2;
    Word32 result = __RV_SRA_U(a, 15);
    var_out = saturate(result);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].mult_r++;
#endif

    return (var_out);
}

STATIC_INLINE Word32 L_shl(Word32 L_var1, Word16 var2) {
    long shift = __RV_SCLIP32(var2, 5);
    // TODO(jdqiu): replace with __RV_CLROV()
    __builtin_riscv_clrov();
    Word32 L_var_out = __RV_KSLRAW(L_var1, shift);
    if (__RV_RDOV()) {
        Overflow = 1;
    }

#if (WMOPS)
    multiCounter[currCounter].L_shl++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word32 L_shr(Word32 L_var1, Word16 var2) {
    Word32 L_var_out;
    var2 = __RV_SCLIP32(-var2, 5);
    L_var_out = __RV_KSLRAW(L_var1, var2);

#if (WMOPS)
    multiCounter[currCounter].L_shr++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word16 shr_r(Word16 var1, Word16 var2) {
    Word16 var_out;
    var2 = __RV_SCLIP32(-var2, 4);
    var_out = __RV_KSLRA16_U(var1, var2);

#if (WMOPS)
    multiCounter[currCounter].shr_r++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 mac_r(Word32 L_var3, Word16 var1, Word16 var2) {
    Word16 var_out;
    L_var3 = __RV_KDMABB(L_var3, var1, var2);
    long result = __RV_SRA_U(L_var3, 16);
    var_out = (Word16)result;
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_mac--;
    multiCounter[currCounter].L_add--;
    multiCounter[currCounter].extract_h--;
    multiCounter[currCounter].mac_r++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 msu_r(Word32 L_var3, Word16 var1, Word16 var2) {
    Word16 var_out;
    L_var3 = L_msu(L_var3, var1, var2);
    long result = __RV_SRA_U(L_var3, 16);
    var_out = (Word16)result;
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_msu--;
    multiCounter[currCounter].L_add--;
    multiCounter[currCounter].extract_h--;
    multiCounter[currCounter].msu_r++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word32 L_shr_r(Word32 L_var1, Word16 var2) {
    Word32 L_var_out;
    if (var2 > 31) {
        L_var_out = 0;
    } else {
        L_var_out = __RV_KSLRAW_U(L_var1, -var2);
    }

#if (WMOPS)
    multiCounter[currCounter].L_shr_r++;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

STATIC_INLINE Word32 L_abs(Word32 L_var1) {
    Word32 L_var_out;
    L_var_out = __RV_KABSW(L_var1);

#if (WMOPS)
    multiCounter[currCounter].L_abs++;
#endif
    BASOP_CHECK();
    return L_var_out;
}

STATIC_INLINE Word16 norm_s(Word16 var1) {
    Word16 var_out;
    if (var1 == 0) {
        var_out = 0;
    } else {
        var_out = __RV_CLRS16(var1);
    }

#if (WMOPS)
    multiCounter[currCounter].norm_s++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word16 div_s(Word16 var1, Word16 var2) {
    Word32 var_out;
    var_out = (var1 << 15) / var2;
    var_out = __RV_SCLIP32(var_out, 15);

#if (WMOPS)
    multiCounter[currCounter].div_s++;
#endif
    BASOP_CHECK();
    return (Word16)var_out;
}

STATIC_INLINE Word16 norm_l(Word32 L_var1) {
    Word16 var_out;
    if (L_var1 == 0) {
        var_out = 0;
    } else {
        var_out = __RV_CLRS32(L_var1);
    }

#if (WMOPS)
    multiCounter[currCounter].norm_l++;
#endif
    BASOP_CHECK();

    return (var_out);
}

STATIC_INLINE Word32 L_mac0(Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    L_var_out = __RV_KMABB(L_var3, var1, var2);
    CHECK_OV();

#if (WMOPS)
    multiCounter[currCounter].L_mac0++;
    multiCounter[currCounter].L_mult0--;
    multiCounter[currCounter].L_add--;
#endif
    BASOP_CHECK();
    return (L_var_out);
}

STATIC_INLINE Word32 L_msu0(Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    L_var_out = L_sub(L_var3, L_mult0(var1, var2));

#if (WMOPS)
    multiCounter[currCounter].L_msu0++;
    multiCounter[currCounter].L_mult0--;
    multiCounter[currCounter].L_sub--;
#endif
    BASOP_CHECK();

    return (L_var_out);
}

#endif // #if defined(SUPPORT_DSP_STD)

#endif /* ifndef _BASIC_OP_H */

/* end of file */
