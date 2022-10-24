/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - PartialAnm2
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
// This is the second sample for playing a joint animation in parts.
//
// By performing the right sort of manipulation on an initialized animation object, partial playback of a joint animation, e.g., just moving an arm, is possible.
// 
//
// Sample Operation
// A person is running. By pressing buttons, the animations of the four limbs can be turned ON/OFF individually.
// By pressing the A Button, the left leg animation is turned ON/OFF.
// By pressing the B Button, the right leg animation is turned ON/OFF.
// By pressing the X Button, the left arm animation is turned ON/OFF.
// By pressing the Y Button, the right arm animation is turned ON/OFF.
//
// HOWTO:
// 1: Set up the animation object the same way as the JointAnm sample.
// 2: Create animation objects which only move their arms and legs using the NNS_G3dAnmObjDisableID function.
//    
// 3: It is possible to start and stop movement of each separate limb by controlling each animation object separately.
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
    anmObjArmL

    Configures an animation object so that when it is manipulated only the left arm is moved.
   ------------------------------------------------------------------------- */
static void anmObjArmL(NNSG3dAnmObj* p, const NNSG3dResMdl* m)
{
    int arm_l1, arm_l2, wrist_l, wrist_l_end;
    int i;

    // Obtain the node ID of the left arm section.
    NNS_G3D_GET_JNTID(m, &arm_l1, "arm_l1");
    NNS_G3D_GET_JNTID(m, &arm_l2, "arm_l2");
    NNS_G3D_GET_JNTID(m, &wrist_l, "wrist_l");
    NNS_G3D_GET_JNTID(m, &wrist_l_end, "wrist_l_end");

    SDK_ASSERT(arm_l1 >= 0 && arm_l2 >= 0 && wrist_l >=0 && wrist_l_end >= 0);

    for (i = 0; i < m->info.numNode; ++i)
    {
        if (i != arm_l1  &&
            i != arm_l2  &&
            i != wrist_l &&
            i != wrist_l_end)
        {
            //
            // Disable the animations except for the left arm.
            //
            NNS_G3dAnmObjDisableID(p, i);
        }
    }
}


/* -------------------------------------------------------------------------
    anmObjArmR

    Configures an animation object so that when it is manipulated only the right arm is moved.
   ------------------------------------------------------------------------- */
static void anmObjArmR(NNSG3dAnmObj* p, const NNSG3dResMdl* m)
{
    int arm_r1, arm_r2, wrist_r, wrist_r_end;
    int i;

    // Obtain the node ID of the right arm section.
    NNS_G3D_GET_JNTID(m, &arm_r1, "arm_r1");
    NNS_G3D_GET_JNTID(m, &arm_r2, "arm_r2");
    NNS_G3D_GET_JNTID(m, &wrist_r, "wrist_r");
    NNS_G3D_GET_JNTID(m, &wrist_r_end, "wrist_r_end");

    SDK_ASSERT(arm_r1 >= 0 && arm_r2 >= 0 && wrist_r >=0 && wrist_r_end >= 0);

    for (i = 0; i < m->info.numNode; ++i)
    {
        if (i != arm_r1  &&
            i != arm_r2  &&
            i != wrist_r &&
            i != wrist_r_end)
        {
            //
            // Disable the animations except for the right arm.
            //
            NNS_G3dAnmObjDisableID(p, i);
        }
    }
}


/* -------------------------------------------------------------------------
    anmObjLegL

    Configures an animation object so that when it is manipulated only the left leg is moved.
   ------------------------------------------------------------------------- */
static void anmObjLegL(NNSG3dAnmObj* p, const NNSG3dResMdl* m)
{
    int leg_l1, leg_l2, foot_l, foot_l_end;
    int i;

    // Obtain the node ID of the left leg section.
    NNS_G3D_GET_JNTID(m, &leg_l1, "leg_l1");
    NNS_G3D_GET_JNTID(m, &leg_l2, "leg_l2");
    NNS_G3D_GET_JNTID(m, &foot_l, "foot_l");
    NNS_G3D_GET_JNTID(m, &foot_l_end, "foot_l_end");

    SDK_ASSERT(leg_l1 >= 0 && leg_l2 >= 0 && foot_l >= 0 && foot_l_end >= 0);

    for (i = 0; i < m->info.numNode; ++i)
    {
        if (i != leg_l1 &&
            i != leg_l2 &&
            i != foot_l &&
            i != foot_l_end)
        {
            //
            // Disable the animations except for the left leg.
            //
            NNS_G3dAnmObjDisableID(p, i);
        }
    }
}


