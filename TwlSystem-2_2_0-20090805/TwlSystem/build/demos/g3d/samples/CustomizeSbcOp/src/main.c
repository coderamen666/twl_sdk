/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - CustomizeSbcOp
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
//
// This sample replaces the SBC command processing function with one customized by the user.
//
// Sample Operation
// Three squares are displayed. One without a billboard, one with, and one a y-axis billboard.
// 
// It is possible to replace the billboard processing function with the one defined in this sample by pressing the A Button.
//  By pushing the B Button, the scale applied to the billboard is doubled.
// In addition, the billboard functions within this sample run faster than those in the library, but they won't display correctly when scaling is involved.
// 
//
// HOWTO:
// - Insert a pointer to a user-defined function corresponding to the command for a customized NNS_G3dFuncSbcTable array.
//   
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);


static void MySbcBBSimple(NNSG3dRS* rs, u32 opt);
static void MySbcBBYSimple(NNSG3dRS* rs, u32 opt);


/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    NNSG3dRenderObj object;
    NNSG3dResMdl*   model;
    BOOL            useCustomizedBB = FALSE;
    BOOL            isScaled = FALSE;

    G3DDemo_InitSystem();
    G3DDemo_InitMemory();
    G3DDemo_InitVRAM();

    InitG3DLib();
    G3DDemo_InitDisplay();

    // Make the manager able to manage 4 slots'-worth of texture image slots, and set it as the default manager.
    // 
    NNS_GfdInitFrmTexVramManager(4, TRUE);

    // Make the manager able to manage 32 KB of palettes, and set it as the default manager.
    // 
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    model = LoadG3DModel("data/billboard.nsbmd");
    SDK_ASSERTMSG(model, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set to a value, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, model);

    G3DDemo_InitCamera(&gCamera, 0 * FX32_ONE, 4 * FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5 * FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "CustomizeSbcOp");
    G3DDemo_Print(0,2, G3DDEMO_COLOR_WHITE, "change:A");
    G3DDemo_Print(0,4, G3DDEMO_COLOR_WHITE, "change:B");

    for(;;)
    {
        OSTick time;

        SVC_WaitVBlankIntr();

        G3DDemo_PrintApplyToHW();
        G3DDemo_ReadGamePad();

        G3DDemo_MoveCamera(&gCamera);
        G3DDemo_CalcCamera(&gCamera);

        G3X_Reset();
        G3X_ResetMtxStack();

        SetCamera(&gCamera);

        {
            VecFx32 scale;

            //
            // The scale applied to the model is changed with the B Button
            //
            if (isScaled)
            {
                scale.x = scale.y = scale.z = 2 * FX32_ONE;
            }
            else
            {
                scale.x = scale.y = scale.z = FX32_ONE;
            }

            NNS_G3dGlbSetBaseScale(&scale);

            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlushP();

            //  Start measuring the time
            time = OS_GetTick();
            {
                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                NNS_G3dDraw(&object);
            }
            // Time measurement result
            time = OS_GetTick() - time;
        }

        {
            // Restore scale to how it was before
            VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
            NNS_G3dGlbSetBaseScale(&scale);
            //
            // G3D: Normally, when any of the G3_XXX functions are called, you have to call the NNS_G3dGeComFlushBuffer function before that and synchronize.
            //      
            //
            NNS_G3dGlbFlushP();
            NNS_G3dGeFlushBuffer();
            G3DDemo_DrawGround(&gGround);
        }

        /* Swap memory relating to geometry and rendering engine */
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        // Change the handler of the SBC billboard command with the A Button.
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (useCustomizedBB)
            {
                useCustomizedBB = FALSE;
                NNS_G3dFuncSbcTable[NNS_G3D_SBC_BB]  = &NNSi_G3dFuncSbc_BB;
                NNS_G3dFuncSbcTable[NNS_G3D_SBC_BBY] = &NNSi_G3dFuncSbc_BBY;
            }
            else
            {
                useCustomizedBB = TRUE;
                NNS_G3dFuncSbcTable[NNS_G3D_SBC_BB]  = &MySbcBBSimple;
                NNS_G3dFuncSbcTable[NNS_G3D_SBC_BBY] = &MySbcBBYSimple;
            }
        }

        // Change the scale applied to the model with the B Button.
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_B))
        {
            if (isScaled)
                isScaled = FALSE;
            else
                isScaled = TRUE;
        }

        if (useCustomizedBB)
        {
            G3DDemo_Printf(0,3, G3DDEMO_COLOR_GREEN, "Use simplified BB Functions");
        }
        else
        {
            G3DDemo_Printf(0,3, G3DDEMO_COLOR_GREEN, "Use default BB Functions   ");
        }

        if (isScaled)
        {
            G3DDemo_Printf(0,5, G3DDEMO_COLOR_GREEN, "Billboards scaled    ");
        }
        else
        {
            G3DDemo_Printf(0,5, G3DDEMO_COLOR_GREEN, "Billboards not scaled");
        }

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06ld usec", OS_TicksToMicroSeconds(time));
    }
}


