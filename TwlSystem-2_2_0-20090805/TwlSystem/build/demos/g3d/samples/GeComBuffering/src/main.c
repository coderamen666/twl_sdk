/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - GeComBuffering
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
// This sample uses the geometry command buffering feature.
//
// Normally, when you want to issue a geometry command, a block is needed until the display list has been sent. By buffering the geometry commands to a separate location, however, that block can be avoided and you can transition to the next process.
// 
// 
// In G3D, by using the NNS_G3dGeSetBuffer function to enable the buffering of geometry commands issued as the display list is being sent, you can avoid blocking the issuing of geometry commands while the display list is in DMA transfer.
// 
// 
// 
// This is effective for models with large display lists.
//
// Sample Operation
// With the A Button, you can enable and disable buffering for issuing geometry command.
// 
//
// HOWTO:
// 1: Allocate NNSG3dGeBuffer from the stack or heap.
// 2: Call the NNS_G3dGeSetBuffer(NNSG3dGeBuffer*) function to enable geometry command buffering.
//    
// 
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);

static int dummyVal = 0;

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
    NNSG3dGeBuffer  geBuf;
    BOOL geBufOn;
    
    {
        ///////////////////////////////////////////////////////////////////////
        //
        // Calling the NNS_G3dGeSetBuffer function will begin buffering geometry commands when the display list is being sent.
        // 
        // 
        //
        NNS_G3dGeSetBuffer(&geBuf);
        geBufOn = TRUE;
    }

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
        }
    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "GeComBuffering");

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

        //
        // The A Button toggles between using geometry command buffer or not.
        //
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (geBufOn)
            {
                geBufOn = FALSE;
                (void)NNS_G3dGeReleaseBuffer();
            }
            else
            {
                geBufOn = TRUE;
                NNS_G3dGeSetBuffer(&geBuf);
            }
        }

        if (geBufOn)
        {
            G3DDemo_Print(0,1, G3DDEMO_COLOR_GREEN, "GeComBuffering ON ");
        }
        else
        {
            G3DDemo_Print(0,1, G3DDEMO_COLOR_GREEN, "GeComBuffering OFF");
        }

        {
            VecFx32 scale = { FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2 };
            NNS_G3dGlbSetBaseScale(&scale);
            NNS_G3dGlbFlushP();
            
            //  Start measuring the time
            time = OS_GetTick();
            {
                NNS_G3dDraw(&object);
            }

            {
                // This is meant to be somewhat intensive processing
                int i;
                for (i = 0; i < 10000; ++i)
                    dummyVal += i;
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