/* -------------------------------------------------------------------------
    anmObjLegR

    Configures an animation object so that when it is manipulated only the right leg is moved.
   ------------------------------------------------------------------------- */
static void anmObjLegR(NNSG3dAnmObj* p, const NNSG3dResMdl* m)
{
    int leg_r1, leg_r2, foot_r, foot_r_end;
    int i;

    // Obtain the node ID of the right leg section.
    NNS_G3D_GET_JNTID(m, &leg_r1, "leg_r1");
    NNS_G3D_GET_JNTID(m, &leg_r2, "leg_r2");
    NNS_G3D_GET_JNTID(m, &foot_r, "foot_r");
    NNS_G3D_GET_JNTID(m, &foot_r_end, "foot_r_end");

    SDK_ASSERT(leg_r1 >= 0 && leg_r2 >= 0 && foot_r >= 0 && foot_r_end >= 0);

    for (i = 0; i < m->info.numNode; ++i)
    {
        if (i != leg_r1 &&
            i != leg_r2 &&
            i != foot_r &&
            i != foot_r_end)
        {
            //
            // Disable the animations except for the right leg.
            //
            NNS_G3dAnmObjDisableID(p, i);
        }
    }
}


/* -------------------------------------------------------------------------
    anmObjMisc

    Configures an animation object so that when it is manipulated only parts other than left arm, right arm, left leg and right leg are moved.
    
   ------------------------------------------------------------------------- */
