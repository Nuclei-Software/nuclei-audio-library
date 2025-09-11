/*====================================================================================
    EVS Codec 3GPP TS26.442 Nov 04, 2021. Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
  ====================================================================================*/

#include "options.h"     /* Compilation switches                   */
#include "prot_fx.h"       /* Function prototypes                    */
#include "rom_com_fx.h"    /* Static table prototypes                */
#include "stl.h"

/*------------------------------------------------------------------
 *
 * This is an implementation of decimation-in-time FFT algorithm for
 * real sequences.  The techniques used here can be found in several
 * books, e.g., i) Proakis and Manolakis, "Digital Signal Processing",
 * 2nd Edition, Chapter 9, and ii) W.H. Press et. al., "Numerical
 * Recipes in C", 2nd Edition, Chapter 12.
 *
 * Input -  There are two inputs to this function:
 *
 *       1) An integer pointer to the input data array
 *       2) An integer value which should be set as +1 for FFT
 *          and some other value, e.g., -1 for ifFT
 *
 * Output - There is no return value.
 *       The input data are replaced with transformed data.  if the
 *       input is a real time domain sequence, it is replaced with
 *       the complex FFT for positive frequencies.  The FFT value
 *       for DC and the foldover frequency are combined to form the
 *       first complex number in the array.  The remaining complex
 *       numbers correspond to increasing frequencies.  if the input
 *       is a complex frequency domain sequence arranged as above,
 *       it is replaced with the corresponding time domain sequence.
 *
 * Notes:
 *
 *       1) This function is designed to be a part of a noise supp-
 *          ression algorithm that requires 128-point FFT of real
 *          sequences.  This is achieved here through a 64-point
 *          complex FFT.  Consequently, the FFT size information is
 *          not transmitted explicitly.  However, some flexibility
 *          is provided in the function to change the size of the
 *          FFT by specifying the size information through "define"
 *          statements.
 *
 *       2) The values of the complex sinusoids used in the FFT
 *          algorithm are computed once (i.e., the first time the
 *          r_fft function is called) and stored in a table. To
 *          further speed up the algorithm, these values can be
 *          precomputed and stored in a ROM table in actual DSP
 *          based implementations.
 *
 *       3) In the c_fft function, the FFT values are divided by
 *          2 after each stage of computation thus dividing the
 *          final FFT values by 64.  No multiplying factor is used
 *          for the ifFT.  This is somewhat different from the usual
 *          definition of FFT where the factor 1/N, i.e., 1/64, is
 *          used for the ifFT and not the FFT.  No factor is used in
 *          the r_fft function.
 *
 *       4) Much of the code for the FFT and ifFT parts in r_fft
 *          and c_fft functions are similar and can be combined.
 *          They are, however, kept separate here to speed up the
 *          execution.
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*
 * c_fft_fx:
 *
 * Computes the complex part of the split-radix FFT
 *------------------------------------------------------------------------*/