/*---------------------------------------------------------------------------*
    MySbcBBSimple
    
    Simplified billboard processing function. High speed, but has the following limitations.
    - No callback point.
    - opt must be NNS_G3D_SBCFLG_000.
    - No support for other than NNS_G3dGlbFlushP.
    - No support for NNS_G3D_RSFLAG_OPT_NOGECMD.
    - Does not display correctly when scaling is applied.
 *---------------------------------------------------------------------------*/
static void
MySbcBBSimple(NNSG3dRS* rs, u32 opt)
{
    //
    // Display a billboard parallel to the camera projection plane
    //
    static u32 bbcmd1[] =
    {
        ((G3OP_MTX_POP << 0)       |
         (G3OP_MTX_MODE << 8)      |
         (G3OP_MTX_LOAD_4x3 << 16)),
        1,
        GX_MTXMODE_POSITION_VECTOR,
        FX32_ONE, 0, 0, 
        0, FX32_ONE, 0,
        0, 0, FX32_ONE,
        0, 0, 0   // This area is subject to change (Trans)
    };
    
    VecFx32* trans = (VecFx32*)&bbcmd1[12];
    MtxFx44 m;

    NNS_G3D_NULL_ASSERT(rs);

    //
    // there is no callback point
    //

    //
    // cannot transfer data to and from the matrix stack
    //
#pragma unused(opt)
    NNS_G3D_ASSERT(opt == NNS_G3D_SBCFLG_000);

    //
    // NNS_G3dGlbFlushAlt is prohibited
    //
    NNS_G3D_ASSERT(!(NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_ALT));

    //
    // NNS_G3D_RSFLAG_OPT_NOGECMD also prohibited
    //
    NNS_G3D_ASSERT(!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD));

    // Flush the buffer
    NNS_G3dGeFlushBuffer();

    // Command transfer:
    // Change to PROJ mode
    // Save the projection matrix
    // Set the unit matrix
    reg_G3X_GXFIFO = ((G3OP_MTX_MODE << 0) |
                      (G3OP_MTX_PUSH << 8) |
                      (G3OP_MTX_IDENTITY << 16));
    reg_G3X_GXFIFO = (u32)GX_MTXMODE_PROJECTION;

    // Also, wait for the geometry engine to shut down
    // Get the current matrix
    while (G3X_GetClipMtx(&m))
        ;

    // Billboard matrix calculation
    trans->x = m._30;
    trans->y = m._31;
    trans->z = m._32;

    // Projection matrix POP
    // Restore to POS_VEC
    // Store in the current matrix
    MI_CpuSend32(&bbcmd1[0],
                 &reg_G3X_GXFIFO,
                 15 * sizeof(u32));

    rs->c += 2;
}


/*---------------------------------------------------------------------------*
    MySbcBBYSimple
    
    Simplified y-axis billboard processing function. High speed, but has the following limitations.
    - No callback point.
    - opt must be NNS_G3D_SBCFLG_000.
    - No support for other than NNS_G3dGlbFlushP.
    - No support for NNS_G3D_RSFLAG_OPT_NOGECMD.
    - Does not display correctly when scaling is applied.
 *---------------------------------------------------------------------------*/
