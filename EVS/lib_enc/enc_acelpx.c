/*====================================================================================
    EVS Codec 3GPP TS26.442 Nov 04, 2021. Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
  ====================================================================================*/


#include <memory.h>
#include <assert.h>
#include "stl.h"
#include "prot_fx.h"
#include "basop_util.h"
#include "options.h"
#include "rom_enc_fx.h"

#include "macro.h"

#define _1_Q11 (2048/*1.0f Q11*/) /* 1.0f in 4Q11 */

static void E_ACELP_update_cor(
    const Word16 pos[],    /* i */
    Word16 nb_pulse,       /* i */
    const Word16 sign[],   /* i */
    const Word16 R[],      /* i */
    const Word16 cor_in[], /* i */
    Word16 cor_out[]       /* o */
)
{
    Word16 sign_x, sign_y;
    const Word16 *pRx, *pRy;
    Word16 i, tmp;

    IF (sub(nb_pulse, 2) == 0)
    {
        /* Update product of autocorrelation and already fixed pulses. with the
         * two newly found ones */
        sign_x = sign[pos[0]];
        move16();
        sign_y = sign[pos[1]];
        move16();

        IF (s_xor(sign_x, sign_y) < 0)
        {
            i = 1;
            move16();
            if (sign_x > 0)
            {
                i = 0;
                move16();
            }
            pRx = R-pos[i];
            pRy = R-pos[1-i];
            /* different sign x and y */
#if defined(SUPPORT_VEC_32X) && defined(SUPPORT_VL256)
            if (cor_in != NULL) {
                vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                vint16m4_t vcor_in = __riscv_vle16_v_i16m4(cor_in, L_SUBFR);
                vint32m8_t vtmp = __riscv_vwsub_vv_i32m8(vrx, vry, L_SUBFR);
                vtmp = __riscv_vwadd_wv_i32m8(vtmp, vcor_in, L_SUBFR);
                vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);
            } else {
                vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                vint32m8_t vtmp = __riscv_vwsub_vv_i32m8(vrx, vry, L_SUBFR);
                vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);
            }
#else
            FOR (i=0; i<L_SUBFR; i++)
            {
                tmp = sub(pRx[i], pRy[i]);
                if (cor_in != NULL)
                {
                    tmp = add(tmp, cor_in[i]);
                }
                cor_out[i] = tmp;
                move16();
            }
#endif
        }
        ELSE
        {
            pRx = R-pos[0];
            pRy = R-pos[1];
            IF (sign_x > 0)
            {
                /* sign x and y is positive */
#if defined(SUPPORT_VEC_32X) && defined(SUPPORT_VL256)
                if (cor_in != NULL) {
                    vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                    vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                    vint16m4_t vcor_in = __riscv_vle16_v_i16m4(cor_in, L_SUBFR);
                    vint32m8_t vtmp = __riscv_vwadd_vv_i32m8(vrx, vry, L_SUBFR);
                    vtmp = __riscv_vwadd_wv_i32m8(vtmp, vcor_in, L_SUBFR);
                    vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                    __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);
                } else {
                    vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                    vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                    vint32m8_t vtmp = __riscv_vwadd_vv_i32m8(vrx, vry, L_SUBFR);
                    vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                    __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);

                }
#else
                FOR (i=0; i<L_SUBFR; i++)
                {
                    tmp = add(pRx[i], pRy[i]);
                    if (cor_in != NULL)
                    {
                        tmp = add(tmp, cor_in[i]);
                    }
                    cor_out[i] = tmp;
                    move16();
                }