static void c_fft_fx(
    const Word16 *phs_tbl,     /* i  : Table of phases            */
    Word16 SIZE,         /* i  : Size of the FFT           */
    Word16 NUM_STAGE,    /* i  : Number of stages           */
    const Word16 *in_ptr,      /* i  : coefficients in the order re[0], re[n/2], re[1], im[1], ..., re[n/2-1], im[n/2-1] */
    Word16 *out_ptr,     /* o  : coefficients in the order re[0], re[n/2], re[1], im[1], ..., re[n/2-1], im[n/2-1] */
    /* in_ptr & out_ptr must not overlap! */
    const Word16 isign)        /* i  : 1=fft, otherwise it is ifft*/
{
    Word16 i, j, k, ii, jj, kk, ji, kj;
    Word32 L_tmp1, L_tmp2;
    Word16 tmp1,tmp2,tmp3,tmp4;
    const Word16 *table_ptr;
    const Word16 *input_ptr1,*input_ptr2,*input_ptr3,*input_ptr4;

    /* Setup Reorder Variables */
    table_ptr = NULL;
    SWITCH (SIZE)
    {
    case 1024:
        table_ptr = FFT_REORDER_1024;
        BREAK;
    case 512:
        table_ptr = FFT_REORDER_512;
        BREAK;
    case 256:
        table_ptr = FFT_reorder_256;
        BREAK;
    case 128:
        table_ptr = FFT_REORDER_128;
        BREAK;
    case 64:
        table_ptr = FFT_reorder_64;
        BREAK;
    }
    /* The FFT part */
    IF (isign != 0)
    {
        /* Unrolled 1st/2nd Stage
         * 1) to take advantage of Table Values (0 & +/- 16384)
         * 2) to perform reordering of Input Values
         */
        FOR (k = 0; k < SIZE; k += 8)
        {
            /*
             * This loop use:
             *   4 Word16 (tmp1...tmp4)
             *   2 Word32 (L_tmp1 & L_tmp2)
             *   4 Pointers (table_ptr, input_ptr1, input_ptr2, input_ptr3)
             *
             * The addition of 'in_ptr' + and index value from 'reorder_ptr'
             * is counted as a move16()
             */

            input_ptr1 = in_ptr + *table_ptr++;

            L_tmp1 = L_mult(*input_ptr1++, 16384);
            L_tmp2 = L_mult(*input_ptr1, 16384);

            input_ptr1 = in_ptr + *table_ptr++;

            tmp1 = msu_r(L_tmp1, *input_ptr1, 16384);
            tmp3 = mac_r(L_tmp1, *input_ptr1++, 16384);

            input_ptr2 = in_ptr + *table_ptr++;
            input_ptr3 = in_ptr + *table_ptr++;

            L_tmp1 = L_mult(*input_ptr2++, 16384);
            tmp2 = mac_r(L_tmp1, *input_ptr3, 16384);
            tmp4 = msu_r(L_tmp1, *input_ptr3++, 16384);

            L_tmp1 = L_mult(tmp3, 16384);
            out_ptr[k] = mac_r(L_tmp1, tmp2, 16384);
            move16();
            out_ptr[k+4] = msu_r(L_tmp1, tmp2, 16384);
            move16();

            tmp2 = mac_r(L_tmp2, *input_ptr1, 16384);
            tmp3 = msu_r(L_tmp2, *input_ptr1, 16384);

            L_tmp2 = L_mult(*input_ptr2, 16384);

            L_tmp1 = L_mult(tmp1, 16384);
            tmp1 = msu_r(L_tmp2, *input_ptr3, 16384);
            out_ptr[k+2] = mac_r(L_tmp1, tmp1, 16384);
            move16();
            out_ptr[k+6] = msu_r(L_tmp1, tmp1, 16384);
            move16();

            L_tmp1 = L_mult(tmp2, 16384);
            tmp2 = mac_r(L_tmp2, *input_ptr3, 16384);
            out_ptr[k+1] = mac_r(L_tmp1, tmp2, 16384);
            move16();
            out_ptr[k+5] = msu_r(L_tmp1, tmp2, 16384);
            move16();

            L_tmp1 = L_mult(tmp3, 16384);
            out_ptr[k+3] = msu_r(L_tmp1, tmp4, 16384);
            move16();
            out_ptr[k+7] = mac_r(L_tmp1, tmp4, 16384);
            move16();
        }

        /* Remaining Stages */
        FOR (i = 2; i < NUM_STAGE; i++)
        {
            /* i is stage counter      */
            jj = shl(2, i);             /* FFT size                */
            kk = shl(jj, 1);            /* 2 * FFT size            */
            ii = shr(SIZE, i);
            ji = 0;
            move16();       /* ji is phase table index */

            FOR (j = 0; j < jj; j += 2)
            {
                /* j is sample counter     */
                FOR (k = j; k < SIZE; k += kk)
                {
                    /* k is butterfly top     */
                    kj = add(k, jj);              /* kj is butterfly bottom */

                    /* Butterfly computations */
                    L_tmp1 = L_msu(L_mult(*(out_ptr + kj), phs_tbl[ji]),
                                   *(out_ptr + kj + 1), phs_tbl[ji + 1]);
                    L_tmp2 = L_mac(L_mult(*(out_ptr + kj + 1), phs_tbl[ji]),
                                   *(out_ptr + kj), phs_tbl[ji + 1]);

                    out_ptr[kj] = mac_r(L_negate(L_tmp1), out_ptr[k], 16384);
                    move16();
                    out_ptr[kj+1] = mac_r(L_negate(L_tmp2), out_ptr[k+1], 16384);
                    move16();
                    out_ptr[k] = mac_r(L_tmp1, out_ptr[k], 16384);
                    move16();
                    out_ptr[k+1] = mac_r(L_tmp2, out_ptr[k+1], 16384);
                    move16();
                }
                ji = add(ji, ii);
            }
        }
    }
    ELSE /* The ifFT part */
    {
        /* Unrolled 1st/2nd Stage
         * 1) to take advantage of Table Values (0 & +/- 16384)
         * 2) to perform reordering of Input Values
         */
        FOR (k = 0; k < SIZE; k += 8)
        {
            /*
             * This loop use:
             *   4 Word16 (tmp1...tmp4)
             *   2 Word32 (L_tmp1 & L_tmp2)
             *   5 Pointers (reorder_ptr, input_ptr1...input_ptr4)
             *
             * The addition of 'in_ptr' + and index value from 'reorder_ptr'
             * is counted as a move16()
             */

            input_ptr1 = in_ptr + *table_ptr++;
            input_ptr2 = in_ptr + *table_ptr++;

            input_ptr3 = in_ptr + *table_ptr++;
            input_ptr4 = in_ptr + *table_ptr++;

            tmp3 = sub(*input_ptr1, *input_ptr2);
            tmp4 = add(*input_ptr1++, *input_ptr2++);

            tmp2 = sub(input_ptr3[0], input_ptr4[0]);
            tmp1 = sub(input_ptr3[1], input_ptr4[1]);

            out_ptr[k+2] = sub(tmp3, tmp1);
            move16();
            out_ptr[k+6] = add(tmp3, tmp1);
            move16();

            tmp1 = sub(*input_ptr1, *input_ptr2);
            out_ptr[k+3] = add(tmp1, tmp2);
            move16();
            out_ptr[k+7] = sub(tmp1, tmp2);
            move16();

            tmp1 = add(input_ptr3[0], input_ptr4[0]);
            tmp3 = add(input_ptr3[1], input_ptr4[1]);

            out_ptr[k] = add(tmp4, tmp1);
            move16();
            out_ptr[k+4] = sub(tmp4, tmp1);
            move16();

            tmp4 = add(*input_ptr1, *input_ptr2);
            out_ptr[k+1] = add(tmp4, tmp3);
            move16();
            out_ptr[k+5] = sub(tmp4, tmp3);
            move16();
        }

        table_ptr = phs_tbl + SIZE;  /* access part of table that is scaled by 2 */

        /* Remaining Stages */
        FOR (i = 2; i < NUM_STAGE; i++)
        {
            /* i is stage counter      */
            jj = shl(2, i);             /* FFT size                */
            kk = shl(jj, 1);            /* 2 * FFT size            */
            ii = shr(SIZE, i);
            ji = 0;
            move16();     /* ji is phase table index */

            FOR (j = 0; j < jj; j += 2)
            {
                /* j is sample counter     */
                /* This can be computed by successive add_fxitions of ii to ji, starting from 0
                   hence line-count it as a one-line add (still need to increment op count!!) */

                FOR (k = j; k < SIZE; k += kk)
                {
                    /* k is butterfly top     */
                    kj = add(k, jj);            /* kj is butterfly bottom */

                    /* Butterfly computations */
                    tmp1 = mac_r(L_mult(out_ptr[kj], table_ptr[ji]),
                    out_ptr[kj+1], table_ptr[ji + 1]);

                    tmp2 = msu_r(L_mult(out_ptr[kj+1], table_ptr[ji]),
                    out_ptr[kj], table_ptr[ji+1]);

                    out_ptr[kj] = sub(out_ptr[k], tmp1);
                    move16();
                    out_ptr[kj+1] = sub(out_ptr[k+1], tmp2);
                    move16();
                    out_ptr[k] = add(out_ptr[k], tmp1);
                    move16();
                    out_ptr[k+1] = add(out_ptr[k+1], tmp2);
                    move16();
                }
                ji = add(ji, ii);
            }
        }
    }
}

