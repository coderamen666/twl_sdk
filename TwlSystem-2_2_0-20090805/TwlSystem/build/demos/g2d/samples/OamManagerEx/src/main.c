/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - OamManagerEx
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/



// ============================================================================
//  Explanation of the demo:
//      Using extended OAM manager,
//      - More than 128 OBJs are displayed using time-division.
//      - OBJs are displayed according to priority by specifying a rendering order for the OBJs.
//
//  Using the Demo
//      +Control Pad Left/Right: Moves character horizontally.
//      +Control Pad Up/Down: Moves character forward and backward.
//      A: Increases the number of OBJs.
//      B: Decreases the number of OBJs.
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

// Size of screen
#define SCREEN_WIDTH            256
#define SCREEN_HEIGHT           192

#define HW_OAM_USE              128     // Number of HW OAMs to use

#define INIT_NUM_OF_LAYER       10      // Initial number of bars arranged from front to back
#define MAX_NUM_OF_LAYER        27      // Maximum number of bars arranged from front to back
#define NUM_OF_LANE             10      // Number of bars arranged from left to right

#define NUM_OF_OAM_CHUNK        (MAX_NUM_OF_LAYER * NUM_OF_LANE + 1)
#define NUM_OF_CHUNK_LIST       (MAX_NUM_OF_LAYER + 1)
#define NUM_OF_AFFINE_PROXY     1

// OBJ size of character and bar
#define BAR_WIDTH               32
#define BAR_HEIGHT              16
#define CHARA_WIDTH             32
#define CHARA_HEIGHT            32

// Difference between the bar in front-back direction and the display position
#define BAR_SHIFT_X             12      // Shift 12 in x direction
#define BAR_SHIFT_Y             6       // Shift 6 more in y direction

// Difference between the bar in left-right bar and the display position
#define LANE_SHIFT              (BAR_WIDTH + 24)

// Size of smallest rectangle encompassing all bars
#define DRAW_AREA_WIDTH         (BAR_SHIFT_X * (MAX_NUM_OF_LAYER - 1) + LANE_SHIFT * (NUM_OF_LANE - 1) + BAR_WIDTH)
#define DRAW_AREA_HEIGHT        (BAR_SHIFT_Y * (MAX_NUM_OF_LAYER - 1) + BAR_HEIGHT)

// Display position of lower-leftmost bar
// Display positions of other bars are calculated from this value.
#define BAR_OFFSET_X            ((SCREEN_WIDTH - DRAW_AREA_WIDTH) / 2)
#define BAR_OFFSET_Y            (((SCREEN_HEIGHT + DRAW_AREA_HEIGHT) / 2) + 10)

// Initial display position of character
// Use the lower-leftmost bar as the origin.
#define CHARA_INIT_POS_X        ((LANE_SHIFT * (NUM_OF_LANE - 1) + BAR_WIDTH) / 2)
#define CHARA_INIT_POS_Y        (- (((BAR_SHIFT_Y * (MAX_NUM_OF_LAYER - 1)) * BAR_SHIFT_X / BAR_SHIFT_Y) / 2))


// Load address offset for resource
#define CHARA_BASE              0x0000
#define PLTT_BASE               0x0000



//-----------------------------------------------------------------------------
// Structure Definitions

// The structure that indicates position
typedef struct Position
{
    s16 x;
    s16 y;
} Position;



//-----------------------------------------------------------------------------
// Global Variables

// Chunk
// The container for the list storing the OAM.
static NNSG2dOamChunk           sOamChunk[NUM_OF_OAM_CHUNK];

// Chunk List
// Maintains a chunk list for each display priority level.
// The same number of lists are required as the number of display priority levels used.
static NNSG2dOamChunkList       sOamChunkList[NUM_OF_CHUNK_LIST];

// Affine Parameter Proxy
// Extends the AffineIndex determination until affine parameters are stored and reflected in the hardware.
// Not used in this demo.
//static NNSG2dAffineParamProxy   sAffineProxy[NUM_OF_AFFINE_PROXY];


//-----------------------------------------------------------------------------
// Prototype Declarations

static u16 CallBackGetOamCapacity(void);
static void InitOamManagerEx(NNSG2dOamManagerInstanceEx* pExMan );
static void LoadResources(void);
static void InitTemplateOam( GXOamAttr* pChara, GXOamAttr* pBar );
static const Position*  GetPosition(void);
void NitroMain(void);
void VBlankIntr(void);



/*---------------------------------------------------------------------------*
  Name:         CallBackGetOamCapacity

  Description:  Callback invoked from the extended OAM manager; conveys the number of OAMs available for use in the extended OAM manager.
                

  Arguments:    None.

  Returns:      Number of available OAMs.
 *---------------------------------------------------------------------------*/
static u16 CallBackGetOamCapacity(void)
{
    return HW_OAM_USE;
}

/*---------------------------------------------------------------------------*
  Name:         CallBackEntryNewOam

  Description:  This function is called back from the extended OAM manager and outputs the OAMs.

  Arguments:    pOam:   Pointer to the OAM to be output.
                index:  OBJ number of the pOam.

  Returns:      If the OAM output succeeded or not.
 *---------------------------------------------------------------------------*/
