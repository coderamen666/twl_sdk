/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - callback3
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
// This is the third sample for using callbacks to customize G3D rendering operations.
//
// You can use NNS_G3dRenderObjSetCallBack to set the callback function that is run during rendering.
//  The NNSG3dRS structure, which is also a callback function argument,
// stores the renderer state during rendering.
// It is possible to customize G3D rendering operations by manipulating this structure.
// 
//
// Sample Operation
// Draws a running man and performs totally unrelated matrix calculations.
// Matrix calculations continue within the callback function as the display list is sent to the geometry engine.
// 
// 
// You can switch the calculations during the sending of the display list on and off with the A Button.
//
// HOWTO:
// 1: Set a callback function to the running man's rendering object.
// 2: The callback function is made to be called at NNS_G3D_SBC_CALLBACK_TIMING_C for NNS_G3D_SBC_SHP.
//    When called here, control switches to the callback function immediately after the display list is sent.
//    
// 3: While sending the display list, calculations unrelated to rendering can be performed inside the callback function.
//    
// 
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);

#define DUMMY_MTX_SIZE 60
static int idxMtx;
static MtxFx43 mtx;
static MtxFx43 mtxArray[DUMMY_MTX_SIZE];

/* -------------------------------------------------------------------------
  dummyCalc

  Perform other calculations while display list is being sent.
 ------------------------------------------------------------------------- */
static void
dummyCalc(NNSG3dRS* rs)
{
    #pragma unused(rs)

    while(NNS_G3dGeIsSendDLBusy() && idxMtx < DUMMY_MTX_SIZE)
    {
        MTX_Concat43(&mtx, &mtxArray[idxMtx], &mtxArray[idxMtx]);
        ++idxMtx;
    }
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
    NNSG3dResMdl*   pModel;
    void*           pAnmRes;
    NNSFndAllocator allocator;
    NNSG3dAnmObj*   pAnmObj;
    BOOL callBackOn;

    G3DDemo_InitSystem();
    G3DDemo_InitMemory();
    G3DDemo_InitVRAM();

    InitG3DLib();
    G3DDemo_InitDisplay();

    //
    // Create allocator with 4-byte alignment
    //
    NNS_FndInitAllocatorForExpHeap(&allocator, G3DDemo_AppHeap, 4);

    // Make the manager able to manage 4 slots'-worth of texture image slots, and set it as the default manager.
    // 
    NNS_GfdInitFrmTexVramManager(4, TRUE);

    // Make the manager able to manage 32 KB of palettes, and set it as the default manager.
    // 
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    pModel = LoadG3DModel("data/human_run_t.nsbmd");
    SDK_ASSERTMSG(pModel, "load failed");

    //
    // NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set to a value, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, pModel);
    
    //
    // Load the joint animation and try making associations.
    //
    {
        const u8* pFile = G3DDemo_LoadFile("data/human_run_t.nsbca");
        SDK_ASSERTMSG( pFile, "anm load failed" );
        {
            // Initialize animation, add it to rendering object
            // 
            // Allocate and initialize the animation object (NNSG3dAnmObj) that acts as an intermediary for the loaded animation file and the NNSG3dRenderObj structure.
            // 
            // All animations are set up with the same code.
            //

            //
            // Allocate from the allocator the memory necessary for the animation object.
            // You must specify and allocate model resources and animation resources.
            //

            // Specify the index #0 animation
            pAnmRes = NNS_G3dGetAnmByIdx(pFile, 0);
            SDK_NULL_ASSERT(pAnmRes);

            // Allocate the necessary amount of memory. Initialization must be done separately.
            pAnmObj = NNS_G3dAllocAnmObj(&allocator, // Specify the allocator to use
                                         pAnmRes,    // Specify the animation resource
                                         pModel);    // Specify the model resource
            SDK_NULL_ASSERT(pAnmObj);

            //
            // Initialize AnmObj. Same for others that are not joint animation
            //
            NNS_G3dAnmObjInit(pAnmObj, // Pointer to animation object
                              pAnmRes, // Pointer to animation resource
                              pModel,  // Pointer to NNSG3dResMdl
                              NULL );  // Pointer to NNSG3dResTex (NULL is OK except for texture pattern animations)

            //
            // Add AnmObj to RenderObj. Same for others that are not joint animation
            //
            NNS_G3dRenderObjAddAnmObj( &object, pAnmObj );

            //
            // Set the callback for the SBC's SHP command.
            //
            callBackOn = TRUE;
            NNS_G3dRenderObjSetCallBack(&object,
                                        &dummyCalc,
                                        NULL,
                                        NNS_G3D_SBC_SHP,
                                        NNS_G3D_SBC_CALLBACK_TIMING_C);
        }
    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "callback3");

    {
        int i;
        MTX_Identity43(&mtx);
        for (i = 0; i < DUMMY_MTX_SIZE; ++i)
        {
            MTX_Identity43(&mtxArray[i]);
        }
    }

    for(;;)
    {
        OSTick time;
        int cntMtxConcat;

        SVC_WaitVBlankIntr();

        G3DDemo_PrintApplyToHW();
        G3DDemo_ReadGamePad();

        G3DDemo_MoveCamera(&gCamera);
        G3DDemo_CalcCamera(&gCamera);

        G3X_Reset();
        G3X_ResetMtxStack();

        SetCamera(&gCamera);

        //
        // Update the animation frame.
        //
        pAnmObj->frame += FX32_ONE;
        if( pAnmObj->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj))
        {
            pAnmObj->frame = 0;
        }

        //
        // Press A Button to toggle between using the callback or not.
        //
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (callBackOn)
            {
                callBackOn = FALSE;
                NNS_G3dRenderObjResetCallBack(&object);
            }
            else
            {
                callBackOn = TRUE;
                NNS_G3dRenderObjSetCallBack(&object,
                                            &dummyCalc,
                                            NULL,
                                            NNS_G3D_SBC_SHP,
                                            NNS_G3D_SBC_CALLBACK_TIMING_C);
            }
        }

        if (callBackOn)
        {
            G3DDemo_Print(0,1, G3DDEMO_COLOR_GREEN, "CallBack ON ");
        }
        else
        {
            G3DDemo_Print(0,1, G3DDEMO_COLOR_GREEN, "CallBack OFF");
        }

        {
            VecFx32 scale = { FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2 };
            NNS_G3dGlbSetBaseScale(&scale);
            NNS_G3dGlbFlushP();
            idxMtx = 0;
            
            //  Start measuring the time
            time = OS_GetTick();
            {
                NNS_G3dDraw(&object);
            }

            cntMtxConcat = idxMtx;

            while(idxMtx < DUMMY_MTX_SIZE)
            {
                MTX_Concat43(&mtx, &mtxArray[idxMtx], &mtxArray[idxMtx]);
                ++idxMtx;
            }

            // Time measurement result
            time = OS_GetTick() - time;
        }

        {
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

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06ld usec", OS_TicksToMicroSeconds(time));
        G3DDemo_Printf(0, 2, G3DDEMO_COLOR_GREEN, "MTX_CONCAT count(NNS_G3dDraw)");
        G3DDemo_Printf(0, 3, G3DDEMO_COLOR_GREEN, "= %3d", cntMtxConcat);
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

   Arguments:    camera:        Pointer to the G3DDemoCamera structure.

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
