/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     g3x.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_G3X_H_
#define NITRO_G3X_H_

#include <nitro/gx/gxcommon.h>

#ifndef SDK_TWL
#include <nitro/hw/ARM9/ioreg_G3X.h>
#include <nitro/hw/ARM9/mmap_global.h>
#else
#include <twl/hw/ARM9/ioreg_G3X.h>
#include <twl/hw/ARM9/mmap_global.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Type definition
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//    Parameters for G3X_SetShading
//----------------------------------------------------------------------------
typedef enum
{
    GX_SHADING_TOON = 0,
    GX_SHADING_HIGHLIGHT = 1
}
GXShading;

#define GX_SHADING_ASSERT(x)              \
    SDK_MINMAX_ASSERT(x, GX_SHADING_TOON, GX_SHADING_HIGHLIGHT)

//----------------------------------------------------------------------------
//    Parameters for G3X_SetFog
//----------------------------------------------------------------------------
typedef enum
{
    GX_FOGBLEND_COLOR_ALPHA = 0,
    GX_FOGBLEND_ALPHA = 1
}
GXFogBlend;

#define GX_FOGBLEND_ASSERT(x)                          \
    SDK_MINMAX_ASSERT(x, GX_FOGBLEND_COLOR_ALPHA, GX_FOGBLEND_ALPHA)

typedef enum
{
    GX_FOGSLOPE_0x8000 = 0,
    GX_FOGSLOPE_0x4000 = 1,
    GX_FOGSLOPE_0x2000 = 2,
    GX_FOGSLOPE_0x1000 = 3,
    GX_FOGSLOPE_0x0800 = 4,
    GX_FOGSLOPE_0x0400 = 5,
    GX_FOGSLOPE_0x0200 = 6,
    GX_FOGSLOPE_0x0100 = 7,
    GX_FOGSLOPE_0x0080 = 8,
    GX_FOGSLOPE_0x0040 = 9,
    GX_FOGSLOPE_0x0020 = 10
}
GXFogSlope;

#define GX_FOGSLOPE_ASSERT(x)                \
    SDK_MINMAX_ASSERT(x, GX_FOGSLOPE_0x8000, GX_FOGSLOPE_0x0020)

//----------------------------------------------------------------------------
//    Parameters for G3X_SetFifoIntrCond
//----------------------------------------------------------------------------
typedef enum
{
    GX_FIFOINTR_COND_DISABLE = 0,
    GX_FIFOINTR_COND_UNDERHALF = 1,
    GX_FIFOINTR_COND_EMPTY = 2
}
GXFifoIntrCond;

#define GX_FIFOINTR_COND_ASSERT(x)                        \
    SDK_MINMAX_ASSERT(x, GX_FIFOINTR_COND_DISABLE, GX_FIFOINTR_COND_EMPTY)

//----------------------------------------------------------------------------
//    Return type of G3X_GetCommandFifoStatus
//----------------------------------------------------------------------------
typedef enum
{
    GX_FIFOSTAT_EMPTY = 6,
    GX_FIFOSTAT_UNDERHALF = 2,
    GX_FIFOSTAT_OVERHALF = 0,
    GX_FIFOSTAT_FULL = 1
}
GXFifoStat;

#define GX_RDLINES_COUNT_MAX   46
#define GX_LISTRAM_COUNT_MAX   2048    // 0x800
#define GX_VTXRAM_COUNT_MAX    6144    // 0x1800


/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))

//----------------------------------------------------------------------------
// Declaration of function
//----------------------------------------------------------------------------

// initialization
void    G3X_Init(void);
void    G3X_InitMtxStack(void);
void    G3X_ResetMtxStack(void);
void    G3X_InitTable(void);
void    G3X_Reset(void);

// for reg_G3X_DISP3DCNT
static void G3X_SetShading(GXShading shading);
static void G3X_AlphaTest(BOOL enable, int ref);
static void G3X_AlphaBlend(BOOL enable);
static void G3X_AntiAlias(BOOL enable);
static void G3X_EdgeMarking(BOOL enable);
void    G3X_SetFog(BOOL enable, GXFogBlend fogMode, GXFogSlope fogSlope, int fogOffset);

// for reg_G3X_GXSTAT
int     G3X_GetMtxStackLevelPV(s32 *level);     // read: GXSTAT(8-12), GXSTAT(14)
int     G3X_GetMtxStackLevelPJ(s32 *level);     // read: GXSTAT(13),   GXSTAT(14)
static void G3X_SetFifoIntrCond(GXFifoIntrCond cond);

