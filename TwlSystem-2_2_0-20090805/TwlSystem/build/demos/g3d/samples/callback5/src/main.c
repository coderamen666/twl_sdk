/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - callback5
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
// This is the fifth sample to use callbacks to customize G3D rendering operations.
//
// By using the NNS_G3dRenderObjSetInitFunc function, a user-defined function is called before rendering, and the registering of multiple callbacks within that function can be recorded.
// 
//
// Sample Operation
// Multiple callback functions are used to customize the rendering of a robot.
// When the A Button is pressed, the processes executed by pressing the A, X and Y Buttons with the callback1 sample are executed at one time.
//                     
//
// HOWTO:
// 1: Use the NNS_G3dRenderObjSetInitFunc function to register the executable functions to NNSG3dRenderObj before rendering.
//    
// 2: The NNS_G3dRSSetCallBack function is used within the registered function to register the necessary multiple callback functions to the NNSG3dRS structure.
//    
//
//---------------------------------------------------------------------------


#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void             InitG3DLib(void);
static NNSG3dResMdl*    LoadG3DModel(const char* path);
static void             SetCamera(G3DDemoCamera* camera);




/* -------------------------------------------------------------------------
    eraseWaist

    This callback is put in the SBC's NODE command (the NNSi_G3dFuncSbc_NODE function).
    If callbacks are registered at timing A or B using the NODE command, you can create and change the results of the visibility calculation.
    
    Can be referenced from rs->pVisAnmResult.

    When called, will not display any rendering nodes named "chest".
 ------------------------------------------------------------------------- */
static void
eraseWaist(NNSG3dRS* rs)
{
    int jntID;
    NNS_G3D_GET_JNTID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &jntID,
                      "chest");
    SDK_ASSERT(jntID >= 0);

    if (NNS_G3dRSGetCurrentNodeID(rs) == jntID)
    {
        NNSG3dVisAnmResult* visResult = NNS_G3dRSGetVisAnmResult(rs);
        visResult->isVisible = FALSE;
        //
        // If there is no longer any need to use callbacks, a reset might have less negative impact on performance.
        // 
        //
        NNS_G3dRSResetCallBack(rs, NNS_G3D_SBC_NODE);
    }
}


/* -------------------------------------------------------------------------
    getTranslucent

    This callback is put in the SBC's MAT command (the NNSi_G3dFuncSbc_MAT function).
    If callbacks are registered at timing A or B using the MAT command, you can create and change the results of the material calculation.
    
    Can be referenced from rs->pMatAnmResult.

    If, when called, the material name is lambert2, it is displayed as translucent. Otherwise it is displayed as wireframe.
    
 ------------------------------------------------------------------------- */
static void
getTranslucent(NNSG3dRS* rs)
{
    int matID;
    NNSG3dMatAnmResult* matResult;
    NNS_G3D_GET_MATID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &matID,
                      "lambert2");
    SDK_ASSERT(matID >= 0);

    matResult = NNS_G3dRSGetMatAnmResult(rs);

    if (NNS_G3dRSGetCurrentMatID(rs) == matID)
    {
        matResult->prmPolygonAttr =
            (matResult->prmPolygonAttr & ~REG_G3_POLYGON_ATTR_ALPHA_MASK) |
            (16 << REG_G3_POLYGON_ATTR_ALPHA_SHIFT);
    }
    else
    {
        matResult->flag |= NNS_G3D_MATANM_RESULTFLAG_WIREFRAME;
    }
}


/* -------------------------------------------------------------------------
    scaleArm

    This callback is put in the SBC's NODEDESC command (the NNSi_G3dFuncSbc_NODEDESC function).
    If callbacks are registered at timing A or B using the NODEDESC command, you can create and change the results of the joint calculation.
    
    Can be referenced from rs->pJntAnmResult.

    The first argument of the NODEDESC command is the joint ID calculated by the NODEDESC command.
    The scale is doubled only when calculating left_upper_arm.
 ------------------------------------------------------------------------- */
static void
scaleArm(NNSG3dRS* rs)
{
    int jntID;
    
    NNS_G3D_GET_JNTID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &jntID,
                      "left_upper_arm");
    SDK_ASSERT(jntID >= 0);

    if (NNS_G3dRSGetCurrentNodeDescID(rs) == jntID)
    {
        NNSG3dJntAnmResult* jntResult;
        jntResult = NNS_G3dRSGetJntAnmResult(rs);

        jntResult->flag &= ~NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE;
        jntResult->scale.x = 2 * FX32_ONE;
        jntResult->scale.y = 2 * FX32_ONE;
        jntResult->scale.z = 2 * FX32_ONE;

        //
        // If there is no longer any need to use callbacks, a reset might have less negative impact on performance.
        // 
        //
        NNS_G3dRSResetCallBack(rs, NNS_G3D_SBC_NODEDESC);
    }
}


static void
setCallback(NNSG3dRS* rs)
{
    NNS_G3dRSSetCallBack(rs, &scaleArm, NNS_G3D_SBC_NODEDESC, NNS_G3D_SBC_CALLBACK_TIMING_B);
    NNS_G3dRSSetCallBack(rs, &getTranslucent, NNS_G3D_SBC_MAT, NNS_G3D_SBC_CALLBACK_TIMING_B);
    NNS_G3dRSSetCallBack(rs, &eraseWaist, NNS_G3D_SBC_NODE, NNS_G3D_SBC_CALLBACK_TIMING_B);
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

    model = LoadG3DModel("data/robot_t.nsbmd");
    SDK_ASSERTMSG(model, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, model);

    G3DDemo_InitCamera(&gCamera, 0, 5*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "callback5");

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
            VecFx32 scale = {FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2};
            NNS_G3dGlbSetBaseScale(&scale);

            //
            // The callback function to be configured, the SBC command to insert callbacks and the command-internal timing can be set with the different buttons pressed.
            // 
            //
            if (G3DDEMO_IS_PAD_PRESS(PAD_BUTTON_A))
            {
                NNS_G3dRenderObjSetInitFunc(&object, &setCallback);
            }
            else
            {
                // Reset callback settings if no button is pressed
                NNS_G3dRenderObjSetInitFunc(&object, NULL);
            }

            //
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlushP();


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
            // Restore scale to original
            VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
            NNS_G3dGlbSetBaseScale(&scale);

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

  Arguments:    path : Model data path name.

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

    // Calls the default initialization function and performs setup
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
