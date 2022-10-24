/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - ProjMap
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
// This sample displays projection-mapped objects.
//
// When the .imd file's <material> element's tex_gen_mode is set to pos, texture mapping will use a projection map.
// 
//
// Sample Operation
// In the sample, the texture will use orthogonal projection for xy coordinates ranging from (-1.0, -1.0) to (1.0, 1.0) (when effect_mtx is the unit matrix).
// 
// Objects are continually moving along the x-axis from -1.0 to 1.0.
//
// Pressing the A Button --> Switches the method for setting the projection matrix prior to rendering.
//                     The texture gets mapped the same way no matter which method is used.
// Pressing the X Button --> Environment map rotates around the x-axis (in the View coordinate system).
// Pressing the Y Button --> Environment map rotates around the y-axis (in the View coordinate system).
// Pressing the B Button --> Environment map rotates around the z-axis (in the View coordinate system).
//
//---------------------------------------------------------------------------


#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void             InitG3DLib(void);
static NNSG3dResMdl*    LoadG3DModel(const char* path);
static void             SetCamera(G3DDemoCamera* camera);


/*-------------------------------------------------------------------------
    rotEnvMap

    Callback function that performs the process of changing the direction of the environment map.
  -------------------------------------------------------------------------*/
static void
rotProjMap(NNSG3dRS* rs)
{
    static MtxFx33 mRot = {
        FX32_ONE, 0, 0,
        0, FX32_ONE, 0,
        0, 0, FX32_ONE
    };
    MtxFx33 m;

    if (G3DDEMO_IS_PAD_PRESS(PAD_BUTTON_B))
    {
        MTX_RotZ33(&m, FX_SinIdx(256), FX_CosIdx(256));
        MTX_Concat33(&m, &mRot, &mRot);
    }

    if (G3DDEMO_IS_PAD_PRESS(PAD_BUTTON_Y))
    {
        MTX_RotY33(&m, FX_SinIdx(256), FX_CosIdx(256));
        MTX_Concat33(&m, &mRot, &mRot);
    }

    if (G3DDEMO_IS_PAD_PRESS(PAD_BUTTON_X))
    {
        MTX_RotX33(&m, FX_SinIdx(256), FX_CosIdx(256));
        MTX_Concat33(&m, &mRot, &mRot);
    }

    // By creating and combining effect_mtx on your own, you can change the direction of the projected texture.
    NNS_G3dGeMultMtx33(&mRot);

    // Set the flag because we are replacing the created part of effect_mtx.
    // By doing this, you can skip the internal library processes that would otherwise be done between Timing B and Timing C.
    rs->flag |= NNS_G3D_RSFLAG_SKIP;
}


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
    NNSG3dResMdl*    model;
    int state = 0;
    u16 sincos = 0;
    fx32 X = -FX32_ONE;

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

    model = LoadG3DModel("data/sphere_proj.nsbmd");
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

    G3DDemo_InitCamera(&gCamera, 0, 5*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "ProjMap");

    //
    // When customizing effect_mtx settings, insert callback processing using timing B for NNS_G3D_SBC_ENVMAP.
    // 
    // 
    //
    NNS_G3dRenderObjSetCallBack(&object,
                                &rotProjMap,
                                NULL,
                                NNS_G3D_SBC_PRJMAP,
                                NNS_G3D_SBC_CALLBACK_TIMING_B);

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
            //
            // The objects are continually rotating
            //
            MtxFx33 mRot;
            VecFx32 mTrans;
            sincos += 256;

            mTrans.x = X;
            mTrans.y = 0;
            mTrans.z = 0;
            
            MTX_RotX33(&mRot, FX_SinIdx(sincos), FX_CosIdx(sincos));
            NNS_G3dGlbSetBaseRot(&mRot);
            NNS_G3dGlbSetBaseTrans(&mTrans);

            X += FX32_ONE >> 8;
            if (X >= FX32_ONE)
                X = -FX32_ONE;
            
            //
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
            {
                state = (state + 1) % 3;
            }

            //
            // Pressing the A Button switches the way the projection matrix gets set up.
            // (The picture that displays is the same.)
            //
            switch(state)
            {
            case 0:
                NNS_G3dGlbFlushP();
                break;
            case 1:
                NNS_G3dGlbFlushVP();
                break;
            case 2:
                NNS_G3dGlbFlushWVP();
                break;
            };

            time = OS_GetTick();

            {
                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                //
                NNS_G3dDraw(&object);
            }

            // Time measurement
            time = OS_GetTick() - time;
        }

        {
            // Restore rotation to its original state.
            MtxFx33 mRot;
            VecFx32 mTrans = {0,0,0};
            MTX_Identity33(&mRot);
            NNS_G3dGlbSetBaseRot(&mRot);
            NNS_G3dGlbSetBaseTrans(&mTrans);

            //
            // G3D: Normally, when any of the G3_XXX functions are called, you have to call the NNS_G3dGeComFlushBuffer function before that and synchronize.
            //      
            //
            NNS_G3dGlbFlush();
            NNS_G3dGeFlushBuffer();
            G3DDemo_DrawGround(&gGround);
        }

        /* Swap memory relating to geometry and rendering engine */
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06ld usec", OS_TicksToMicroSeconds(time));
        switch(state)
        {
        case 0:
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_YELLOW, "NNS_G3dGlbFlushP  ");
            break;
        case 1:
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_YELLOW, "NNS_G3dGlbFlushVP ");
            break;
        case 2:
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_YELLOW, "NNS_G3dGlbFlushWVP");
            break;
        };
    }
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
    SDK_NULL_ASSERT(resFile);

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

  Arguments:    camera:		Pointer to the G3DDemoCamera structure.

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