/*--------------------------------------------------------------------------------*
 * r_fft_fx:
 *
 * Perform FFT fixed-point for real-valued sequences of length 32, 64 or 128
 *--------------------------------------------------------------------------------*/
void r_fft_fx_lc(
    const Word16 *phs_tbl,    /* i  : Table of phase            */
    const Word16 SIZE,        /* i  : Size of the FFT           */
    const Word16 SIZE2,       /* i  : Size / 2                  */
    const Word16 NUM_STAGE,   /* i  : Number of stage           */
    const Word16 *in_ptr,     /* i  : coefficients in the order re[0], re[1], ... re[n/2], im[n/2-1], im[n/2-2], ..., im[1] */
    Word16 *out_ptr,    /* o  : coefficients in the order re[0], re[1], ... re[n/2], im[n/2-1], im[n/2-2], ..., im[1] */
    const Word16 isign        /* i  : 1=fft, otherwize it's ifft                                                      */
)
{
    Word16 tmp2_real, tmp2_imag;
    Word32 Ltmp1_real, Ltmp1_imag;
    Word16 i;
    Word32 Ltmp1;
    const Word16 *phstbl_ptrDn;
    Word16 *ptrDn;
    Word16 temp[1024];  /* Accommodates real input FFT size up to 1024. */

    /* Setup Pointers */
    phstbl_ptrDn = &phs_tbl[SIZE-1];

    /* The FFT part */
    IF (isign != 0)
    {
        Word16 *ptRealUp, *ptRealDn, *ptImaUp, *ptImaDn;

        /* Perform the complex FFT */
        c_fft_fx(phs_tbl, SIZE, NUM_STAGE, in_ptr, temp, isign);

        /* First, handle the DC and foldover frequencies */
        out_ptr[SIZE2] = sub(temp[0], temp[1]);
        move16();
        out_ptr[0] = sub(add(temp[0], temp[1]), shr(NUM_STAGE, 1));
        move16();/* DC have a small offset */

        ptrDn = &temp[SIZE-1];

        ptImaDn = &out_ptr[SIZE-1];
        ptRealUp = &out_ptr[1];
        ptImaUp = &out_ptr[SIZE2+1];
        ptRealDn = &out_ptr[SIZE2-1];

        /* Now, handle the remaining positive frequencies */
        FOR (i = 2; i <= SIZE2; i += 2)
        {
            Ltmp1_imag = L_mult(temp[i+1], 16384);
            Ltmp1_imag = L_msu(Ltmp1_imag, *ptrDn, 16384);
            tmp2_real = add(temp[i+1], *ptrDn--);

            Ltmp1_real = L_mult(temp[i], 16384);
            Ltmp1_real = L_mac(Ltmp1_real, *ptrDn, 16384);
            tmp2_imag = sub(*ptrDn--, temp[i]);


            *ptRealUp++ = msu_r(L_mac(Ltmp1_real, tmp2_real, phs_tbl[i]), tmp2_imag, phs_tbl[i+1]);
            move16();
            *ptImaDn-- = mac_r(L_mac(Ltmp1_imag, tmp2_imag, phs_tbl[i]), tmp2_real, phs_tbl[i+1]);
            move16();
            Ltmp1 = L_mac(L_negate(Ltmp1_imag), tmp2_real, *phstbl_ptrDn);
            Ltmp1_real = L_mac(Ltmp1_real, tmp2_imag, *phstbl_ptrDn--);
            *ptImaUp++ = msu_r(Ltmp1, tmp2_imag, *phstbl_ptrDn);
            move16();
            *ptRealDn-- = mac_r(Ltmp1_real, tmp2_real, *phstbl_ptrDn--);
            move16();
        }
    }
    ELSE /* The ifFT part */
    {
        const Word16 *ptRealUp, *ptRealDn, *ptImaUp, *ptImaDn;

        /* First, handle the DC and foldover frequencies */
        Ltmp1 = L_mult(in_ptr[0], 16384);
        temp[0] = mac_r(Ltmp1, in_ptr[SIZE2], 16384);
        move16();
        temp[1] = msu_r(Ltmp1, in_ptr[SIZE2], 16384);
        move16();

        ptrDn = &temp[SIZE-1];

        /* Here we cast to Word16 * from a const Word16 *. */
        /* This is ok because we use these pointers for    */
        /* reading only. This is just to avoid declaring a */
        /* bunch of 4 other pointer with const Word16 *.   */
        ptImaDn = &in_ptr[SIZE-1];
        ptRealUp = &in_ptr[1];
        ptImaUp = &in_ptr[SIZE2+1];
        ptRealDn = &in_ptr[SIZE2-1];

        /* Now, handle the remaining positive frequencies */
        FOR (i = 2; i <= SIZE2; i += 2)
        {
            Ltmp1_imag = L_mult(*ptImaDn, 16384);
            Ltmp1_imag = L_msu(Ltmp1_imag, *ptImaUp, 16384);
            tmp2_real = add(*ptImaDn--, *ptImaUp++);
            Ltmp1_real = L_mult(*ptRealUp, 16384);
            Ltmp1_real = L_mac(Ltmp1_real, *ptRealDn, 16384);
            tmp2_imag = sub(*ptRealUp++, *ptRealDn--);


            temp[i] = mac_r(L_msu(Ltmp1_real, tmp2_real, phs_tbl[i]), tmp2_imag, phs_tbl[i+1]);
            move16();
            temp[i+1] = mac_r(L_mac(Ltmp1_imag, tmp2_imag, phs_tbl[i]), tmp2_real, phs_tbl[i+1]);
            move16();
            Ltmp1 = L_mac(L_negate(Ltmp1_imag), tmp2_real, *phstbl_ptrDn);
            Ltmp1_real = L_msu(Ltmp1_real, tmp2_imag, *phstbl_ptrDn--);
            *ptrDn-- = msu_r(Ltmp1, tmp2_imag, *phstbl_ptrDn);
            move16();
            *ptrDn-- = msu_r(Ltmp1_real, tmp2_real, *phstbl_ptrDn--);
            move16();
        }

        /* Perform the complex ifFT */
        c_fft_fx(phs_tbl, SIZE, NUM_STAGE, temp, out_ptr, isign);
    }
}

