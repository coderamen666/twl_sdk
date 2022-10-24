/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - TexPatternAnm
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
// This sample plays a texture pattern animation.
//
// To play back a texture pattern animation, use an NSBTP file converted from an ITP file using g3dcvtr.
// 
//
// Sample Operation
// The texture pattern animation is played with looped repeat.
//
// HOWTO:
// 1: Load the .nsbtp file into main memory.
// 2: Using functions such as NNS_G3dGetAnmByIdx and NNS_G3dGetAnmByName, get a pointer to the texture pattern animation to be played back.
//    
// 3: Use NNS_G3dAllocAnmObj to get the region for NNSG3dAnmObj.
//    When deallocating, use NNS_G3dFreeAnmObj.
// 4: Use NNS_G3dAnmObjInit to initialize the acquired region.
// 5: Use NNS_G3dRenderObjAddAnmObj to associate it with an NNSG3dRenderObj.
//    You cannot associate to multiple NNSG3dRenderObj objects.
//    In such cases, a separate NNSG3dAnmObj has to be allocated (but the data within the NSBTP file can be shared).
//    
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void                InitG3DLib(void);
static BOOL 
LoadG3DModel(const char*            path,
             NNSG3dResMdl**         ppModel,
             NNSG3dResFileHeader**  ppResFile);

static void SetCamera(G3DDemoCamera* camera);


/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    NNSG3dRenderObj      object;
    NNSG3dResMdl*        pModel;
    NNSG3dResTex*        pTexture;
    NNSG3dResFileHeader* fileHeader;
    BOOL                 bResult;
    void*                pAnmRes;
    NNSFndAllocator      allocator;
    NNSG3dAnmObj*        pAnmObj;

    G3DDemo_InitSystem();
    G3DDemo_InitMemory();
    G3DDemo_InitVRAM();

    InitG3DLib();
    G3DDemo_InitDisplay();

    //
    // Create allocator with 4-byte alignment
    //
    NNS_FndInitAllocatorForExpHeap(&allocator, G3DDemo_AppHeap, 4);
    
    // Make the manager able to manage 4 slots' worth of texture image slots, and set it as the default manager.
    // 
    NNS_GfdInitFrmTexVramManager(4, TRUE);

    // Make the manager able to manage 32 KB of palettes, and set it as the default manager.
    // 
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    bResult = LoadG3DModel("data/itp_sample.nsbmd", &pModel, &fileHeader );
    SDK_ASSERTMSG( bResult, "load failed");
    
    //
    // Get texture data.
    //
    pTexture = NNS_G3dGetTex(fileHeader);
    
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
    // Read texture pattern/animation and associate.
    //
    {
        //
        // Loads file
        //
        const u8* pFile = G3DDemo_LoadFile("data/itp_sample.nsbtp");
        SDK_ASSERTMSG( pFile, "anm load failed" );
        {
            // Initialize animation, add it to rendering object
            // 
            // Allocate and initialize the loaded animation file and the animation object (NNSG3dAnmObj) that acts as an intermediary for the NNSG3dRenderObj structure.
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
            // Initialize AnmObj.
            //
            NNS_G3dAnmObjInit(pAnmObj,    // Pointer to animation object
                              pAnmRes,    // Pointer to animation resource
                              pModel,     // Pointer to NNSG3dResMdl
                              pTexture ); // Pointer to NNSG3dResTex (NULL is OK except for texture pattern animations)

            //
            // Add AnmObj to RenderObj.
            //
            NNS_G3dRenderObjAddAnmObj( &object, pAnmObj );
        }
    }

    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 5*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

	G3DDemo_InitConsole();
	G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "TEX PATTERN ANM");

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

        // Update the animation frame.
        pAnmObj->frame += FX32_ONE;
        if( pAnmObj->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj))
        {
            pAnmObj->frame = 0;
        }

        {
            // adjust position and size
            VecFx32 scale = {FX32_ONE << 1, FX32_ONE << 1, FX32_ONE << 1};
            VecFx32 trans = {0, FX32_ONE * 5, 0};
            NNS_G3dGlbSetBaseScale(&scale);
            NNS_G3dGlbSetBaseTrans(&trans);

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
                //
                NNS_G3dDraw(&object);
            }
            // Time measurement result
            time = OS_GetTick() - time;
        }

        {
            // Restore scale and trans to their original state (for drawing the ground mesh)
            VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
            VecFx32 trans = {0, 0, 0};
            NNS_G3dGlbSetBaseScale(&scale);
            NNS_G3dGlbSetBaseTrans(&trans);

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

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06d usec", OS_TicksToMicroSeconds(time));
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

  Arguments:    path:       Model data path name.
                ppModel:    pointer to pointer to model
                ppResFile: pointer to pointer to file of data to load
                
  Returns:      Success/failure of load
   ------------------------------------------------------------------------- */
static BOOL 
LoadG3DModel(const char* path, NNSG3dResMdl** ppModel, NNSG3dResFileHeader** ppResFile )
{
    *ppResFile = G3DDemo_LoadFile(path);

    if(*ppResFile != NULL)
    {
        BOOL    status;

        // To use DMA transfers inside functions such as NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
        // 
        // After a certain size, for large resources it is faster to just call DC_StoreAll.
        // For information on DC_StoreRange and DC_StoreAll, see the NITRO-SDK Function Reference Manual.
        DC_StoreRange(*ppResFile, (*ppResFile)->fileSize);

        // For cases when the default initialization function is called to perform setup
        status = NNS_G3dResDefaultSetup(*ppResFile);
        NNS_G3D_ASSERTMSG(status, "NNS_G3dResDefaultSetup failed");

        //
        // G3D: Gets model
        // Because nsbmd can include multiple models, get the pointer to a single model by specifying its index. When there is only one model, use 0 for the index.
        // 
        // --------------------------------------------------------------------
        *ppModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(*ppResFile), 0);
        
        return TRUE;
    }

    return FALSE;
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
