/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - CameraMtx
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
// This sample explains the setup method for the Projection matrix and Position/Vector matrix.
//
// G3D normally sets up the projection matrix, current matrix and such of the position/vector matrix
// by using NNS_G3dGlbFlushP, NNS_G3dGlbFlushVP
// and NNS_G3dGlbFlushWVP, before drawing a model with NNS_G3dDraw.
// The difference between the above three functions is what is set to the projection matrix and position/vector matrix.
// With the NNS_G3dGlbFlushP function, projective transformation matrix is set to the projection matrix,
// and the combination of the camera matrix and the object's BaseSRT matrix is set to the position/vector matrix.
// With the NNS_G3dGlbFlushVP function, the combination of the projective transformation matrix and camera matrix is set to the projection matrix,
// and the object's BaseSRT matrix is set to the position/vector matrix.
// With the NNS_G3dGlbFlushWVP function, the combination of the transformative projection matrix, the camera matrix,
// and the object BaseSRT matrix are set to the projection matrix,
// and the unit matrix is set in the position/vector matrix.
//
// There are two reasons for supporting multiple matrix setup methods.
// 1: There are some instances such as where you want to take the position/vector matrix to process it,
//    where you need coordinates in the camera space, where you need world coordinates,
//    or where you need local coordinates of the model (joints).
//    If setup has taken place in advance with the right function, these values can be obtained without any matrix multiplication.
// 2: If the local coordinates of the model (joints) are calculated beforehand, and a computed matrix is set on the position/vector matrix stack, you can change the position of the entire object and the camera position just by making changes to the projection matrix.
//    
//    
//    
//
// Sample Operation
// Use the A Button to select one of the following functions to be called before rendering objects: NNS_G3dGlbFlushP, NNS_G3dGlbFlushVP, or NNS_G3dGlbFlushWVP.
// 
// Also keep in mind the following are abbreviations:
// P: Projection
// V: View
// W: World
//  
//
// HOWTO:
// Call the NNS_G3dGlbFlushP, NNS_G3dGlbFlushVP, or NNS_G3dGlbFlushWVP function before rendering the object.
// 
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
    enum
    {
        Enum_NNS_G3dGlbFlushP   = 0,
        Enum_NNS_G3dGlbFlushVP  = 1,
        Enum_NNS_G3dGlbFlushWVP = 2
    };

    //  Which function do we use?
    int cameraFlag = Enum_NNS_G3dGlbFlushP;

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

    model = LoadG3DModel("data/billboard.nsbmd");
    SDK_ASSERTMSG(model, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set to a value, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, model);

    G3DDemo_InitCamera(&gCamera, 4 * FX32_ONE, 16 * FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5 * FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "CameraMtx");
    G3DDemo_Print(0,2, G3DDEMO_COLOR_WHITE, "change:A");

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
            VecFx32 scale = {FX32_ONE << 2, FX32_ONE << 2, FX32_ONE << 2};
            NNS_G3dGlbSetBaseScale(&scale);

            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //

            // When using NNS_G3dGlbFlushAlt, the current projection matrix is multiplied by the projective transformation matrix, camera matrix, and base SRT matrix.
            // Only the local conversion matrix is stored in the position/vector matrix (stack).
            // As a result, by changing the current projection matrix, it is possible to render multiple models in the same pose without re-calculating the Position/Vector matrix (stack) (although calculation results must be remaining within the matrix stack).
            // 
            switch(cameraFlag) {
            case Enum_NNS_G3dGlbFlushP:
                //  Projection matrix will not be multiplied by camera matrix
                NNS_G3dGlbFlushP();
                break;
            case Enum_NNS_G3dGlbFlushVP:
                //
                NNS_G3dGlbFlushVP();
                break;
            case Enum_NNS_G3dGlbFlushWVP:
                //  Projection matrix will be multiplied by camera matrix
                NNS_G3dGlbFlushWVP();
                break;
            }

            //  Start measuring the time
            time = OS_GetTick();
            {
                //
                // G3D: NNS_G3dDraw
                // Passing the configured NNSG3dRenderObj structure can perform rendering in any situation.
                // 
                // The resulting picture is the same whether NNS_G3dGlbFlush() or NNS_G3dGlbFlushAlt() is used.
                NNS_G3dDraw(&object);
            }
            // Time measurement result
            time = OS_GetTick() - time;
        }

        {
            // Restore scale to how it was before
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

        //  Switches cameraFlag every time A Button is pressed
        if(G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A)) {
            cameraFlag++;
            cameraFlag = cameraFlag % 3;
        }
        
        //   Display which function is being called now
        switch(cameraFlag)
        {
        case Enum_NNS_G3dGlbFlushP:
            G3DDemo_Printf(0,4, G3DDEMO_COLOR_GREEN, "G3dGlbFlushP  ");
            break;
        case Enum_NNS_G3dGlbFlushVP:
            G3DDemo_Printf(0,4, G3DDEMO_COLOR_GREEN, "G3dGlbFlushVP ");
            break;
        case Enum_NNS_G3dGlbFlushWVP:
            G3DDemo_Printf(0,4, G3DDEMO_COLOR_GREEN, "G3dGlbFlushWVP");
            break;
        }
        
        //   Display the contents of the current projection matrix and current pos/vector matrix
        switch(cameraFlag)
        {
        case Enum_NNS_G3dGlbFlushP:
            G3DDemo_Printf(0,6, G3DDEMO_COLOR_WHITE, "Projection Mtx:");
            G3DDemo_Printf(2,7, G3DDEMO_COLOR_MAGENTA, "Pj                   ");
            G3DDemo_Printf(0,8, G3DDEMO_COLOR_WHITE, "Pos/Vector Mtx:");
            G3DDemo_Printf(2,9, G3DDEMO_COLOR_MAGENTA, "Camera * BaseSRT ");
            break;
        case Enum_NNS_G3dGlbFlushVP:
            G3DDemo_Printf(0,6, G3DDEMO_COLOR_WHITE, "Projection Mtx:");
            G3DDemo_Printf(2,7, G3DDEMO_COLOR_MAGENTA, "Pj * Camera          ");
            G3DDemo_Printf(0,8, G3DDEMO_COLOR_WHITE, "Pos/Vector Mtx:");
            G3DDemo_Printf(2,9, G3DDEMO_COLOR_MAGENTA, "BaseSRT              ");
            break;
        case Enum_NNS_G3dGlbFlushWVP:
            G3DDemo_Printf(0,6, G3DDEMO_COLOR_WHITE, "Projection Mtx:");
            G3DDemo_Printf(2,7, G3DDEMO_COLOR_MAGENTA, "Pj * Camera * BaseSRT");
            G3DDemo_Printf(0,8, G3DDEMO_COLOR_WHITE, "Pos/Vector Mtx:");
            G3DDemo_Printf(2,9, G3DDEMO_COLOR_MAGENTA, "Identity Mtx         ");
            break;
        }
        
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
    NNS_G3D_NULL_ASSERT(resFile);

    // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
    // 
    // After a certain size, for large resources it is faster to just call DC_StoreAll.
    // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
    DC_StoreRange(resFile, resFile->fileSize);

    // Calls the default initialization function and performs setup.
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
