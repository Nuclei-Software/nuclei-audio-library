#ifndef _MATH_32_H_
#define _MATH_32_H_

#include "typedef.h"
#include "basop32.h"

__STATIC_FORCEINLINE_DSP Word32 Mult_32_16(Word32 a, Word16 b);
__STATIC_FORCEINLINE_DSP Word32 Madd_32_16(Word32 L_num, Word32 a, Word16 b);
__STATIC_FORCEINLINE_DSP Word32 Msub_32_16(Word32 L_num, Word32 a, Word16 b);
__STATIC_FORCEINLINE_DSP Word32 Mult_32_32(Word32 a, Word32 b);

#if defined(SUPPORT_DSP_STD)
__STATIC_FORCEINLINE_DSP Word32 Mult_32_16(Word32 a, Word16 b) {
    return __RV_KMMWB2(a, b);
}

__STATIC_FORCEINLINE_DSP Word32 Madd_32_16(Word32 L_num, Word32 a, Word16 b) {
    return L_add(L_num, __RV_KMMWB2(a, b));
}

__STATIC_FORCEINLINE_DSP Word32 Msub_32_16(Word32 L_num, Word32 a, Word16 b) {
    return L_sub(L_num, __RV_KMMWB2(a, b));
}

__STATIC_FORCEINLINE_DSP Word32 Mult_32_32(Word32 a, Word32 b) {
    return __RV_KWMMUL(a, b);
}
#endif /* #if defined(SUPPORT_DSP_STD) */

#endif /* ifndef _MATH_32_H_ */
