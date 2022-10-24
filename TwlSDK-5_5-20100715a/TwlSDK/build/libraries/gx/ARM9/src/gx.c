/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     gx.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-03-23#$
  $Rev: 11314 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <nitro/gx/gx.h>
#include <nitro/mi/dma.h>

#ifdef SDK_TWL
#include <twl/mi/common/dma.h>
#endif

#include <nitro/os/common/spinLock.h>
#include "../include/gxstate.h"

#ifdef  SDK_ARM9
#include <nitro/os/ARM9/vramExclusive.h>
#include <nitro/spi/ARM9/pm.h>
#endif


u32     GXi_DmaId = GX_DEFAULT_DMAID;

#ifdef  SDK_ARM9
vu16    GXi_VRamLockId = 0;
#endif

/*---------------------------------------------------------------------------*
  Name:         GX_Init

  Description:  Initializes the registers for graphics(except 3D).

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
void GX_Init()
{
    const u16 bg_mtx_elem_one = 1 << 8;

    reg_GX_POWCNT |= (1 << REG_GX_POWCNT_DSEL_SHIFT);
    GX_SetPower(GX_POWER_ALL);
    GXi_PowerLCD(TRUE);
    GX_InitGXState();

#ifdef  SDK_ARM9
    {
        s32     lockResult;

        while (GXi_VRamLockId == 0)
        {
            lockResult = OS_GetLockID();
            if (lockResult == OS_LOCK_ID_ERROR)
            {
                OS_TPanic("Could not get lock ID for VRAM exclusive.\n");
            }
            GXi_VRamLockId = (u16)lockResult;
        }
    }
#endif

#if 1
    // smaller binary size

    //
    // Main engine
    //
    reg_GX_DISPSTAT = 0;
    reg_GX_DISPCNT = 0;
    if (GXi_DmaId != GX_DMA_NOT_USE)
    {
#ifdef SDK_TWL
        if (GXi_DmaId > 3)
        {
            MI_NDmaFill(GXi_DmaId-4, (void *)REG_BG0CNT_ADDR, 0,
                         REG_DISP_MMEM_FIFO_ADDR - REG_BG0CNT_ADDR);
            reg_GX_MASTER_BRIGHT = 0;
            //
            // Sub engine
            //
            MI_NDmaFill(GXi_DmaId-4, (void *)REG_DB_DISPCNT_ADDR, 0,
                         REG_DB_MASTER_BRIGHT_ADDR - REG_DB_DISPCNT_ADDR + 4);
        }
        else
#endif
        {
            MI_DmaFill32(GXi_DmaId, (void *)REG_BG0CNT_ADDR, 0,
                         REG_DISP_MMEM_FIFO_ADDR - REG_BG0CNT_ADDR);
            reg_GX_MASTER_BRIGHT = 0;
            //
            // Sub engine
            //
            MI_DmaFill32(GXi_DmaId, (void *)REG_DB_DISPCNT_ADDR, 0,
                         REG_DB_MASTER_BRIGHT_ADDR - REG_DB_DISPCNT_ADDR + 4);
        }
    }
    else
    {
        MI_CpuFill32((void *)REG_BG0CNT_ADDR, 0, REG_DISP_MMEM_FIFO_ADDR - REG_BG0CNT_ADDR);
        reg_GX_MASTER_BRIGHT = 0;
        //
        // Sub engine
        //
        MI_CpuFill32((void *)REG_DB_DISPCNT_ADDR, 0,
                     REG_DB_MASTER_BRIGHT_ADDR - REG_DB_DISPCNT_ADDR + 4);
    }


    //
    // Initialize BG matrices as identity ones.
    //
    reg_G2_BG2PA = bg_mtx_elem_one;
    reg_G2_BG2PD = bg_mtx_elem_one;
    reg_G2_BG3PA = bg_mtx_elem_one;
    reg_G2_BG3PD = bg_mtx_elem_one;
    reg_G2S_DB_BG2PA = bg_mtx_elem_one;
    reg_G2S_DB_BG2PD = bg_mtx_elem_one;
    reg_G2S_DB_BG3PA = bg_mtx_elem_one;
    reg_G2S_DB_BG3PD = bg_mtx_elem_one;

#else
    //
    // Main engine
    //
    reg_GX_DISPSTAT = 0;
    reg_GX_DISPCNT = 0;

    // init capture register
    reg_GX_DISPCAPCNT = 0;

    // init BGxCNT
    reg_G2_BG0CNT = 0;
    reg_G2_BG1CNT = 0;
    reg_G2_BG2CNT = 0;
    reg_G2_BG3CNT = 0;

    // reset BG offsets
    reg_G2_BG0OFS = 0;
    reg_G2_BG1OFS = 0;
    reg_G2_BG2OFS = 0;
    reg_G2_BG3OFS = 0;

    // init affine BG
    reg_G2_BG2X = 0;
    reg_G2_BG2Y = 0;
    reg_G2_BG3X = 0;
    reg_G2_BG3Y = 0;
    reg_G2_BG2PA = bg_mtx_elem_one;
    reg_G2_BG2PB = 0;
    reg_G2_BG2PC = 0;
    reg_G2_BG2PD = bg_mtx_elem_one;
    reg_G2_BG3PA = bg_mtx_elem_one;
    reg_G2_BG3PB = 0;
    reg_G2_BG3PC = 0;
    reg_G2_BG3PD = bg_mtx_elem_one;

    // init windows
    reg_G2_WININ = 0;
    reg_G2_WINOUT = 0;
    reg_G2_WIN0H = 0;
    reg_G2_WIN1H = 0;
    reg_G2_WIN0V = 0;
    reg_G2_WIN1V = 0;

    // init mosaic
    reg_G2_MOSAIC = 0;

    // init blending
    reg_G2_BLDCNT = 0;
    reg_G2_BLDALPHA = 0;
    reg_G2_BLDY = 0;

    // init master brightness
    reg_GX_MASTER_BRIGHT = 0;

    //
    // Sub engine
    //
    reg_GXS_DB_DISPCNT = 0;

    // init BGxCNT
    reg_G2S_DB_BG0CNT = 0;
    reg_G2S_DB_BG1CNT = 0;
    reg_G2S_DB_BG2CNT = 0;
    reg_G2S_DB_BG3CNT = 0;

    // reset BG offsets
    reg_G2S_DB_BG0OFS = 0;
    reg_G2S_DB_BG1OFS = 0;
    reg_G2S_DB_BG2OFS = 0;
    reg_G2S_DB_BG3OFS = 0;

    // init affine BG
    reg_G2S_DB_BG2X = 0;
    reg_G2S_DB_BG2Y = 0;
    reg_G2S_DB_BG3X = 0;
    reg_G2S_DB_BG3Y = 0;
    reg_G2S_DB_BG2PA = bg_mtx_elem_one;
    reg_G2S_DB_BG2PB = 0;
    reg_G2S_DB_BG2PC = 0;
    reg_G2S_DB_BG2PD = bg_mtx_elem_one;
    reg_G2S_DB_BG3PA = bg_mtx_elem_one;
    reg_G2S_DB_BG3PB = 0;
    reg_G2S_DB_BG3PC = 0;
    reg_G2S_DB_BG3PD = bg_mtx_elem_one;

    // init windows
    reg_G2S_DB_WININ = 0;
    reg_G2S_DB_WINOUT = 0;
    reg_G2S_DB_WIN0H = 0;
    reg_G2S_DB_WIN1H = 0;
    reg_G2S_DB_WIN0V = 0;
    reg_G2S_DB_WIN1V = 0;

    // init mosaic
    reg_G2S_DB_MOSAIC = 0;

    // init blending
    reg_G2S_DB_BLDCNT = 0;
    reg_G2S_DB_BLDALPHA = 0;
    reg_G2S_DB_BLDY = 0;

    // init master brightness
    reg_GXS_DB_MASTER_BRIGHT = 0;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetVCountEqVal

  Description:  Specifies the V-counter agreement value.

  Arguments:    val        the value of V-counter

  Returns:      none
 *---------------------------------------------------------------------------*/
