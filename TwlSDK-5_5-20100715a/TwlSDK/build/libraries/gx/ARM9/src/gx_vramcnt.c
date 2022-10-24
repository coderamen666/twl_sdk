/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     gx_vramcnt.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro/gx/gx_vramcnt.h>
#ifdef SDK_NITRO
#include <nitro/hw/ARM9/ioreg_G3X.h>
#include <nitro/hw/ARM9/ioreg_GX.h>
#include <nitro/hw/ARM9/ioreg_GXS.h>
#include <nitro/hw/ARM9/mmap_vram.h>
#else
#include <twl/hw/ARM9/ioreg_G3X.h>
#include <twl/hw/ARM9/ioreg_GX.h>
#include <twl/hw/ARM9/ioreg_GXS.h>
#include <twl/hw/ARM9/mmap_vram.h>
#endif
#include "gxstate.h"


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-A (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_A_DISABLE = 0,
    GX_VRAMCNT_A_LCDC_0x06800000 =
        (0 << REG_GX_VRAMCNT_A_MST_SHIFT) | (0 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_A_MST_SHIFT) | (0 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_BG_0x06020000 =
        (1 << REG_GX_VRAMCNT_A_MST_SHIFT) | (1 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_BG_0x06040000 =
        (1 << REG_GX_VRAMCNT_A_MST_SHIFT) | (2 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_BG_0x06060000 =
        (1 << REG_GX_VRAMCNT_A_MST_SHIFT) | (3 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_OBJ_0x06400000 =
        (2 << REG_GX_VRAMCNT_A_MST_SHIFT) | (0 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_OBJ_0x06420000 =
        (2 << REG_GX_VRAMCNT_A_MST_SHIFT) | (1 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_TEX_0 =
        (3 << REG_GX_VRAMCNT_A_MST_SHIFT) | (0 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_TEX_1 =
        (3 << REG_GX_VRAMCNT_A_MST_SHIFT) | (1 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_TEX_2 =
        (3 << REG_GX_VRAMCNT_A_MST_SHIFT) | (2 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT),
    GX_VRAMCNT_A_TEX_3 =
        (3 << REG_GX_VRAMCNT_A_MST_SHIFT) | (3 << REG_GX_VRAMCNT_A_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_A_E_SHIFT)
}
GX_VRAMCNT_A;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-B (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_B_DISABLE = 0,
    GX_VRAMCNT_B_LCDC_0x06820000 =
        (0 << REG_GX_VRAMCNT_B_MST_SHIFT) | (0 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_B_MST_SHIFT) | (0 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_BG_0x06020000 =
        (1 << REG_GX_VRAMCNT_B_MST_SHIFT) | (1 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_BG_0x06040000 =
        (1 << REG_GX_VRAMCNT_B_MST_SHIFT) | (2 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_BG_0x06060000 =
        (1 << REG_GX_VRAMCNT_B_MST_SHIFT) | (3 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_OBJ_0x06400000 =
        (2 << REG_GX_VRAMCNT_B_MST_SHIFT) | (0 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_OBJ_0x06420000 =
        (2 << REG_GX_VRAMCNT_B_MST_SHIFT) | (1 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_TEX_0 =
        (3 << REG_GX_VRAMCNT_B_MST_SHIFT) | (0 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_TEX_1 =
        (3 << REG_GX_VRAMCNT_B_MST_SHIFT) | (1 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_TEX_2 =
        (3 << REG_GX_VRAMCNT_B_MST_SHIFT) | (2 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT),
    GX_VRAMCNT_B_TEX_3 =
        (3 << REG_GX_VRAMCNT_B_MST_SHIFT) | (3 << REG_GX_VRAMCNT_B_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_B_E_SHIFT)
}
GX_VRAMCNT_B;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-C (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_C_DISABLE = 0,
    GX_VRAMCNT_C_LCDC_0x06840000 =
        (0 << REG_GX_VRAMCNT_C_MST_SHIFT) | (0 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_C_MST_SHIFT) | (0 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_BG_0x06020000 =
        (1 << REG_GX_VRAMCNT_C_MST_SHIFT) | (1 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_BG_0x06040000 =
        (1 << REG_GX_VRAMCNT_C_MST_SHIFT) | (2 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_BG_0x06060000 =
        (1 << REG_GX_VRAMCNT_C_MST_SHIFT) | (3 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_ARM7_0x06000000 =
        (2 << REG_GX_VRAMCNT_C_MST_SHIFT) | (0 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_ARM7_0x06020000 =
        (2 << REG_GX_VRAMCNT_C_MST_SHIFT) | (1 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_TEX_0 =
        (3 << REG_GX_VRAMCNT_C_MST_SHIFT) | (0 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_TEX_1 =
        (3 << REG_GX_VRAMCNT_C_MST_SHIFT) | (1 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_TEX_2 =
        (3 << REG_GX_VRAMCNT_C_MST_SHIFT) | (2 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_TEX_3 =
        (3 << REG_GX_VRAMCNT_C_MST_SHIFT) | (3 << REG_GX_VRAMCNT_C_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_C_E_SHIFT),
    GX_VRAMCNT_C_SUBBG_0x06200000 =
        (4 << REG_GX_VRAMCNT_C_MST_SHIFT) | (1 << REG_GX_VRAMCNT_C_E_SHIFT)
}
GX_VRAMCNT_C;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-D (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_D_DISABLE = 0,
    GX_VRAMCNT_D_LCDC_0x06860000 =
        (0 << REG_GX_VRAMCNT_D_MST_SHIFT) | (0 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_D_MST_SHIFT) | (0 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_BG_0x06020000 =
        (1 << REG_GX_VRAMCNT_D_MST_SHIFT) | (1 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_BG_0x06040000 =
        (1 << REG_GX_VRAMCNT_D_MST_SHIFT) | (2 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_BG_0x06060000 =
        (1 << REG_GX_VRAMCNT_D_MST_SHIFT) | (3 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_ARM7_0x06000000 =
        (2 << REG_GX_VRAMCNT_D_MST_SHIFT) | (0 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_ARM7_0x06020000 =
        (2 << REG_GX_VRAMCNT_D_MST_SHIFT) | (1 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_TEX_0 =
        (3 << REG_GX_VRAMCNT_D_MST_SHIFT) | (0 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_TEX_1 =
        (3 << REG_GX_VRAMCNT_D_MST_SHIFT) | (1 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_TEX_2 =
        (3 << REG_GX_VRAMCNT_D_MST_SHIFT) | (2 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_TEX_3 =
        (3 << REG_GX_VRAMCNT_D_MST_SHIFT) | (3 << REG_GX_VRAMCNT_D_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_D_E_SHIFT),
    GX_VRAMCNT_D_SUBOBJ_0x06600000 =
        (4 << REG_GX_VRAMCNT_D_MST_SHIFT) | (1 << REG_GX_VRAMCNT_D_E_SHIFT)
}
GX_VRAMCNT_D;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-E (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_E_DISABLE = 0,
    GX_VRAMCNT_E_LCDC_0x06880000 =
        (0 << REG_GX_VRAMCNT_E_MST_SHIFT) | (1 << REG_GX_VRAMCNT_E_E_SHIFT),
    GX_VRAMCNT_E_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_E_MST_SHIFT) | (1 << REG_GX_VRAMCNT_E_E_SHIFT),
    GX_VRAMCNT_E_OBJ_0x06400000 =
        (2 << REG_GX_VRAMCNT_E_MST_SHIFT) | (1 << REG_GX_VRAMCNT_E_E_SHIFT),
    GX_VRAMCNT_E_TEXPLTT_0123 = (3 << REG_GX_VRAMCNT_E_MST_SHIFT) | (1 << REG_GX_VRAMCNT_E_E_SHIFT),
    GX_VRAMCNT_E_BGEXTPLTT_0123 =
        (4 << REG_GX_VRAMCNT_E_MST_SHIFT) | (1 << REG_GX_VRAMCNT_E_E_SHIFT)
}
GX_VRAMCNT_E;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-F (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_F_DISABLE = 0,
    GX_VRAMCNT_F_LCDC_0x06890000 =
        (0 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BG_0x06004000 =
        (1 << REG_GX_VRAMCNT_F_MST_SHIFT) | (1 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BG_0x06010000 =
        (1 << REG_GX_VRAMCNT_F_MST_SHIFT) | (2 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BG_0x06014000 =
        (1 << REG_GX_VRAMCNT_F_MST_SHIFT) | (3 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_OBJ_0x06400000 =
        (2 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_OBJ_0x06404000 =
        (2 << REG_GX_VRAMCNT_F_MST_SHIFT) | (1 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_OBJ_0x06410000 =
        (2 << REG_GX_VRAMCNT_F_MST_SHIFT) | (2 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_OBJ_0x06414000 =
        (2 << REG_GX_VRAMCNT_F_MST_SHIFT) | (3 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_TEXPLTT_0 =
        (3 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_TEXPLTT_1 =
        (3 << REG_GX_VRAMCNT_F_MST_SHIFT) | (1 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_TEXPLTT_4 =
        (3 << REG_GX_VRAMCNT_F_MST_SHIFT) | (2 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_TEXPLTT_5 =
        (3 << REG_GX_VRAMCNT_F_MST_SHIFT) | (3 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BGEXTPLTT_01 =
        (4 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_BGEXTPLTT_23 =
        (4 << REG_GX_VRAMCNT_F_MST_SHIFT) | (1 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT),
    GX_VRAMCNT_F_OBJEXTPLTT =
        (5 << REG_GX_VRAMCNT_F_MST_SHIFT) | (0 << REG_GX_VRAMCNT_F_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_F_E_SHIFT)
}
GX_VRAMCNT_F;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-G (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_G_DISABLE = 0,
    GX_VRAMCNT_G_LCDC_0x06894000 =
        (0 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BG_0x06000000 =
        (1 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BG_0x06004000 =
        (1 << REG_GX_VRAMCNT_G_MST_SHIFT) | (1 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BG_0x06010000 =
        (1 << REG_GX_VRAMCNT_G_MST_SHIFT) | (2 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BG_0x06014000 =
        (1 << REG_GX_VRAMCNT_G_MST_SHIFT) | (3 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_OBJ_0x06400000 =
        (2 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_OBJ_0x06404000 =
        (2 << REG_GX_VRAMCNT_G_MST_SHIFT) | (1 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_OBJ_0x06410000 =
        (2 << REG_GX_VRAMCNT_G_MST_SHIFT) | (2 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_OBJ_0x06414000 =
        (2 << REG_GX_VRAMCNT_G_MST_SHIFT) | (3 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_TEXPLTT_0 =
        (3 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_TEXPLTT_1 =
        (3 << REG_GX_VRAMCNT_G_MST_SHIFT) | (1 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_TEXPLTT_4 =
        (3 << REG_GX_VRAMCNT_G_MST_SHIFT) | (2 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_TEXPLTT_5 =
        (3 << REG_GX_VRAMCNT_G_MST_SHIFT) | (3 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BGEXTPLTT_01 =
        (4 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_BGEXTPLTT_23 =
        (4 << REG_GX_VRAMCNT_G_MST_SHIFT) | (1 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT),
    GX_VRAMCNT_G_OBJEXTPLTT =
        (5 << REG_GX_VRAMCNT_G_MST_SHIFT) | (0 << REG_GX_VRAMCNT_G_OFS_SHIFT) | (1 <<
                                                                                 REG_GX_VRAMCNT_G_E_SHIFT)
}
GX_VRAMCNT_G;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-H (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_H_DISABLE = 0,
    GX_VRAMCNT_H_LCDC_0x06898000 =
        (0 << REG_GX_VRAMCNT_H_MST_SHIFT) | (1 << REG_GX_VRAMCNT_H_E_SHIFT),
    GX_VRAMCNT_H_SUBBG_0x06200000 =
        (1 << REG_GX_VRAMCNT_H_MST_SHIFT) | (1 << REG_GX_VRAMCNT_H_E_SHIFT),
    GX_VRAMCNT_H_SUBBGEXTPLTT_0123 =
        (2 << REG_GX_VRAMCNT_H_MST_SHIFT) | (1 << REG_GX_VRAMCNT_H_E_SHIFT)
}
GX_VRAMCNT_H;


//---------------------------------------------------------------------------
// Enum values for VRAMCNT-I (internal use only)
//---------------------------------------------------------------------------
typedef enum
{
    GX_VRAMCNT_I_DISABLE = 0,
    GX_VRAMCNT_I_LCDC_0x068A0000 =
        (0 << REG_GX_VRAMCNT_I_MST_SHIFT) | (1 << REG_GX_VRAMCNT_I_E_SHIFT),
    GX_VRAMCNT_I_SUBBG_0x06208000 =
        (1 << REG_GX_VRAMCNT_I_MST_SHIFT) | (1 << REG_GX_VRAMCNT_I_E_SHIFT),
    GX_VRAMCNT_I_SUBOBJ_0x06600000 =
        (2 << REG_GX_VRAMCNT_I_MST_SHIFT) | (1 << REG_GX_VRAMCNT_I_E_SHIFT),
    GX_VRAMCNT_I_SUBOBJEXTPLTT = (3 << REG_GX_VRAMCNT_I_MST_SHIFT) | (1 << REG_GX_VRAMCNT_I_E_SHIFT)
}
GX_VRAMCNT_I;

#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/code32.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetLCDC_ (internal use only)

  Description:  Assigns banks onto LCDC.
                DO NOT INLINE THIS.

  Arguments:    lcdc:       Banks assigned to LCDC.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GX_VRAMCNT_SetLCDC_(int lcdc)
{
    if (lcdc & GX_VRAM_LCDC_A)
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_LCDC_0x06800000;
    if (lcdc & GX_VRAM_LCDC_B)
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_LCDC_0x06820000;
    if (lcdc & GX_VRAM_LCDC_C)
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_LCDC_0x06840000;
    if (lcdc & GX_VRAM_LCDC_D)
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_LCDC_0x06860000;
    if (lcdc & GX_VRAM_LCDC_E)
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_LCDC_0x06880000;
    if (lcdc & GX_VRAM_LCDC_F)
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_LCDC_0x06890000;
    if (lcdc & GX_VRAM_LCDC_G)
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_LCDC_0x06894000;
    if (lcdc & GX_VRAM_LCDC_H)
        reg_GX_VRAMCNT_H = (u8)GX_VRAMCNT_H_LCDC_0x06898000;
    if (lcdc & GX_VRAM_LCDC_I)
        reg_GX_VRAMCNT_I = (u8)GX_VRAMCNT_I_LCDC_0x068A0000;
}

/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetBG_ (internal use only)

  Description:  Assigns banks onto BG.
                The banks are mapped consecutively from HW_BG_VRAM(0x06000000).
                This is called only by GX_SetBankForBG.

  Arguments:    bg:         Banks assigned to BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetBG_(GXVRamBG bg)
{
    switch (bg)
    {
    case GX_VRAM_BG_128_D:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06000000;
        break;

    case GX_VRAM_BG_256_CD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06020000;
        // don't break
    case GX_VRAM_BG_128_C:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06000000;
        break;

    case GX_VRAM_BG_384_BCD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_256_BC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06020000;
        // don't break
    case GX_VRAM_BG_128_B:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06000000;
        break;

    case GX_VRAM_BG_512_ABCD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06060000;
        // don't break
    case GX_VRAM_BG_384_ABC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_256_AB:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06020000;
        // don't break
    case GX_VRAM_BG_128_A:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06000000;
        // don't break
    case GX_VRAM_BG_NONE:
        break;

    case GX_VRAM_BG_384_ABD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06000000;
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06020000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        break;

    case GX_VRAM_BG_384_ACD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_256_AC:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06000000;
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06020000;
        break;

    case GX_VRAM_BG_256_AD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06000000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06020000;
        break;

    case GX_VRAM_BG_256_BD:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06000000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06020000;
        break;

    case GX_VRAM_BG_96_EFG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06014000;
        // don't break;
    case GX_VRAM_BG_80_EF:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_BG_0x06010000;
        // don't break;
    case GX_VRAM_BG_64_E:
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_BG_0x06000000;
        break;

    case GX_VRAM_BG_80_EG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06010000;
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_BG_0x06000000;
        break;

    case GX_VRAM_BG_32_FG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06004000;
        // don't break
    case GX_VRAM_BG_16_F:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_BG_0x06000000;
        break;

    case GX_VRAM_BG_16_G:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06000000;
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamBG, 0x%x specified.", bg);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetBGEx1_ (internal use only)

  Description:  Assigns banks onto BG.
                The banks are mapped consecutively from HW_BG_VRAM(0x06000000).
                This is called only by GX_SetBankForBG.

  Arguments:    bg:         Banks assigned to BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetBGEx1_(GXVRamBG bg)
{
    switch (bg)
    {
    case GX_VRAM_BG_96_EFG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06014000;
        // don't break;
    case GX_VRAM_BG_80_EF:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_BG_0x06010000;
        // don't break;
    case GX_VRAM_BG_64_E:
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_BG_0x06000000;
        break;

    case GX_VRAM_BG_80_EG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06010000;
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_BG_0x06000000;
        break;

    case GX_VRAM_BG_32_FG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06004000;
        // don't break
    case GX_VRAM_BG_16_F:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_BG_0x06000000;
        break;

    case GX_VRAM_BG_16_G:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BG_0x06000000;
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamBG, 0x%x specified.", bg);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetBGEx2_ (internal use only)

  Description:  Assigns banks onto BG.
                The banks are mapped consecutively from HW_BG_VRAM(0x06000000).
                This is called only by GX_SetBankForBG.

  Arguments:    bg:         Banks assigned to BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetBGEx2_(GXVRamBG bg)
{
    switch (bg)
    {
    case GX_VRAM_BG_128_D:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06020000;
        break;

    case GX_VRAM_BG_256_CD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_128_C:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06020000;
        break;

    case GX_VRAM_BG_384_BCD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06060000;
        // don't break
    case GX_VRAM_BG_256_BC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_128_B:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06020000;
        break;

    case GX_VRAM_BG_384_ABC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06060000;
        // don't break
    case GX_VRAM_BG_256_AB:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06040000;
        // don't break
    case GX_VRAM_BG_128_A:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06020000;
        // don't break
    case GX_VRAM_BG_NONE:
        break;

    case GX_VRAM_BG_384_ABD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06020000;
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06040000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06060000;
        break;

    case GX_VRAM_BG_384_ACD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06060000;
        // don't break
    case GX_VRAM_BG_256_AC:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06020000;
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_BG_0x06040000;
        break;

    case GX_VRAM_BG_256_AD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_BG_0x06020000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        break;

    case GX_VRAM_BG_256_BD:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_BG_0x06020000;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_BG_0x06040000;
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamBG, 0x%x specified.", bg);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetOBJ_ (internal use only)

  Description:  Assigns banks onto OBJ.
                The banks are mapped consecutively from HW_OBJ_VRAM(0x06400000).
                This is called only by GX_SetBankForOBJ.

  Arguments:    obj:        Banks assigned to OBJ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetOBJ_(GXVRamOBJ obj)
{
    switch (obj)
    {
    case GX_VRAM_OBJ_256_AB:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_OBJ_0x06420000;
        // don't break
    case GX_VRAM_OBJ_128_A:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_OBJ_0x06400000;
        // don't break
    case GX_VRAM_OBJ_NONE:
        break;

    case GX_VRAM_OBJ_128_B:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_OBJ_0x06400000;
        break;

    case GX_VRAM_OBJ_96_EFG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_OBJ_0x06414000;
        // don't break
    case GX_VRAM_OBJ_80_EF:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_OBJ_0x06410000;
        // don't break
    case GX_VRAM_OBJ_64_E:
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_OBJ_0x06400000;
        break;

    case GX_VRAM_OBJ_80_EG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_OBJ_0x06410000;
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_OBJ_0x06400000;
        break;

    case GX_VRAM_OBJ_32_FG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_OBJ_0x06404000;
        // don't break
    case GX_VRAM_OBJ_16_F:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_OBJ_0x06400000;
        break;

    case GX_VRAM_OBJ_16_G:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_OBJ_0x06400000;
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamOBJ, 0x%x specified.", obj);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetARM7_ (internal use only)

  Description:  Assigns banks to ARM7.
                The banks are mapped consecutively from ARM7's 0x06000000.
                This is called only by GX_SetBankForARM7.

  Arguments:    arm7:       Banks assigned to ARM7

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetARM7_(GXVRamARM7 arm7)
{
    switch (arm7)
    {
    case GX_VRAM_ARM7_256_CD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_ARM7_0x06020000;
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_ARM7_0x06000000;
        break;
    case GX_VRAM_ARM7_128_C:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_ARM7_0x06000000;
        break;
    case GX_VRAM_ARM7_128_D:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_ARM7_0x06000000;
        // don't break
    case GX_VRAM_ARM7_NONE:
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamARM7, 0x%x specified.", arm7);
        break;
    }
}

static inline void texOn_()
{
    reg_G3X_DISP3DCNT = (u16)((reg_G3X_DISP3DCNT &
                               ~(REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK)) |
                              REG_G3X_DISP3DCNT_TME_MASK);
}

static inline void texOff_()
{
    reg_G3X_DISP3DCNT &= (u16)~(REG_G3X_DISP3DCNT_TME_MASK |
                                REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK);
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetTEX_ (internal use only)

  Description:  Assigns banks to texture image slots.
                The banks are mapped consecutively from the slot #0.
                This is called only by GX_SetBankForTex.

  Arguments:    tex:       Banks assigned to texture image slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetTEX_(GXVRamTex tex)
{
    if (tex == GX_VRAM_TEX_NONE)
    {
        texOff_();
        return;
    }
    texOn_();

    switch (tex)
    {
    case GX_VRAM_TEX_01_AC:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_0;
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_1;
        break;

    case GX_VRAM_TEX_01_AD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_0;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_1;
        break;

    case GX_VRAM_TEX_01_BD:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_TEX_0;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_1;
        break;

    case GX_VRAM_TEX_012_ABD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_0;
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_TEX_1;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_2;
        break;

    case GX_VRAM_TEX_012_ACD:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_0;
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_1;
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_2;
        break;

    case GX_VRAM_TEX_0_D:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_0;
        break;

    case GX_VRAM_TEX_01_CD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_1;
        // don't break
    case GX_VRAM_TEX_0_C:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_0;
        break;

    case GX_VRAM_TEX_012_BCD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_2;
        // don't break
    case GX_VRAM_TEX_01_BC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_1;
        // don't break
    case GX_VRAM_TEX_0_B:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_TEX_0;
        break;

    case GX_VRAM_TEX_0123_ABCD:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_3;
        // don't break;
    case GX_VRAM_TEX_012_ABC:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_2;
        // don't break;
    case GX_VRAM_TEX_01_AB:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_TEX_1;
        // don't break;
    case GX_VRAM_TEX_0_A:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_0;
        break;
    default:
        SDK_INTERNAL_ERROR("unknown GXVRamTex, 0x%x specified.", tex);
        break;
    }
}

static inline void clearImageOn_()
{
    reg_G3X_DISP3DCNT |= REG_G3X_DISP3DCNT_PRI_MASK;
}

static inline void clearImageOff_()
{
    reg_G3X_DISP3DCNT &= ~REG_G3X_DISP3DCNT_PRI_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetCLRIMG_ (internal use only)

  Description:  Assigns banks onto ClearImage.
                The banks are mapped onto the texture
                image slot #2, #3(clear image slot).
                This is called only by GX_SetBankForClearImage.

  Arguments:    clrImg:      Banks assigned to CLRIMG

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetCLRIMG_(GXVRamClearImage clrImg)
{
    switch (clrImg)
    {
    case GX_VRAM_CLEARIMAGE_256_AB:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_2;
        // don't break
    case GX_VRAM_CLEARDEPTH_128_B:
        reg_GX_VRAMCNT_B = (u8)GX_VRAMCNT_B_TEX_3;
        clearImageOn_();
        break;

    case GX_VRAM_CLEARIMAGE_256_CD:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_2;
        // don't break
    case GX_VRAM_CLEARDEPTH_128_D:
        reg_GX_VRAMCNT_D = (u8)GX_VRAMCNT_D_TEX_3;
        clearImageOn_();
        break;

    case GX_VRAM_CLEARIMAGE_NONE:
        clearImageOff_();
        break;

    case GX_VRAM_CLEARDEPTH_128_A:
        reg_GX_VRAMCNT_A = (u8)GX_VRAMCNT_A_TEX_3;
        clearImageOn_();
        break;

    case GX_VRAM_CLEARDEPTH_128_C:
        reg_GX_VRAMCNT_C = (u8)GX_VRAMCNT_C_TEX_3;
        clearImageOn_();
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamClearImage, 0x%x specified.", clrImg);
        break;
    }
}



/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetTEXPLTT_ (internal use only)

  Description:  Assigns banks to the texture palette slots.
                The banks are mapped consecutively from the slot #0.
                This is called only by GX_SetBankForTexPltt.

  Arguments:    texPltt:       Banks assigned to the texture palette slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetTEXPLTT_(GXVRamTexPltt texPltt)
{
    switch (texPltt)
    {
    case GX_VRAM_TEXPLTT_01_FG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_TEXPLTT_1;
        // don't break
    case GX_VRAM_TEXPLTT_0_F:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_TEXPLTT_0;
        break;

    case GX_VRAM_TEXPLTT_0_G:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_TEXPLTT_0;
        break;

    case GX_VRAM_TEXPLTT_012345_EFG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_TEXPLTT_5;
        // don't break
    case GX_VRAM_TEXPLTT_01234_EF:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_TEXPLTT_4;
        // don't break
    case GX_VRAM_TEXPLTT_0123_E:
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_TEXPLTT_0123;
        break;

    case GX_VRAM_TEXPLTT_NONE:
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamTexPltt, 0x%x specified.", texPltt);
        break;
    }
}

static inline void bgExtPlttOn_()
{
    reg_GX_DISPCNT |= REG_GX_DISPCNT_BG_MASK;
}

static inline void bgExtPlttOff_()
{
    reg_GX_DISPCNT &= ~REG_GX_DISPCNT_BG_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetBGEXTPLTT_ (internal use only)

  Description:  Assigns banks to BG extended palettes.
                The banks are mapped consecutively from the slot #0 or #2.
                This is called only by GX_SetBankForBGExtPltt.

  Arguments:    bgExtPltt:     Banks assigned to the BG extended palettes

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetBGEXTPLTT_(GXVRamBGExtPltt bgExtPltt)
{
    switch (bgExtPltt)
    {
    case GX_VRAM_BGEXTPLTT_0123_E:
        bgExtPlttOn_();
        reg_GX_VRAMCNT_E = (u8)GX_VRAMCNT_E_BGEXTPLTT_0123;
        break;

    case GX_VRAM_BGEXTPLTT_23_G:
        bgExtPlttOn_();
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BGEXTPLTT_23;
        break;

    case GX_VRAM_BGEXTPLTT_0123_FG:
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_BGEXTPLTT_23;
        // don't break
    case GX_VRAM_BGEXTPLTT_01_F:
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_BGEXTPLTT_01;
        bgExtPlttOn_();
        break;

    case GX_VRAM_BGEXTPLTT_NONE:
        bgExtPlttOff_();
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamBGExtPltt, 0x%x specified.", bgExtPltt);
        break;
    }
}

static inline void objExtPlttOn_()
{
    reg_GX_DISPCNT |= REG_GX_DISPCNT_O_MASK;
}

static inline void objExtPlttOff_()
{
    reg_GX_DISPCNT &= ~REG_GX_DISPCNT_O_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetOBJEXTPLTT_ (internal use only)

  Description:  Assigns a bank to OBJ extended palettes.
                This is called only by GX_SetBankForOBJExtPltt.

  Arguments:    objExtPltt:     Banks assigned to the OBJ extended palettes

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetOBJEXTPLTT_(GXVRamOBJExtPltt objExtPltt)
{
    switch (objExtPltt)
    {
    case GX_VRAM_OBJEXTPLTT_0_F:
        objExtPlttOn_();
        reg_GX_VRAMCNT_F = (u8)GX_VRAMCNT_F_OBJEXTPLTT;
        break;

    case GX_VRAM_OBJEXTPLTT_0_G:
        objExtPlttOn_();
        reg_GX_VRAMCNT_G = (u8)GX_VRAMCNT_G_OBJEXTPLTT;
        break;

    case GX_VRAM_OBJEXTPLTT_NONE:
        objExtPlttOff_();
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamOBJExtPltt, 0x%x specified.", objExtPltt);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetSubBG_ (internal use only)

  Description:  Assigns a bank to sub 2D engine's BG.
                This is called only by GX_SetBankForSubBG.

  Arguments:    bg:     Banks assigned to sub 2D engine's BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetSubBG_(GXVRamSubBG bg)
{
    switch (bg)
    {
    case GX_VRAM_SUB_BG_128_C:
        reg_GX_VRAMCNT_C = GX_VRAMCNT_C_SUBBG_0x06200000;
        break;

    case GX_VRAM_SUB_BG_48_HI:
        reg_GX_VRAMCNT_I = GX_VRAMCNT_I_SUBBG_0x06208000;
        // don't break
    case GX_VRAM_SUB_BG_32_H:
        reg_GX_VRAMCNT_H = GX_VRAMCNT_H_SUBBG_0x06200000;
        break;

    case GX_VRAM_SUB_BG_NONE:
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamSubBG, 0x%x specified.", bg);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetSubOBJ_ (internal use only)

  Description:  Assigns a bank to sub 2D engine's OBJ.
                This is called only by GX_SetBankForSubOBJ.

  Arguments:    obj:     Banks assigned to sub 2D engine's OBJ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetSubOBJ_(GXVRamSubOBJ obj)
{
    switch (obj)
    {
    case GX_VRAM_SUB_OBJ_128_D:
        reg_GX_VRAMCNT_D = GX_VRAMCNT_D_SUBOBJ_0x06600000;
        break;

    case GX_VRAM_SUB_OBJ_16_I:
        reg_GX_VRAMCNT_I = GX_VRAMCNT_I_SUBOBJ_0x06600000;
        break;

    case GX_VRAM_SUB_OBJ_NONE:
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamSubOBJ, 0x%x specified.", obj);
        break;
    }
}

static inline void subBGExtPlttOn_()
{
    reg_GXS_DB_DISPCNT |= REG_GXS_DB_DISPCNT_BG_MASK;
}

static inline void subBGExtPlttOff_()
{
    reg_GXS_DB_DISPCNT &= ~REG_GXS_DB_DISPCNT_BG_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetSubBGExtPltt_ (internal use only)

  Description:  Assigns a bank to sub 2D engine's BGExtPltt.
                This is called only by GX_SetBankForSubBGExtPltt.

  Arguments:    bgExtPltt:     Banks assigned to sub 2D engine's BGExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetSubBGExtPltt_(GXVRamSubBGExtPltt bgExtPltt)
{
    switch (bgExtPltt)
    {
    case GX_VRAM_SUB_BGEXTPLTT_0123_H:
        subBGExtPlttOn_();
        reg_GX_VRAMCNT_H = GX_VRAMCNT_H_SUBBGEXTPLTT_0123;
        break;

    case GX_VRAM_SUB_BGEXTPLTT_NONE:
        subBGExtPlttOff_();
        break;

    default:
        SDK_INTERNAL_ERROR("unknown GXVRamSubBGExtPltt, 0x%x specified.", bgExtPltt);
        break;
    }
}

static inline void subOBJExtPlttOn_()
{
    reg_GXS_DB_DISPCNT |= REG_GXS_DB_DISPCNT_O_MASK;
}

static inline void subOBJExtPlttOff_()
{
    reg_GXS_DB_DISPCNT &= ~REG_GXS_DB_DISPCNT_O_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         GX_VRAMCNT_SetSubOBJExtPltt_ (internal use only)

  Description:  Assigns a bank to sub 2D engine's OBJExtPltt.
                This is called only by GX_SetBankForSubOBJExtPltt.

  Arguments:    objExtPltt:     Banks assigned to sub 2D engine's OBJExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void GX_VRAMCNT_SetSubOBJExtPltt_(GXVRamSubOBJExtPltt objExtPltt)
{
    switch (objExtPltt)
    {
    case GX_VRAM_SUB_OBJEXTPLTT_0_I:
        subOBJExtPlttOn_();
        reg_GX_VRAMCNT_I = GX_VRAMCNT_I_SUBOBJEXTPLTT;
        break;

    case GX_VRAM_SUB_OBJEXTPLTT_NONE:
        subOBJExtPlttOff_();
        break;
    }
}

extern vu16 GXi_VRamLockId;
/*---------------------------------------------------------------------------*
  Name:         GxCheckExclusive

  Description:  Sets an exclusive lock on VRAM when banks are swapped.
                If an exclusive lock has already been set by other library, we should wait for it to be released in order to be proper, but instead we panic here.
                

  Arguments:    vramMap -   Logical OR of bank IDs for the group of exclusively locked VRAMs.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GxCheckExclusive(u16 vramMap)
{
    if (!OSi_TryLockVram(vramMap, GXi_VRamLockId))
    {
        OS_TPanic("VRAM bank is locked by another library.\n");
    }
}

//----------------------------------------------------------------------------
// GX_SetBankForX:
//
// 1 Error checking of arg.
// 2 Sets the state for LCDC((LCDC | X) & ~arg).
// 3 Sets the state for X(arg).
// 4 Checks conflicts with other regions.
// 5 Write VRAMCNTs for LCDC.
// 6 Write VRAMCNTs for X.
//----------------------------------------------------------------------------


/*---------------------------------------------------------------------------*/
static inline void GxSetBankForBG(GXVRamBG bg)
{
    // Check parameter
    GX_VRAM_BG_ASSERT(bg);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~bg & (gGXState.vramCnt.lcdc | gGXState.vramCnt.bg));
    gGXState.vramCnt.bg = bg;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks.
    GX_VRAMCNT_SetBG_(bg);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForBG

  Description:  Assigns banks onto BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bg:         Banks to be assigned to BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForBG(GXVRamBG bg)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)bg);
#endif
    GxSetBankForBG(bg);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForBG

  Description:  Try to assign banks onto BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bg:         Banks to be assigned to BG

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForBG(GXVRamBG bg)
{
    if (!OSi_TryLockVram((u16)bg, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForBG(bg);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
/* 
 * Although we want to strive for even small speed increases through inlining, if this function is declared inline the CW compiler will internally make three separate inline functions, which increases the function call overhead even more.
 * 
 * 
 * For this reason, as of 2005/02/15 this function has been defined as a real function.
 */
static void GxSetBankForBGEx(GXVRamBG bg1, GXVRamBG bg2)
{
    // Check parameter validity
    GX_VRAM_BG_ASSERT_EX_1(bg1);
    GX_VRAM_BG_ASSERT_EX_2(bg2);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~(bg1 | bg2) & (gGXState.vramCnt.lcdc | gGXState.vramCnt.bg));
    gGXState.vramCnt.bg = (u16)(bg1 | bg2);
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetBGEx1_(bg1);
    GX_VRAMCNT_SetBGEx2_(bg2);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForBGEx

  Description:  Assigns banks onto BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bg1:         Banks to be assigned to BG ( only bank EFG )
                bg2:         Banks to be assigned to BG ( only bank ABCD )

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForBGEx(GXVRamBG bg1, GXVRamBG bg2)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)(bg1 | bg2));
#endif
    GxSetBankForBGEx(bg1, bg2);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForBGEx

  Description:  Try to assign banks onto BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bg1:         Banks to be assigned to BG ( only bank EFG )
                bg2:         Banks to be assigned to BG ( only bank ABCD )

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForBGEx(GXVRamBG bg1, GXVRamBG bg2)
{
    if (!OSi_TryLockVram((u16)(bg1 | bg2), GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForBGEx(bg1, bg2);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForOBJ(GXVRamOBJ obj)
{
    // Check parameter validity
    GX_VRAM_OBJ_ASSERT(obj);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~obj & (gGXState.vramCnt.lcdc | gGXState.vramCnt.obj));
    gGXState.vramCnt.obj = obj;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetOBJ_(obj);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForOBJ

  Description:  Assigns banks onto OBJ.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    obj:         Banks to be assigned to OBJ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForOBJ(GXVRamOBJ obj)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)obj);
#endif
    GxSetBankForOBJ(obj);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForOBJ

  Description:  Try to assign banks onto OBJ.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    obj:         Banks to be assigned to OBJ

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForOBJ(GXVRamOBJ obj)
{
    if (!OSi_TryLockVram((u16)obj, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForOBJ(obj);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForBGExtPltt(GXVRamBGExtPltt bgExtPltt)
{
    // Check parameter validity
    GX_VRAM_BGEXTPLTT_ASSERT(bgExtPltt);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc =
        (u16)(~bgExtPltt & (gGXState.vramCnt.lcdc | gGXState.vramCnt.bgExtPltt));
    gGXState.vramCnt.bgExtPltt = bgExtPltt;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetBGEXTPLTT_(bgExtPltt);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForBGExtPltt

  Description:  Assigns banks onto BGExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bgExtPltt:         Banks to be assigned to bgExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForBGExtPltt(GXVRamBGExtPltt bgExtPltt)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)bgExtPltt);
#endif
    GxSetBankForBGExtPltt(bgExtPltt);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForBGExtPltt

  Description:  Try to assign banks onto BGExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    bgExtPltt:         Banks to be assigned to bgExtPltt

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForBGExtPltt(GXVRamBGExtPltt bgExtPltt)
{
    if (!OSi_TryLockVram((u16)bgExtPltt, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForBGExtPltt(bgExtPltt);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForOBJExtPltt(GXVRamOBJExtPltt objExtPltt)
{
    // Check parameter validity
    GX_VRAM_OBJEXTPLTT_ASSERT(objExtPltt);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc =
        (u16)(~objExtPltt & (gGXState.vramCnt.lcdc | gGXState.vramCnt.objExtPltt));
    gGXState.vramCnt.objExtPltt = objExtPltt;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetOBJEXTPLTT_(objExtPltt);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForOBJExtPltt

  Description:  Assigns banks onto OBJExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    objExtPltt:         Banks to be assigned to objExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForOBJExtPltt(GXVRamOBJExtPltt objExtPltt)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)objExtPltt);
#endif
    GxSetBankForOBJExtPltt(objExtPltt);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForOBJExtPltt

  Description:  Try to assign banks onto OBJExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    objExtPltt:         Banks to be assigned to objExtPltt

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForOBJExtPltt(GXVRamOBJExtPltt objExtPltt)
{
    if (!OSi_TryLockVram((u16)objExtPltt, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForOBJExtPltt(objExtPltt);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForTex(GXVRamTex tex)
{
    // Check parameter validity
    GX_VRAM_TEX_ASSERT(tex);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~tex & (gGXState.vramCnt.lcdc | gGXState.vramCnt.tex));
    gGXState.vramCnt.tex = tex;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetTEX_(tex);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForTex

  Description:  Assigns banks onto texture image slots.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    tex:         Banks to be assigned to texture image slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForTex(GXVRamTex tex)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)tex);
#endif
    GxSetBankForTex(tex);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForTex

  Description:  Try to assign banks onto texture image slots.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    tex:         Banks to be assigned to texture image slots

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForTex(GXVRamTex tex)
{
    if (!OSi_TryLockVram((u16)tex, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForTex(tex);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForTexPltt(GXVRamTexPltt texPltt)
{
    // Check parameter validity
    GX_VRAM_TEXPLTT_ASSERT(texPltt);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~texPltt & (gGXState.vramCnt.lcdc | gGXState.vramCnt.texPltt));
    gGXState.vramCnt.texPltt = texPltt;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetTEXPLTT_(texPltt);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForTexPltt

  Description:  Assigns banks onto texture palette slots.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    texPltt:     Banks to be assigned to texture palette slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForTexPltt(GXVRamTexPltt texPltt)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)texPltt);
#endif
    GxSetBankForTexPltt(texPltt);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForTexPltt

  Description:  Try to assign banks onto texture palette slots.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    texPltt:     Banks to be assigned to texture palette slots

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForTexPltt(GXVRamTexPltt texPltt)
{
    if (!OSi_TryLockVram((u16)texPltt, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForTexPltt(texPltt);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForClearImage(GXVRamClearImage clrImg)
{
    // Check parameter validity
    GX_VRAM_CLRIMG_ASSERT(clrImg);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~clrImg & (gGXState.vramCnt.lcdc | gGXState.vramCnt.clrImg));
    gGXState.vramCnt.clrImg = clrImg;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetCLRIMG_(clrImg);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForClearImage

  Description:  Assigns banks onto clear image slots(texture image slot #2,#3).
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    clrImg:       Banks to be assigned to clear image slots

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForClearImage(GXVRamClearImage clrImg)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)clrImg);
#endif
    GxSetBankForClearImage(clrImg);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForClearImage

  Description:  Try to assign banks onto clear image slots(texture image slot #2,#3).
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    clrImg:       Banks to be assigned to clear image slots

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForClearImage(GXVRamClearImage clrImg)
{
    if (!OSi_TryLockVram((u16)clrImg, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForClearImage(clrImg);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForARM7(GXVRamARM7 arm7)
{
    // Check parameter validity
    GX_VRAM_ARM7_ASSERT(arm7);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~arm7 & (gGXState.vramCnt.lcdc | gGXState.vramCnt.arm7));
    gGXState.vramCnt.arm7 = arm7;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetARM7_(arm7);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForARM7

  Description:  Assigns banks onto ARM7 memory space.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    arm7:     Banks to be assigned to ARM7 memory space

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForARM7(GXVRamARM7 arm7)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)arm7);
#endif
    GxSetBankForARM7(arm7);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForARM7

  Description:  Try to assign banks onto ARM7 memory space.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    arm7:     Banks to be assigned to ARM7 memory space

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForARM7(GXVRamARM7 arm7)
{
    if (!OSi_TryLockVram((u16)arm7, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForARM7(arm7);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForLCDC(int lcdc)
{
    // Check parameter validity
    GX_VRAM_LCDC_ASSERT(lcdc);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc |= lcdc;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetLCDC_(lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForLCDC

  Description:  Assigns banks onto LCDC memory space.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    lcdc:     Banks to be assigned to LCDC memory space

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForLCDC(int lcdc)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)lcdc);
#endif
    GxSetBankForLCDC(lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForLCDC

  Description:  Try to assign banks onto LCDC memory space.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    lcdc:     Banks to be assigned to LCDC memory space

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForLCDC(int lcdc)
{
    if (!OSi_TryLockVram((u16)lcdc, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForLCDC(lcdc);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForSubBG(GXVRamSubBG sub_bg)
{
    // Check parameter validity
    GX_VRAM_SUB_BG_ASSERT(sub_bg);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~sub_bg & (gGXState.vramCnt.lcdc | gGXState.vramCnt.sub_bg));
    gGXState.vramCnt.sub_bg = sub_bg;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetSubBG_(sub_bg);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForSubBG

  Description:  Assigns banks onto sub engine's BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_bg:     Banks to be assigned to sub engine's BG

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForSubBG(GXVRamSubBG sub_bg)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)sub_bg);
#endif
    GxSetBankForSubBG(sub_bg);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForSubBG

  Description:  Try to assign banks onto sub engine's BG.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_bg:     Banks to be assigned to sub engine's BG

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForSubBG(GXVRamSubBG sub_bg)
{
    if (!OSi_TryLockVram((u16)sub_bg, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForSubBG(sub_bg);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForSubOBJ(GXVRamSubOBJ sub_obj)
{
    // Check parameter validity
    GX_VRAM_SUB_OBJ_ASSERT(sub_obj);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc = (u16)(~sub_obj & (gGXState.vramCnt.lcdc | gGXState.vramCnt.sub_obj));
    gGXState.vramCnt.sub_obj = sub_obj;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetSubOBJ_(sub_obj);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForSubOBJ

  Description:  Assigns banks onto sub engine's OBJ.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_obj:     Banks to be assigned to sub engine's OBJ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForSubOBJ(GXVRamSubOBJ sub_obj)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)sub_obj);
#endif
    GxSetBankForSubOBJ(sub_obj);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForSubOBJ

  Description:  Try to assign banks onto sub engine's OBJ.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_obj:     Banks to be assigned to sub engine's OBJ

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForSubOBJ(GXVRamSubOBJ sub_obj)
{
    if (!OSi_TryLockVram((u16)sub_obj, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForSubOBJ(sub_obj);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForSubBGExtPltt(GXVRamSubBGExtPltt sub_bgExtPltt)
{
    // Check parameter validity
    GX_VRAM_SUB_BGEXTPLTT_ASSERT(sub_bgExtPltt);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc =
        (u16)(~sub_bgExtPltt & (gGXState.vramCnt.lcdc | gGXState.vramCnt.sub_bgExtPltt));
    gGXState.vramCnt.sub_bgExtPltt = sub_bgExtPltt;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetSubBGExtPltt_(sub_bgExtPltt);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForSubBGExtPltt

  Description:  Assigns banks onto sub engine's BGExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_bgExtPltt:     Banks to be assigned to sub engine's BGExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForSubBGExtPltt(GXVRamSubBGExtPltt sub_bgExtPltt)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)sub_bgExtPltt);
#endif
    GxSetBankForSubBGExtPltt(sub_bgExtPltt);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForSubBGExtPltt

  Description:  Try to assign banks onto sub engine's BGExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_bgExtPltt:     Banks to be assigned to sub engine's BGExtPltt

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForSubBGExtPltt(GXVRamSubBGExtPltt sub_bgExtPltt)
{
    if (!OSi_TryLockVram((u16)sub_bgExtPltt, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForSubBGExtPltt(sub_bgExtPltt);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static inline void GxSetBankForSubOBJExtPltt(GXVRamSubOBJExtPltt sub_objExtPltt)
{
    // Check parameter validity
    GX_VRAM_SUB_OBJEXTPLTT_ASSERT(sub_objExtPltt);
    // Change BG banks and LCDC ones.
    gGXState.vramCnt.lcdc =
        (u16)(~sub_objExtPltt & (gGXState.vramCnt.lcdc | gGXState.vramCnt.sub_objExtPltt));
    gGXState.vramCnt.sub_objExtPltt = sub_objExtPltt;
    // Check conflicts( DEBUG build ).
    GX_StateCheck_VRAMCnt();
    // Switch VRAM banks
    GX_VRAMCNT_SetSubOBJExtPltt_(sub_objExtPltt);
    GX_VRAMCNT_SetLCDC_(gGXState.vramCnt.lcdc);
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetBankForSubOBJExtPltt

  Description:  Assigns banks onto sub engine's OBJExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_objExtPltt:     Banks to be assigned to sub engine's OBJExtPltt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GX_SetBankForSubOBJExtPltt(GXVRamSubOBJExtPltt sub_objExtPltt)
{
#ifndef SDK_FINALROM
    GxCheckExclusive((u16)sub_objExtPltt);
#endif
    GxSetBankForSubOBJExtPltt(sub_objExtPltt);
}

/*---------------------------------------------------------------------------*
  Name:         GX_TrySetBankForSubOBJExtPltt

  Description:  Try to assign banks onto sub engine's OBJExtPltt.
                The specified banks must be disabled or mapped to LCDC.

  Arguments:    sub_objExtPltt:     Banks to be assigned to sub engine's OBJExtPltt

  Returns:      BOOL.       If VRAM banks could not be locked, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL GX_TrySetBankForSubOBJExtPltt(GXVRamSubOBJExtPltt sub_objExtPltt)
{
    if (!OSi_TryLockVram((u16)sub_objExtPltt, GXi_VRamLockId))
    {
        return FALSE;
    }
    GxSetBankForSubOBJExtPltt(sub_objExtPltt);
    return TRUE;
}

#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/codereset.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         resetBankForX_

  Description:  Assigns the argument banks onto LCDC memory space.

  Arguments:    g3bit:    A pointer to bank information

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int resetBankForX_(u16 *g3bit)
{
    int     rval = *g3bit;
    *g3bit = 0;                        // GX_VRAMCNT_xxxxxx_NONE

    // maps to LCDC
    gGXState.vramCnt.lcdc |= rval;
    GX_VRAMCNT_SetLCDC_(rval);

    return rval;
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForBG

  Description:  Switches BG banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to BG.
 *---------------------------------------------------------------------------*/
GXVRamBG GX_ResetBankForBG()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.bg, GXi_VRamLockId))
    {
        return GX_VRAM_BG_NONE;
    }
#endif
    return (GXVRamBG)resetBankForX_(&gGXState.vramCnt.bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForOBJ

  Description:  Switches OBJ banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to OBJ.
 *---------------------------------------------------------------------------*/
GXVRamOBJ GX_ResetBankForOBJ()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.obj, GXi_VRamLockId))
    {
        return GX_VRAM_OBJ_NONE;
    }
#endif
    return (GXVRamOBJ)resetBankForX_(&gGXState.vramCnt.obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForBGExtPltt

  Description:  Switches BGExtPltt banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamBGExtPltt GX_ResetBankForBGExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.bgExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_BGEXTPLTT_NONE;
    }
#endif
    bgExtPlttOff_();
    return (GXVRamBGExtPltt)resetBankForX_(&gGXState.vramCnt.bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForOBJExtPltt

  Description:  Switches OBJExtPltt banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamOBJExtPltt GX_ResetBankForOBJExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.objExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_OBJEXTPLTT_NONE;
    }
#endif
    objExtPlttOff_();
    return (GXVRamOBJExtPltt)resetBankForX_(&gGXState.vramCnt.objExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForTex

  Description:  Switches TexImage banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to TexImage.
 *---------------------------------------------------------------------------*/
GXVRamTex GX_ResetBankForTex()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.tex, GXi_VRamLockId))
    {
        return GX_VRAM_TEX_NONE;
    }
#endif
    return (GXVRamTex)resetBankForX_(&gGXState.vramCnt.tex);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForTexPltt

  Description:  Switches TexPltt banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to TexPltt.
 *---------------------------------------------------------------------------*/
GXVRamTexPltt GX_ResetBankForTexPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.texPltt, GXi_VRamLockId))
    {
        return GX_VRAM_TEXPLTT_NONE;
    }
#endif
    return (GXVRamTexPltt)resetBankForX_(&gGXState.vramCnt.texPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForClearImage

  Description:  Switches ClearImage banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to ClearImage.
 *---------------------------------------------------------------------------*/
GXVRamClearImage GX_ResetBankForClearImage()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.clrImg, GXi_VRamLockId))
    {
        return GX_VRAM_CLEARIMAGE_NONE;
    }
#endif
    return (GXVRamClearImage)resetBankForX_(&gGXState.vramCnt.clrImg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForARM7

  Description:  Switches ARM7 banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to ARM7.
 *---------------------------------------------------------------------------*/
GXVRamARM7 GX_ResetBankForARM7()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.arm7, GXi_VRamLockId))
    {
        return GX_VRAM_ARM7_NONE;
    }
#endif
    return (GXVRamARM7)resetBankForX_(&gGXState.vramCnt.arm7);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForSubBG

  Description:  Switches sub engine's BG banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's BG.
 *---------------------------------------------------------------------------*/
GXVRamSubBG GX_ResetBankForSubBG()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_bg, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_BG_NONE;
    }
#endif
    return (GXVRamSubBG)resetBankForX_(&gGXState.vramCnt.sub_bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForSubOBJ

  Description:  Switches sub engine's OBJ banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's OBJ.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJ GX_ResetBankForSubOBJ()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_obj, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_OBJ_NONE;
    }
#endif
    return (GXVRamSubOBJ)resetBankForX_(&gGXState.vramCnt.sub_obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForSubBGExtPltt

  Description:  Switches sub engine's BGExtPltt banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubBGExtPltt GX_ResetBankForSubBGExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_bgExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_BGEXTPLTT_NONE;
    }
#endif
    subBGExtPlttOff_();
    return (GXVRamSubBGExtPltt)resetBankForX_(&gGXState.vramCnt.sub_bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_ResetBankForSubOBJExtPltt

  Description:  Switches sub engine's OBJExtPltt banks onto LCDC memory space.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJExtPltt GX_ResetBankForSubOBJExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_objExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_OBJEXTPLTT_NONE;
    }
#endif
    subOBJExtPlttOff_();
    return (GXVRamSubOBJExtPltt)resetBankForX_(&gGXState.vramCnt.sub_objExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         disableBankForX_

  Description:  Disables the argument banks.

  Arguments:    g3bit:    A pointer to bank information

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/code32.h>
#endif
static int disableBankForX_(u16 *g3bit)
{
    int     rval = *g3bit;
    *g3bit = 0;                        // GX_VRAMCNT_xxxxxx_NONE

    if (rval & GX_VRAM_LCDC_A)
        reg_GX_VRAMCNT_A = 0;
    if (rval & GX_VRAM_LCDC_B)
        reg_GX_VRAMCNT_B = 0;
    if (rval & GX_VRAM_LCDC_C)
        reg_GX_VRAMCNT_C = 0;
    if (rval & GX_VRAM_LCDC_D)
        reg_GX_VRAMCNT_D = 0;
    if (rval & GX_VRAM_LCDC_E)
        reg_GX_VRAMCNT_E = 0;
    if (rval & GX_VRAM_LCDC_F)
        reg_GX_VRAMCNT_F = 0;
    if (rval & GX_VRAM_LCDC_G)
        reg_GX_VRAMCNT_G = 0;
    if (rval & GX_VRAM_LCDC_H)
        reg_GX_VRAMCNT_H = 0;
    if (rval & GX_VRAM_LCDC_I)
        reg_GX_VRAMCNT_I = 0;

    OSi_UnlockVram((u16)rval, GXi_VRamLockId);

    return rval;
}

#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/codereset.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForBG

  Description:  Disables the banks assigned to BG.

  Arguments:    None.

  Returns:      The banks that were assigned to BG.
 *---------------------------------------------------------------------------*/
GXVRamBG GX_DisableBankForBG()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.bg, GXi_VRamLockId))
    {
        return GX_VRAM_BG_NONE;
    }
#endif
    return (GXVRamBG)disableBankForX_(&gGXState.vramCnt.bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForOBJ

  Description:  Disables the banks assigned to OBJ.

  Arguments:    None.

  Returns:      The banks that were assigned to OBJ.
 *---------------------------------------------------------------------------*/
GXVRamOBJ GX_DisableBankForOBJ()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.obj, GXi_VRamLockId))
    {
        return GX_VRAM_OBJ_NONE;
    }
#endif
    return (GXVRamOBJ)disableBankForX_(&gGXState.vramCnt.obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForBGExtPltt

  Description:  Disables the banks assigned to BGExtPltt.

  Arguments:    None.

  Returns:      The banks that were assigned to BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamBGExtPltt GX_DisableBankForBGExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.bgExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_BGEXTPLTT_NONE;
    }
#endif
    bgExtPlttOff_();
    return (GXVRamBGExtPltt)disableBankForX_(&gGXState.vramCnt.bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForOBJExtPltt

  Description:  Disables the banks assigned to OBJExtPltt.

  Arguments:    None.

  Returns:      The banks that were assigned to OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamOBJExtPltt GX_DisableBankForOBJExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.objExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_OBJEXTPLTT_NONE;
    }
#endif
    objExtPlttOff_();
    return (GXVRamOBJExtPltt)disableBankForX_(&gGXState.vramCnt.objExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForTex

  Description:  Disables the banks assigned to Tex.

  Arguments:    None.

  Returns:      The banks that were assigned to Tex.
 *---------------------------------------------------------------------------*/
GXVRamTex GX_DisableBankForTex()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.tex, GXi_VRamLockId))
    {
        return GX_VRAM_TEX_NONE;
    }
#endif
    return (GXVRamTex)disableBankForX_(&gGXState.vramCnt.tex);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForTexPltt

  Description:  Disables the banks assigned to TexPltt.

  Arguments:    None.

  Returns:      The banks that were assigned to TexPltt.
 *---------------------------------------------------------------------------*/
GXVRamTexPltt GX_DisableBankForTexPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.texPltt, GXi_VRamLockId))
    {
        return GX_VRAM_TEXPLTT_NONE;
    }
#endif
    return (GXVRamTexPltt)disableBankForX_(&gGXState.vramCnt.texPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForClearImage

  Description:  Disables the banks assigned to ClearImage.

  Arguments:    None.

  Returns:      The banks that were assigned to ClearImage.
 *---------------------------------------------------------------------------*/
GXVRamClearImage GX_DisableBankForClearImage()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.clrImg, GXi_VRamLockId))
    {
        return GX_VRAM_CLEARIMAGE_NONE;
    }
#endif
    return (GXVRamClearImage)disableBankForX_(&gGXState.vramCnt.clrImg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForARM7

  Description:  Disables the banks assigned to ARM7.

  Arguments:    None.

  Returns:      The banks that were assigned to ARM7.
 *---------------------------------------------------------------------------*/
GXVRamARM7 GX_DisableBankForARM7()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.arm7, GXi_VRamLockId))
    {
        return GX_VRAM_ARM7_NONE;
    }
#endif
    return (GXVRamARM7)disableBankForX_(&gGXState.vramCnt.arm7);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForLCDC

  Description:  Disables the banks assigned to LCDC.

  Arguments:    None.

  Returns:      The banks that were assigned to LCDC.
 *---------------------------------------------------------------------------*/
GXVRamLCDC GX_DisableBankForLCDC()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.lcdc, GXi_VRamLockId))
    {
        return GX_VRAM_LCDC_NONE;
    }
#endif
    return (GXVRamLCDC)disableBankForX_(&gGXState.vramCnt.lcdc);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForSubBG

  Description:  Disables the banks assigned to sub engine's BG.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's BG.
 *---------------------------------------------------------------------------*/
GXVRamSubBG GX_DisableBankForSubBG()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_bg, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_BG_NONE;
    }
#endif
    return (GXVRamSubBG)disableBankForX_(&gGXState.vramCnt.sub_bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForSubOBJ

  Description:  Disables the banks assigned to sub engine's OBJ.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's OBJ.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJ GX_DisableBankForSubOBJ()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_obj, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_OBJ_NONE;
    }
#endif
    return (GXVRamSubOBJ)disableBankForX_(&gGXState.vramCnt.sub_obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForSubBGExtPltt

  Description:  Disables the banks assigned to sub engine's BGExtPltt.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubBGExtPltt GX_DisableBankForSubBGExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_bgExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_BGEXTPLTT_NONE;
    }
#endif
    subBGExtPlttOff_();
    return (GXVRamSubBGExtPltt)disableBankForX_(&gGXState.vramCnt.sub_bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_DisableBankForSubOBJExtPltt

  Description:  Disables the banks assigned to sub engine's OBJExtPltt.

  Arguments:    None.

  Returns:      The banks that were assigned to sub engine's OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJExtPltt GX_DisableBankForSubOBJExtPltt()
{
#ifndef SDK_FINALROM
    if (!OSi_TryLockVram((u16)gGXState.vramCnt.sub_objExtPltt, GXi_VRamLockId))
    {
        return GX_VRAM_SUB_OBJEXTPLTT_NONE;
    }
#endif
    subOBJExtPlttOff_();
    return (GXVRamSubOBJExtPltt)disableBankForX_(&gGXState.vramCnt.sub_objExtPltt);
}





/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForBG

  Description:  Returns the bank assigned to BG.

  Arguments:    None.

  Returns:      The banks that are assigned to BG.
 *---------------------------------------------------------------------------*/
GXVRamBG GX_GetBankForBG()
{
    return (GXVRamBG)gGXState.vramCnt.bg;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForOBJ

  Description:  Returns the bank assigned to OBJ.

  Arguments:    None.

  Returns:      The banks that are assigned to OBJ.
 *---------------------------------------------------------------------------*/
GXVRamOBJ GX_GetBankForOBJ()
{
    return (GXVRamOBJ)gGXState.vramCnt.obj;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForBGExtPltt

  Description:  Returns the bank assigned to BGExtPltt.

  Arguments:    None.

  Returns:      The banks that are assigned to BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamBGExtPltt GX_GetBankForBGExtPltt()
{
    return (GXVRamBGExtPltt)gGXState.vramCnt.bgExtPltt;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForOBJExtPltt

  Description:  Returns the bank assigned to OBJExtPltt.

  Arguments:    None.

  Returns:      The banks that are assigned to OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamOBJExtPltt GX_GetBankForOBJExtPltt()
{
    return (GXVRamOBJExtPltt)gGXState.vramCnt.objExtPltt;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForTex

  Description:  Returns the bank assigned to Tex.

  Arguments:    None.

  Returns:      The banks that are assigned to Tex.
 *---------------------------------------------------------------------------*/
GXVRamTex GX_GetBankForTex()
{
    return (GXVRamTex)gGXState.vramCnt.tex;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForTexPltt

  Description:  Returns the bank assigned to TexPltt.

  Arguments:    None.

  Returns:      The banks that are assigned to TexPltt.
 *---------------------------------------------------------------------------*/
GXVRamTexPltt GX_GetBankForTexPltt()
{
    return (GXVRamTexPltt)gGXState.vramCnt.texPltt;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForClearImage

  Description:  Returns the bank assigned to ClearImage.

  Arguments:    None.

  Returns:      The banks that are assigned to ClearImage.
 *---------------------------------------------------------------------------*/
GXVRamClearImage GX_GetBankForClearImage()
{
    return (GXVRamClearImage)gGXState.vramCnt.clrImg;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForARM7

  Description:  Returns the bank assigned to ARM7.

  Arguments:    None.

  Returns:      The banks that are assigned to ARM7.
 *---------------------------------------------------------------------------*/
GXVRamARM7 GX_GetBankForARM7()
{
    return (GXVRamARM7)gGXState.vramCnt.arm7;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForLCDC

  Description:  Returns the bank assigned to LCDC.

  Arguments:    None.

  Returns:      The banks that are assigned to LCDC.
 *---------------------------------------------------------------------------*/
GXVRamLCDC GX_GetBankForLCDC()
{
    return (GXVRamLCDC)gGXState.vramCnt.lcdc;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForSubBG

  Description:  Returns the bank assigned to sub engine's BG.

  Arguments:    None.

  Returns:      The banks that are assigned to sub engine's BG.
 *---------------------------------------------------------------------------*/
GXVRamSubBG GX_GetBankForSubBG()
{
    return (GXVRamSubBG)gGXState.vramCnt.sub_bg;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForSubOBJ

  Description:  Returns the bank assigned to sub engine's OBJ.

  Arguments:    None.

  Returns:      The banks that are assigned to sub engine's OBJ.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJ GX_GetBankForSubOBJ()
{
    return (GXVRamSubOBJ)gGXState.vramCnt.sub_obj;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForSubBGExtPltt

  Description:  Returns the bank assigned to sub engine's BGExtPltt.

  Arguments:    None.

  Returns:      The banks that are assigned to sub engine's BGExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubBGExtPltt GX_GetBankForSubBGExtPltt()
{
    return (GXVRamSubBGExtPltt)gGXState.vramCnt.sub_bgExtPltt;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetBankForSubOBJExtPltt

  Description:  Returns the bank assigned to sub engine's OBJExtPltt.

  Arguments:    None.

  Returns:      The banks that are assigned to sub engine's OBJExtPltt.
 *---------------------------------------------------------------------------*/
GXVRamSubOBJExtPltt GX_GetBankForSubOBJExtPltt()
{
    return (GXVRamSubOBJExtPltt)gGXState.vramCnt.sub_objExtPltt;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfX_ (internal use only)

  Description:  Returns the size of VRAM allocated to BG/OBJ/Tex etc.

  Arguments:    bit:          The value of GXVRamXXXXX

  Returns:      The size of resource in bytes.
 *---------------------------------------------------------------------------*/
static u32 GX_GetSizeOfX_(u32 bit /* gGXState.vramCnt.????? */ )
{
    u32     size = 0;
    if (bit & GX_VRAM_LCDC_A)
        size += HW_VRAM_A_SIZE;
    if (bit & GX_VRAM_LCDC_B)
        size += HW_VRAM_B_SIZE;
    if (bit & GX_VRAM_LCDC_C)
        size += HW_VRAM_C_SIZE;
    if (bit & GX_VRAM_LCDC_D)
        size += HW_VRAM_D_SIZE;
    if (bit & GX_VRAM_LCDC_E)
        size += HW_VRAM_E_SIZE;
    if (bit & GX_VRAM_LCDC_F)
        size += HW_VRAM_F_SIZE;
    if (bit & GX_VRAM_LCDC_G)
        size += HW_VRAM_G_SIZE;
    if (bit & GX_VRAM_LCDC_H)
        size += HW_VRAM_H_SIZE;
    if (bit & GX_VRAM_LCDC_I)
        size += HW_VRAM_I_SIZE;
    return size;
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfBG

  Description:  Returns the size of the memory allocated to the main engine's
                BG.

  Arguments:    None.

  Returns:      The size of the main engine's BG.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfBG(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfOBJ

  Description:  Returns the size of the memory allocated to the main engine's OBJ.

  Arguments:    None.

  Returns:      The size of the main engine's OBJ.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfOBJ(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfBGExtPltt

  Description:  Returns the size of the memory allocated to the main engine's
                BGExtPltt.

  Arguments:    None.

  Returns:      The size of the main engine's BGExtPltt.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfBGExtPltt(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfOBJExtPltt

  Description:  Returns the size of the memory allocated to the main engine's
                OBJExtPltt.

  Arguments:    None.

  Returns:      The size of the main engine's OBJExtPltt.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfOBJExtPltt(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.objExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfTex

  Description:  Returns the size of the memory allocated to texture image slots.

  Arguments:    None.

  Returns:      The size of the texture image memory.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfTex(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.tex);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfTexPltt

  Description:  Returns the size of the memory allocated to texture palette
                slots.

  Arguments:    None.

  Returns:      The size of the texture palette memory.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfTexPltt(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.texPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfClearImage

  Description:  Returns the size of the memory allocated to clear image slots.

  Arguments:    None.

  Returns:      The size of the clear image memory.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfClearImage(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.clrImg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfSubBG

  Description:  Returns the size of the memory allocated to the sub engine's BG.

  Arguments:    None.

  Returns:      The size of the sub engine's BG.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfSubBG(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.sub_bg);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfSubOBJ

  Description:  Returns the size of the memory allocated to the sub engine's OBJ.

  Arguments:    None.

  Returns:      The size of the sub engine's OBJ.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfSubOBJ(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.sub_obj);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfSubBGExtPltt

  Description:  Returns the size of the memory allocated to the sub engine's
                BGExtPltt.

  Arguments:    None.

  Returns:      The size of the sub engine's BGExtPltt.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfSubBGExtPltt(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.sub_bgExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfSubOBJExtPltt

  Description:  Returns the size of the memory allocated to the sub engine's
                OBJExtPltt.

  Arguments:    None.

  Returns:      The size of the sub engine's OBJExtPltt.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfSubOBJExtPltt(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.sub_objExtPltt);
}


/*---------------------------------------------------------------------------*
  Name:         GX_GetSizeOfARM7

  Description:  Returns the size of VRAM allocated to ARM7 memory space.

  Arguments:    None.

  Returns:      The size of VRAM allocated to ARM7 memory space.
 *---------------------------------------------------------------------------*/
u32 GX_GetSizeOfARM7(void)
{
    return GX_GetSizeOfX_(gGXState.vramCnt.arm7);
}
