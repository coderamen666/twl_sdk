/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - DisposeTex
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
// This sample destroys the texture/palette region on main memory after VRAM transfer.
//
// When the model textures and palettes for main characters, etc., are made to reside in the VRAM region, there is no need to maintain those model textures and palettes in main memory.
// 
// In such a case, you can deallocate the texture/palette region that is at the end of the resource file, and conserve memory.
// 
//
// Sample Operation
// During the CollapseResourceMemory function within the sample, the initial addresses for the textures and palettes are obtained from NNSH3dResTex, and the region following that address is returned to the heap.
// 
//
// HOWTO
// By making tex a pointer to the NNSG3dResTex structure, the starting address for the texture and palette data becomes (u8*)tex + tex->texInfo.ofsTex.
// 
// Memory after this address can be overwritten.
//
//---------------------------------------------------------------------------


#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void SetCamera(G3DDemoCamera* camera);
static void CollapseResourceMemory(void* memBlock, NNSG3dResTex* texture);
static void TextureZeroClear(NNSG3dResTex* pTex);

/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.
                Zeroes out the texture region with TextureZeroClear.
                This indicates that the sample will work without problems even if the texture region is zeroed out after RAM transfer.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    NNSG3dRenderObj object;
    NNSG3dResMdl*   model;

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

    //
    // After initialization, including loading the data internally with LoadG3DModel and setting the textures to VRAM, zero-clear the texture data portion of memory, reduce the size of the heap and deallocate only the texture data portion.
    // 
    // 
    //
    model = LoadG3DModel("data/human_run_t.nsbmd");
    SDK_ASSERTMSG(model, "load failed");

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // NNSG3dRenderObj is a structure that holds a single model, and every type of information related to the animations applied to that model.
    // 
    // The area specified by the internal member pointer must be obtained and released by the user.
    // Here mdl can be NULL. (But it must be set, not NULL, during NNS_G3dDraw.)
    //
    NNS_G3dRenderObjInit(&object, model);

    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "DisposeTex");

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
    NNSG3dResMdl* model   = NULL;
    NNSG3dResTex* texture = NULL;
    NNSG3dResFileHeader* resFile = G3DDemo_LoadFile(path);
    BOOL status;

    SDK_NULL_ASSERT(resFile);

    // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
    // 
    // After a certain size, for large resources it is faster to call DC_StoreAll.
    // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
    DC_StoreRange(resFile, resFile->fileSize);

    status = NNS_G3dResDefaultSetup(resFile);
    NNS_G3D_ASSERTMSG(status, "NNS_G3dResDefaultSetup failed");

    //
    // Zeros out the texture data part of the texture block.
    //
    TextureZeroClear(NNS_G3dGetTex(resFile));

    //
    // Shrink the heap and release the texture data part.
    //
    CollapseResourceMemory(resFile, NNS_G3dGetTex(resFile));

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


/* -------------------------------------------------------------------------
  Name:         CollapseResourceMemory

  Description:  Shrinks the memory block by an amount equal to the size of the texture image.

                This assumes that only the texture image is at the tail-end of the memory block.
                  

  Arguments:    memBlock:  The block to be collapsed.
                texture:   Pointer to the texture data.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
CollapseResourceMemory(void* memBlock, NNSG3dResTex* texture)
{
    u8* texImgStartAddr;
    u32 newSize;

    // Textures and palettes are saved in order of the non-4x4COMP textures, the 4x4COMP textures and then the palettes.
    // Thus, everything after the starting address for non-4x4COMP textures becomes unneeded.
    // Even when there are only texture images in the 4x4COMP format, an appropriate value is entered in texture->texInfo.ofsTex.
    // 
    SDK_ASSERT(texture->texInfo.ofsTex != 0);
    texImgStartAddr = (u8*)texture + texture->texInfo.ofsTex;

    // Size from start of heap to texture image
    newSize = (u32)(texImgStartAddr - (u8*)memBlock);

    // Shrink the memory block.
    // As a result, an amount of memory equal to the texture image is returned to the heap.
    (void)NNS_FndResizeForMBlockExpHeap(G3DDemo_AppHeap, memBlock, newSize);

}


/*-------------------------------------------------------------------------
    TextureZeroClear

    Zeros out the texture data part of the texture block.
    Dictionaries and other information remains, so it can be bound and released.
  ------------------------------------------------------------------------- */
static void
TextureZeroClear(NNSG3dResTex* pTex)
{
    MI_CpuClearFast((u8*)pTex + pTex->texInfo.ofsTex,
                    pTex->header.size - pTex->texInfo.ofsTex);
}