#if defined(SUPPORT_VEC_32X)

const int16_t cfft_twd_len128_re_q15[];
const int16_t cfft_twd_len128_im_q15[];
const uint16_t cfft_bridx_len128_q15[];
const int16_t rfft_twd_len256_re_q15[];
const int16_t rfft_twd_len256_im_q15[];

void rvv_cfft_128(const Word16 *in_ptr, Word16 *out_ptr, Word16 *buffer) {
    const long N = 128;
    int16_t *buf[2] = {buffer, buffer + N * 2};
    int buf_idx = 0; // data in buf_idx

    const int16_t *ptwd_re = cfft_twd_len128_re_q15;
    const int16_t *ptwd_im = cfft_twd_len128_im_q15;

    int32_t avl = N >> 1;
    size_t vl;
    const int16_t *px = in_ptr;
    int16_t *py = buf[buf_idx];
    for (; (vl = __riscv_vsetvl_e16m2(avl)) > 0; avl -= vl) {
        vint16m2x2_t v_tuple = __riscv_vlseg2e16_v_i16m2x2(px, vl);
        vint16m2_t va_re = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 0);
        vint16m2_t va_im = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 1);
        v_tuple = __riscv_vlseg2e16_v_i16m2x2(px + N, vl);
        vint16m2_t vb_re = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 0);
        vint16m2_t vb_im = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 1);
        px += 2 * vl;

        // scale down to prevent overflow
        va_re = __riscv_vsra_vx_i16m2(va_re, 1, vl);
        va_im = __riscv_vsra_vx_i16m2(va_im, 1, vl);
        vb_re = __riscv_vsra_vx_i16m2(vb_re, 1, vl);
        vb_im = __riscv_vsra_vx_i16m2(vb_im, 1, vl);

        vint16m2_t vre0 = __riscv_vsadd_vv_i16m2(va_re, vb_re, vl);
        vre0 = __riscv_vsra_vx_i16m2(vre0, 1, vl);
        vint16m2_t vim0 = __riscv_vsadd_vv_i16m2(va_im, vb_im, vl);
        vim0 = __riscv_vsra_vx_i16m2(vim0, 1, vl);

        vint16m2_t vtmp_re = __riscv_vssub_vv_i16m2(va_re, vb_re, vl);
        vtmp_re = __riscv_vsra_vx_i16m2(vtmp_re, 1, vl);
        vint16m2_t vtmp_im = __riscv_vssub_vv_i16m2(va_im, vb_im, vl);
        vtmp_im = __riscv_vsra_vx_i16m2(vtmp_im, 1, vl);

        vint16m2_t vtwd_re = __riscv_vle16_v_i16m2(ptwd_re, vl);
        ptwd_re += vl;
        vint16m2_t vtwd_im = __riscv_vle16_v_i16m2(ptwd_im, vl);
        ptwd_im += vl;

        vint16m2_t vre1 = __riscv_vssub_vv_i16m2(
            __riscv_vsmul_vv_i16m2(vtmp_re, vtwd_re, __RISCV_VXRM_RNU, vl),
            __riscv_vsmul_vv_i16m2(vtmp_im, vtwd_im, __RISCV_VXRM_RNU, vl), vl);
        vint16m2_t vim1 = __riscv_vsadd_vv_i16m2(
            __riscv_vsmul_vv_i16m2(vtmp_im, vtwd_re, __RISCV_VXRM_RNU, vl),
            __riscv_vsmul_vv_i16m2(vtmp_re, vtwd_im, __RISCV_VXRM_RNU, vl), vl);

        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vre0);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vre1);
        __riscv_vsseg2e16_v_i16m2x2(py, v_tuple, vl);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vim0);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vim1);
        __riscv_vsseg2e16_v_i16m2x2(py + N, v_tuple, vl);
        py += 2 * vl;
    }

    for (int a = N >> 1, stage = 0; a > 2; a >>= 1, stage++) {
        avl = N >> 1;
        const int16_t *px = buf[buf_idx];
        int16_t *py = buf[1 - buf_idx];

        for (; (vl = __riscv_vsetvl_e16m2(avl)) > 0; avl -= vl) {
            vint16m2_t va_re = __riscv_vle16_v_i16m2(px, vl);
            vint16m2_t va_im = __riscv_vle16_v_i16m2(px + N, vl);
            vint16m2_t vb_re = __riscv_vle16_v_i16m2(px + N / 2, vl);
            vint16m2_t vb_im = __riscv_vle16_v_i16m2(px + N * 3 / 2, vl);
            px += vl;

            vint16m2_t vre0 = __riscv_vadd_vv_i16m2(va_re, vb_re, vl);
            vre0 = __riscv_vsra_vx_i16m2(vre0, 1, vl);
            vint16m2_t vim0 = __riscv_vadd_vv_i16m2(va_im, vb_im, vl);
            vim0 = __riscv_vsra_vx_i16m2(vim0, 1, vl);

            vint16m2_t vtmp_re = __riscv_vsub_vv_i16m2(va_re, vb_re, vl);
            vtmp_re = __riscv_vsra_vx_i16m2(vtmp_re, 1, vl);
            vint16m2_t vtmp_im = __riscv_vsub_vv_i16m2(va_im, vb_im, vl);
            vtmp_im = __riscv_vsra_vx_i16m2(vtmp_im, 1, vl);

            vint16m2_t vtwd_re = __riscv_vle16_v_i16m2(ptwd_re, vl);
            ptwd_re += vl;
            vint16m2_t vtwd_im = __riscv_vle16_v_i16m2(ptwd_im, vl);
            ptwd_im += vl;

            vint16m2_t vre1 = __riscv_vssub_vv_i16m2(
                __riscv_vsmul_vv_i16m2(vtmp_re, vtwd_re, __RISCV_VXRM_RNU, vl),
                __riscv_vsmul_vv_i16m2(vtmp_im, vtwd_im, __RISCV_VXRM_RNU, vl),
                vl);
            vint16m2_t vim1 = __riscv_vsadd_vv_i16m2(
                __riscv_vsmul_vv_i16m2(vtmp_re, vtwd_im, __RISCV_VXRM_RNU, vl),
                __riscv_vsmul_vv_i16m2(vtmp_im, vtwd_re, __RISCV_VXRM_RNU, vl),
                vl);

            vint16m2x2_t v_tuple;
            v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vre0);
            v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vre1);
            __riscv_vsseg2e16_v_i16m2x2(py, v_tuple, vl);

            v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vim0);
            v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vim1);
            __riscv_vsseg2e16_v_i16m2x2(py + N, v_tuple, vl);
            py += 2 * vl;
        }
        buf_idx = 1 - buf_idx;
    }

    avl = N >> 1;
    px = buf[buf_idx];
    py = out_ptr;
    const uint16_t *pidx = cfft_bridx_len128_q15;
    for (; (vl = __riscv_vsetvl_e16m2(avl)) > 0; avl -= vl) {
        vint16m2_t va_re = __riscv_vle16_v_i16m2(px, vl);
        vint16m2_t va_im = __riscv_vle16_v_i16m2(px + N, vl);
        vint16m2_t vb_re = __riscv_vle16_v_i16m2(px + N / 2, vl);
        vint16m2_t vb_im = __riscv_vle16_v_i16m2(px + N * 3 / 2, vl);
        px += vl;

        vint16m2_t vre0 = __riscv_vadd_vv_i16m2(va_re, vb_re, vl);
        vint16m2_t vim0 = __riscv_vadd_vv_i16m2(va_im, vb_im, vl);

        vint16m2_t vre1 = __riscv_vsub_vv_i16m2(va_re, vb_re, vl);
        vint16m2_t vim1 = __riscv_vsub_vv_i16m2(va_im, vb_im, vl);

        vuint16m2_t vidx = __riscv_vle16_v_u16m2(pidx, vl);
        vint16m2x2_t v_tuple;
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vre0);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vim0);
        __riscv_vsoxseg2ei16_v_i16m2x2(py, vidx, v_tuple, vl);

        vidx = __riscv_vle16_v_u16m2(pidx + (N >> 1), vl);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 0, vre1);
        v_tuple = __riscv_vset_v_i16m2_i16m2x2(v_tuple, 1, vim1);

        pidx += vl;
        __riscv_vsoxseg2ei16_v_i16m2x2(py, vidx, v_tuple, vl);
    }
}

