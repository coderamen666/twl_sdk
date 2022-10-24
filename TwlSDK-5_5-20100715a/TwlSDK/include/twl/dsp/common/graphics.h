/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dsp - 
  File:     dsp_graphics.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-02#$
  $Rev: 8184 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef TWL_DSP_GRAPHICS_H_
#define TWL_DSP_GRAPHICS_H_

#include <twl/dsp/common/pipe.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Constants */
typedef DSPWord DSPGraphicsScalingMode;
#define DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR (DSPGraphicsScalingMode)0x0001
#define DSP_GRAPHICS_SCALING_MODE_BILINEAR   (DSPGraphicsScalingMode)0x0002
#define DSP_GRAPHICS_SCALING_MODE_BICUBIC    (DSPGraphicsScalingMode)0x0003

#define DSP_GRAPHICS_SCALING_MODE_NPARTSHRINK (DSPGraphicsScalingMode)0x000A

// Registers to use
#define DSP_GRAPHICS_COM_REGISTER 0    // Register number for processing content notifications
#define DSP_GRAPHICS_REP_REGISTER 1    // Register number for processing result notifications

/*---------------------------------------------------------------------------*/
/* Structures and enumerated types used between the DSP and ARM processors */

// Values exchanged with the semaphore register
typedef enum DspState
{
    DSP_STATE_FAIL        = 0x0000,
    DSP_STATE_SUCCESS     = 0x0001
//    DSP_STATE_END_ONEPROC = 0x0002
} DSPSTATE;

// Values that will be specified by an ARM processor, indicating the process for the DSP to run
typedef enum _GraphicsFuncID
{
    DSP_G_FUNCID_SCALING = 0x0001,
    DSP_G_FUNCID_YUV2RGB,
    
    DSP_FUNCID_NUM
} DSPGraphicsFuncID;

// Data structures to use when converting between YUV and RGB
typedef struct _Yuv2RgbParam
{
    u32 size;
    u32 src;
    u32 dst;
} DSPYuv2RgbParam;

// Data structures to use for scaling
typedef struct _ScalingParam
{
    u32 src;
    u32 dst;
    u16 mode;
    u16 img_width;
    u16 img_height;
    u16 rate_w;
    u16 rate_h;
    u16 block_x;
    u16 block_y;
    u16 width;
    u16 height;
    u16 pad[1];    // This makes the size of _ScalingParam a multiple of 4
} DSPScalingParam;

typedef void (*DSP_GraphicsCallback)(void);  // User-specified callback function to invoke when processing is complete

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
    Macro Definitions
*---------------------------------------------------------------------------*/
// Macros that calculate the size to be output with DSP_Scaling* functions
#define DSP_CALC_SCALING_SIZE(value, ratio) ((u32)(value * (u32)(ratio * 1000) / 1000))

#define DSP_CALC_SCALING_SIZE_FX(value, ratio) ((u32)( value * (u32)(ratio * 1000 / 4096.0f) / 1000))

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
*---------------------------------------------------------------------------*/
static DSP_GraphicsCallback callBackFunc;   // Callback function to invoke when asynchronous processing is complete

static volatile BOOL isBusy;                         // TRUE if some process is running on the DSP
static volatile BOOL isAsync;                        // TRUE if some asynchronous process is running on the DSP

/*---------------------------------------------------------------------------*
    Internal core functions placed on LTDMAIN
*---------------------------------------------------------------------------*/
void DSPi_OpenStaticComponentGraphicsCore(FSFile *file);
BOOL DSPi_LoadGraphicsCore(FSFile *file, int slotB, int slotC);
void DSPi_UnloadGraphicsCore(void);
BOOL DSPi_ConvertYuvToRgbCore(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback, BOOL async);
BOOL DSPi_ScalingCore(const void* src, void* dst, u16 img_width, u16 img_height, f32 rw, f32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async);
BOOL DSPi_ScalingFxCore(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rw, fx32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async);