#endif
            }
            ELSE
            {
                /* sign x and y is negative */
#if defined(SUPPORT_VEC_32X) && defined(SUPPORT_VL256)
                if (cor_in != NULL) {
                    vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                    vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                    vint16m4_t vcor_in = __riscv_vle16_v_i16m4(cor_in, L_SUBFR);
                    vint32m8_t vtmp = __riscv_vwsub_vv_i32m8(vcor_in, vrx, L_SUBFR);
                    vtmp = __riscv_vwsub_wv_i32m8(vtmp, vry, L_SUBFR);
                    vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                    __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);
                } else {
                    vint16m4_t vrx = __riscv_vle16_v_i16m4(pRx, L_SUBFR);
                    vint16m4_t vry = __riscv_vle16_v_i16m4(pRy, L_SUBFR);
                    vint32m8_t vtmp = __riscv_vwadd_vv_i32m8(vrx, vry, L_SUBFR);
                    vtmp = __riscv_vneg_v_i32m8(vtmp, L_SUBFR);
                    vint16m4_t vtmp16 = __riscv_vnclip_wx_i16m4(vtmp, 0, __RISCV_VXRM_RNU, L_SUBFR);
                    __riscv_vse16_v_i16m4(cor_out, vtmp16, L_SUBFR);
                }
#else
                FOR (i=0; i<L_SUBFR; i++)
                {
                    tmp = add(pRx[i], pRy[i]);
                    if (cor_in != NULL)
                    {
                        tmp = sub(cor_in[i], tmp);
                    }
                    if (cor_in == NULL)
                    {
                        tmp = negate(tmp);
                    }
                    cor_out[i] = tmp;
                    move16();
                }
#endif
            }
        }
    }
    ELSE IF (sub(nb_pulse, 4) == 0)
    {
        E_ACELP_update_cor(pos, 2, sign, R, cor_in, cor_out);
        E_ACELP_update_cor(pos+2, 2, sign, R, cor_out, cor_out);
    }
    else
    {
        assert(!"Number of pulses not supported");
    }
}


