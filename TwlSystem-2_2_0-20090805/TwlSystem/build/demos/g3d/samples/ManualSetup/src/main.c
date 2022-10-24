/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - ManualSetup
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
// This Sample sets up the model resource without using NNS_G3dResDefaultSetup.
//
// Performs setup by loading model binary data into main memory without using the NNS_G3dResDefaultSetup function and displays the model on screen.
// 
//
// Sample Operation
// Displays model.
// 
// HOWTO:
// Refer to the explanations in the code.
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void           InitG3DLib(void);
static NNSG3dResMdl*  LoadG3DModel(const char* path);
static void           SetCamera(G3DDemoCamera* camera);


static void
ManualSetup(void* pResFile /* pResFile is a pointer to the loaded .nsbmd file. */)
{
    //
    // Set it up yourself without using NNS_G3dResDefaultSetup.
    // This code sets up only the index #0 model inside the .nsbmd.
    //
    NNSG3dResMdlSet* mdlSet = NNS_G3dGetMdlSet(pResFile);
    NNSG3dResTex* texture = NNS_G3dGetTex(pResFile); // This returns NULL if there is no texture data.

    //
    // The following procedure is required for initializing model data.
    // The initialization process is not actually required for models that do not have textures.
    //
    // 1: Obtain the size of the memory area required for textures and palettes.
    // 2: Create a key (same format as the gfd library) for the VRAM region that is used for texture and palettes.
    // 3: Set the key to the texture block of the model data.
    // 4: Transfer textures or palettes to VRAM.
    // 5: Bind the materials and textures that are in the model resource.
    //
    // Steps 4 and 5 can be swapped without a problem.
    //
    // In addition, this data must be written back from the cache into main memory before this setup function is called, because, as with the NNS_G3dResDefaultSetup function, texture and palette transfers use DMA.
    // 
    // 
    //
    if (texture)
    {
        //
        // 1: Obtain the size of the memory required for textures and palettes.
        //
        u32 szTex     = NNS_G3dTexGetRequiredSize(texture);
        u32 szTex4x4  = NNS_G3dTex4x4GetRequiredSize(texture);
        u32 szPltt    = NNS_G3dPlttGetRequiredSize(texture);

        //
        // 2: Create a key (same format as the gfd library) for the VRAM region that is used for texture and palettes.
        //    
        //
        // The size of the region that is set to a key must be at least the required size.
        // Also, the user can create an original, unique key as long as it is valid.
        // Keys can be created directly by using NNS_GfdMakeTexKey and NNS_GfdMakePlttKey.
        // 
        //
        u32 keyTex    = NNS_GfdAllocTexVram(szTex, FALSE, 0);
        u32 keyTex4x4 = NNS_GfdAllocTexVram(szTex4x4, TRUE, 0);
        u32 keyPltt   = NNS_GfdAllocPlttVram(szPltt, FALSE, TRUE);

        OS_Printf("keyTex    = 0x%08x addr = 0x%08x size = 0x%08x\n", keyTex   , NNS_GfdGetTexKeyAddr(keyTex   ), NNS_GfdGetTexKeySize(keyTex   ));
        OS_Printf("keyTex4x4 = 0x%08x addr = 0x%08x size = 0x%08x\n", keyTex4x4, NNS_GfdGetTexKeyAddr(keyTex4x4), NNS_GfdGetTexKeySize(keyTex4x4));
        OS_Printf("keyPltt   = 0x%08x addr = 0x%08x size = 0x%08x\n", keyPltt  , NNS_GfdGetPlttKeyAddr(keyPltt ), NNS_GfdGetPlttKeySize(keyPltt ));

        //
        // 3: Set the key you created to the texture block of the model data.
        // 
        NNS_G3dTexSetTexKey(texture, keyTex, keyTex4x4);
        NNS_G3dPlttSetPlttKey(texture, keyPltt);

        //
        // 4: Transfer textures or palettes to VRAM.
        //
        // Transfer the textures and palettes to VRAM in accordance with the information of the set key.
        // If TRUE is set to the second argument, BeginLoadTex and EndLoadTex will be executed internally every cycle.
        // If set to FALSE, it does not do anything. (BeginLoadTex and EndLoadTex must be executed externally.)
        //
        NNS_G3dTexLoad(texture, TRUE);
        NNS_G3dPlttLoad(texture, TRUE);

        //
        // 5: Bind the materials and textures that are in the model resource.
        //
        // Using the NNS_G3dBindMdlSet function, texture-related data (TexImageParam, TexPlttBase) within model resource materials are set up.
        // 
        // 
        // If the function has a BOOL-type return value and that value is TRUE, all materials that have textures inside the model will be bound to the corresponding texture.
        // 
        // If the value is FALSE, it means that materials not bound to a texture still remain.
        //
        // When FALSE, there are some materials that are not bound to the texture.
        // (Essentially, if multiple texture blocks with the same texture name are bound,
        // the textures that were bound first are the ones which are valid.)
        //
        {
            BOOL result;
            result = NNS_G3dBindMdlSet(mdlSet, texture);
            SDK_ASSERT(result);
        }
    }
}