/*---------------------------------------------------------------------------*
  Name:         DSP_OpenStaticComponentGraphics

  Description:  Opens the memory file for a graphics component.
                Although you will no longer have to make file system preparations in advance, this will be linked as static memory and thus increase the program size.
                
  
  Arguments:    file: The FSFile structure to open the memory file.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DSP_OpenStaticComponentGraphics(FSFile *file)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_OpenStaticComponentGraphicsCore(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadGraphics

  Description:  Loads a DSP graphics component.

  Arguments:    file: The graphics component file.
                slotB: WRAM-B that was allowed to be used for code memory.
                slotC: WRAM-C that was allowed to be used for data memory.
  
  Returns:      TRUE if the graphics component is loaded properly in the DSP.
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_LoadGraphics(FSFile *file, int slotB, int slotC)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSPi_LoadGraphicsCore(file, slotB, slotC);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadGraphics

  Description:  Shuts down the DSP graphics component.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DSP_UnloadGraphics(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_UnloadGraphicsCore();
    }
}

//--------------------------------------------------------------------------------
//    YUV-to-RGB conversion
//
/*---------------------------------------------------------------------------*
  Name:         DSPi_YuvToRgbConvert[Async]

  Description:  Converts from YUV to RGB.

  Arguments:    src: Address of the data to process
                dest: Address to store the processed data
                size: The src data size
     'Async' only [callback: Pointer to the callback function to invoke when conversion has finished]

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_ConvertYuvToRgb(const void* src, void* dst, u32 size)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ConvertYuvToRgbCore(src, dst, size, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ConvertYuvToRgbAsync(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ConvertYuvToRgbCore(src, dst, size, callback, TRUE);
    }
    return FALSE;
}

//--------------------------------------------------------------------------------
// 
//

//--------------------------------------------------------------------------------
//    Scaling
//

/*---------------------------------------------------------------------------*
  Name:         DSP_Scaling[Fx][Async][Ex]

  Description:  Enlarges or shrinks image data.
                *You can process this asynchronously with the 'Async' version. When it has completed, the registered callback will be invoked.
                *You can specify an arbitrary image region with the 'Ex' version.

  Arguments:    src: Pointer to the image data to process (2-byte aligned)
                dst: Pointer to store the processing results (this must be 2-byte aligned with the processed data size allocated)
                img_width: Width of src
                img_height: Height of src
                rx: Horizontal factor (31-0.001, truncated at the fourth decimal place) [This is the same as the 'Fx' version]
                ry: Vertical factor (31-0.001, truncated at the fourth decimal place) [This is the same as the 'Fx' version]
                mode: Interpolation method (nearest-neighbor, bilinear, or bicubic)
       'Ex' only [x: x-coordinate at the upper-left of the region to process]
       'Ex' only [y: y-coordinate at the upper-left of the region to process]
       'Ex' only [width: Width of the region to process]
       'Ex' only [height: Height of the region to process]
    'Async' only [callback: Callback function to invoke when processing is complete]

  Returns:      Returns TRUE on success.
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_Scaling(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingAsync(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry,
                             DSPGraphicsScalingMode mode, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxAsync(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry,
                             DSPGraphicsScalingMode mode, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingEx(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxEx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingAsyncEx(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxAsyncEx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                                  u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, callback, TRUE);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CalcScalingFactor[Fx]

  Description:  This function reverse-calculates the factor to specify to DSP_Scaling from an input and an output value.
                This contains error correction for truncated values less than 0.001, but due to factor restrictions it is not guaranteed to perfectly derive factors that can be calculated for any output size.
                

  Arguments:    src_size: Input size
                dst_size: Output size

  Returns:      Returns the factor (f32 or fx32) that will result in dst_size.
 *---------------------------------------------------------------------------*/
static inline f32 DSP_CalcScalingFactorF32(const u16 src_size, const u16 dst_size)
{
    // Add 0.0009 to fix errors in the DSP_Scaling arguments, which are restricted to approximately 0.001
    return (dst_size / (f32)src_size + 0.0009f);
}

static inline fx32 DSP_CalcScalingFactorFx32(const u16 src_size, const u16 dst_size)
{
    return FX_F32_TO_FX32(dst_size / (f32)src_size + 0.0009f);
}

/*===========================================================================*/

#endif    // SDK_TWL

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_GRAPHICS_H_ */
