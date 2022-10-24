/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - ModifyNsbmd
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
// Sample to replace the data within a model resource.
//
// Within the G3D library, the data within a model resource is not replaced, except at the library's initialization and termination. It is possible to do so, however, if the user explicitly writes code to do so.
// 
//  In this sample, functions declared in model.h for G3D are used to access each attribute in a material.
// Each of the attributes within the material are accessed.
//
// Sample Operation
// The material data internal to the model resource is changed with each frame.
// Move the menu (each material attribute) up and down with the B and X Buttons.
// Whether the global information configured as material data by the NNSG3dGlbXXXX functions will be used instead of the material data within the model resource is configured by the A Button.
// Press the button to switch the setting.
// 
//
// HOWTO:
// * Use NNS_G3dMdlUseMdlXXXX to make the material data inside the model resource be reflected in rendering (default).
// * The material data configured by the NSG3dGlbXXXX functions is reflected in the rendering by the NNS_G3dMdlUseGlbXXXX functions.
// * Use NNS_G3dMdlSetMdlXXXX to overwrite the material data inside the model resource.
// * Use NNS_G3dMdlGetMdlXXXX to get the material data inside the model resource.
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void          InitG3DLib(void);
static NNSG3dResMdl* LoadG3DModel(const char* path);
static void          SetCamera(G3DDemoCamera* camera);

//  Timer for changing the materials in the model over time
static int mdlTimer = 0;
static void changeMdlMat(NNSG3dResMdl* pMdl);
static GXRgb calcNextColor(GXRgb rgb);

#define	NUM_ITEM	9


/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.
                Select item with X and B; switch between Glb and Mdl with A.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    //  Definitions of variables related to the items
