#ifndef FIXED_RISCV_H
#define FIXED_RISCV_H

#include "evalsoc.h"

#if defined(__riscv_xxldsp)

// #define PSHR32(a, shift) (SHR32((a) + ((EXTEND32(1) << ((shift)) >> 1)),
// shift))
#undef PSHR32
static inline int32_t PSHR32(int32_t a, uint16_t shift)
{
    return __RV_SRA_U(a, shift);
}

// #define VSHR32(a, shift) (((shift) > 0) ? SHR32(a, shift) : SHL32(a,
// -(shift)))
#undef VSHR32
static inline int32_t VSHR32(int32_t a, int32_t shift)
{
    return __RV_KSLRAW(a, -shift);
}

// #define MULT16_16_Q15(a, b) (SHR(MULT16_16((a), (b)), 15))
#undef MULT16_16_Q15
static inline int16_t MULT16_16_Q15(int16_t a, int16_t b)
{
    return __RV_KHMBB(a, b);
}

#endif

#endif /*FIXED_RISCV_H*/