// for reg_G3X_DISP_1DOT_DEPTH
void    G3X_SetDisp1DotDepth(fx32 w);  // write: DISP_1DOT_DEPTH

int     G3X_GetBoxTestResult(s32 *in); // read: GXSTAT(0-1)
int     G3X_GetPositionTestResult(VecFx32 *vec, fx32 *w);       // read: GXSTAT(0), POS_RESULT_X, POS_RESULT_Y, POS_RESULT_Z, POS_RESULT_W
int     G3X_GetVectorTestResult(fx16 *vec);     // read: GXSTAT(0), VEC_RESULT_X, VEC_RESULT_Y, VEC_RESULT_Z

int     G3X_GetClipMtx(MtxFx44 *m);    // read: CLIPMTX_RESULT_0-15
int     G3X_GetVectorMtx(MtxFx33 *m);

// for reg_G2_BG0OFS
void    G3X_SetHOffset(int hOffset);

void    G3X_SetEdgeColorTable(const GXRgb *rgb_8);
void    G3X_SetFogTable(const u32 *fogTable);
void    G3X_SetToonTable(const GXRgb *rgb_32);
void    G3X_SetClearColor(GXRgb rgb, int alpha, int depth, int polygonID, BOOL fog);
static void G3X_SetClearImageOffset(int xOffset, int yOffset);
static void G3X_SetFogColor(GXRgb rgb, int alpha);

static s32 G3X_IsMtxStackOverflow(s32 *overflow);
static s32 G3X_IsLineBufferUnderflow(void);
static s32 G3X_IsListRamOverflow(void);
static s32 G3X_IsGeometryBusy(void);

static void G3X_ResetMtxStackOverflow(void);
static void G3X_ResetLineBufferUnderflow(void);
static void G3X_ResetListRamOverflow(void);
void    G3X_ClearFifo(void);

static s32 G3X_GetPolygonListRamCount(void);
static s32 G3X_GetVtxListRamCount(void);
static s32 G3X_GetRenderedLineCount(void);
static s32 G3X_GetCommandFifoCount(void);
static GXFifoStat G3X_GetCommandFifoStatus(void);

//----------------------------------------------------------------------------
// Implementation of inline function
//----------------------------------------------------------------------------


