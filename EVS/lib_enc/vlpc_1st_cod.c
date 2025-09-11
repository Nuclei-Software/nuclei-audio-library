/*====================================================================================
    EVS Codec 3GPP TS26.442 Nov 04, 2021. Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
  ====================================================================================*/

#include <assert.h>
#include "stl.h"
#include "control.h"
#include "cnst_fx.h"
#include "prot_fx.h"

#define  ISF_ONE  3/*1.0f*1.28f Q1*/  /*=1.0f in 14Q1*1.28*/

extern const Word16 dico_lsf_abs_8b[];

/*outputs only the weightings, doesn't do anything with the lsfq*/
static void lsf_weight(
    const Word16 *lsfq,  /* input: quantized lsf coefficients (14Q1*1.28)*/
    Word16 *w    /* output: lsf weighting vector (0Q15)          */
)
{
    Word16  i;
    Word16  inv_di0, inv_di1;



    /* weighting function */
    /*use the second element as the base to avoid the following division by 0*/
    /*this happens when the ac function is nearly flat*/
    i = lsfq[0];
    move16();
    if (lsfq[0] == 0)
    {
        i = lsfq[1];
        move16();
    }
    inv_di0 = 0x7fff;
    move16();
    if (sub(i, ISF_ONE) > 0)
    {
        inv_di0 = div_s(ISF_ONE,i);              /*0Q15*/ /*inv_di0 = 1.0f / lsfq[0];*/
    }

    /* Allow saturation during weight calculation, because the values that are
       weighted later are used for a minimum search and experimental saturation
       avoidance also showed no improvement. */
    BASOP_SATURATE_WARNING_OFF
    FOR (i=1; i<(M-2); i+=2)                /*for (i=1; i<(M-2); i+=2)*/
    {
        inv_di1 = div_s(ISF_ONE,s_max(ISF_ONE, sub(lsfq[i],lsfq[i-1])));  /*0Q15*/ /*inv_di1 = 1.0f / (lsfq[i] - lsfq[i-1]);*/
        w[i-1] = add(inv_di0,inv_di1);
        move16();   /*0Q15*/ /*w[i-1] = inv_di0 + inv_di1;*/
        inv_di0 = div_s(ISF_ONE,s_max(ISF_ONE, sub(lsfq[i+1],lsfq[i])));  /*0Q15*/ /*inv_di0 = 1.0f / (lsfq[i+1] - lsfq[i]);*/
        w[i] = add(inv_di1,inv_di0);
        move16();   /*0Q15*/ /*w[i] = inv_di1 + inv_di0;*/
    }
    inv_di1 = div_s(ISF_ONE, s_max(ISF_ONE, sub(lsfq[i],lsfq[i-1])));    /*inv_di1 = 1.0f / (lsfq[i] - lsfq[i-1]);*/
    w[i-1] = add(inv_di0,inv_di1);
    move16();            /*w[i-1] = inv_di0 + inv_di1;*/
    inv_di0 = div_s(ISF_ONE, s_max(ISF_ONE, sub(FREQ_MAX,lsfq[i])));        /*inv_di0 = 1.0f / (FREQ_MAX - lsfq[i]);*/
    w[i] = add(inv_di1,inv_di0);
    move16();            /*w[i] = inv_di1 + inv_di0;*/

    BASOP_SATURATE_WARNING_ON


    return;
}

Word16 vlpc_1st_cod(              /* output: codebook index                   */
    const Word16 *lsf,            /* input:  vector to quantize (14Q1*1.28)   */
    Word16 *lsfq            /* o:      quantized lsf      (14Q1*1.28)   */
    ,Word16 *wout              /* o: lsf weights */
    ,Word16 rf_mode
)
{
    Word16    i, j, index, diff, wdiff;
    Word16    w[M];
    Word32    dist_min, dist;
    const Word16 *p_dico;



    /* weighting */
    lsf_weight(lsf, w);/*lsf:14Q1*1.28=>w:0Q15*/
    IF(sub(rf_mode, 1) == 0)
    {
        Word16 s;
        s = Find_Max_Norm16(w, M);
        Scale_sig(w, M, s);
    }
    Copy(w, wout, M);
    /* remove lsf prediction/means */

#if defined(SUPPORT_VEC_32X)
    size_t avl = 256;
    size_t vl;
    ptrdiff_t bstride = sizeof(Word16) * M;
    p_dico = dico_lsf_abs_8b; /*14Q1*1.28*/

    vint32m1_t vmin = __riscv_vmv_s_x_i32m1(MAX_32, 1);
    int acc_cnt = 0;
    for (; (vl = __riscv_vsetvl_e16m4(avl)) > 0; avl -= vl) {
        const Word16 *pp_dico = p_dico;
        vint32m8_t vdist = __riscv_vmv_v_x_i32m8(0, vl);
        for (int i = 0; i < M; ++i) {
            vint16m4_t vdisco = __riscv_vlse16_v_i16m4(pp_dico, bstride, vl);
            pp_dico++;
            vdisco = __riscv_vrsub_vx_i16m4(vdisco, lsf[i], vl);
            vint16m4_t vwdiff =
                __riscv_vsmul_vx_i16m4(vdisco, w[i], __RISCV_VXRM_RNU, vl);
            vwdiff = __riscv_vsra_vx_i16m4(vwdiff, 4, vl);
            vint32m8_t vmul = __riscv_vwmul_vv_i32m8(vwdiff, vdisco, vl);
            vdist = __riscv_vadd_vv_i32m8(vdist, vmul, vl);
        }

        Word32 old = __riscv_vmv_x_s_i32m1_i32(vmin);
        vmin = __riscv_vredmin_vs_i32m8_i32m1(vdist, vmin, vl);
        Word32 new = __riscv_vmv_x_s_i32m1_i32(vmin);

        if (old != new) {
            vbool4_t veq = __riscv_vmseq_vx_i32m8_b4(vdist, new, vl);
            Word16 idx = __riscv_vfirst_m_b4(veq, vl);
            index = acc_cnt + idx;
        }

        p_dico += vl * M;
        acc_cnt += vl;
    }

    vint16m2_t vdico = __riscv_vle16_v_i16m2(dico_lsf_abs_8b + index * M, M);
    __riscv_vse16_v_i16m2(lsfq, vdico, M);
#else
    /*dist_min = 1.0e30f;*/
    dist_min = L_add(MAX_32, 0);
    p_dico = dico_lsf_abs_8b;        /*14Q1*1.28*/
    index = 0;
    move16();

    FOR (i = 0; i < 256; i++)
    {
        dist = L_add(0,0);
        FOR (j = 0; j < M; j++)
        {
            diff = sub(lsf[j], p_dico[j]);
            wdiff = shr(mult_r(w[j],diff),4);
            dist = L_mac(dist,wdiff,diff);
        }
        p_dico += M;

        if (L_sub(dist,dist_min) < 0)
        {
            index = add(i,0);        /* store index of new minimum */
        }
        dist_min = L_min(dist,dist_min);
    }

    /* quantized vector */
    p_dico = &dico_lsf_abs_8b[index * M];

    FOR (j = 0; j < M; j++)
    {
        /*lsfq[j] += *p_dico++;*/ /* += cause it's differential#
                                     -> since isfq[i] is 0, only data move is sufficient*/
        /*lsfq[j] = add(lsfq[j],*p_dico++);*/
        lsfq[j] = *p_dico++;
        move16();
    }
#endif


    return index;
}



