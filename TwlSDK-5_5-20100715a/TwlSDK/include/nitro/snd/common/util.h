/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - common
  File:     utl.h

  Copyright 2004-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SND_COMMON_UTIL_H_
#define NITRO_SND_COMMON_UTIL_H_

#include <nitro/types.h>

#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))
#include <nitro/misc.h>
#else
#define SDK_MINMAX_ASSERT(exp, min, max)           ((void) 0)
#endif /* SDK_FROM_TOOL */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
    Macro Definitions
 ******************************************************************************/

#define SND_PITCH_DIVISION_BIT     6   // Half-tone resolution. (Number of bits.)
#define SND_PITCH_DIVISION_RANGE ( 1 << SND_PITCH_DIVISION_BIT )        // Half-tone resolution

#define SND_PITCH_TABLE_SIZE     ( 12 * SND_PITCH_DIVISION_RANGE )      // Pitch calculation table size
#define SND_PITCH_TABLE_BIAS       0x10000
#define SND_PITCH_TABLE_SHIFT     16

#define SND_VOLUME_DB_MIN        (-723) // -72.3.4 dB = -inf
#define SND_VOLUME_DB_MAX         0
#define SND_VOLUME_TABLE_SIZE ( SND_VOLUME_DB_MAX - SND_VOLUME_DB_MIN + 1 )

#define SND_CALC_DECIBEL_SCALE_MAX    127

#define SND_DECIBEL_TABLE_SIZE        128
#define SND_DECIBEL_SQUARE_TABLE_SIZE 128

#define SND_SIN_TABLE_SIZE 32
#define SND_SIN_PERIOD ( SND_SIN_TABLE_SIZE * 4 )

/******************************************************************************
    Private Variable Declarations
 ******************************************************************************/

extern const s16 SNDi_DecibelTable[SND_DECIBEL_TABLE_SIZE];
extern const s16 SNDi_DecibelSquareTable[SND_DECIBEL_SQUARE_TABLE_SIZE];

/******************************************************************************
    Inline Function Definitions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         SND_CalcDecibel

  Description:  Calculate decibels from scale factor.

  Arguments:    scale:      scale factor

  Returns:      decibel
 *---------------------------------------------------------------------------*/
#ifndef _MSC_VER
static  inline
#else
__inline
#endif
s16 SND_CalcDecibel(int scale)
{
    SDK_MINMAX_ASSERT(scale, 0, SND_CALC_DECIBEL_SCALE_MAX);

    return SNDi_DecibelTable[scale];
}

/*---------------------------------------------------------------------------*
  Name:         SND_CalcDecibelSquare

  Description:  Calculate decibels from square scale factor.

  Arguments:    scale:      square scale factor

  Returns:      decibel
 *---------------------------------------------------------------------------*/
#ifndef _MSC_VER
static  inline
#else
__inline
#endif
s16 SND_CalcDecibelSquare(int scale)
{
    SDK_MINMAX_ASSERT(scale, 0, SND_CALC_DECIBEL_SCALE_MAX);

    return SNDi_DecibelSquareTable[scale];
}

/******************************************************************************
    Public Function Declarations
 ******************************************************************************/

u16     SND_CalcTimer(int orgTimer, int pitch);
u16     SND_CalcChannelVolume(int dB);

#ifdef SDK_ARM7

u16     SND_CalcRandom(void);
s8      SND_SinIdx(int index);

#endif /* SDK_ARM7 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_COMMON_UTIL_H_ */