/* Iterations: nb_pos_ix*16 */
static void E_ACELP_2pulse_searchx(Word16 nb_pos_ix, Word16 track_x,
                                   Word16 track_y, Word16 *R, Word16 *ps, Word16 *alp,
                                   Word16 *ix, Word16 *iy, Word16 dn[],
                                   Word16 *dn2, Word16 cor[], Word16 sign[], Word16 sign_val_2)
{
    Word16 i,x;
    Word32 y;
    Word16 *pos_x, pos[2];
    Word32 xy_save;
    Word16 ps0, ps1, alp2_16, ps2, sq;
    Word32 alp0, alp1, alp2, s;
    Word16 *pR, sgnx;
    Word16 ik;
#if defined(SUPPORT_VEC_32X)
    Word16 sq_arr[L_SUBFR / 4];
    Word16 alp2_arr[L_SUBFR / 4];
    union {
        Word16 i16[2];
        Word32 i32;
    } sqk_union, alpk_union;
    Word16 *sqk = sqk_union.i16;
    Word16 *alpk = alpk_union.i16;
#else
    Word16 sqk[2], alpk[2];
#endif
    static int flag = 0;


    /* eight dn2 max positions per track */
    pos_x = &dn2[shl(track_x,3)];
    move16();
    /* save these to limit memory searches */
    ps0 = *ps;
    move16();
    /*alp0 = *alp + 2.0f*R[0];                         move16();*/
    alp0 = L_deposit_h(*alp);               /* Qalp = Q_R*Q_signval */
    alp0 = L_mac(alp0, R[0], sign_val_2);

    /* Ensure that in the loop below s > 0 in the first iteration, the actual values do not matter. */
    sqk[0] = -1;
    move16();
    alpk[0] = 1;
    move16();
    x = pos_x[0];
    move16();
    sgnx = sign[track_y];
    move16();
    if (sign[x] < 0)
    {
        sgnx = negate(sgnx);
    }
    if (mac_r(L_mac(L_mac(alp0, cor[x], sign[x]), cor[track_y], sign[track_y]), R[track_y-x], sgnx) < 0)
    {
        sqk[0] = 1;
        move16();
    }
    ik = 0;
    move16();

    xy_save = L_mac0(L_deposit_l(track_y), track_x, L_SUBFR);

    /* loop track 1 */
    FOR (i=0; i<nb_pos_ix; i++)
    {
        x = pos_x[i];
        move16();
        sgnx = sign[x];
        move16();
        /* dn[x] has only nb_pos_ix positions saved */
        /*ps1 = ps0 + dn[x];                            INDIRECT(1);ADD(1);*/
        ps1 = add(ps0, dn[x]);
        /*alp1 = alp0 + 2*sgnx*cor[x];                  INDIRECT(1);MULT(1); MAC(1);*/
        alp1 = L_mac(alp0, cor[x], sgnx); /* Qalp = (Q_R=Q_cor)*Q_signval */

        pR = R-x;

#if defined(SUPPORT_VEC_32X)
        size_t vl = L_SUBFR / 4;
        ptrdiff_t bstride = sizeof(Word16) * 4;
        vint16m2_t vdn = __riscv_vlse16_v_i16m2(dn + track_y, bstride, vl);
        vdn = __riscv_vadd_vx_i16m2(vdn, ps1, vl);
        vdn = __riscv_vsmul_vv_i16m2(vdn, vdn, __RISCV_VXRM_RNU, vl);
        __riscv_vse16_v_i16m2(sq_arr, vdn, vl);

        vint16m2_t vcor = __riscv_vlse16_v_i16m2(cor + track_y, bstride, vl);
        vint16m2_t vsign = __riscv_vlse16_v_i16m2(sign + track_y, bstride, vl);
        vint32m4_t vmul32 = __riscv_vwcvt_x_x_v_i32m4(vcor, vl);
        vmul32 = __riscv_vsll_vx_i32m4(vmul32, 16, vl);
        vint32m4_t vsign32 = __riscv_vwcvt_x_x_v_i32m4(vsign, vl);
        vsign32 = __riscv_vsll_vx_i32m4(vsign32, 16, vl);
        vmul32 = __riscv_vsmul_vv_i32m4(vmul32, vsign32, __RISCV_VXRM_RNU, vl);

        vint32m4_t valp2 = __riscv_vmv_v_x_i32m4(alp1, vl);
        valp2 = __riscv_vsadd_vv_i32m4(valp2, vmul32, vl);

        vint16m2_t vr = __riscv_vlse16_v_i16m2(pR + track_y, bstride, vl);
        vmul32 = __riscv_vwcvt_x_x_v_i32m4(vr, vl);
        vmul32 = __riscv_vsll_vx_i32m4(vmul32, 16, vl);
        // assert(sgnx != 0);
        if (sgnx < 0) {
            vsign32 = __riscv_vneg_v_i32m4(vsign32, vl);
        }

        vmul32 = __riscv_vsmul_vv_i32m4(vmul32, vsign32, __RISCV_VXRM_RNU, vl);
        valp2 = __riscv_vsadd_vv_i32m4(valp2, vmul32, vl);
        valp2 = __riscv_vsadd_vx_i32m4(valp2, 0x8000, vl);
        vint16m2_t valp2_16 = __riscv_vnsra_wx_i16m2(valp2, 16, vl);
        __riscv_vse16_v_i16m2(alp2_arr, valp2_16, vl);

        for(int i = 0; i < vl; ++i)
        {
#if defined(SUPPORT_DSP_STD)
            alpk[1-ik] = alp2_arr[i];
            sqk[1-ik] = sq_arr[i];
            if(ik == 0) {
                s = __RV_SMXDS(sqk_union.i32, alpk_union.i32);
            } else {
                s = __RV_SMXDS(alpk_union.i32, sqk_union.i32);
            }
#else
            alpk[1-ik] = alp2_arr[i];
            sqk[1-ik] = sq_arr[i];
            /*s = (alpk * sq) - (sqk * alp2);            MULT(1);MAC(1);*/
            s = L_msu(L_mult(alpk[ik], sq_arr[i]), sqk[ik], alp2_arr[i]);	/* Q_sq = Q_sqk, Q_alpk = Q_alp */
#endif
            if (s > 0)
            {
                ik = sub(1, ik);
                xy_save = L_mac0(i * 4 + track_y, x, L_SUBFR);
            }
            // assert( ((s >= 0 && i==0 && y == track_y)) || (y > track_y) || (i > 0));
        }
#else
        FOR (y = track_y; y < L_SUBFR; y += 4)
        {
            /*ps2 = ps1 + dn[y];                         ADD(1);*/
            ps2 = add(ps1, dn[y]);

            /*alp2 = alp1 + 2.0f*sign[y]*(cor[y] + sgnx*pR[y]);   MULT(1); MAC(2);*/
            /*alp2 = alp1 + 2.0f*sign[y]*cor[y] + 2.0f*sign[y]*sgnx*pR[y];   MULT(1); MAC(2);*/
            assert(sign[y] == sign_val_2 || sign[y] == -sign_val_2);

            /* Compiler warning workaround (not instrumented) */
            assert(sgnx != 0);
            alp2_16 = 0;

            alp2 = L_mac(alp1, cor[y], sign[y]); /* Qalp = (Q_R=Q_cor)*Q_signval */
            if (sgnx > 0)
            {
                alp2_16 = mac_r(alp2, pR[y], sign[y]); /* Qalp = (Q_R=Q_cor)*Q_signval */
            }
            if (sgnx < 0)
            {
                alp2_16 = msu_r(alp2, pR[y], sign[y]);	/* Qalp = (Q_R=Q_cor)*Q_signval */
            }
            alpk[1-ik] = alp2_16;
            move16();

            /*sq = ps2 * ps2;                            MULT(1);*/
            sq = mult_r(ps2, ps2);	/* (3+3)Q -> 6Q9 */
            sqk[1-ik] = sq;
            move16();


            /*s = (alpk * sq) - (sqk * alp2);            MULT(1);MAC(1);*/
            s = L_msu(L_mult(alpk[ik], sq), sqk[ik], alp2_16);	/* Q_sq = Q_sqk, Q_alpk = Q_alp */
            if (s > 0)
            {
                ik = sub(1, ik);
            }
            if (s > 0)
            {
                xy_save = L_mac0(y, x, L_SUBFR);
            }
            assert( ((s >= 0 && i==0 && y == track_y)) || (y > track_y) || (i > 0));
        }
#endif
    }
    ps1 = extract_l(xy_save);
    pos[1] = s_and(ps1, L_SUBFR-1);
    move16();
    pos[0] = lshr(ps1, 6);
    move16();
    /* Update numerator */
    *ps = add(add(ps0,dn[pos[0]]),dn[pos[1]]);
    move16();

    /* Update denominator */
    *alp = alpk[ik];
    move16();

    E_ACELP_update_cor(pos, 2, sign, R, cor, cor);

    *ix = pos[0];
    move16();
    *iy = pos[1];
    move16();

    assert(((pos[0] & 3) == track_x) && ((pos[1] & 3) == track_y)); /* sanity check */
}