void r_fft_fx_lc_256(
    const Word16 *in_ptr, /* i  : coefficients in the order re[0], re[1], ...
                             re[n/2], im[n/2-1], im[n/2-2], ..., im[1] */
    Word16 *out_ptr /* o  : coefficients in the order re[0], re[1], ... re[n/2],
                       im[n/2-1], im[n/2-2], ..., im[1] */
) {
    const int N = 256;
    Word16 buffer[256 * 3];
    Word16 *y = buffer + 512;
    rvv_cfft_128(in_ptr, y, buffer);

    // Y[0] = F[0] + G[0]
    const int16_t F0 = y[0];
    const int16_t G0 = y[1];
    out_ptr[0] = F0 + G0;
    out_ptr[N / 2] = F0 - G0;

    size_t avl = (N >> 1) - 1;
    size_t vl;
    const int16_t *py = y + 2;
    const int16_t *py_inv = y + N - 2;
    int16_t *pout_re = out_ptr + 1;
    int16_t *pout_im = out_ptr + N -1;
    ptrdiff_t bstride = -sizeof(int16_t);
    const int16_t *ptwd_re = rfft_twd_len256_re_q15 + 1;
    const int16_t *ptwd_im = rfft_twd_len256_im_q15 + 1;
    for (; (vl = __riscv_vsetvl_e16m2(avl)) > 0; avl -= vl) {
        // load vx_re, vx_im
        vint16m2x2_t v_tuple = __riscv_vlseg2e16_v_i16m2x2(py, vl);
        py += vl * 2;
        vint16m2_t vy_re = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 0);
        vint16m2_t vy_im = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 1);

        v_tuple =
            __riscv_vlsseg2e16_v_i16m2x2(py_inv, -sizeof(int16_t) * 2, vl);
        py_inv -= vl * 2;
        vint16m2_t vy_inv_re = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 0);
        vint16m2_t vy_inv_im = __riscv_vget_v_i16m2x2_i16m2(v_tuple, 1);

        // vFr
        vint16m2_t vFr_re = __riscv_vadd_vv_i16m2(vy_re, vy_inv_re, vl);
        vFr_re = __riscv_vsra_vx_i16m2(vFr_re, 1, vl);
        vint16m2_t vFr_im = __riscv_vsub_vv_i16m2(vy_im, vy_inv_im, vl);
        vFr_im = __riscv_vsra_vx_i16m2(vFr_im, 1, vl);

        // vGr
        vint16m2_t vGr_re = __riscv_vadd_vv_i16m2(vy_inv_im, vy_im, vl);
        vGr_re = __riscv_vsra_vx_i16m2(vGr_re, 1, vl);
        vint16m2_t vGr_im = __riscv_vsub_vv_i16m2(vy_inv_re, vy_re, vl);
        vGr_im = __riscv_vsra_vx_i16m2(vGr_im, 1, vl);

        vint16m2_t vtwd_re = __riscv_vle16_v_i16m2(ptwd_re, vl);
        ptwd_re += vl;
        vint16m2_t vtwd_im = __riscv_vle16_v_i16m2(ptwd_im, vl);
        ptwd_im += vl;

        vint16m2_t vout_re = __riscv_vssub_vv_i16m2(
            __riscv_vsmul_vv_i16m2(vGr_re, vtwd_re, __RISCV_VXRM_RNU, vl),
            __riscv_vsmul_vv_i16m2(vGr_im, vtwd_im, __RISCV_VXRM_RNU, vl), vl);
        vint16m2_t vout_im = __riscv_vsadd_vv_i16m2(
            __riscv_vsmul_vv_i16m2(vGr_im, vtwd_re, __RISCV_VXRM_RNU, vl),
            __riscv_vsmul_vv_i16m2(vGr_re, vtwd_im, __RISCV_VXRM_RNU, vl), vl);

        vout_re = __riscv_vadd_vv_i16m2(vFr_re, vout_re, vl);
        vout_im = __riscv_vadd_vv_i16m2(vFr_im, vout_im, vl);

        __riscv_vse16_v_i16m2(pout_re, vout_re, vl);
        __riscv_vsse16_v_i16m2(pout_im, bstride, vout_im, vl);
        pout_re += vl;
        pout_im -= vl;
    }
}

