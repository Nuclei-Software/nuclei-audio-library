/*
  ===========================================================================
   File: ENH40.H                                         v.2.3 - 30.Nov.2009
  ===========================================================================

            ITU-T  STL  BASIC OPERATORS

            40-BIT ARITHMETIC OPERATORS

   History:
   07 Nov 04   v2.0     Incorporation of new 32-bit / 40-bit / control
                        operators for the ITU-T Standard Tool Library as 
                        described in Geneva, 20-30 January 2004 WP 3/16 Q10/16
                        TD 11 document and subsequent discussions on the
                        wp3audio@yahoogroups.com email reflector.

   March 06    v2.1     Changed to improve portability.

   31 Mar 15   v2.1E    Removal of operators not used in the EVS codec.

  ============================================================================
*/


#ifndef _ENH40_H
#define _ENH40_H

// #include "stl.h"
#include "basop32.h"

 /*****************************************************************************
 *
 *  Prototypes for enhanced 40 bit arithmetic operators
 *
 *****************************************************************************/

__STATIC_FORCEINLINE_DSP void Mpy_32_16_ss( Word32 L_var1, Word16 var2,   Word32 *L_varout_h, UWord16 *varout_l);
__STATIC_FORCEINLINE_DSP void Mpy_32_32_ss( Word32 L_var1, Word32 L_var2, Word32 *L_varout_h, UWord32 *L_varout_l);

#if defined(SUPPORT_DSP_STD)
__STATIC_FORCEINLINE_DSP void Mpy_32_16_ss( Word32 L_var1, Word16 var2,   Word32 *L_varout_h, UWord16 *varout_l) {
    /* WARNING: if L_var1 == 0x80000000, var2 == 0x8000, the varout_l is wrong */
    *L_varout_h = __RV_KMMWB2(L_var1, var2);
    *varout_l = (UWord16)(L_var1 * var2) << 1;
}

__STATIC_FORCEINLINE_DSP void Mpy_32_32_ss( Word32 L_var1, Word32 L_var2, Word32 *L_varout_h, UWord32 *L_varout_l) {
    /* WARNING: if L_var1 == 0x80000000, L_var2 == 0x80000000, the L_varout_l is wrong */
    *L_varout_h = __RV_KWMMUL(L_var1, L_var2);
    *L_varout_l = (UWord32)(L_var1 * L_var2) << 1;
}
#endif

#endif /*_ENH40_H*/


/* end of file */


