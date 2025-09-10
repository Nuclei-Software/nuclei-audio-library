/*====================================================================================
    EVS Codec 3GPP TS26.442 Nov 04, 2021. Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
  ====================================================================================*/


#include "options.h"      /* Compilation switches                   */
#include "cnst_fx.h"      /* Common constants                       */
#include "rom_com_fx.h"   /* Static table prototypes                */
#include "prot_fx.h"      /* Function prototypes                    */
#include "stl.h"

#include "macro.h"


/*-------------------------------------------------------------------*
 * Function  pred_lt4:                                               *
 *           ~~~~~~~~~                                               *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractional       *
 * interpolation of resolution 1/4.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/

void pred_lt4(
    const Word16 excI[],        /* in : excitation buffer       */
    Word16 excO[],        /* out: excitation buffer       */
    Word16 T0,            /* input : integer pitch lag    */
    Word16 frac,          /* input : fraction of lag      */
    Word16 L_subfr,       /* input : subframe size        */
    const Word16 *win,          /* i  : interpolation window    */
    const Word16 nb_coef,       /* i  : nb of filter coef       */
    const Word16 up_sample      /* i  : up_sample               */

)
{
    Word16   i, j;
    Word32   s;
    const Word16 *x0, *x1, *x2, *c1, *c2;
    x0 = &excI[-T0];


    frac = negate(frac);

    IF ( frac < 0 )
    {
        frac = add(frac,up_sample);
        x0--;
    }

#if defined(SUPPORT_VEC_32X)
    Word16 *pexcO = excO;
    if(L_subfr == 65) {
        x1 = x0++;
        x2 = x1+1;

        vint16m4_t vc1 = __riscv_vlse16_v_i16m4(win + frac, sizeof(int16_t) * up_sample, nb_coef);
        vint16m4_t vc2 = __riscv_vlse16_v_i16m4(win + up_sample - frac, sizeof(int16_t) * up_sample, nb_coef);

        vint16m4_t vx1 = __riscv_vlse16_v_i16m4(x1, -sizeof(int16_t), nb_coef);
        vint16m4_t vx2 = __riscv_vlse16_v_i16m4(x2, sizeof(int16_t), nb_coef);

        vint32m8_t vsum = __riscv_vmv_v_x_i32m8(0, nb_coef);
        vsum = __riscv_vwmacc_vv_i32m8(vsum, vx1, vc1, nb_coef);
        vsum = __riscv_vwmacc_vv_i32m8(vsum, vx2, vc2, nb_coef);

        vint32m1_t vs = __riscv_vmv_s_x_i32m1(0, 1);
        vs = __riscv_vredsum_vs_i32m8_i32m1(vsum, vs, nb_coef);
        s = __riscv_vmv_x_s_i32m1_i32(vs);

#if (INTERP_EXP != -1)
        s = L_shl(s,INTERP_EXP+1);
#endif
        *pexcO++ = round_fx(s);
    }

    const size_t vl = 16;
    for (int loop = 0; loop < 4; ++loop) {
        x1 = x0 + loop * 16;
        x2 = x1 + 1;

        vint16m2_t vx1 = __riscv_vle16_v_i16m2(x1, vl);
        vint16m2_t vx2 = __riscv_vle16_v_i16m2(x2, vl);
        vint32m4_t vsum = __riscv_vmv_v_x_i32m4(0, vl);
        const Word16 *px1 = x1 - 1;
        const Word16 *px2 = x2 + vl;
        c1 = (&win[frac]);
        c2 = (&win[up_sample - frac]);

        for (int i = 0; i < nb_coef - 1; ++i) {
            vsum = __riscv_vwmacc_vx_i32m4(vsum, *c1, vx1, vl);
            vsum = __riscv_vwmacc_vx_i32m4(vsum, *c2, vx2, vl);
            vx1 = __riscv_vslide1up_vx_i16m2(vx1, *px1--, vl);
            vx2 = __riscv_vslide1down_vx_i16m2(vx2, *px2++, vl);
            c1 += up_sample;
            c2 += up_sample;
        }
        vsum = __riscv_vwmacc_vx_i32m4(vsum, *c1, vx1, vl);
        vsum = __riscv_vwmacc_vx_i32m4(vsum, *c2, vx2, vl);

#if (INTERP_EXP != -1)
        vsum = __riscv_vsll_vx_i32m4(vsum, 1, vl);
#endif

        vsum = __riscv_vadd_vx_i32m4(vsum, 0x8000, vl);
        vint16m2_t vsum16 = __riscv_vnsra_wx_i16m2(vsum, 16, vl);
        __riscv_vse16_v_i16m2(pexcO, vsum16, vl);
        pexcO += vl;
    }

#else
    FOR (j=0; j<L_subfr; j++)
    {
        x1 = x0++;
        x2 = x1+1;
        c1 = (&win[frac]);
        c2 = (&win[up_sample-frac]);

        s = L_deposit_l(0);
        FOR(i=0; i<nb_coef; i++)
        {
            /*s += (*x1--) * (*c1) + (*x2++) * (*c2);*/
            s = L_mac0(s, (*x1--),(*c1));
            s = L_mac0(s, (*x2++),(*c2));

            c1+=up_sample;
            c2+=up_sample;
        }
#if (INTERP_EXP != -1)
        s = L_shl(s,INTERP_EXP+1);
#endif

        excO[j] = round_fx(s);
    }
#endif
    return;
}