const uint16_t cfft_bridx_len128_q15[] = {
    0,   128, 64,  192, 32,  160, 96,  224, 16,  144, 80,  208, 48,  176, 112,
    240, 8,   136, 72,  200, 40,  168, 104, 232, 24,  152, 88,  216, 56,  184,
    120, 248, 4,   132, 68,  196, 36,  164, 100, 228, 20,  148, 84,  212, 52,
    180, 116, 244, 12,  140, 76,  204, 44,  172, 108, 236, 28,  156, 92,  220,
    60,  188, 124, 252, 256, 384, 320, 448, 288, 416, 352, 480, 272, 400, 336,
    464, 304, 432, 368, 496, 264, 392, 328, 456, 296, 424, 360, 488, 280, 408,
    344, 472, 312, 440, 376, 504, 260, 388, 324, 452, 292, 420, 356, 484, 276,
    404, 340, 468, 308, 436, 372, 500, 268, 396, 332, 460, 300, 428, 364, 492,
    284, 412, 348, 476, 316, 444, 380, 508};

const int16_t rfft_twd_len256_re_q15[] = {
    32767,  32758,  32728,  32679,  32610,  32521,  32413,  32285,  32138,
    31971,  31785,  31581,  31357,  31114,  30852,  30572,  30273,  29956,
    29621,  29269,  28898,  28511,  28106,  27684,  27245,  26790,  26319,
    25832,  25330,  24812,  24279,  23732,  23170,  22594,  22005,  21403,
    20787,  20159,  19519,  18868,  18204,  17530,  16846,  16151,  15446,
    14732,  14010,  13278,  12539,  11793,  11039,  10278,  9512,   8739,
    7961,   7179,   6392,   5602,   4808,   4011,   3211,   2410,   1607,
    804,    0,      -804,   -1607,  -2410,  -3211,  -4011,  -4808,  -5602,
    -6392,  -7179,  -7961,  -8739,  -9512,  -10278, -11039, -11793, -12539,
    -13278, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868,
    -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23732, -24279,
    -24812, -25330, -25832, -26319, -26790, -27245, -27684, -28106, -28511,
    -28898, -29269, -29621, -29956, -30273, -30572, -30852, -31114, -31357,
    -31581, -31785, -31971, -32138, -32285, -32413, -32521, -32610, -32679,
    -32728, -32758};

