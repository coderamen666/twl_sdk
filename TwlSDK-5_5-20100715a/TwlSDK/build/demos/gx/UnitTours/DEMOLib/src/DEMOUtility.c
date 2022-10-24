/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOUtility.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "DEMOUtility.h"

#define DEMO_INTENSITY_DF   23
#define DEMO_INTENSITY_AM   8
#define DEMO_INTENSITY_SP   31

static const GXRgb DEMO_DIFFUSE_COL =
GX_RGB(DEMO_INTENSITY_DF, DEMO_INTENSITY_DF, DEMO_INTENSITY_DF);
static const GXRgb DEMO_AMBIENT_COL =
GX_RGB(DEMO_INTENSITY_AM, DEMO_INTENSITY_AM, DEMO_INTENSITY_AM);
static const GXRgb DEMO_SPECULAR_COL =
GX_RGB(DEMO_INTENSITY_SP, DEMO_INTENSITY_SP, DEMO_INTENSITY_SP);
static const GXRgb DEMO_EMISSION_COL = GX_RGB(0, 0, 0);

static const u32 LIGHTING_L_DOT_S_SHIFT = 8;


void DEMO_Set3DDefaultMaterial(BOOL bUsediffuseAsVtxCol, BOOL bUseShininessTbl)
{
    G3_MaterialColorDiffAmb(DEMO_DIFFUSE_COL,   // diffuse
                            DEMO_AMBIENT_COL,   // ambient
                            bUsediffuseAsVtxCol // use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(DEMO_SPECULAR_COL,  // specular
                            DEMO_EMISSION_COL,  // emission
                            bUseShininessTbl    // use shininess table if TRUE
        );
}


#include <nitro/code32.h>              // avoid byte access problems
void DEMO_Set3DDefaultShininessTable()
{
    u8      i;
    u32     shininess_table[32];
    u8     *pShininess = (u8 *)&shininess_table[0];
    const u8 tableLength = 32 * sizeof(u32);

    for (i = 0; i < tableLength - 1; i++)
    {
        // ShininessTable is ( 0.8 ) fixed point foramt, so we have to right-shift 8 bit for One multiply.
        // pShininess = (i*2+1)^4
        pShininess[i] = (u8)(((s64)(i * 2 + 1) * (i * 2 + 1) * (i * 2 + 1) * (i * 2 + 1)) >> (LIGHTING_L_DOT_S_SHIFT * (4 - 1)));       // byte access
    }

    // set the max value specialy   
    pShininess[tableLength - 1] = 0xFF;

    G3_Shininess(&shininess_table[0]);
}

#include <nitro/codereset.h>
