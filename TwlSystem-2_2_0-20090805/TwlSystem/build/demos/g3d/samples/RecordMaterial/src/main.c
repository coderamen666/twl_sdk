/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - RecordMaterial
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
// This sample pre-calculates a material's calculation results.
//
// Buffering a material's calculation results can save CPU calculation costs when a material doesn't change.
// 
//
// Sample Operation
// The joint animation is played with looped repeat.
// Press the A Button to switch between using the buffered material calculation result or to recalculate for each rendering.
// 
//
// HOWTO:
// 1: Secure the buffer region with NNS_G3dAllocRecBufferMat.
// 2: Attach the buffer to the rendering object with the NNS_G3dRenderObjSetMatAnmBuffer function.
//    
// 3: Enable NNS_G3D_RENDEROBJ_FLAG_RECORD with NNS_G3dRenderObjSetFlag.
//    When you call NNS_G3dDraw with this flag set, the calculation results are stored in the buffer.
// 4: In addition, no data is sent to the geometry engine when NNS_G3dDraw is run when the NNS_G3D_RENDEROBJ_FLAG_NOGECMD flag is set.
//    
// 5: Execute NNS_G3dDraw and store the data in the buffer.
// 6: The NNS_G3D_RENDEROBJ_FLAG_RECORD and (NNS_G3D_RENDEROBJ_FLAG_NOGECMD) flags are reset with the NNS_G3dRenderObjResetFlag function.
//    
// 7: Beginning with the next NNS_G3dDraw, instead of calculating the material data, reference the buffer data and render.
// 8: You can release the buffer with NNS_G3dRenderObjReleaseMatAnmBuffer.
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);


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
    NNSG3dMatAnmResult* pBufMatAnmResult;
    BOOL useRecord = TRUE;

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

    pModel = LoadG3DModel("data/envelope_simple.nsbmd");
    SDK_ASSERTMSG(pModel, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
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
        const u8* pFile = G3DDemo_LoadFile("data/envelope_simple.nsbca");
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
        }

        {
            //
            // Allocate the material calculation result buffer and attach it to the rendering object.
            //
            pBufMatAnmResult = NNS_G3dAllocRecBufferMat(&allocator, pModel);
            NNS_G3dRenderObjSetMatAnmBuffer(&object, pBufMatAnmResult);

            //
            // Record the material calculation results in the buffer.
            //
            NNS_G3dRenderObjSetFlag(&object, NNS_G3D_RENDEROBJ_FLAG_NOGECMD); // do not send to geometry engine
            NNS_G3dRenderObjSetFlag(&object, NNS_G3D_RENDEROBJ_FLAG_RECORD);  // record data in pBufMatAnmResult

            NNS_G3dDraw(&object);

            NNS_G3dRenderObjResetFlag(&object, NNS_G3D_RENDEROBJ_FLAG_NOGECMD); // send to geometry engine
            NNS_G3dRenderObjResetFlag(&object, NNS_G3D_RENDEROBJ_FLAG_RECORD);  // use the pBufMatAnmResult data
        }

    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 2*FX32_ONE, 10*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "RecordMaterial");

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
        pAnmObj->frame += FX32_ONE;
        if( pAnmObj->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj))
        {
            pAnmObj->frame = 0;
        }

        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (useRecord)
            {
                useRecord = FALSE;
                (void)NNS_G3dRenderObjReleaseMatAnmBuffer(&object);
            }
            else
            {
                useRecord = TRUE;
                NNS_G3dRenderObjSetMatAnmBuffer(&object, pBufMatAnmResult);
            }
        }

        if (useRecord)
        {
            G3DDemo_Printf(0, 2, G3DDEMO_COLOR_GREEN, "use recorded material data    ");
        }
        else
        {
            G3DDemo_Printf(0, 2, G3DDEMO_COLOR_GREEN, "calc material data every frame");
        }

        {
            VecFx32 scale = { FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2 };
            NNS_G3dGlbSetBaseScale(&scale);
            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlushP();

            //  Start measuring the time
            time = OS_GetTick();
            //
            // G3D: NNS_G3dDraw
            // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
            // 
            //
            NNS_G3dDraw(&object);

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

        //
        // Time measurement results screen display.
        //
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