const int16_t rfft_twd_len256_im_q15[] = {
    0,      -804,   -1607,  -2410,  -3211,  -4011,  -4808,  -5602,  -6392,
    -7179,  -7961,  -8739,  -9512,  -10278, -11039, -11793, -12539, -13278,
    -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519,
    -20159, -20787, -21403, -22005, -22594, -23170, -23732, -24279, -24812,
    -25330, -25832, -26319, -26790, -27245, -27684, -28106, -28511, -28898,
    -29269, -29621, -29956, -30273, -30572, -30852, -31114, -31357, -31581,
    -31785, -31971, -32138, -32285, -32413, -32521, -32610, -32679, -32728,
    -32758, -32768, -32758, -32728, -32679, -32610, -32521, -32413, -32285,
    -32138, -31971, -31785, -31581, -31357, -31114, -30852, -30572, -30273,
    -29956, -29621, -29269, -28898, -28511, -28106, -27684, -27245, -26790,
    -26319, -25832, -25330, -24812, -24279, -23732, -23170, -22594, -22005,
    -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151,
    -15446, -14732, -14010, -13278, -12539, -11793, -11039, -10278, -9512,
    -8739,  -7961,  -7179,  -6392,  -5602,  -4808,  -4011,  -3211,  -2410,
    -1607,  -804};

