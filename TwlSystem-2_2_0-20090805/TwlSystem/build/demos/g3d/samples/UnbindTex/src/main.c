/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - UnbindTex
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
// Sample of texture removal and switching
//
// Removes textures from model and switches textures with a different model.
//
// Sample Operation
// There are textures attached to cubes and rectangular solids.
// A Button -> Switches textures.
// B Button -> Removes textures.
//
// HOWTO:
// * Bind model to texture using NNS_G3dBindMdlTex and NNS_G3dBindMdlPltt.
// * You can cancel the binding of models and textures with the NNS_G3dReleaseMdlTex and NNS_G3dReleaseMdlPltt functions.
//   
// - The NNS_G3dBindMdlTexEx and NNS_G3dBindMdlPlttEx functions bind textures to specified names within a model.
//   
// - The NNS_G3dReleaseMdlTexEx and NNS_G3dReleaseMdlPlttEx functions unbind textures from specified names within a model.
//   
//
//---------------------------------------------------------------------------


#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResTex* LoadG3DTexture(NNSG3dResFileHeader* resFile);
static void          SetCamera(G3DDemoCamera* camera);


/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.
                Removes the texture using B, and switches the texture using A.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    
    BOOL changeTexture = TRUE;
    
    //For two models
    NNSG3dResFileHeader* resFileCube = NULL;
    NNSG3dResFileHeader* resFileBox  = NULL;
    
    NNSG3dRenderObj cubeObject;
    NNSG3dResMdl*   cubeModel = NULL;
    NNSG3dResTex*   cubeTexture = NULL;
    
    NNSG3dRenderObj boxObject;
    NNSG3dResMdl*   boxModel = NULL;
    NNSG3dResTex*   boxTexture = NULL;
    
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
        // Load Box Model and initialize.
        // Get the NNSG3dResMdl* and NNSG3dResTex* to use later.
        //
        resFileBox = G3DDemo_LoadFile("data/TextureBox.nsbmd");

        // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
        // 
        // After a certain size, for large resources it is faster to just call DC_StoreAll.
        // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
        DC_StoreRange(resFileBox, resFileBox->fileSize);

        (void)NNS_G3dResDefaultSetup(resFileBox);

        SDK_ASSERTMSG(resFileBox, "load failed");
        boxModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFileBox), 0);
        SDK_NULL_ASSERT(boxModel);

        boxTexture = NNS_G3dGetTex(resFileBox);
    }

    {
        //
        // Load Cube Model and initialize.
        // Get the NNSG3dResMdl* and NNSG3dResTex* to use later.
        //
        resFileCube = G3DDemo_LoadFile("data/TextureCube.nsbmd");

        // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
        // 
        // After a certain size, for large resources it is faster to just call DC_StoreAll.
        // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
        DC_StoreRange(resFileCube, resFileCube->fileSize);

        (void)NNS_G3dResDefaultSetup(resFileCube);

        SDK_ASSERTMSG(resFileCube, "load failed");
        cubeModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFileCube), 0);
        SDK_NULL_ASSERT(cubeModel);

        cubeTexture = NNS_G3dGetTex(resFileCube);
    }

    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&cubeObject, cubeModel);
    NNS_G3dRenderObjInit(&boxObject, boxModel);

    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "UNBINDTEX SAMPLE");

    for(;;)
    {
        OSTick time;

        //Texture switching using A
        //binding will not be valid before a release
        if(G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A)) 
        {
            // Substitutes in a texture having the name 'same_name'.
            // There is no need to swap palettes because the textures used in this sample are direct textures.
            // 
            NNS_G3D_DEFRESNAME(texName, "same_name");
            (void)NNS_G3dReleaseMdlTexEx(boxModel, &texName.resName);
            (void)NNS_G3dReleaseMdlTexEx(cubeModel, &texName.resName);

            if (changeTexture)
            {
                //
                // Bind the texture of the other model.
                //
                (void)NNS_G3dBindMdlTexEx(boxModel, cubeTexture, &texName.resName);
                (void)NNS_G3dBindMdlTexEx(cubeModel, boxTexture, &texName.resName);
            }
            else
            {
                //
                // Bind the texture of your own model.
                //
                (void)NNS_G3dBindMdlTexEx(boxModel, boxTexture, &texName.resName);
                (void)NNS_G3dBindMdlTexEx(cubeModel, cubeTexture, &texName.resName);
            }
            changeTexture = (changeTexture) ? FALSE : TRUE;
        }                            
        else if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_B)) // Strip off the textures with the B Button
        {
            NNS_G3dReleaseMdlTex(boxModel);
            NNS_G3dReleaseMdlPltt(boxModel); // Do nothing when the palette does not exist.
            
            NNS_G3dReleaseMdlTex(cubeModel);
            NNS_G3dReleaseMdlPltt(cubeModel); // Do nothing when the palette does not exist.
        }

        SVC_WaitVBlankIntr();
        G3DDemo_PrintApplyToHW();
        G3DDemo_ReadGamePad();

        G3DDemo_MoveCamera(&gCamera);
        G3DDemo_CalcCamera(&gCamera);



        G3X_Reset();
        G3X_ResetMtxStack();

        SetCamera(&gCamera);

        time = OS_GetTick();
        {
            //boxModel rendering
            VecFx32 trans = {FX32_ONE<<2, FX32_ONE<<2, 0};
            NNS_G3dGlbSetBaseTrans(&trans);
            NNS_G3dGlbFlushP();
            NNS_G3dDraw(&boxObject);
        }

        {
            //cubeModel rendering&#8206;
            VecFx32 trans = {-FX32_ONE<<2, FX32_ONE<<2, 0};
            NNS_G3dGlbSetBaseTrans(&trans);
            NNS_G3dGlbFlushP();
            NNS_G3dDraw(&cubeObject);
        }
        time = OS_GetTick() - time;

        {
            // Set translation before rendering the ground.
            VecFx32 trans = {0,0,0};
            NNS_G3dGlbSetBaseTrans(&trans);
        }
        
        //
        // G3D: Normally, when any of the G3_XXX functions are called, you have to call the NNS_G3dGeComFlushBuffer function before that and synchronize.
        //      
        //
        NNS_G3dGlbFlushP();
        NNS_G3dGeFlushBuffer();
        G3DDemo_DrawGround(&gGround);

        /* Swap memory relating to geometry and rendering engine */
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06d usec", OS_TicksToMicroSeconds(time));
        
        //information display
        G3DDemo_Printf(0,2, G3DDEMO_COLOR_GREEN, "change Texture:A");
        G3DDemo_Printf(0,3, G3DDEMO_COLOR_GREEN, "release Texture:B");
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