// DISP3DCNT(1)
/*---------------------------------------------------------------------------*
  Name:         G3X_SetShading

  Description:  Select Toon or Highlight shading

  Arguments:    shading      select toon/highlight shading

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_SetShading(GXShading shading)
{
    reg_G3X_DISP3DCNT = (u16)((reg_G3X_DISP3DCNT & ~(REG_G3X_DISP3DCNT_THS_MASK |
                                                     REG_G3X_DISP3DCNT_RO_MASK |
                                                     REG_G3X_DISP3DCNT_GO_MASK)) |
                              (shading << REG_G3X_DISP3DCNT_THS_SHIFT));
}


// DISP3DCNT(2) and ALPHA_TEST_REF
/*---------------------------------------------------------------------------*
  Name:         G3X_AlphaTest

  Description:  Set the condition for Alpha Testing

  Arguments:    enable     if FALSE, alpha test is disabled.
                           otherwise, alpha test is enabled.
                ref        if alpha is less than or equal to 'ref',
                           polygons are not drawn.

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_AlphaTest(BOOL enable, int ref)
{
    if (enable)
    {
        GX_ALPHA_ASSERT(ref);
        reg_G3X_DISP3DCNT = (u16)(reg_G3X_DISP3DCNT &
                                  ~(REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK) |
                                  REG_G3X_DISP3DCNT_ATE_MASK);
        reg_G3X_ALPHA_TEST_REF = (u16)ref;
    }
    else
    {
        reg_G3X_DISP3DCNT &= (u16)~(REG_G3X_DISP3DCNT_ATE_MASK |
                                    REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK);
    }
}


// DISP3DCNT(3)
/*---------------------------------------------------------------------------*
  Name:         G3X_AlphaBlend

  Description:  Enable/Disable alpha blending.
                Blends color buffer's color and fragment color by the fragment's alpha

  Arguments:    enable     if FALSE, alpha blending is disabled.
                           otherwise, alpha blending is enabled.

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_AlphaBlend(BOOL enable)
{
    if (enable)
    {
        reg_G3X_DISP3DCNT = (u16)(reg_G3X_DISP3DCNT &
                                  ~(REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK) |
                                  REG_G3X_DISP3DCNT_ABE_MASK);
    }
    else
    {
        reg_G3X_DISP3DCNT &= (u16)~(REG_G3X_DISP3DCNT_ABE_MASK |
                                    REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK);
    }
}


// DISP3DCNT(4)
/*---------------------------------------------------------------------------*
  Name:         G3X_AntiAlias

  Description:  Enable/Disable anti-aliasing

  Arguments:    enable     if FALSE, anti-aliasing is disabled.
                           otherwise, anti-aliasing is enabled.

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_AntiAlias(BOOL enable)
{
    if (enable)
    {
        reg_G3X_DISP3DCNT = (u16)(reg_G3X_DISP3DCNT &
                                  ~(REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK) |
                                  REG_G3X_DISP3DCNT_AAE_MASK);
    }
    else
    {
        reg_G3X_DISP3DCNT &= (u16)~(REG_G3X_DISP3DCNT_AAE_MASK |
                                    REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK);
    }
}


// DISP3DCNT(5)
/*---------------------------------------------------------------------------*
  Name:         G3X_EdgeMarking

  Description:  Enable/Disable edge marking

  Arguments:    enable     if FALSE, edge marking is disabled.
                           otherwise, edge marking is enabled.

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_EdgeMarking(BOOL enable)
{
    if (enable)
    {
        reg_G3X_DISP3DCNT = (u16)(reg_G3X_DISP3DCNT &
                                  ~(REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK) |
                                  REG_G3X_DISP3DCNT_EME_MASK);
    }
    else
    {
        reg_G3X_DISP3DCNT &= (u16)~(REG_G3X_DISP3DCNT_EME_MASK |
                                    REG_G3X_DISP3DCNT_RO_MASK | REG_G3X_DISP3DCNT_GO_MASK);
    }
}


/*---------------------------------------------------------------------------*
  Name:         G3X_SetFifoIntrCond

  Description:  Specifies the condition for the geometry command FIFO interrupt
                request generation.

  Arguments:    cond       specify disable/underhalf/empty
  
  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_SetFifoIntrCond(GXFifoIntrCond cond)
{
    GX_FIFOINTR_COND_ASSERT(cond);

    reg_G3X_GXSTAT = ((reg_G3X_GXSTAT & ~REG_G3X_GXSTAT_FI_MASK) |
                      (cond << REG_G3X_GXSTAT_FI_SHIFT));
}


/*---------------------------------------------------------------------------*
  Name:         G3X_SetClearImageOffset

  Description:  The offset of clearimage can be specified if enabled.

  Arguments:    xOffset    X offset (in dot)
                yOffset    Y offset (in dot)
  
  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_SetClearImageOffset(int xOffset, int yOffset)
{
    reg_G3X_CLRIMAGE_OFFSET = (u16)(((xOffset << REG_G3X_CLRIMAGE_OFFSET_OFFSETX_SHIFT) &
                                     REG_G3X_CLRIMAGE_OFFSET_OFFSETX_MASK) | ((yOffset <<
                                                                               REG_G3X_CLRIMAGE_OFFSET_OFFSETY_SHIFT)
                                                                              &
                                                                              REG_G3X_CLRIMAGE_OFFSET_OFFSETY_MASK));
}


/*---------------------------------------------------------------------------*
  Name:         G3X_SetFogColor

  Description:  Specifies the color of Fog.

  Arguments:    rgb       RGB of fog
                alpha     alpha of fog
  
  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_SetFogColor(GXRgb rgb, int alpha)
{
    GXRGB_ASSERT(rgb);
    GX_ALPHA_ASSERT(alpha);

    reg_G3X_FOG_COLOR = (u32)((rgb << REG_G3X_FOG_COLOR_FOG_RED_SHIFT) |
                              (alpha << REG_G3X_FOG_COLOR_FOG_ALPHA_SHIFT));
}


/*---------------------------------------------------------------------------*
  Name:         G3X_IsMtxStackOverflow

  Description:  Test if the matrix stack overflows

  Arguments:    overflow    a pointer to overflow flag of matrix stack.
                            0 if the stack isn't overflow.
                            others the stack is overflow.

  Returns:      0 if *overflow has the result (not busy)
                others the matrix stack is busy.
 *---------------------------------------------------------------------------*/