const int16_t cfft_twd_len128_re_q15[] = {
    32767,  32728,  32610,  32413,  32138,  31785,  31357,  30852,  30273,
    29621,  28898,  28106,  27245,  26319,  25330,  24279,  23170,  22005,
    20787,  19519,  18204,  16846,  15446,  14010,  12539,  11039,  9512,
    7961,   6392,   4808,   3211,   1607,   0,      -1607,  -3211,  -4808,
    -6392,  -7961,  -9512,  -11039, -12539, -14010, -15446, -16846, -18204,
    -19519, -20787, -22005, -23170, -24279, -25330, -26319, -27245, -28106,
    -28898, -29621, -30273, -30852, -31357, -31785, -32138, -32413, -32610,
    -32728, 32767,  32767,  32610,  32610,  32138,  32138,  31357,  31357,
    30273,  30273,  28898,  28898,  27245,  27245,  25330,  25330,  23170,
    23170,  20787,  20787,  18204,  18204,  15446,  15446,  12539,  12539,
    9512,   9512,   6392,   6392,   3211,   3211,   0,      0,      -3211,
    -3211,  -6392,  -6392,  -9512,  -9512,  -12539, -12539, -15446, -15446,
    -18204, -18204, -20787, -20787, -23170, -23170, -25330, -25330, -27245,
    -27245, -28898, -28898, -30273, -30273, -31357, -31357, -32138, -32138,
    -32610, -32610, 32767,  32767,  32767,  32767,  32138,  32138,  32138,
    32138,  30273,  30273,  30273,  30273,  27245,  27245,  27245,  27245,
    23170,  23170,  23170,  23170,  18204,  18204,  18204,  18204,  12539,
    12539,  12539,  12539,  6392,   6392,   6392,   6392,   0,      0,
    0,      0,      -6392,  -6392,  -6392,  -6392,  -12539, -12539, -12539,
    -12539, -18204, -18204, -18204, -18204, -23170, -23170, -23170, -23170,
    -27245, -27245, -27245, -27245, -30273, -30273, -30273, -30273, -32138,
    -32138, -32138, -32138, 32767,  32767,  32767,  32767,  32767,  32767,
    32767,  32767,  30273,  30273,  30273,  30273,  30273,  30273,  30273,
    30273,  23170,  23170,  23170,  23170,  23170,  23170,  23170,  23170,
    12539,  12539,  12539,  12539,  12539,  12539,  12539,  12539,  0,
    0,      0,      0,      0,      0,      0,      0,      -12539, -12539,
    -12539, -12539, -12539, -12539, -12539, -12539, -23170, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, -30273, -30273, -30273, -30273,
    -30273, -30273, -30273, -30273, 32767,  32767,  32767,  32767,  32767,
    32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
    32767,  32767,  23170,  23170,  23170,  23170,  23170,  23170,  23170,
    23170,  23170,  23170,  23170,  23170,  23170,  23170,  23170,  23170,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, 32767,  32767,  32767,  32767,
    32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
    32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
    32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
    32767,  0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0};

const int16_t cfft_twd_len128_im_q15[] = {
    0,      -1607,  -3211,  -4808,  -6392,  -7961,  -9512,  -11039, -12539,
    -14010, -15446, -16846, -18204, -19519, -20787, -22005, -23170, -24279,
    -25330, -26319, -27245, -28106, -28898, -29621, -30273, -30852, -31357,
    -31785, -32138, -32413, -32610, -32728, -32768, -32728, -32610, -32413,
    -32138, -31785, -31357, -30852, -30273, -29621, -28898, -28106, -27245,
    -26319, -25330, -24279, -23170, -22005, -20787, -19519, -18204, -16846,
    -15446, -14010, -12539, -11039, -9512,  -7961,  -6392,  -4808,  -3211,
    -1607,  0,      0,      -3211,  -3211,  -6392,  -6392,  -9512,  -9512,
    -12539, -12539, -15446, -15446, -18204, -18204, -20787, -20787, -23170,
    -23170, -25330, -25330, -27245, -27245, -28898, -28898, -30273, -30273,
    -31357, -31357, -32138, -32138, -32610, -32610, -32768, -32768, -32610,
    -32610, -32138, -32138, -31357, -31357, -30273, -30273, -28898, -28898,
    -27245, -27245, -25330, -25330, -23170, -23170, -20787, -20787, -18204,
    -18204, -15446, -15446, -12539, -12539, -9512,  -9512,  -6392,  -6392,
    -3211,  -3211,  0,      0,      0,      0,      -6392,  -6392,  -6392,
    -6392,  -12539, -12539, -12539, -12539, -18204, -18204, -18204, -18204,
    -23170, -23170, -23170, -23170, -27245, -27245, -27245, -27245, -30273,
    -30273, -30273, -30273, -32138, -32138, -32138, -32138, -32768, -32768,
    -32768, -32768, -32138, -32138, -32138, -32138, -30273, -30273, -30273,
    -30273, -27245, -27245, -27245, -27245, -23170, -23170, -23170, -23170,
    -18204, -18204, -18204, -18204, -12539, -12539, -12539, -12539, -6392,
    -6392,  -6392,  -6392,  0,      0,      0,      0,      0,      0,
    0,      0,      -12539, -12539, -12539, -12539, -12539, -12539, -12539,
    -12539, -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170,
    -30273, -30273, -30273, -30273, -30273, -30273, -30273, -30273, -32768,
    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -30273, -30273,
    -30273, -30273, -30273, -30273, -30273, -30273, -23170, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, -12539, -12539, -12539, -12539,
    -12539, -12539, -12539, -12539, 0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      -23170, -23170, -23170, -23170, -23170, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170,
    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768,
    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170, -23170,
    -23170, -23170, -23170, -23170, -23170, 0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768,
    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768,
    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768,
    -32768, -32768, -32768, -32768, -32768, -32768};

#endif