/* static */
static void E_ACELP_1pulse_searchx(UWord8 tracks[2],
                                   Word16 *R, Word16 *ps, Word16 *alp,
                                   Word16 *ix, Word16 dn[],
                                   Word16 cor[], Word16 sign[], Word16 sign_val_1)
{
    Word16 x, x_save = 0;
    Word16 ps0;
    Word32 alp0;
    Word16 ps1, sq;
    Word16 alp1;
    Word32 s;
    Word16 ntracks, t;
    Word16 ik;
#if defined(SUPPORT_VEC_32X)
    Word16 sq_arr[L_SUBFR / 4];
    Word16 alp1_arr[L_SUBFR / 4];
    union {
        Word16 i16[2];
        Word32 i32;
    } sqk_union, alpk_union;
    Word16 *sqk = sqk_union.i16;
    Word16 *alpk = alpk_union.i16;
#else
    Word16 sqk[2], alpk[2];
#endif

    /* save these to limit memory searches */
    /*alp0 = *alp + R[0];                              INDIRECT(1);*/
    ps0 = *ps;
    move16();
    alp0 = L_deposit_h(*alp);
    alp0 = L_mac(alp0, R[0], sign_val_1);    /* Qalp = (Q_R=Q_cor)*Q_signval */

    /* Ensure that in the loop below s > 0 in the first iteration, the actual values do not matter. */
    move16();
    move16();
    alpk[0] = 1;
    sqk[0] = -1;
    ik = 0;
    move16();
    if (mac_r(alp0, cor[tracks[0]], sign[tracks[0]]) < 0)
    {
        sqk[0] = 1;
        move16();
    }

    x_save = tracks[0];
    move16();

    ntracks = 1;
    if (sub(tracks[1], tracks[0]) != 0)
    {
        ntracks = 2;
        move16();
    }
    FOR (t=0; t<ntracks; ++t)
    {
#if defined(SUPPORT_VEC_32X)
        size_t vl = L_SUBFR / 4;
        ptrdiff_t bstride = sizeof(Word16) * 4;
        vint16m2_t vdn = __riscv_vlse16_v_i16m2(dn + tracks[t], bstride, vl);
        vdn = __riscv_vsadd_vx_i16m2(vdn, ps0, vl);
        vdn = __riscv_vsmul_vv_i16m2(vdn, vdn, __RISCV_VXRM_RNU, vl);
        __riscv_vse16_v_i16m2(sq_arr, vdn, vl);

        vint16m2_t vcor = __riscv_vlse16_v_i16m2(cor + tracks[t], bstride, vl);
        vint16m2_t vsign = __riscv_vlse16_v_i16m2(sign + tracks[t], bstride, vl);
        vint32m4_t vmul32 = __riscv_vwcvt_x_x_v_i32m4(vcor, vl);
        vmul32 = __riscv_vsll_vx_i32m4(vmul32, 16, vl);
        vint32m4_t vsign32 = __riscv_vwcvt_x_x_v_i32m4(vsign, vl);
        vsign32 = __riscv_vsll_vx_i32m4(vsign32, 16, vl);
        vmul32 = __riscv_vsmul_vv_i32m4(vmul32, vsign32, __RISCV_VXRM_RNU, vl);

        vint32m4_t valp1 = __riscv_vmv_v_x_i32m4(alp0, vl);
        valp1 = __riscv_vsadd_vv_i32m4(valp1, vmul32, vl);
        valp1 = __riscv_vsadd_vx_i32m4(valp1, 0x8000, vl);
        vint16m2_t valp1_16 = __riscv_vnsra_wx_i16m2(valp1, 16, vl);
        __riscv_vse16_v_i16m2(alp1_arr, valp1_16, vl);

        for(int i = 0; i < vl; ++i)
        {
#if defined(SUPPORT_DSP_STD)
            alpk[1-ik] = alp1_arr[i];
            sqk[1-ik] = sq_arr[i];
            if(ik == 0) {
                s = __RV_SMXDS(sqk_union.i32, alpk_union.i32);
            } else {
                assert(ik == 1);
                s = __RV_SMXDS(alpk_union.i32, sqk_union.i32);
            }
#else
            alpk[1-ik] = alp1_arr[i];
            sqk[1-ik] = sq_arr[i];
            /*s = (alpk * sq) - (sqk * alp2);            MULT(1);MAC(1);*/
            s = L_msu(L_mult(alpk[ik], sq_arr[i]), sqk[ik], alp1_arr[i]);	/* Q_sq = Q_sqk, Q_alpk = Q_alp */
#endif
            if (s > 0)
            {
                ik = sub(1, ik);
                x_save = i * 4 + tracks[t];
            }
            // assert( ((s >= 0 && i==0 && y == track_y)) || (y > track_y) || (i > 0));
        }
#else
        FOR (x = tracks[t]; x < L_SUBFR; x += 4)
        {
            /* ps1 = ps0 + dn[x];                             ADD(1);*/
            ps1 = add(ps0, dn[x]);
            /* alp1 = alp0 + 2*sign[x]*cor[x];                MAC(1); MULT(1);*/
            assert(sign[x] == sign_val_1<<1 || sign[x] == -(sign_val_1<<1));
            alp1 = mac_r(alp0, cor[x], sign[x]);  /* Qalp = (Q_R=Q_cor)*Q_signval */
            alpk[1-ik] = alp1;
            move16();


            /*sq = ps1 * ps1;                                MULT(1);*/
            sq = mult_r(ps1, ps1);   /* 6Q9 */
            sqk[1-ik] = sq;
            move16();

            /*s = (alpk[ik] * sq) - (sqk[ik] * alp1);                MULT(1);MAC(1);*/
            s = L_msu(L_mult(alpk[ik], sq), sqk[ik], alp1);

            if (s > 0)
            {
                ik = sub(1, ik);
            }
            if (s > 0)
            {
                x_save = x;
                move16();
            }
            assert( t>0 || ((s >= 0) && (x == tracks[t])) || x > tracks[t]);
        }
#endif
    }

    *ps = add(ps0, dn[x_save]);
    move16();
    *alp = alpk[ik];
    move16();
    *ix = x_save;
    move16();
}