static inline s32 G3X_IsMtxStackOverflow(s32 *overflow)
{
    SDK_NULL_ASSERT(overflow);
    if (reg_G3X_GXSTAT & REG_G3X_GXSTAT_SB_MASK)
    {
        return -1;
    }

    *overflow = (s32)(reg_G3X_GXSTAT & REG_G3X_GXSTAT_SE_MASK);
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_IsLineBufferUnderflow

  Description:  Test if the line buffer underflow

  Arguments:    none

  Returns:      0 if the buffer does not overflow
                others if the buffer overflows
 *---------------------------------------------------------------------------*/
#define G3X_IsLineBufferOverflow    G3X_IsLineBufferUnderflow

static inline s32 G3X_IsLineBufferUnderflow(void)
{
    return (reg_G3X_DISP3DCNT & REG_G3X_DISP3DCNT_RO_MASK);
}


/*---------------------------------------------------------------------------*
  Name:         G3X_IsListRamOverflow

  Description:  Test if the polygon list RAM or the vertex RAM overflow

  Arguments:    none

  Returns:      0 if the list does not overflow
                others if the list overflows
 *---------------------------------------------------------------------------*/
static inline s32 G3X_IsListRamOverflow(void)
{
    return (reg_G3X_DISP3DCNT & REG_G3X_DISP3DCNT_GO_MASK);
}


/*---------------------------------------------------------------------------*
  Name:         G3X_IsGeometryBusy

  Description:  Test if geometry engine is busy

  Arguments:    none

  Returns:      0 if geometry engine is not busy
                others if geometry engine is busy
 *---------------------------------------------------------------------------*/
static inline s32 G3X_IsGeometryBusy(void)
{
    return (s32)(reg_G3X_GXSTAT & REG_G3X_GXSTAT_B_MASK);
}


/*---------------------------------------------------------------------------*
  Name:         G3X_ResetMtxStackOverflow

  Description:  Reset overflow state of the matrix stack

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_ResetMtxStackOverflow(void)
{
    reg_G3X_GXSTAT |= REG_G3X_GXSTAT_SE_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_ResetLineBufferUnderflow

  Description:  Reset overflow state of the line buffer

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
#define G3X_ResetLineBufferOverflow     G3X_ResetLineBufferUnderflow

static inline void G3X_ResetLineBufferUnderflow(void)
{
    reg_G3X_DISP3DCNT |= REG_G3X_DISP3DCNT_RO_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_ResetListRamOverflow

  Description:  Reset overflow state of the polygon list RAM and the vertex RAM

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
static inline void G3X_ResetListRamOverflow(void)
{
    reg_G3X_DISP3DCNT |= REG_G3X_DISP3DCNT_GO_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_GetPolygonListRamCount

  Description:  Returns the number of polygons in the polygon list RAM

  Arguments:    none

  Returns:      the number of polygons
 *---------------------------------------------------------------------------*/
static inline s32 G3X_GetPolygonListRamCount(void)
{
    return reg_G3X_LISTRAM_COUNT;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_GetVtxListRamCount

  Description:  Returns the number of vertexes in the vertex RAM

  Arguments:    none

  Returns:      the number of vertexes
 *---------------------------------------------------------------------------*/
static inline s32 G3X_GetVtxListRamCount(void)
{
    return reg_G3X_VTXRAM_COUNT;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_GetRenderedLineCount

  Description:  Returns the minimum number of the rendered lines in the frame just before

  Arguments:    none

  Returns:      the minimum number of the rendered lines
 *---------------------------------------------------------------------------*/
static inline s32 G3X_GetRenderedLineCount(void)
{
    return reg_G3X_RDLINES_COUNT;
}


/*---------------------------------------------------------------------------*
  Name:         G3X_GetCommandFifoCount

  Description:  Returns the number of the geometry command/data in the command FIFO

  Arguments:    none

  Returns:      the number of the geometry command/data in the command FIFO
 *---------------------------------------------------------------------------*/
static inline s32 G3X_GetCommandFifoCount(void)
{
    return (s32)((reg_G3X_GXSTAT & REG_G3X_GXSTAT_FIFOCNT_MASK) >> REG_G3X_GXSTAT_FIFOCNT_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         G3X_GetCommandFifoStatus

  Description:  Return the status of the command FIFO

  Arguments:    none

  Returns:      the status of the command FIFO
 *---------------------------------------------------------------------------*/
static inline GXFifoStat G3X_GetCommandFifoStatus(void)
{
    return (GXFifoStat)((reg_G3X_GXSTAT & (REG_G3X_GXSTAT_F_MASK |
                                           REG_G3X_GXSTAT_H_MASK |
                                           REG_G3X_GXSTAT_E_MASK)) >> REG_G3X_GXSTAT_F_SHIFT);
}



#endif // SDK_FROM_TOOL

#ifdef __cplusplus
}/* extern "C" */
#endif

/* NITRO_G3X_H_ */
#endif