//  const int NumItem = 9;

    int itemNo = 0;
    const char* ItemName[NUM_ITEM][2] = {
        {"MdlDiff","GlbDiff"},
        {"MdlAmb","GlbAmb"},
        {"MdlSpec","GlbSpec"},
        {"MdlEmi","GlbEmi"},
        {"MdlLightFlag","GlbLightFlag"},
        {"MdlPolyMode","GlbPolyMode"},
        {"MdlCullMode","GlbCullMode"},
        {"MdlPolyID","GlbPolyID"},
        {"MdlAlpha","GlbAlpha"}
    };
    int itemFlag[NUM_ITEM] = {
        0,0,0,0,0,0,0,0,0
    };
    

    NNSG3dRenderObj object;
    NNSG3dResMdl*    model;

    G3DDemo_InitSystem();
    G3DDemo_InitMemory();
    G3DDemo_InitVRAM();

    InitG3DLib();
    G3DDemo_InitDisplay();

    // Make the manager able to manage 4 slots' worth of texture image slots, and set it as the default manager.
    // 
    NNS_GfdInitFrmTexVramManager(4, TRUE);

    // Make the manager able to manage 32 KB of palettes, and set it as the default manager.
    // 
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    model = LoadG3DModel("data/human_run_t.nsbmd");
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

    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "ModifyNsBmd");

    for(;;)
    {
        OSTick time;

        SVC_WaitVBlankIntr();

        G3DDemo_PrintApplyToHW();
        G3DDemo_ReadGamePad();
        
        //  Sets whether to use global parameters or model parameters.
        //  Use the functions declared in model.h
        if (itemFlag[0] == 0)
            NNS_G3dMdlUseMdlDiff(model);
        else
            NNS_G3dMdlUseGlbDiff(model);

        if (itemFlag[1] == 0)
            NNS_G3dMdlUseMdlAmb(model);
        else
            NNS_G3dMdlUseGlbAmb(model);

        if (itemFlag[2] == 0)
            NNS_G3dMdlUseMdlSpec(model);
        else
            NNS_G3dMdlUseGlbSpec(model);

        if (itemFlag[3] == 0)
            NNS_G3dMdlUseMdlEmi(model);
        else
            NNS_G3dMdlUseGlbEmi(model);

        if (itemFlag[4] == 0)
            NNS_G3dMdlUseMdlLightEnableFlag(model);
        else
            NNS_G3dMdlUseGlbLightEnableFlag(model);

        if (itemFlag[5] == 0)
            NNS_G3dMdlUseMdlPolygonMode(model);
        else
            NNS_G3dMdlUseGlbPolygonMode(model);

        if (itemFlag[6] == 0)
            NNS_G3dMdlUseMdlCullMode(model);
        else
            NNS_G3dMdlUseGlbCullMode(model);

        if (itemFlag[7] == 0)
            NNS_G3dMdlUseMdlPolygonID(model);
        else
            NNS_G3dMdlUseGlbPolygonID(model);

        if (itemFlag[8] == 0)
            NNS_G3dMdlUseMdlAlpha(model);
        else
            NNS_G3dMdlUseGlbAlpha(model);

        //  Change the parameters of the materials in the model over time
        changeMdlMat(model);

        G3DDemo_MoveCamera(&gCamera);
        G3DDemo_CalcCamera(&gCamera);

        G3X_Reset();
        G3X_ResetMtxStack();

        SetCamera(&gCamera);

        {
            VecFx32 scale = {FX32_ONE >> 2, FX32_ONE >> 2, FX32_ONE >> 2};
            VecFx32 trans = {FX32_ONE * 4, 0, 0};

            //
            // Set the model's base scale and base translation
            //
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
            // Time measurement results screen display.
            time = OS_GetTick() - time;
        }

        // Restore scale to how it was before
        {
            VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
            NNS_G3dGlbSetBaseScale(&scale);
        }
        //  Also restore trans to original
        {
            VecFx32 trans = {0, 0, 0};
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
        
        
        //  Switch between items with X and B; use button to change them.
        {
            if(G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_X))
            {
                itemNo --;
                if(itemNo < 0)
                {
                    itemNo = NUM_ITEM - 1;
                }
            }
            if(G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_B))
            {
                itemNo ++;
                if(itemNo >= NUM_ITEM)
                {
                    itemNo = 0;
                }
            }
            if(G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
            {
                itemFlag[itemNo] = 1 - itemFlag[itemNo];
            }
        }
        
        
        //  Display item's current state
        {
            int item;
            G3DDemo_Printf(0,2, G3DDEMO_COLOR_GREEN, "select:X,B");
            G3DDemo_Printf(0,3, G3DDEMO_COLOR_GREEN, "change:A");
            for(item = 0;item < NUM_ITEM;item ++)
            {
                int color = G3DDEMO_COLOR_WHITE;
                if(item == itemNo)
                {
                    color = G3DDEMO_COLOR_RED;
                }
                G3DDemo_Printf(0,5 + item, color, "%s", ItemName[item][itemFlag[item]]);
            }
        }

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06d usec", OS_TicksToMicroSeconds(time));
    }
}


/* -------------------------------------------------------------------------
  Name:         changeMdlMat

  Description:  Changes the settings of the materials in the model over time.

  Arguments:    Pointer to model: NNSG3dResMdl* pMdl

  Returns:      None.
   ------------------------------------------------------------------------- */