/*======================================================================*/
/* FUNCTION : pred_lt4_tc_fx() */
/*-----------------------------------------------------------------------*/
/* PURPOSE :   * adapt. search of the second impulse in the same subframe (when appears) */
/* On return, exc[0..L_subfr-1] contains the interpolated signal         */
/*   (adaptive codebook excitation)                                      */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/*  INPUT ARGUMENTS :                                                    */
/* _ (Word16 []) exc  : excitation buffer             Q0                 */
/* _ (Word16) L_subfr : subframe size                 Q0                 */
/* _ (Word16 ) T0 : integer pitch lag                 Q0                 */
/* _ (Word16 ) frac : fraction of lag                 Q0                 */
/* _ (Word16 ) imp_pos : glottal impulse position     Q0                 */
/* _ (Word16 *) win : Interpolation window used       Q14                */
/*-----------------------------------------------------------------------*/
/* OUTPUT ARGUMENTS :                                                    */
/* _ (Word16 []) exc  : output excitation buffer      Q0                 */
/*-----------------------------------------------------------------------*/
/* INPUT OUTPUT ARGUMENTS                                                */
/* NONE																	 */
/*-----------------------------------------------------------------------*/
/* RETURN ARGUMENTS :                                                    */
/* NONE                                                                  */
/*=======================================================================*/
void pred_lt4_tc_fx(
    Word16 exc[],   /* i/o: excitation buffer        */
    const Word16 T0,      /* i  : integer pitch lag        */
    Word16 frac,    /* i:   fraction of lag          */
    const Word16 *win,    /* i  : interpolation window     */
    const Word16 imp_pos, /* i  : glottal impulse position */
    const Word16 i_subfr  /* i  : subframe index           */
)
{
    Word16 i, j,k,l;
    const Word16 *x0;
    Word16 excO[L_SUBFR+1];
    Word32 L_sum;
    Word16 excI[2*L_SUBFR];
    Copy( exc + sub(i_subfr, L_SUBFR), excI, shl(L_SUBFR,1) );

    test();
    IF (sub(add(T0, sub(imp_pos, L_IMPULSE2)), L_SUBFR) < 0 && sub(T0, L_SUBFR) < 0)
    {
        set16_fx(&excI[sub(L_SUBFR,T0)], 0, T0);
        set16_fx(excO, 0, L_SUBFR+1 );
        x0 = excI + sub(L_SUBFR, L_INTERPOL2-1);

        IF (frac > 0)
        {
            frac = sub(frac,UP_SAMP);
            x0--;
        }

        l = add(UP_SAMP-1, frac);
        FOR (j = T0; j < L_SUBFR+1; j++)
        {
            k = l;
            move16();
            L_sum = L_mult(x0[0], win[k]);
            FOR (i = 1; i < 2 * L_INTERPOL2; i++)
            {
                /*
                 * Here, additions with UP_SAMP are not counted
                 ki* because, the window could easily be modified
                 * so that the values needed are contiguous.
                 */
                k += UP_SAMP;
                L_sum = L_mac(L_sum, x0[i], win[k]);    /*Q1 */
            }
            L_sum = L_shl(L_sum, 1);  /*Q0h */

            excO[j] = round_fx(L_sum);

            x0++;
        }
        FOR (i = T0; i < L_SUBFR; i++)
        {
            exc[i+i_subfr] = add(exc[i+i_subfr], mult_r(PIT_SHARP_fx, excO[i]));
            move16();
        }
    }

    return;

}
