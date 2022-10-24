/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - ShadowVolume
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
// Shadow Volume Display Sample
//
// The following is an example of a method of rendering shadow volumes using G3D.
//
// Sample Operation
// Shadow volume can be used to cast shadows for objects repeatedly playing visibility animations.
// 
//
// HOWTO:
// 1: Confirm that manual sort mode has been designated with G3_SwapBuffers.
// 2: Set and render the shadow polygon material for the mask for the same models, and then set and render the shadow polygon material for the rendering.
//    
//    For further details, see the comments in the DrawShadowVolume function in the sample.
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);


/* -------------------------------------------------------------------------
    DrawShadowVolume

    Renders the shadow volumes.
 ------------------------------------------------------------------------- */
static void
DrawShadowVolume(NNSG3dResMdl* p, BOOL isShadow)
{
    //
    // Shadow volume rendering
    //
    // To begin with, if rendering even one shadow volume, you must use manual sort mode.
    // 
    // Specifically, use either G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W) or G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z).
    // 
    // 
    //
    // You can cast a shadow for an object by setting and rendering the shadow polygon material for masking for one model, and then setting and rendering the shadow polygon material for rendering for that same model.
    // 
    // 
    // Bear in mind that the model used for the shadow volumes must pass through the objects on which the shadow is cast.
    //
    {
        // Position adjustment
        VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
        VecFx32 trans = {0, 5 * FX32_ONE, 0};
        NNS_G3dGlbSetBaseScale(&scale);
        NNS_G3dGlbSetBaseTrans(&trans);
        NNS_G3dGlbFlushP();
        trans.y = 0;
        NNS_G3dGlbSetBaseTrans(&trans);
    }

    if (isShadow)
    {
        {
            //
            // Renders the shadow polygons for masking.
            // Set the polygon ID to 0, the alpha to a value between 1 and 30, and the polygon mode to GX_POLYGONMODE_SHADOW and then render just the rear surface (GX_CULL_FRONT).
            // 
            //
            NNS_G3dMdlSetMdlLightEnableFlag( p, 0, 0 );
            NNS_G3dMdlSetMdlPolygonID( p, 0, 0 );                       // Set the polygon ID to 0
            NNS_G3dMdlSetMdlCullMode( p, 0, GX_CULL_FRONT );            // Render only the back surfaces
            NNS_G3dMdlSetMdlAlpha( p, 0, 10 );
            NNS_G3dMdlSetMdlPolygonMode( p, 0, GX_POLYGONMODE_SHADOW ); // Use shadow polygon

            // render matID=0, shpID=0 (do not omit sending the material)
            NNS_G3dDraw1Mat1Shp(p, 0, 0, TRUE);
        }

        {
            //
            // Render the shadow polygons for rendering in the same location.
            // Set the polygon ID to a value between 1 and 63, the alpha to a value between 1 and 30, and the polygon mode to GX_POLYGONMODE_SHADOW and then render both surfaces (GX_CULL_NONE).
            // 
            //
            // If GX_CULL_BACK is used, there are often cases where rendering is incorrect because the masking shadow polygon goes out of bounds.
            // 
            //
            // Make the polygon ID is different from those of the objects on which the shadow is cast.
            // If the polygon IDs are the same, the shadow will not be rendered.
            // When you don't want a shadow to fall on the object casting the shadow, give the object casting the shadow and the rendering shadow polygon the same polygon ID.
            // 
            //
            NNS_G3dMdlSetMdlLightEnableFlag( p, 0, 0 );
            NNS_G3dMdlSetMdlPolygonID( p, 0, 1 );                       // Set polygon ID to 1-63
            NNS_G3dMdlSetMdlCullMode( p, 0, GX_CULL_NONE );             // Render both surfaces
            NNS_G3dMdlSetMdlAlpha( p, 0, 10 );
            NNS_G3dMdlSetMdlPolygonMode( p, 0, GX_POLYGONMODE_SHADOW ); // Use shadow polygon

            // Render matID=0, shpID=0 (do not omit sending the material)
            NNS_G3dDraw1Mat1Shp(p, 0, 0, TRUE);
        }
    }
    else
    {
        //
        // Render not as a shadow volume but as a normal translucent polygon.
        // The model being used in this sample is cylindrical.
        // 
        NNS_G3dMdlSetMdlLightEnableFlag( p, 0, 0 );
        NNS_G3dMdlSetMdlPolygonID( p, 0, 0 );
        NNS_G3dMdlSetMdlCullMode( p, 0, GX_CULL_BACK ); 
        NNS_G3dMdlSetMdlAlpha( p, 0, 10 );
        NNS_G3dMdlSetMdlPolygonMode( p, 0, GX_POLYGONMODE_MODULATE );

        // render matID=0, shpID=0 (do not omit sending the material)
        NNS_G3dDraw1Mat1Shp(p, 0, 0, TRUE);
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
    NNSFndAllocator allocator;
    
    NNSG3dRenderObj object;
    NNSG3dResMdl*   pModel;
    void*           pAnmRes;
    NNSG3dAnmObj*   pAnmObj;
    BOOL isShadow = TRUE;

    //
    // A shadow volume normally comprises one material and one shape so it can be rendered with the NNS_G3dDraw1Mat1Shp function.
    // As a result, since NNSG3dRenderObj is not needed, only NNSG3dResMdl is needed for setup.
    // 
    //
    NNSG3dResMdl*   pShadow;

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

    pModel = LoadG3DModel("data/brother.nsbmd");
    SDK_ASSERTMSG(pModel, "load failed");

    pShadow = LoadG3DModel("data/shadow.nsbmd");
    SDK_ASSERTMSG(pShadow, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, pModel);
    
    //
    // Load visibility animation and associate it.
    //
    {
        const u8* pFile = G3DDemo_LoadFile("data/brother.nsbva");
        SDK_ASSERTMSG( pFile, "anm load failed" );
        {
            //
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
            // Initialize AnmObj.
            //
            NNS_G3dAnmObjInit(pAnmObj, // Pointer to animation object
                              pAnmRes, // Pointer to animation resource
                              pModel,  // Pointer to NNSG3dResMdl
                              NULL );  // Pointer to NNSG3dResTex (NULL is OK except for texture pattern animations)

            //
            // Add AnmObj to RenderObj.
            //
            NNS_G3dRenderObjAddAnmObj( &object, pAnmObj );
        }
    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 2*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "ShadowVolume");

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

        if (pAnmObj->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObj))
        {
            pAnmObj->frame = 0;
        }

        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            isShadow = isShadow ? FALSE : TRUE;
        }

        time = OS_GetTick();
        {
            {
                // Apply scaling and enlarge
                VecFx32 scale = {FX32_ONE << 1, FX32_ONE << 1 , FX32_ONE << 1};
                NNS_G3dGlbSetBaseScale(&scale);

                //
                // G3D:
                // Collects and transfers the states set with G3dGlbXXXX.
                // Camera and projection matrix, and the like, are set.
                //
                NNS_G3dGlbFlushP();

                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                //
                NNS_G3dDraw(&object);
            }

            DrawShadowVolume(pShadow, isShadow);
        }
        time = OS_GetTick() - time;

        {
            //
            // G3D: Normally, when any of the G3_XXX functions are called, you have to call the NNS_G3dGeComFlushBuffer function before that and synchronize.
            //      
            //
            NNS_G3dGlbFlushP();
            NNS_G3dGeFlushBuffer();
            G3DDemo_DrawGround(&gGround);
        }

        //
        // NOTICE:
        // When using shadow polygons, you must use manual sort mode.
        // When set to auto-sort, the user cannot set the rendering order of translucent polygons. Therefore, shadow volumes cannot be rendered correctly.
        // 
        //
        G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);

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
    // After a certain size, for large resources it is faster to call DC_StoreAll.
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