void
changeMdlMat(NNSG3dResMdl* pMdl)
{
    u32 MatID;

    mdlTimer ++;
    if(mdlTimer >= 32 * 32 * 32)
    {
        mdlTimer = 0;
    }

    for (MatID = 0; MatID < pMdl->info.numMat; ++MatID)
    {
        //
        // Changes the model's material color with each frame.
        //
        NNS_G3dMdlSetMdlDiff(pMdl,
                             MatID,
                             calcNextColor(NNS_G3dMdlGetMdlDiff(pMdl, MatID)));

        NNS_G3dMdlSetMdlAmb(pMdl,
                            MatID,
                            calcNextColor(NNS_G3dMdlGetMdlAmb(pMdl, MatID)));

        NNS_G3dMdlSetMdlSpec(pMdl,
                             MatID,
                             calcNextColor(NNS_G3dMdlGetMdlSpec(pMdl, MatID)));

        NNS_G3dMdlSetMdlEmi(pMdl,
                            MatID,
                            calcNextColor(NNS_G3dMdlGetMdlEmi(pMdl, MatID)));

        //
        // Changes the light shining on the model between ON and OFF.
        //
        NNS_G3dMdlSetMdlLightEnableFlag(pMdl,
                                        MatID,
                                        mdlTimer / 55 % 16);

        //
        // Changes the model's polygon mode.
        //
        NNS_G3dMdlSetMdlPolygonMode(pMdl,
                                    MatID,
                                    (GXPolygonMode)(mdlTimer / 60 % 2));

        //
        // Changes the model's cull mode.
        //
        NNS_G3dMdlSetMdlCullMode(pMdl,
                                 MatID,
                                 (GXCull)(mdlTimer / 73 % 2 + 1));

        //
        // Set the model's polygon ID.
        //
        NNS_G3dMdlSetMdlPolygonID(pMdl,
                                  MatID,
                                  31);

        //
        // Changes the model's alpha.
        //
        NNS_G3dMdlSetMdlAlpha(pMdl,
                              MatID,
                              mdlTimer / 7 % 32);
    }
}


/* -------------------------------------------------------------------------
  Name:         calcNextColor

  Description:  Calculates the next color from the current color in order to change the material color.

  Arguments:    Current color: GXRgb rgb

  Returns:      Next color: GXRgb
   ------------------------------------------------------------------------- */
GXRgb
calcNextColor(GXRgb rgb) {
    //  Extract each component
    u16 R = (u16)((rgb & GX_RGB_R_MASK) >> GX_RGB_R_SHIFT);
    u16 G = (u16)((rgb & GX_RGB_G_MASK) >> GX_RGB_G_SHIFT);
    u16 B = (u16)((rgb & GX_RGB_B_MASK) >> GX_RGB_B_SHIFT);
    
    const int ChangeSpeed = 2;
    
    R += ChangeSpeed;
    if(R > 31)
    {
        R -= 31;
        G += ChangeSpeed;
        if(G > 31)
        {
            G -= 31;
            B += ChangeSpeed;
            if(B > 31)
            {
                B -= ChangeSpeed;
            }
        }
    }

    return GX_RGB(R, G, B);
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

    //
    // G3D:
    // Global light settings
    // ------------------------------------------------------------------------
    NNS_G3dGlbLightVector(GX_LIGHTID_0, -FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
    NNS_G3dGlbLightVector(GX_LIGHTID_1,  FX16_SQRT1_3,  FX16_SQRT1_3,  FX16_SQRT1_3);
    NNS_G3dGlbLightVector(GX_LIGHTID_2,             0,   -FX16_ONE  ,             0);
    NNS_G3dGlbLightVector(GX_LIGHTID_3,             0,    FX16_ONE-1,             0);

    NNS_G3dGlbLightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
    NNS_G3dGlbLightColor(GX_LIGHTID_1, GX_RGB(10, 10, 10));
    NNS_G3dGlbLightColor(GX_LIGHTID_2, GX_RGB(16,  0,  0));
    NNS_G3dGlbLightColor(GX_LIGHTID_3, GX_RGB(16, 16,  0));

    NNS_G3dGlbMaterialColorDiffAmb(GX_RGB(20, 10, 10), GX_RGB(6, 6, 6), FALSE);
    NNS_G3dGlbMaterialColorSpecEmi(GX_RGB(10, 16, 0), GX_RGB(6, 6, 14), FALSE);
    
    //  Global polygon attribute settings
    NNS_G3dGlbPolygonAttr(
            0x000f,                                   // all lights ON
            GX_POLYGONMODE_MODULATE,
            GX_CULL_BACK,
            0,
            28,
            GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE
    );
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
    BOOL status;
    NNSG3dResMdl* model = NULL;
    NNSG3dResTex* texture = NULL;
    NNSG3dResFileHeader* resFile = G3DDemo_LoadFile(path);

    SDK_NULL_ASSERT(resFile);

    // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw, all model resources have to be stored in memory before calling those functions.
    // 
    // After a certain size, for large resources it is faster to just call DC_StoreAll.
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