static void anmObjMisc(NNSG3dAnmObj* p, const NNSG3dResMdl* m)
{
    int human_all, body_model, skl_root, hip, spin1, spin2, head, head_end;
    int i;

    // Obtain the node IDs of the parts other than the left arm, right arm, left leg or right leg.
    NNS_G3D_GET_JNTID(m, &human_all, "human_all");
    NNS_G3D_GET_JNTID(m, &body_model, "body_model");
    NNS_G3D_GET_JNTID(m, &skl_root, "skl_root");
    NNS_G3D_GET_JNTID(m, &hip, "hip");
    NNS_G3D_GET_JNTID(m, &spin1, "spin1");
    NNS_G3D_GET_JNTID(m, &spin2, "spin2");
    NNS_G3D_GET_JNTID(m, &head, "head");
    NNS_G3D_GET_JNTID(m, &head_end, "head_end");

    SDK_ASSERT(human_all >= 0 && body_model >= 0 && skl_root >= 0 && hip >= 0 &&
               spin1 >= 0 && spin2 >= 0 && head >= 0 && head_end >= 0);

    for (i = 0; i < m->info.numNode; ++i)
    {
        if (i != human_all && i != body_model &&
            i != skl_root && i != hip &&
            i != spin1 && i != spin2 &&
            i != head && i != head_end)
        {
            //
            // Disable animation of left arm, right arm, left leg and right leg.
            //
            NNS_G3dAnmObjDisableID(p, i);
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
    void*           pAnmRes;
    NNSFndAllocator allocator;
    
    NNSG3dAnmObj*   pAnmObjArmL;
    NNSG3dAnmObj*   pAnmObjArmR;
    NNSG3dAnmObj*   pAnmObjLegL;
    NNSG3dAnmObj*   pAnmObjLegR;
    NNSG3dAnmObj*   pAnmObjMisc;

    BOOL armL = TRUE;
    BOOL armR = TRUE;
    BOOL legL = TRUE;
    BOOL legR = TRUE;

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

            {
                // Allocate the animation object to use
                pAnmObjArmL = NNS_G3dAllocAnmObj(&allocator, pAnmRes, pModel);
                SDK_NULL_ASSERT(pAnmObjArmL);
                pAnmObjArmR = NNS_G3dAllocAnmObj(&allocator, pAnmRes, pModel);
                SDK_NULL_ASSERT(pAnmObjArmR);
                pAnmObjLegL = NNS_G3dAllocAnmObj(&allocator, pAnmRes, pModel);
                SDK_NULL_ASSERT(pAnmObjLegL);
                pAnmObjLegR = NNS_G3dAllocAnmObj(&allocator, pAnmRes, pModel);
                SDK_NULL_ASSERT(pAnmObjLegR);
                pAnmObjMisc = NNS_G3dAllocAnmObj(&allocator, pAnmRes, pModel);
                SDK_NULL_ASSERT(pAnmObjMisc);
            }

            {
                // Initialize the animation object to use
                NNS_G3dAnmObjInit(pAnmObjArmL, pAnmRes, pModel, NULL);
                NNS_G3dAnmObjInit(pAnmObjArmR, pAnmRes, pModel, NULL);
                NNS_G3dAnmObjInit(pAnmObjLegL, pAnmRes, pModel, NULL);
                NNS_G3dAnmObjInit(pAnmObjLegR, pAnmRes, pModel, NULL);
                NNS_G3dAnmObjInit(pAnmObjMisc, pAnmRes, pModel, NULL);
            }

            {
                // Set the animation objects so that only a part of the body will move.
                anmObjArmL(pAnmObjArmL, pModel);
                anmObjArmR(pAnmObjArmR, pModel);
                anmObjLegL(pAnmObjLegL, pModel);
                anmObjLegR(pAnmObjLegR, pModel);
                anmObjMisc(pAnmObjMisc, pModel);
            }

            {
                // Add each AnmObj to RenderObj.
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjArmL );
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjArmR );
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjLegL );
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjLegR );
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjMisc );
            }
        }
    }
    
    //
    // Demo Common Initialization
    //
    G3DDemo_InitCamera(&gCamera, 10*FX32_ONE, 16*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "PartialAnm2");

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
        // Since each of the objects' initial frames are distinct, the body's movement is disjointed.
        // 
        //
        pAnmObjArmL->frame += FX32_ONE;
        if( pAnmObjArmL->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObjArmL))
            pAnmObjArmL->frame = 0;

        pAnmObjArmR->frame += FX32_ONE;
        if( pAnmObjArmR->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObjArmR))
            pAnmObjArmR->frame = 0;

        pAnmObjLegL->frame += FX32_ONE;
        if( pAnmObjLegL->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObjLegL))
            pAnmObjLegL->frame = 0;

        pAnmObjLegR->frame += FX32_ONE;
        if( pAnmObjLegR->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObjLegR))
            pAnmObjLegR->frame = 0;

        pAnmObjMisc->frame += FX32_ONE;
        if( pAnmObjMisc->frame >= NNS_G3dAnmObjGetNumFrame(pAnmObjMisc))
            pAnmObjMisc->frame = 0;

        // A Button turns the left leg ON/OFF
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (legL)
            {
                legL = FALSE;
                NNS_G3dRenderObjRemoveAnmObj( &object, pAnmObjLegL );
            }
            else
            {
                legL = TRUE;
                pAnmObjLegL->frame = 0;
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjLegL );
            }
        }

        // B Button turns the right leg ON/OFF
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_B))
        {
            if (legR)
            {
                legR = FALSE;
                NNS_G3dRenderObjRemoveAnmObj( &object, pAnmObjLegR );
            }
            else
            {
                legR = TRUE;
                pAnmObjLegR->frame = 0;
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjLegR );
            }
        }

        // X button turns the left arm ON/OFF
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_X))
        {
            if (armL)
            {
                armL = FALSE;
                NNS_G3dRenderObjRemoveAnmObj( &object, pAnmObjArmL );
            }
            else
            {
                armL = TRUE;
                pAnmObjArmL->frame = 0;
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjArmL );
            }
        }

        // Y Button turns the right arm ON/OFF
        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_Y))
        {
            if (armR)
            {
                armR = FALSE;
                NNS_G3dRenderObjRemoveAnmObj( &object, pAnmObjArmR );
            }
            else
            {
                armR = TRUE;
                pAnmObjArmR->frame = 0;
                NNS_G3dRenderObjAddAnmObj( &object, pAnmObjArmR );
            }
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

        // Display ON/OFF status of the four limb animations
        if (armL)
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_GREEN, "Left  Arm(X): ON ");
        else
            G3DDemo_Printf(0, 1, G3DDEMO_COLOR_GREEN, "Left  Arm(X): OFF");

        if (armR)
            G3DDemo_Printf(0, 2, G3DDEMO_COLOR_GREEN, "Right Arm(Y): ON ");
        else
            G3DDemo_Printf(0, 2, G3DDEMO_COLOR_GREEN, "Right Arm(Y): OFF");

        if (legL)
            G3DDemo_Printf(0, 3, G3DDEMO_COLOR_GREEN, "Left  Leg(A): ON ");
        else
            G3DDemo_Printf(0, 3, G3DDEMO_COLOR_GREEN, "Left  Leg(A): OFF");

        if (legR)
            G3DDemo_Printf(0, 4, G3DDEMO_COLOR_GREEN, "Right Leg(B): ON ");
        else
            G3DDemo_Printf(0, 4, G3DDEMO_COLOR_GREEN, "Right Leg(B): OFF");

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