void GX_SetVCountEqVal(s32 val)
{
    SDK_MINMAX_ASSERT(val, 0, 262);
    reg_GX_DISPSTAT = (u16)((reg_GX_DISPSTAT & (REG_GX_DISPSTAT_VBLK_MASK |
                                                REG_GX_DISPSTAT_HBLK_MASK |
                                                REG_GX_DISPSTAT_LYC_MASK |
                                                REG_GX_DISPSTAT_VBI_MASK |
                                                REG_GX_DISPSTAT_HBI_MASK |
                                                REG_GX_DISPSTAT_VQI_MASK)) |
                            ((val & 0xff) << 8) | ((val & 0x100) >> 1));
}


/*---------------------------------------------------------------------------*
  Name:         GX_HBlankIntr

  Description:  Enables/Disables the H-Blank interrupt generation.

  Arguments:    enable     disable if FALSE, enable otherwise.

  Returns:      none
 *---------------------------------------------------------------------------*/
s32 GX_HBlankIntr(BOOL enable)
{
    s32     rval = (reg_GX_DISPSTAT & REG_GX_DISPSTAT_HBI_MASK);
    if (enable)
    {
        reg_GX_DISPSTAT |= REG_GX_DISPSTAT_HBI_MASK;
    }
    else
    {
        reg_GX_DISPSTAT &= ~REG_GX_DISPSTAT_HBI_MASK;
    }
    return rval;
}