static void
MySbcBBYSimple(NNSG3dRS* rs, u32 opt)
{
    MtxFx44 m;
    VecFx32 lz;

    static u32 bbcmd1[] =
    {
        ((G3OP_MTX_POP << 0)       |
         (G3OP_MTX_MODE << 8)      |
         (G3OP_MTX_LOAD_4x3 << 16)),
        1,
        GX_MTXMODE_POSITION_VECTOR,
        FX32_ONE, 0, 0, // This area is subject to change (4x3Mtx)
        0, FX32_ONE, 0,
        0, 0, FX32_ONE,
        0, 0, 0
    };
    VecFx32* trans = (VecFx32*)&bbcmd1[12];
    MtxFx43* mtx   = (MtxFx43*)&bbcmd1[3];

    NNS_G3D_NULL_ASSERT(rs);
    
    //
    // there is no callback point
    //

    //
    // cannot transfer data to and from the matrix stack
    //
#pragma unused(opt)
    NNS_G3D_ASSERT(opt == NNS_G3D_SBCFLG_000);

    //
    // NNS_G3dGlbFlushAlt is prohibited
    //
    NNS_G3D_ASSERT(!(NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_ALT));

    //
    // NNS_G3D_RSFLAG_OPT_NOGECMD also prohibited
    //
    NNS_G3D_ASSERT(!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD));

    // Flush the buffer
    NNS_G3dGeFlushBuffer();

    // Command transfer:
    // Change to PROJ mode
    // Save the projection matrix
    // Set the unit matrix
    reg_G3X_GXFIFO = ((G3OP_MTX_MODE << 0) |
                      (G3OP_MTX_PUSH << 8) |
                      (G3OP_MTX_IDENTITY << 16));
    reg_G3X_GXFIFO = (u32)GX_MTXMODE_PROJECTION;

    // Also, wait for the geometry engine to shut down
    // Get the current matrix (clip matrix)
    while (G3X_GetClipMtx(&m))
        ;

    // Billboard matrix calculation
    trans->x = m._30;
    trans->y = m._31;
    trans->z = m._32;

    // mtx_00, _01, and _02 are unchanged
    VEC_Normalize((VecFx32*)&m._10, (VecFx32*)&mtx->_10);

    lz.x = 0;
    lz.y = -m._12;
    lz.z = m._11;
    VEC_Normalize(&lz, (VecFx32*)&mtx->_20);

    // Projection matrix POP
    // Restore to POS_VEC
    // Store in the current matrix
    MI_CpuSend32(&bbcmd1[0],
                 &reg_G3X_GXFIFO,
                 15 * sizeof(u32));

    rs->c += 2;
}


/* -------------------------------------------------------------------------
  Name:         InitG3DLib

  Description:  Initializes G3D library.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
InitG3DLib(void)
{
    //
    // G3D:
    // Performs default initialization.
    // ------------------------------------------------------------------------
    NNS_G3dInit();
}


/* -------------------------------------------------------------------------
  Name:         LoadG3DModel

  Description:  Loads model data from file.

  Arguments:    path: Model data path name.

  Returns:      Pointer to model.
   ------------------------------------------------------------------------- */
static NNSG3dResMdl*
LoadG3DModel(const char* path)
{
    NNSG3dResMdl*        model   = NULL;
    NNSG3dResTex*        texture = NULL;
    NNSG3dResFileHeader* resFile = G3DDemo_LoadFile(path);
    BOOL status;
    NNS_G3D_NULL_ASSERT(resFile);

    // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
    // 
    // After a certain size, for large resources it is faster to just call DC_StoreAll.
    // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
    DC_StoreRange(resFile, resFile->fileSize);

    // Calls the default initialization function and performs setup.
    status = NNS_G3dResDefaultSetup(resFile);
    NNS_G3D_ASSERTMSG(status, "NNS_G3dResDefaultSetup failed");
    
    //
    // G3D: Gets model
    // Since nsbmd can include multiple models, gets the pointer to a single model by specifying its index. When there is only one model, use 0.
    // 
    // --------------------------------------------------------------------
    model = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFile), 0);
    return model;
}


/* -------------------------------------------------------------------------
  Name:         SetCamera

  Description:  Sets camera matrix and projection matrix in G3D global state.

  Arguments:    camera:     Pointer to the G3DDemoCamera structure.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
SetCamera(G3DDemoCamera* camera)
{
    G3DDemoLookAt* lookat = &camera->lookat;
    G3DDemoPersp*  persp  = &camera->persp;

    NNS_G3dGlbPerspective(persp->fovySin, persp->fovyCos, persp->aspect, persp->nearClip, persp->farClip);
    NNS_G3dGlbLookAt(&lookat->camPos, &lookat->camUp, &lookat->target);
}
