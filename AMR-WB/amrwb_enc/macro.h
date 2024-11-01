#ifndef MACRO_H
#define MACRO_H

#if defined(__riscv_dsp)
#include "evalsoc.h"
#include "nmsis_core.h"

#if defined(__riscv_xxldspn3x)
#define SUPPORT_DSP_N3X
#define SUPPORT_DSP_N2X
#define SUPPORT_DSP_N1X
#define SUPPORT_DSP_STD
#elif defined(__riscv_xxldspn2x)
#define SUPPORT_DSP_N2X
#define SUPPORT_DSP_N1X
#define SUPPORT_DSP_STD
#elif defined(__riscv_xxldspn1x)
#define SUPPORT_DSP_N1X
#define SUPPORT_DSP_STD
#endif // defined(__riscv_xxldspn3x)

#endif // defined(__riscv_dsp)

#if defined(__riscv_vector)
#include <riscv_vector.h>

#if defined(__riscv_v) || defined(__riscv_zve64d)
#define SUPPORT_VEC_64D
#define SUPPORT_VEC_64F
#define SUPPORT_VEC_64X
#define SUPPORT_VEC_32F
#define SUPPORT_VEC_32X
#elif defined(__riscv_zve64f)
#define SUPPORT_VEC_64F
#define SUPPORT_VEC_64X
#define SUPPORT_VEC_32F
#define SUPPORT_VEC_32X
#elif defined(__riscv_zve64x)
#define SUPPORT_VEC_64X
#define SUPPORT_VEC_32X
#elif defined(__riscv_zve32f)
#define SUPPORT_VEC_32F
#define SUPPORT_VEC_32X
#elif defined(__riscv_zve32x)
#define SUPPORT_VEC_32X
#endif // defined(__riscv_v) || defined(__riscv_zve64d)

#endif // defined(__riscv_vector)

#endif