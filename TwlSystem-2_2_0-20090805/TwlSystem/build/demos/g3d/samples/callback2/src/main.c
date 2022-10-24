/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - callback2
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
// This is the second sample to use callbacks to customize G3D rendering operations.
//
// You can use the NNS_G3dRenderObjSetCallBack function to set the callback function run at rendering.
//  The NNSG3dRS structure, which is also a callback argument,
// stores the renderer state during rendering.
// It is possible to customize G3D rendering operations by manipulating this structure.
// 
// 
// Sample Operation
// A person is running while holding a walking robot in their left hand.
//
// HOWTO:
// 1: Set a callback function to the running man's rendering object.
// 2: The callback function is made to be called at NNS_G3D_SBC_CALLBACK_TIMING_C for NNS_G3D_SBC_NODEDESC.
//    When called here, the current matrix is configured as the rendering matrix.
//    
// 3: The callback function determines whether the left hand's matrix is being calculated and saves the current matrix to the open area of the matrix stack.
//    
// 4: After rendering the running person and before rendering the robot, restore the saved matrix to the current matrix.
//    
// 5: When the robot is rendered with the NNS_G3dDraw function, the running person's left hand will be holding the robot (in other words, the robot will be stuck to the person's hand).
//    
// 
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);


//
// Matrix stack index for temporary save.
//
#define SAVE_MTX_IDX 30


/* -------------------------------------------------------------------------
    storeJntMtx

    Save the left hand matrix in an unused location of the matrix stack.
 ------------------------------------------------------------------------- */
static void
storeJntMtx(NNSG3dRS* rs)
{
    //
    // Called at the NODEDESC command's callback C.
    //
    int jntID;
    NNS_G3D_GET_JNTID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &jntID,
                      "wrist_l_end");
    SDK_ASSERT(jntID >= 0);

    if (NNS_G3dRSGetCurrentNodeDescID(rs) == jntID)
    {
        // Process wrist_l_end
        fx32 posScale = NNS_G3dRSGetPosScale(rs);
        fx32 invPosScale = NNS_G3dRSGetInvPosScale(rs);

        //
        // Confirm that the save destination stack Idx is not used when model is rendered
        //
        SDK_ASSERTMSG(SAVE_MTX_IDX >= NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs))->info.firstUnusedMtxStackID,
                      "SAVE_MTX_IDX will be destroyed");

        //
        // Because posScale scaling only occurs right before rendering, you must perform explicit scaling.
        // 
        //
        NNS_G3dGeScale(posScale, posScale, posScale);

        //
        // Save the current matrix to a location where nothing else will get stored during rendering
        //
        NNS_G3dGeStoreMtx(SAVE_MTX_IDX);

        //
        // Revert scaling, since the current matrix might be used for rendering.
        // 
        //
        NNS_G3dGeScale(invPosScale, invPosScale, invPosScale);

        //
        // If there is no longer any need to use callbacks, a reset might have less negative impact on performance.
        // 
        //
        NNS_G3dRSResetCallBack(rs, NNS_G3D_SBC_NODEDESC);
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
    NNSG3dRenderObj object2;
    NNSG3dResMdl*   pModel;
    void*           pAnmRes;
    NNSG3dResMdl*   pModel2;
    void*           pAnmRes2;
    NNSFndAllocator allocator;
    NNSG3dAnmObj*   pAnmObj;
    NNSG3dAnmObj*   pAnmObj2;

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

    pModel2 = LoadG3DModel("data/robot_t.nsbmd");
    SDK_ASSERTMSG(pModel2, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, pModel);
    NNS_G3dRenderObjInit(&object2, pModel2);
    
    //
    // Load the joint animation and try making associations.
    //
    {
        //
        // Running person
        //
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
            // From the allocator, allocate the memory necessary for the animation object.
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
            // Set the callback
            //
            NNS_G3dRenderObjSetCallBack(&object,
                                        &storeJntMtx,
                                        NULL,
                                        NNS_G3D_SBC_NODEDESC,
                                        NNS_G3D_SBC_CALLBACK_TIMING_C);
        }
    }

    {
        //
        // Walking robot
        //
        const u8* pFile = G3DDemo_LoadFile("data/robot_t.nsbca");
        SDK_ASSERTMSG( pFile, "anm load failed" );

        pAnmRes2 = NNS_G3dGetAnmByIdx(pFile, 0);
        SDK_NULL_ASSERT(pAnmRes2);

        pAnmObj2 = NNS_G3dAllocAnmObj(&allocator,
                                      pAnmRes2,
                                      pModel2);
        SDK_NULL_ASSERT(pAnmObj2);

        NNS_G3dAnmObjInit(pAnmObj2,
                          pAnmRes2,
                          pModel2,
                          NULL);

        NNS_G3dRenderObjAddAnmObj(&object2, pAnmObj2);
    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "callback2");

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

        //
        // Update the animation frame.
        //

        // Slow down the animation of the person in order to make the robot easy to see.
        pAnmObj->frame += (FX32_ONE >> 1);
        if( pAnmObj->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj))
        {
            pAnmObj->frame = 0;
        }

        // Speed up robot animation so you can see that the robot is moving.
        pAnmObj2->frame += FX32_ONE * 2;
        if ( pAnmObj2->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj2))
        {
            pAnmObj2->frame = 0;
        }

        //  Start measuring the time
        time = OS_GetTick();

        {
            //
            // Render the running person
            //

            VecFx32 scale = { FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2 };
            NNS_G3dGlbSetBaseScale(&scale);
            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlushP();

            {
                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                //
                NNS_G3dDraw(&object);
            }

        }
 
        {
            //
            // Render the walking robot held in person's left hand
            //

            // In this case, no need to call NNS_G3dGlbFlushP

            //
            // To have the person hold (attach) the robot in his left hand, the left hand matrix that was saved by the callback during the person's rendering is restored as the current matrix.
            // 
            // 
            //
            NNS_G3dGeRestoreMtx(SAVE_MTX_IDX);
            NNS_G3dDraw(&object2);
        }
        
        
        // Time measurement result
        time = OS_GetTick() - time;

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

        {
            static s64 total_time = 0;
            static u32 exec_cnt = 0;

            total_time += OS_TicksToMicroSeconds(time);
            exec_cnt++;
            G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%f usec", (f32)(total_time / exec_cnt));
        }
        
//        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06ld usec", OS_TicksToMicroSeconds(time));
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
