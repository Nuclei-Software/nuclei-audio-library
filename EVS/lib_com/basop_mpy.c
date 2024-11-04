/*====================================================================================
    EVS Codec 3GPP TS26.442 Nov 04, 2021. Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
  ====================================================================================*/

#include "basop_mpy.h"
#include "stl.h"
#include "options.h" /* Needed for Stack Counting Mechanism Macros (when Instrumented) */

Word32 Mpy_32_16_1(Word32 x, Word16 y)
{
    Word32 mh;
    UWord16 ml;

    Mpy_32_16_ss(x, y, &mh, &ml);

    return (mh);
}

Word32 Mpy_32_16_r(Word32 x, Word16 y)
{
    Word32 mh;
    UWord16 ml;

    Mpy_32_16_ss(x, y, &mh, &ml);

    if(s_and(ml, -32768 /* 0x8000 */))
    {
        mh = L_add(mh, 1);
    }

    return (mh);
}

Word32 Mpy_32_32(Word32 x, Word32 y)
{
    Word32 mh;
    UWord32 ml;

    Mpy_32_32_ss(x, y, &mh, &ml);

    return (mh);
}

Word32 Mpy_32_32_r(Word32 x, Word32 y)
{
    Word32 mh;
    UWord32 ml;

    Mpy_32_32_ss(x, y, &mh, &ml);

    if(L_and(ml, 0x80000000))
    {
        mh = L_add(mh, 1);
    }

    return (mh);
}

