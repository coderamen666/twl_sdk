/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dsp
  File:     dsp_graphics.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/graphics.h>

#include "dsp_process.h"

extern const u8 DSPiFirmware_graphics[];

/*---------------------------------------------------------------------------*/
/* Variables */

static DSPPipe binOut[1];
static BOOL DSPiGraphicsAvailable = FALSE;
static DSPProcessContext DSPiProcessGraphics[1];

static u16 replyReg;

/* Determine whether limitation conditions have been met (FALSE if so) */
static BOOL CheckLimitation(f32 rx, f32 ry, DSPGraphicsScalingMode mode, u16 src_width);

/*---------------------------------------------------------------------------*/
/* Functions */
/*---------------------------------------------------------------------------*
  Name:         CheckLimitation

  Description:  Determines if limitation conditions set by the DSP_Scaling* functions' specifications have been met.
  
  Arguments:    mode: Scaling mode
                rx, ry: Horizontal and vertical scaling factors
                src_width: Width of the input image
  
  Returns:      FALSE if limitation conditions have been met
 *---------------------------------------------------------------------------*/
static BOOL CheckLimitation(f32 rx, f32 ry, DSPGraphicsScalingMode mode, u16 src_width)
{
    // When the processed data size is at least as large as the original data size
    if (rx * ry >= 1.0f)
    {
        switch(mode)
        {
        case DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR:
        case DSP_GRAPHICS_SCALING_MODE_BILINEAR:
            if (DSP_CALC_SCALING_SIZE(src_width, rx) * ry >= 8192)
            {
                return FALSE;
            }
            break;
        case DSP_GRAPHICS_SCALING_MODE_BICUBIC:
            if (DSP_CALC_SCALING_SIZE(src_width, rx) * ry >= 4096)
            {
                return FALSE;
            }
            break;
        }
    }
    else    // When the processed data size is less than the original data size
    {
        switch(mode)
        {
        case DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR:
        case DSP_GRAPHICS_SCALING_MODE_BILINEAR:
            if (src_width >= 8192)
            {
                return FALSE;
            }
            break;
        case DSP_GRAPHICS_SCALING_MODE_BICUBIC:
            if (src_width >= 4096)
            {
                return FALSE;
            }
            break;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_OpenStaticComponentGraphicsCore

  Description:  Opens the memory file(s) for the graphics component.
                It is no longer necessary to prepare a file system in advance, but because it is linked as static memory, the program size increases.
                
  
  Arguments:    file: FSFile structure that opens the memory file
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_OpenStaticComponentGraphicsCore(FSFile *file)
{
    extern const u8 DSPiFirmware_graphics[];
    (void)DSPi_CreateMemoryFile(file, DSPiFirmware_graphics);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_GraphicsEvents(void *userdata)

  Description:  Event handler of the DSP graphics component.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_GraphicsEvents(void *userdata)
{
    (void)userdata;
    
    // When running asynchronous processing, check for the presence of completion notices
    if ( isAsync )
    {
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        if (replyReg == DSP_STATE_SUCCESS)
        {
            isBusy = FALSE;
            isAsync = FALSE;
                
            if ( callBackFunc != NULL)
            {
                callBackFunc();
            }
        }
        else if (replyReg == DSP_STATE_FAIL)
        {
            OS_TWarning("a process on DSP is failed.\n");
            isBusy = FALSE;
            isAsync = FALSE;
        }
        else
        {
            OS_TWarning("unknown error occured.\n");
            isBusy = FALSE;
            isAsync = FALSE;
        }
    }
    else    // Synchronous version
    {
//        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
//        isBusy = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadGraphicsCore

  Description:  Loads graphics component to DSP.
  
  Arguments:    file: Graphics component file
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory
  
  Returns:      TRUE if the graphics component is correctly loaded to DSP.
 *---------------------------------------------------------------------------*/
BOOL DSPi_LoadGraphicsCore(FSFile *file, int slotB, int slotC)
{
    if (!DSPiGraphicsAvailable)
    {
        isBusy = FALSE;
        isAsync = FALSE;
        DSP_InitProcessContext(DSPiProcessGraphics, "graphics");
        // (If there is specification to a linker script file on the DSP side, this explicit setting is unnecessary)
        DSPiProcessGraphics->flags |= DSP_PROCESS_FLAG_USING_OS;
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
        if (DSP_ExecuteProcess(DSPiProcessGraphics, file, slotB, slotC))
        {
            DSPiGraphicsAvailable = TRUE;
        }
        
        (void)DSP_LoadPipe(binOut, DSP_PIPE_BINARY, DSP_PIPE_OUTPUT);
    }
    
    return DSPiGraphicsAvailable;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadGraphicsCore

  Description:  Ends the DSP graphics component.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_UnloadGraphicsCore(void)
{
    if(DSPiGraphicsAvailable)
    {
        DSP_QuitProcess(DSPiProcessGraphics);
        DSPiGraphicsAvailable = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_YuvToRgbConvertCore

  Description:  Converts YUV to RGB.

  Arguments:    src: Address of (source) data to process
                dest: Address for storing data (destination) after processing
                size: Size of src data
                callback: Pointer for callback function run after conversion ends
                async: TRUE when running asynchronously

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL DSPi_ConvertYuvToRgbCore(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback, BOOL async)
{
    DSPYuv2RgbParam yr_param;
    u32 offset = 0;
    u16 command;
    
    // Check one more time whether DSP is busy
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // Register the callback function
    callBackFunc = callback;
    isAsync = async;
    
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // Notify the DSP of the content of processes to start
    command = DSP_G_FUNCID_YUV2RGB;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);
    
    // Send parameters to DSP
    yr_param.size = size;
    yr_param.src = (u32)((u32)src + offset);
    yr_param.dst = (u32)((u32)dst + offset);
    
    DSP_WritePipe(binOut, &yr_param, sizeof(DSPYuv2RgbParam));
    
    if (async)
    {
        return TRUE;
    }
    else
    {
        // Wait for a reply from the DSP
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ScalingCore

  Description:  Scales the image data.

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL DSPi_ScalingCore(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async)
{
    DSPScalingParam sc_param;
    
    u16 command;
    
    // Check limitation conditions
    SDK_TASSERTMSG(CheckLimitation(rx, ry, mode, width), "DSP_Scaling: arguments exceed the limit.");
    
    // Check one more time whether DSP is busy
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // Register the callback function
    callBackFunc = callback;
    isAsync = async;
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // Notify the DSP of the content of processes to start
    command = DSP_G_FUNCID_SCALING;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);

    // Transfer parameters to DSP
    sc_param.src = (u32)src;
    sc_param.dst = (u32)dst;
    sc_param.mode = mode;
    sc_param.img_width = img_width;
    sc_param.img_height = img_height;
    sc_param.rate_w = (u16)(rx * 1000);
    sc_param.rate_h = (u16)(ry * 1000);
    sc_param.block_x = x;
    sc_param.block_y = y;
    sc_param.width = width;
    sc_param.height = height;

    DSP_WritePipe(binOut, &sc_param, sizeof(DSPScalingParam));
    
    // Branch if processing asynchronously
    if(isAsync)
    {
        return TRUE;
    }
    else
    {
        // Wait for a reply from the DSP
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

BOOL DSPi_ScalingFxCore(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async)
{
    DSPScalingParam sc_param;
    
    u16 command;
    
    // Check limitation conditions
    SDK_TASSERTMSG(CheckLimitation(FX_FX32_TO_F32(rx), FX_FX32_TO_F32(ry), mode, width), "DSP_Scaling: arguments exceed the limit.");
    
    // Check one more time whether DSP is busy
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // Register the callback function
    callBackFunc = callback;
    isAsync = async;
    
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // Notify the DSP of the content of processes to start
    command = DSP_G_FUNCID_SCALING;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);
    
    // Transfer parameters to DSP
    sc_param.src = (u32)src;
    sc_param.dst = (u32)dst;
    sc_param.mode = mode;
    sc_param.img_width = img_width;
    sc_param.img_height = img_height;
    sc_param.rate_w = (u16)(rx / 4096.0f * 1000);
    sc_param.rate_h = (u16)(ry / 4096.0f * 1000);
    sc_param.block_x = x;
    sc_param.block_y = y;
    sc_param.width = width;
    sc_param.height = height;

    DSP_WritePipe(binOut, &sc_param, sizeof(DSPScalingParam));
    
    // Branch if processing asynchronously
    if(isAsync)
    {
        return TRUE;
    }
    else
    {
        // Wait for a reply from the DSP
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

