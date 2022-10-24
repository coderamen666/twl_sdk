/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - MultiModel
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
// This sample is for situations when there are multiple models in one .nsbmd file.
//
// If multiple .imd files are given as input to the g3dcvtr, the multiple models can be collected in a single .nsbmd file.
// When this occurs, textures and palettes having the same name and content can be combined into a single component, using less memory as a result.
// 
//
// Sample Operation
// Gets the data for each model resource, specifying an index from the model set within the resource using the NNS_G3dGetMdlByIdx function.
// The various model resources share textures. (This can be confirmed in the g3dcvtr ./data/two_cubes.nsbmd file.)
// 
//
// HOWTO
// 1: E.g., when g3dcvtr file1.imd file2.imd ... fileN.imd -o files.nsbmd are used, all of the models from file 1 through file N are combined into a single file.
//    
//    In this case, textures and palettes with the same name and contents are collected in one texture/palette.
// 2: The NNS_G3dResDefaultSetup function binds the VRAM loading of textures and palettes to the various model resources.
//    
// 3: With the NNS_G3dGetMdlByIdx function, you can get each model resource by specifying the index.
//    In addition, use of the NNS_G3D_GET_MDL macro (defined in util.h) allows the IMD file name to specified as a text string literal, without its extension.
//    
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void InitG3DLib(void);
static void LoadG3DModel(const char* path,NNSG3dResMdl** pModel1,NNSG3dResMdl** pModel2);
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
    //  There are two models
    NNSG3dRenderObj object1,object2;
    NNSG3dResMdl*    model1 = NULL;
    NNSG3dResMdl*    model2 = NULL;

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

    //  Gets two models from one binary
    LoadG3DModel("data/two_cubes.nsbmd", &model1, &model2);
    SDK_ASSERTMSG(model1, "model1 load failed");
    SDK_ASSERTMSG(model2, "model2 load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object1, model1);
    NNS_G3dRenderObjInit(&object2, model2);

    G3DDemo_InitCamera(&gCamera, 2*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "MULTI MODEL");

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
            // object1 configuration and rendering
            //
            VecFx32 scale = {FX32_ONE << 2, FX32_ONE << 2, FX32_ONE << 2};
            VecFx32 trans = {FX32_ONE * 4, 0, 0};

            // Apply scaling and enlarge
            NNS_G3dGlbSetBaseScale(&scale);

            //  Move slightly to the right
            NNS_G3dGlbSetBaseTrans(&trans);

            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlushP();

            // Start measuring the time
            time = OS_GetTick();
            {
                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                //
                NNS_G3dDraw(&object1);
            }
            // Time measurement result
            time = OS_GetTick() - time;
        }

        {
            //
            // object2 configuration and rendering
            //
            VecFx32 trans = {-FX32_ONE * 4, 0, 0};
            // Set the scale to the same setting as when object1 was rendered.
            
            //  Move a little to the left
            NNS_G3dGlbSetBaseTrans(&trans);

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
            NNS_G3dDraw(&object2);
        }

        {
            // Render the ground
            // First, restore scale and trans to their original states
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

  Arguments:    path : Model data path name.
                    pModel1, pModel2: address of pointers specifying the models

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
LoadG3DModel(const char* path, NNSG3dResMdl** pModel1, NNSG3dResMdl** pModel2)
{
    BOOL status;
    NNSG3dResTex*        texture = NULL;
    NNSG3dResFileHeader* resFile = G3DDemo_LoadFile(path);

    SDK_NULL_ASSERT(resFile);

    // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
    // 
    // After a certain size, for large resources it is faster to call DC_StoreAll.
    // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
    DC_StoreRange(resFile, resFile->fileSize);

    // For cases when the default initialization function is called to perform setup
    status = NNS_G3dResDefaultSetup(resFile);
    NNS_G3D_ASSERTMSG(status, "NNS_G3dResDefaultSetup failed");

    //
    // G3D: Gets model
    // Since nsbmd can include multiple models, gets the pointer to a single model by specifying its index. When there is only one model, use 0.
    // 
    // --------------------------------------------------------------------
    //  Gets two models from one binary
    //  The texture differs only for the blue star and red circle part of the die's "1"
    //  The data volume is reduced by sharing the other parts
    *pModel1 = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFile), 0);
    *pModel2 = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFile), 1);
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
