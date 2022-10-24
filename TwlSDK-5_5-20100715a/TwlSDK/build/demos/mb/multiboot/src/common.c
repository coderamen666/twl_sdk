/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     common.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif


#include "common.h"


/*
 * Common features used by this entire demo
 */


/******************************************************************************/
/* Constants */

/* The array of program information that this demo downloads */
const MBGameRegistry mbGameList[GAME_PROG_MAX] = {
    {
     "/em.srl",                        // The file path of the multiboot child device binary
     L"edgeDemo",                      // Game name
     L"edgemarking demo\ntesttesttest", // Game content description
     "/data/icon.char",                // Icon character data file path
     "/data/icon.plt",                 // Icon palette data file path
     0x12123434,                       // GameGroupID(GGID)
     16,                               // Max number of players
     },
    {
     "/pol_toon.srl",
     L"PolToon",
     L"toon rendering",
     "/data/icon.char",
     "/data/icon.plt",
     0x56567878,
     8,
     },
};


/******************************************************************************/
/* Variables */

/* The work region to be allocated to the MB library */
u32     cwork[MB_SYSTEM_BUF_SIZE / sizeof(u32)];

/* The game sequence state of this demo */
u8      prog_state;


/******************************************************************************/
/* Functions */

/* Key trigger detection */
u16 ReadKeySetTrigger(u16 keyset)
{
    static u16 cont = 0xffff;          /* Measures to handle A Button presses inside IPL */
    u16     trigger = (u16)(keyset & (keyset ^ cont));
    cont = keyset;
    return trigger;
}

/* Rotate the val value the offset amount within the min - max range */
BOOL RotateU8(u8 *val, u8 min, u8 max, s8 offset)
{
    int     lVal = (int)*val - min;
    int     div = max - min + 1;

    if (div == 0)
        return FALSE;

    lVal = (lVal + offset + div) % div + min;

    *val = (u8)lVal;
    return TRUE;
}