/*---------------------------------------------------------------------------*
  Name:         GX_VBlankIntr

  Description:  Enables/Disables the V-Blank interrupt generation.

  Arguments:    enable     disable if FALSE, enable otherwise.

  Returns:      none
 *---------------------------------------------------------------------------*/
s32 GX_VBlankIntr(BOOL enable)
{
    s32     rval = (reg_GX_DISPSTAT & REG_GX_DISPSTAT_VBI_MASK);
    if (enable)
    {
        reg_GX_DISPSTAT |= REG_GX_DISPSTAT_VBI_MASK;
    }
    else
    {
        reg_GX_DISPSTAT &= ~REG_GX_DISPSTAT_VBI_MASK;
    }
    return rval;
}


//---------------------------------------------------------------------------
// Internal state for GX_DispOff, GX_DispOn, GX_SetGraphicsMode
//---------------------------------------------------------------------------
// Use u16 to avoid byte access problem(thumb mode).
static u16 sDispMode = 0;
static u16 sIsDispOn = TRUE;


/*---------------------------------------------------------------------------*
  Name:         GX_IsDispOn

  Description:  Get displaying state.

  Arguments:    none

  Returns:      if disp is ON  then TRUE 
                else FALSE 
 *---------------------------------------------------------------------------*/
BOOL GX_IsDispOn(void)
{
    return sIsDispOn;
}


/*---------------------------------------------------------------------------*
  Name:         GX_DispOff

  Description:  Stops displaying the output of the MAIN engine.

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
void GX_DispOff(void)
{
    u32     tmp = reg_GX_DISPCNT;

    sIsDispOn = FALSE;
    sDispMode = (u16)((tmp & REG_GX_DISPCNT_MODE_MASK) >> REG_GX_DISPCNT_MODE_SHIFT);

    reg_GX_DISPCNT = tmp & ~REG_GX_DISPCNT_MODE_MASK;

	//---- record disp off counter
	PMi_SetDispOffCount();
}


/*---------------------------------------------------------------------------*
  Name:         GX_DispOn

  Description:  Starts displaying the output of the MAIN engine.

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
void GX_DispOn(void)
{
    sIsDispOn = TRUE;
    if (sDispMode != (u16)GX_DISPMODE_OFF)
    {
        // restore
        reg_GX_DISPCNT =
            ((reg_GX_DISPCNT & ~REG_GX_DISPCNT_MODE_MASK) |
             (sDispMode << REG_GX_DISPCNT_MODE_SHIFT));
    }
    else
    {
        // GX_DISPMODE_GRAPHICS if sDispMode is GX_DISPMODE_OFF
        reg_GX_DISPCNT = ((reg_GX_DISPCNT | (GX_DISPMODE_GRAPHICS << REG_GX_DISPCNT_MODE_SHIFT)));
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_SetGraphicsMode

  Description:  Specifies display mode and BG mode(MAIN engine).

  Arguments:    dispMode   display mode
                bgMode     BG mode
                bg0_2d3D   select 2D/3D for BG #0

  Returns:      none
 *---------------------------------------------------------------------------*/
