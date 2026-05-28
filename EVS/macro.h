/**
 * @file macro.h
 * @brief riscv macros
 * @author Jiandong Qiu <qiujiandong@nucleisys.com>
 * @date 2026-01-08
 *
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#if defined(__riscv_dsp)

#include "evalsoc.h"

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
#elif defined(__riscv_xxldsp)
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

#if defined(__riscv_zvl1024b)
#define SUPPORT_VL1024
#define SUPPORT_VL512
#define SUPPORT_VL256
#elif defined(__riscv_zvl512b)
#define SUPPORT_VL512
#define SUPPORT_VL256
#elif defined(__riscv_zvl256b)
#define SUPPORT_VL256
#endif
// VLEN128 is alway supported
#define SUPPORT_VL128
#endif // defined(__riscv_vector)

#define L_MAC_N(sum, init, len, x, y)                                          \
    do {                                                                       \
        const Word16x2 *_px = (const Word16x2 *)(x);                           \
        const Word16x2 *_py = (const Word16x2 *)(y);                           \
        int _i = 0;                                                            \
        sum = init;                                                            \
        for (; _i + 8 <= len; _i += 8) {                                       \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
        }                                                                      \
        for (; _i + 4 <= len; _i += 4) {                                       \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
        }                                                                      \
        for (; _i + 2 <= len; _i += 2) {                                       \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMATT(sum, _px++->u32, _py++->u32);                    \
        }                                                                      \
        if (_i < len) {                                                        \
            sum = __RV_KDMABB(sum, _px->u32, _py->u32);                        \
        }                                                                      \
    } while (0)

#define L_MAC0_N(sum, init, len, x, y)                                         \
    do {                                                                       \
        const Word16x2 *_px = (const Word16x2 *)(x);                           \
        const Word16x2 *_py = (const Word16x2 *)(y);                           \
        int _i = 0;                                                            \
        sum = init;                                                            \
        for (; _i + 8 <= len; _i += 8) {                                       \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
        }                                                                      \
        for (; _i + 4 <= len; _i += 4) {                                       \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
        }                                                                      \
        for (; _i + 2 <= len; _i += 2) {                                       \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
            sum = __RV_KMATT(sum, _px++->u32, _py++->u32);                     \
        }                                                                      \
        if (_i < len) {                                                        \
            sum = __RV_KMABB(sum, _px->u32, _py->u32);                         \
        }                                                                      \
    } while (0)

#define L_MAC_N_back(sum, init, len, x, y)                                     \
    do {                                                                       \
        const Word16x2 *_px = (const Word16x2 *)(x);                           \
        const Word16x2 *_py = (const Word16x2 *)(y);                           \
        int _i = 1;                                                            \
        if (len <= 0) {                                                        \
            sum = init;                                                        \
            break;                                                             \
        }                                                                      \
        sum = __RV_KDMABB(init, _px--->u32, _py--->u32);                       \
        for (; _i + 8 <= len; _i += 8) {                                       \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
        }                                                                      \
        for (; _i + 4 <= len; _i += 4) {                                       \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
        }                                                                      \
        for (; _i + 2 <= len; _i += 2) {                                       \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
            sum = __RV_KDMABB(sum, _px--->u32, _py--->u32);                    \
        }                                                                      \
        if (_i < len) {                                                        \
            sum = __RV_KDMATT(sum, _px->u32, _py->u32);                        \
        }                                                                      \
    } while (0)

#define CPLX_32X16_B(res_r, res_i, a_r, a_i, b_r, b_i)                         \
    res_r = __RV_KSUBW(__RV_KMMWB2(a_r, b_r), __RV_KMMWB2(a_i, b_i));          \
    res_i = __RV_KMMAWB2(__RV_KMMWB2(a_r, b_i), a_i, b_r);
#define CPLX_32X16_T(res_r, res_i, a_r, a_i, b_r, b_i)                         \
    res_r = __RV_KSUBW(__RV_KMMWT2(a_r, b_r), __RV_KMMWT2(a_i, b_i));          \
    res_i = __RV_KMMAWT2(__RV_KMMWT2(a_r, b_i), a_i, b_r);