/* Autocorrelation method for searching pulse positions effectively
 * Algorithm is identical to traditional covariance method. */
void E_ACELP_4tsearchx(Word16 dn[], const Word16 cn[], Word16 Rw[], Word16 code[], const PulseConfig *config, Word16 ind[])
{
    Word16 sign[L_SUBFR], vec[L_SUBFR];
    Word16 cor[L_SUBFR];
    Word16 R_buf[2*L_SUBFR-1], *R;
    Word16 dn2[L_SUBFR];
    Word16 ps2k, ps /* same format as dn[] */, ps2, alpk, alp = 0 /* Q13 and later Q_Rw*Q_signval=Q_cor*Q_signval */;
    Word32 s;
    Word16 codvec[NB_PULSE_MAX];
    Word16 pos_max[4];
    Word16 dn2_pos[8 * 4];
    UWord8 ipos[NB_PULSE_MAX];
    Word16 i, j, k, st, pos = 0;
    Word16 scale;
    Word16 sign_val_1, sign_val_2;
    Word16 nb_pulse, nb_pulse_m2;

    ps = 0;      /* to avoid compilation warnings */



    alp = config->alp; /* Q13 */                                                move16();
    nb_pulse = config->nb_pulse;
    move16();
    nb_pulse_m2 = sub(nb_pulse, 2);

    /* Init to avoid crash when the search does not find a solution */
#if defined(SUPPORT_VEC_32X)
    {
        vint16m8_t vid =
            __riscv_vreinterpret_v_u16m8_i16m8(__riscv_vid_v_u16m8(nb_pulse));
        __riscv_vse16_v_i16m8(codvec, vid, nb_pulse);
    }
#else
    FOR (k=0; k<nb_pulse; k++)
    {
        codvec[k] = k;
        move16();
    }
#endif

    scale = 0;
    move16();
#if defined(SUPPORT_VEC_32X)
    size_t vl = L_SUBFR / 2;
    vint16m4_t vrw0 = __riscv_vle16_v_i16m4(Rw, vl);
    vint16m4_t vrw1 = __riscv_vle16_v_i16m4(Rw + vl, vl);
    vint32m8_t vmul0 = __riscv_vwmul_vv_i32m8(vrw0, vrw0, vl);
    vint32m8_t vmul1 = __riscv_vwmul_vv_i32m8(vrw1, vrw1, vl);
    vint32m1_t vsum = __riscv_vmv_s_x_i32m1(0, 1);
    vsum = __riscv_vredsum_vs_i32m8_i32m1(vmul0, vsum, vl);
    vsum = __riscv_vredsum_vs_i32m8_i32m1(vmul1, vsum, vl);
    s = __riscv_vmv_x_s_i32m1_i32(vsum);
#else
    s = L_mult0(Rw[0], Rw[0]);
    FOR (i = 1; i < L_SUBFR; i++)
    {
        s = L_mac0(s, Rw[i], Rw[i]);
    }
#endif
    if (s_and(sub(nb_pulse, 9) >= 0, L_sub(s, 0x800000) > 0))
    {
        scale = -1;
        move16();
    }
    if (s_and(sub(nb_pulse, 13) >= 0, L_sub(s, 0x4000000) > 0))
    {
        scale = -2;
        move16();
    }
    IF (sub(nb_pulse, 18) >= 0)
    {
        if (L_sub(s, 0x200000) > 0)
        {
            scale = -1;
            move16();
        }
        if (L_sub( s, 0x400000 ) > 0)
        {
            scale = -2;
            move16();
        }
        if (L_sub( s, 0x4000000 ) > 0)
        {
            scale = -3;
            move16();
        }
    }
    if (s_and(sub(nb_pulse, 28) >= 0, L_sub(s, 0x800000) > 0))
    {
        scale = -3;
        move16();
    }
    if (s_and(sub(nb_pulse, 36) >= 0, L_sub(s, 0x4000000) > 0))
    {
        scale = -4;
        move16();
    }

    /* Set up autocorrelation vector */
    R = R_buf+L_SUBFR-1;
    Copy_Scale_sig(Rw, R, L_SUBFR, scale);
#if defined(SUPPORT_VEC_32X)
    {
        vint16m8_t vr = __riscv_vle16_v_i16m8(R, L_SUBFR);
        __riscv_vsse16_v_i16m8(R, -sizeof(Word16), vr, L_SUBFR);
    }
#else
    FOR (k=1; k<L_SUBFR; k++)
    {
        R[-k] = R[k];
        move16();
    }
#endif

    /* Sign value */
    sign_val_2 = 0x2000;
    move16();
    if (sub(nb_pulse, 24) >= 0)
    {
        sign_val_2 = shr(sign_val_2, 1);
    }
    sign_val_1 = shr(sign_val_2, 1);

    /*
     * Find sign for each pulse position.
     */
    E_ACELP_pulsesign(cn, dn, dn2, sign, vec, alp, sign_val_2, L_SUBFR);

    /*
     * Select the most important 8 position per track according to dn2[].
     */
    E_ACELP_findcandidates(dn2, dn2_pos, pos_max);

    /*
     * Deep first search:
     */

    /* Ensure that in the loop below s > 0 in the first iteration, the actual values do not matter. */
    ps2k = -1;
    move16();
    alpk = 1;
    move16();

    /* Number of iterations */
    FOR (k = 0; k < config->nbiter; k++)
    {
        E_ACELP_setup_pulse_search_pos(config, k, ipos);

        /* index to first non-fixed position */
        pos = config->fixedpulses;
        move16();

        IF (config->fixedpulses == 0)/* 1100, 11, 1110, 1111, 2211 */
        {
            ps = 0;
            move16();
            alp = 0;
            move16();
            set16_fx(cor, 0, L_SUBFR);
        }
        ELSE
        {
            assert(config->fixedpulses == 2 || config->fixedpulses == 4);

            /* set fixed positions */
            FOR (i=0; i<pos; ++i)
            {
                ind[i] = pos_max[ipos[i]];
                move16();
            }

            /* multiplication of autocorrelation with signed fixed pulses */
            E_ACELP_update_cor(ind, config->fixedpulses, sign, R, NULL, cor);

            /* normalisation contribution of fixed part */
            s = L_mult0(cor[ind[0]], sign[ind[0]]);
            ps = dn[ind[0]];
            move16();
            FOR (i=1; i<pos; ++i)
            {
                s = L_mac0(s, cor[ind[i]], sign[ind[i]]);   /*Q12+Q9+1=Q6 */
                ps = add(ps, dn[ind[i]]);
            }
            alp = round_fx(s);                          /*mac0 >>1 sign = 2*/
        }

        /* other stages of 2 pulses */
        st = 0;
        move16();
        FOR (j = pos; j < nb_pulse; j += 2)
        {
            IF (sub(nb_pulse_m2, j) >= 0) /* pair-wise search */
            {
                /*
                 * Calculate correlation of all possible positions
                 * of the next 2 pulses with previous fixed pulses.
                 * Each pulse can have 16 possible positions.
                 */

                E_ACELP_2pulse_searchx(config->nbpos[st], ipos[j], ipos[j + 1], R, &ps, &alp,
                                       &ind[j], &ind[j+1], dn, dn2_pos, cor, sign, sign_val_2);

            }
            ELSE /* single pulse search */
            {
                E_ACELP_1pulse_searchx(&ipos[j], R, &ps, &alp,
                &ind[j], dn, cor, sign, sign_val_1);
            }


            st = add(st, 1);
        }

        /* memorise the best codevector */
        /*ps2 = ps * ps;                                            MULT(1);*/
        ps2 = mult(ps, ps);

        /*s = (alpk * ps2) - (ps2k * alp);                          MULT(2);ADD(1);*/
        s = L_msu(L_mult(alpk, ps2), ps2k, alp);

        IF (s > 0)
        {
            ps2k = ps2;
            move16();
            alpk = alp;
            move16();
            Copy(ind, codvec, nb_pulse);
        }
    }


    /*
     * Store weighted energy of code, build the codeword and index of codevector.
     */
    E_ACELP_build_code(nb_pulse, codvec, sign, code, ind);
}