void GX_SetGraphicsMode(GXDispMode dispMode, GXBGMode bgMode, GXBG0As bg0_2d3d)
{
    u32     cnt = reg_GX_DISPCNT;

    GX_DISPMODE_ASSERT(dispMode);
    SDK_WARNING(dispMode != GX_DISPMODE_OFF,
                "A parameter GX_DISPMODE_OFF is obsolete. Use GX_DispOff() instead.");

    sDispMode = (u16)dispMode;
    if (!sIsDispOn)
    {
        dispMode = GX_DISPMODE_OFF;
    }

    GX_BGMODE_ASSERT(bgMode);
    GX_BG0_AS_ASSERT(bg0_2d3d);
    cnt &= ~(REG_GX_DISPCNT_BGMODE_MASK |
             REG_GX_DISPCNT_BG02D3D_MASK | REG_GX_DISPCNT_MODE_MASK | REG_GX_DISPCNT_VRAM_MASK);

    reg_GX_DISPCNT = (u32)(cnt |
                           (dispMode << REG_GX_DISPCNT_MODE_SHIFT) |
                           (bgMode << REG_GX_DISPCNT_BGMODE_SHIFT) | (bg0_2d3d <<
                                                                      REG_GX_DISPCNT_BG02D3D_SHIFT));

    // needless when GX_DISPMODE_OFF is removed.
    if (sDispMode == GX_DISPMODE_OFF)
    {
        sIsDispOn = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GXS_SetGraphicsMode

  Description:  Specifies BG mode(SUB engine).

  Arguments:    bgMode     BG mode

  Returns:      none
 *---------------------------------------------------------------------------*/
void GXS_SetGraphicsMode(GXBGMode bgMode)
{
    reg_GXS_DB_DISPCNT = ((reg_GXS_DB_DISPCNT & ~REG_GXS_DB_DISPCNT_BGMODE_MASK) |
                          (bgMode << REG_GXS_DB_DISPCNT_BGMODE_SHIFT));
}

//
// Internal use
//
void GXx_SetMasterBrightness_(vu16 *reg, int brightness)
{
    SDK_MINMAX_ASSERT(brightness, -16, 16);

    if (brightness == 0)
    {
        *reg = 0;
    }
    else if (brightness > 0)
    {
        *reg = (u16)((1 << REG_GX_MASTER_BRIGHT_E_MOD_SHIFT) | brightness);
    }
    else
    {
        *reg = (u16)((2 << REG_GX_MASTER_BRIGHT_E_MOD_SHIFT) | (-brightness));
    }
}

//
// Internal use
//
int GXx_GetMasterBrightness_(vu16 *reg)
{
    u16     mode = (u16)(*reg & REG_GX_MASTER_BRIGHT_E_MOD_MASK);

    if (mode == 0)
    {
        return 0;
    }
    else if (mode == (1 << REG_GX_MASTER_BRIGHT_E_MOD_SHIFT))
    {
        return *reg & REG_GX_MASTER_BRIGHT_E_VALUE_MASK;
    }
    else if (mode == (2 << REG_GX_MASTER_BRIGHT_E_MOD_SHIFT))
    {
        return -(*reg & REG_GX_MASTER_BRIGHT_E_VALUE_MASK);
    }
    else
    {
        OS_TWarning("Illegal MasterBright mode!\n");
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GX_SetDefaultDMA

  Description:  set default DMA channel

  Arguments:    dma_no           default dma channel for GX
                                 if out of range(0-3),
                                 use CpuCopy instead of DMA.

  Returns:      previous DMA channel.
 *---------------------------------------------------------------------------*/
u32 GX_SetDefaultDMA(u32 dma_no)
{
    u32     previous = GXi_DmaId;
    OSIntrMode enabled;

    SDK_ASSERT((dma_no <= MI_DMA_MAX_NUM) || (dma_no == GX_DMA_NOT_USE));

    if (GXi_DmaId != GX_DMA_NOT_USE)
    {
#ifdef SDK_TWL
        if (GXi_DmaId > 3)
        {
            MI_WaitNDma(GXi_DmaId-4);
        }
        else
#endif
        {
            MI_WaitDma(GXi_DmaId);
        }
    }

    enabled = OS_DisableInterrupts();

    GXi_DmaId = dma_no;

    (void)OS_RestoreInterrupts(enabled);

    if (previous > 3)
    {
        return GX_DMA_NOT_USE;
    }
    return previous;
}

/*---------------------------------------------------------------------------*
  Name:         GX_SetDefaultNDMA

  Description:  set default NDMA channel

  Arguments:    ndma_no           default ndma channel for GX
                                 if out of range(0-3),
                                 use CpuCopy instead of NDMA.

  Returns:      previous DMA channel.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
u32 GX_SetDefaultNDMA(u32 ndma_no)
{
    u32     previous = GXi_DmaId;
    OSIntrMode enabled;

    if( OS_IsRunOnTwl() )
    {
        SDK_ASSERT((ndma_no <= MI_DMA_MAX_NUM) || (ndma_no == GX_DMA_NOT_USE));

        if (GXi_DmaId != GX_DMA_NOT_USE)
        {
            if (GXi_DmaId > 3)
            {
                MI_WaitNDma(GXi_DmaId-4);
            }
            else
            {
                MI_WaitDma(GXi_DmaId);
            }
        }

        enabled = OS_DisableInterrupts();

        GXi_DmaId = ndma_no;
        if(ndma_no != GX_DMA_NOT_USE)
        {
            GXi_DmaId += 4;
        }

        (void)OS_RestoreInterrupts(enabled);

        if ((previous > 3)&&(previous != GX_DMA_NOT_USE))
        {
            return previous-4;
        }
    }
    return GX_DMA_NOT_USE;
}
#endif
