#ifndef FIXED_RISCV_H
#define FIXED_RISCV_H

#include "evalsoc.h"

#if defined(__riscv_xxldsp)

#undef SATURATE16
static inline int16_t SATURATE16(int32_t x)
{
    return (int16_t)__RV_SCLIP32(x, 15);
}

#undef MAX32
#define MAX32(a, b) (__RV_MAXW(a, b))

#undef MIN32
#define MIN32(a, b) (__RV_MINW(a, b))

#undef VSHR32
#define VSHR32(a, shift) (__RV_KSLRAW(a, -shift))

#undef MULT16_16_Q15
#define MULT16_16_Q15(a, b) (__RV_KHMBB(a, b))

#undef PSHR32
#define PSHR32(a, shift) (__RV_SRA_U(a, shift))

#endif /* defined(__riscv_xxldsp) */

#endif /* FIXED_C6X_H */