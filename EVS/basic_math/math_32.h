#ifndef _MATH_32_H_
#define _MATH_32_H_

#include "typedef.h"
#include "basop32.h"

STATIC_INLINE Word32 Mult_32_16(Word32 a, Word16 b);
STATIC_INLINE Word32 Madd_32_16(Word32 L_num, Word32 a, Word16 b);
STATIC_INLINE Word32 Msub_32_16(Word32 L_num, Word32 a, Word16 b);
STATIC_INLINE Word32 Mult_32_32(Word32 a, Word32 b);

#if defined(SUPPORT_DSP_STD)
STATIC_INLINE Word32 Mult_32_16(Word32 a, Word16 b) {
    return __RV_KMMWB2(a, b);
}

STATIC_INLINE Word32 Madd_32_16(Word32 L_num, Word32 a, Word16 b) {
    return L_add(L_num, __RV_KMMWB2(a, b));
}

STATIC_INLINE Word32 Msub_32_16(Word32 L_num, Word32 a, Word16 b) {
    return L_sub(L_num, __RV_KMMWB2(a, b));
}

STATIC_INLINE Word32 Mult_32_32(Word32 a, Word32 b) {
    return __RV_KWMMUL(a, b);
}
#endif

#endif // #ifndef _MATH_32_H_