static void
ManualRelease(void* pResFile /* pResFile is a pointer to the loaded .nsbmd file. */)
{
    //
    // Release it yourself without using NNS_G3dResDefaultRelease.
    //

    NNSG3dResTex* texture = NNS_G3dGetTex(pResFile); // This returns NULL if there is no texture data.

    //
    // Releasing the model data requires the following procedure.
    // The release process is not actually required for models that do not have textures.
    //
    // 1: Clear the association to the texture resource that is set internally in the model resource.
    // 2: Remove the key from the texture resource.
    // 3: (If used) Use the VRAM manager and release the VRAM region that corresponds to the key.
    //
    if (texture)
    {
        NNSG3dPlttKey plttKey;
        NNSG3dTexKey  texKey, tex4x4Key;
        int status;
        NNSG3dResMdlSet* mdlSet = NNS_G3dGetMdlSet(pResFile);

        //
        // 1: Clear the association to the texture resource that is set internally in the model resource.
        //
        NNS_G3dReleaseMdlSet(mdlSet);

        // 2: Remove the key from the texture resource.
        plttKey = NNS_G3dPlttReleasePlttKey(texture);
        NNS_G3dTexReleaseTexKey(texture, &texKey, &tex4x4Key);

        //
        // 3: (If used) Use the VRAM manager and release the VRAM region that corresponds to the key.
        //
        // When the VRAM manager is in use, it is used to deallicate the VRAM region.
        // 
        // However, when the manager is the frame method used by this sample, regions cannot be separately deallocated, so the same region cannot be reused.
        // 
        // If the user is managing them in a proprietary manner, the code below must be made proprietary to the user.
        // 
        //
        if (plttKey > 0)
        {
            // If stored in texture palette slot, release
            status = NNS_GfdFreePlttVram(plttKey);
            NNS_G3D_ASSERTMSG(!status, "NNS_GfdFreePlttVram failed");
        }

        if (tex4x4Key > 0)
        {
            // If stored in texture image slot, release
            status = NNS_GfdFreeTexVram(tex4x4Key);
            NNS_G3D_ASSERTMSG(!status, "NNS_GfdFreeTexVram failed");
        }

        if (texKey > 0)
        {
            // If stored in texture image slot, release
            status = NNS_GfdFreeTexVram(texKey);
            NNS_G3D_ASSERTMSG(!status, "NNS_GfdFreeTexVram failed");
        }
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
    void*           pResFile;
    BOOL            isSetup = TRUE;

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

    {
        //
        // Load model resource
        //
        pResFile = G3DDemo_LoadFile("data/human_run_t.nsbmd");
        SDK_ASSERTMSG(pResFile, "load failed");

        // To use DMA transfers within functions like ManualSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
        // 
        // After a certain size, for large resources it is faster to call DC_StoreAll.
        // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
        DC_StoreRange(pResFile, ((NNSG3dResFileHeader*)pResFile)->fileSize);

        ManualSetup(pResFile);

        //
        // Gets the index #0 model from the model set inside the model resource.
        //
        pModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(pResFile), 0);
        SDK_ASSERTMSG(pModel, "model not found");

        //
        // G3D: NNSG3dRenderObj initialization
        //
        // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
        // 
        // The area specified by the internal member pointer must be obtained and released by the user.
        // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
        //
        NNS_G3dRenderObjInit(&object, pModel);
    }

    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "ManualSetup");

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

        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (isSetup)
            {
                ManualRelease(pResFile);
                pModel = NULL;
                isSetup = FALSE;
            }
            else
            {
                ManualSetup(pResFile);
                pModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(pResFile), 0);
                SDK_ASSERTMSG(pModel, "model not found");
                NNS_G3dRenderObjInit(&object, pModel);

                isSetup = TRUE;
            }
        }

        if (isSetup)
        {
            // Apply scaling and make it smaller
            VecFx32 scale = {FX32_ONE>>2, FX32_ONE>>2, FX32_ONE>>2};
            NNS_G3dGlbSetBaseScale(&scale);

            //
            // G3D:
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
            NNS_G3dGlbFlushP();
            NNS_G3dGeFlushBuffer();
            G3DDemo_DrawGround(&gGround);
        }

        /* Swap memory relating to geometry and rendering engine */
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        if (isSetup)
        {
            G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06d usec", OS_TicksToMicroSeconds(time));
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_GREEN, "Press A Button to Release");
        }
        else
        {
            G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:           ");
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_GREEN, "Press A Button to Setup  ");
        }
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