static BOOL CallBackEntryNewOam(const GXOamAttr* pOam, u16 index)
{
    const u32 szByte = sizeof( GXOamAttr );

    DC_FlushRange( (void*)pOam, szByte );
    GX_LoadOAM( pOam, index * szByte, szByte );

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         InitOamManagerEx

  Description:  Initialize the extended OAM manager instance.

  Arguments:    pExMan:     Pointer to the extended OAM manager instance to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManagerEx(NNSG2dOamManagerInstanceEx* pExMan )
{
    BOOL bSuccess;
    NNSG2dOamExEntryFunctions funcs;

    // If only the extended OAM manager is used,
    // NNS_G2dInitOamManagerModule() 
    // does not need to be called.
    // When using the OAM manager in an implementation of the extended OAM manager callback function, run the NNS_G2dInitOamManagerModule function.
    // 

    // Initialize the extended OAM manager instance
    bSuccess = NNS_G2dGetOamManExInstance(
                    pExMan,
                    sOamChunkList,
                    (u8)NUM_OF_CHUNK_LIST,
                    NUM_OF_OAM_CHUNK,
                    sOamChunk,
                    0,                      // Affine parameters are not managed
                    NULL);                  // Affine parameters are not managed

    SDK_ASSERT( bSuccess );
    
    // Set the functions to be registered to the extended OAM manager.
    // As the extended OAM manager does not have a default registration function, this function must be run in advance to configure the registration function.
    // 
    {
        // Register the group of functions called back by NNS_G2dApplyOamManExToBaseModule().
        // 
        funcs.getOamCapacity    = CallBackGetOamCapacity;
        funcs.entryNewOam       = CallBackEntryNewOam;
        funcs.getAffineCapacity = NULL;
        funcs.entryNewAffine    = NULL;
        
        NNS_G2dSetOamManExEntryFunctions( pExMan, &funcs );
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Reads character and palette data from a file and loads it to the VRAM.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources(void)
{
    void* pBuf;

    //-------------------------------------------------------------------------
    // Load character data for 2D
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/OamManagerEx.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading for 2D Graphics Engine
        DC_FlushRange( pCharData->pRawData, pCharData->szByte );
        GX_LoadOBJ( pCharData->pRawData, CHARA_BASE, pCharData->szByte );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
        G2DDemo_Free( pBuf );
    }

    //-------------------------------------------------------------------------
    // Load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/OamManagerEx.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading for 2D Graphics Engine
        DC_FlushRange( pPlttData->pRawData, pPlttData->szByte );
        GX_LoadOBJPltt( pPlttData->pRawData, PLTT_BASE, pPlttData->szByte );

        G2DDemo_Free( pBuf );
    }
}



/*---------------------------------------------------------------------------*
  Name:         InitTemplateOam

  Description:  Creates an OAM to use as the template for OBJ to be displayed.

  Arguments:    pChara:     Pointer to OAM to be used as template for character OBJ.
                pBar:       Pointer to OAM to be used as template for bar OBJ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitTemplateOam( GXOamAttr* pChara, GXOamAttr* pBar )
{
    // Template for character moved by +Control Pad
    G2_SetOBJAttr(
        pChara,                 // OAM data
        0,                      // x-coordinate
        0,                      // y-coordinate
        0,                      // Display priority level
        GX_OAM_MODE_NORMAL,     // OBJ Mode
        FALSE,                  // Mosaic
        GX_OAM_EFFECT_NONE,     // Effect
        GX_OAM_SHAPE_32x32,     // Shape
        GX_OAM_COLORMODE_16,    // Color mode
        0x204,                  // Character name
        0x8,                    // Color parameter
        0);                     // Affine index

    // Template for the bar which will be placed in many locations
    G2_SetOBJAttr(
        pBar,                   // OAM data
        0,                      // x-coordinate
        0,                      // y-coordinate
        0,                      // Display priority level
        GX_OAM_MODE_NORMAL,     // OBJ Mode
        FALSE,                  // Mosaic
        GX_OAM_EFFECT_NONE,     // Effect
        GX_OAM_SHAPE_32x16,     // Shape
        GX_OAM_COLORMODE_16,    // Color mode
        0x200,                  // Character name
        0xD,                    // Color parameter
        0);                     // Affine index
}



/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Reads pad input and changes character position and number of bar layers.
                

  Arguments:    pPos:       Pointer to the position where input is reflected in the character position.
                            
                pNumLayer:  Pointer to s8 where input is reflected as the number of bar layers.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(Position* pPos, s8* pNumLayer)
{
    G2DDemo_ReadGamePad();

    // Increase or decrease number of bars
    if( G2DDEMO_IS_PRESS( PAD_BUTTON_A ) )
    {
        (*pNumLayer)++;
    }
    if( G2DDEMO_IS_PRESS( PAD_BUTTON_B ) )
    {
        (*pNumLayer)--;
    }
    if( *pNumLayer > MAX_NUM_OF_LAYER )
    {
        *pNumLayer = MAX_NUM_OF_LAYER;
    }
    if( *pNumLayer < 0 )
    {
        *pNumLayer = 0;
    }

    // Move character
    if( G2DDEMO_IS_PRESS( PAD_KEY_LEFT ) )
    {
        pPos->x--;
    }
    if( G2DDEMO_IS_PRESS( PAD_KEY_RIGHT ) )
    {
        pPos->x++;
    }
    if( G2DDEMO_IS_PRESS( PAD_KEY_UP ) )
    {
        pPos->y--;
    }
    if( G2DDEMO_IS_PRESS( PAD_KEY_DOWN ) )
    {
        pPos->y++;
    }
}



/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    NNSG2dOamManagerInstanceEx  oamManagerEx;   // Extended OAM manager
    GXOamAttr chara;                            // Character template OAM
    GXOamAttr bar;                              // Bar template OAM


    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        LoadResources();
        InitOamManagerEx( &oamManagerEx );
        InitTemplateOam( &chara, &bar );
    }

    // start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }


    // Main loop
    while( TRUE )
    {
        // Position of character
        static Position pos = {CHARA_INIT_POS_X, CHARA_INIT_POS_Y};

        static s8 numLayer = INIT_NUM_OF_LAYER; // Number of bar layers
        u8 charaPriority;                       // Character display priority
        int numObjDrawn = 0;                    // Number of OBJs displayed
        static OSTick          timeForEntery, timeForApply;;
        


        // Input process
        {
            ProcessInput(&pos, &numLayer);
        }


        // Calculate character display priority
        {
            charaPriority = (u8)((- pos.y + BAR_SHIFT_X) / BAR_SHIFT_X);

            if( charaPriority >= MAX_NUM_OF_LAYER )
            {
                charaPriority = MAX_NUM_OF_LAYER;
            }
        }


        //
        // Register with OAM manager
        //
        {
            BOOL bSuccess;
            int i, r;

            // Register character with OAM manager
            {
                G2_SetOBJPosition(
                    &chara,
                    BAR_OFFSET_X + pos.x - pos.y,
                    BAR_OFFSET_Y - CHARA_HEIGHT + (pos.y * BAR_SHIFT_Y) / BAR_SHIFT_X
                );

                // Register together with display priority
                bSuccess = NNS_G2dEntryOamManExOam( &oamManagerEx, &chara, charaPriority );
                SDK_ASSERT( bSuccess );

                numObjDrawn++;
            }

            // Register bar with OAM manager
            for( i = 0; i < numLayer; ++i )
            {
                int x, y;               // Display position of bar
                u8 priority = (u8)i;    // Display priority of bar

                // Ensure that priority does not conflict with the character display priority
                // The smaller the value, the higher the priority (=displayed first)
                if( priority >= charaPriority )
                {
                    priority++;
                }

                // Off-screen items not displayed (y)
                y = BAR_OFFSET_Y - BAR_HEIGHT - i * BAR_SHIFT_Y;

                if( y < - BAR_HEIGHT || SCREEN_HEIGHT <= y )
                {
                    continue;
                }

                timeForEntery = OS_GetTick();
                for( r = 0; r < NUM_OF_LANE; ++r )
                {
                    // Off-screen items not displayed (x)
                    x = BAR_OFFSET_X + i * BAR_SHIFT_X + r * LANE_SHIFT;

                    if( x < - BAR_WIDTH || SCREEN_WIDTH <= x )
                    {
                        continue;
                    }

                    G2_SetOBJPosition(&bar, x, y);

                    // Register together with display priority
                    bSuccess = NNS_G2dEntryOamManExOam( &oamManagerEx, &bar, priority );
                    SDK_ASSERT( bSuccess );

                    numObjDrawn++;
                }
                timeForEntery = OS_GetTick() - timeForEntery;
            }
        }


        // Output display information
        {
            G2DDemo_PrintOutf( 0, 0, "Chara Position: %4d / %4d", pos.x, pos.y);
            G2DDemo_PrintOutf( 0, 1, "Chara Priority: %4d", charaPriority);
            G2DDemo_PrintOutf( 0, 2, "Number of OBJ:  %4d", numObjDrawn);
            
            G2DDemo_PrintOutf( 0, 3, "Entry:%06ld usec\n", OS_TicksToMicroSeconds(timeForEntery) );
            G2DDemo_PrintOutf( 0, 4, "Apply:%06ld usec\n", OS_TicksToMicroSeconds(timeForApply) );
        }


        // Wait for V-Blank
        SVC_WaitVBlankIntr();


        //
        // Write buffer contents to HW
        //
        {
            // Write display information
            G2DDemo_PrintApplyToHW();

            timeForApply = OS_GetTick();
            // Write OBJ
            NNS_G2dApplyOamManExToBaseModule( &oamManagerEx );

            // Reset the extended OAM manager.
            // Even after reset, the information relating to time division is retained.
            NNS_G2dResetOamManExBuffer( &oamManagerEx );
            timeForApply = OS_GetTick() - timeForApply;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Handles VBlank interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );    // Checking VBlank interrupt
}


