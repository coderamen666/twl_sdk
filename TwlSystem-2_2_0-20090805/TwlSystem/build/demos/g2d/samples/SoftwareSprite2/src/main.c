/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - SoftwareSprite2
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
//      Dynamically changes the texture IV values and modulation color on a rotating software sprite.
//      
//
//  Using the Demo
//      +Control Pad Up & Down: Selects RGB
//      +Control Pad L & R: Changes RGB values (changes modulation color)
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


#define SCREEN_WIDTH        256                     // screen width
#define SCREEN_HEIGHT       192                     // screen height

#define SPRITE_ALPHA_MAX    31                      // Maximum alpha value


// Sprite size (square)
#define SPRITE_SIZE         128

// Color bar size
#define BAR_HEIGHT          6
#define BAR_WIDTH_UNIT      6

// Color bar position (x-coordinate)
#define BAR_POS_RX          45
#define BAR_POS_GX          BAR_POS_RX
#define BAR_POS_BX          BAR_POS_RX

// Color bar position (y-coordinate)
#define LINE_HEIGHT         8
#define LINE_MARGIN         ((LINE_HEIGHT - BAR_HEIGHT) / 2)
#define BAR_POS_RY          (BAR_POS_GY - LINE_HEIGHT)
#define BAR_POS_GY          (BAR_POS_BY - LINE_HEIGHT)
#define BAR_POS_BY          (SCREEN_HEIGHT - LINE_HEIGHT + LINE_MARGIN)

// Color
#define COLOR_RED           GX_RGB(31, 0, 0)
#define COLOR_GREEN         GX_RGB(0, 31, 0)
#define COLOR_BLUE          GX_RGB(0, 0, 31)

// Speed of UV value change
#define MOPH_SPEED1         -(FX32_ONE >> 3)
#define MOPH_SPEED2         -(FX32_ONE >> 2)
#define MOPH_SPEED3         (FX32_ONE >> 1)
#define MOPH_SPEED4         (FX32_ONE >> 0)

// Range of UV value change
#define MOPH_MIN            0
#define MOPH_MAX            (SPRITE_SIZE << FX32_SHIFT)
#define MOPH_CENTER         ((MOPH_MAX - MOPH_MIN) / 2)

// Sprite rotation speed
#define ROT_UNIT            0x3F

// Texture address, etc.
#define TEX_BASE            0
#define TEX_PLTT_BASE       0
#define TEX_PLTT_NO         0

// Number of color bars
#define NUM_OF_BAR          3


//------------------------------------------------------------------------------
// Structure definitions

// Color data structure to hold RGB data separately
typedef struct RGB
{
    u8 r;
    u8 g;
    u8 b;
} RGB;

//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Prototype Declarations

static void LoadResources(NNSG2dImageProxy* pImgProxy, NNSG2dImagePaletteProxy* pPltProxy);
static void SetSpriteDefaultSetting(
                NNSG2dImageProxy* pImgProxy,
                const NNSG2dImagePaletteProxy* pPltProxy);
static void InitSoftwareSpriteEnv(
                NNSG2dImageProxy* pImgProxy,
                const NNSG2dImagePaletteProxy* pPltProxy);
static void InitSprite( NNSG2dExtendedSprite* pSprite );
static void InitBar( NNSG2dBasicSprite* pBar, GXRgb color, s16 x, s16 y );
static void SetColorBarValue( NNSG2dBasicSprite* pBar, u8 value );
static void BounceValue( fx32* v, fx32* dv, fx32 min, fx32 max );
static void ProcessInput(RGB* color, s8* pos);



/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Loads textures and texture palettes and stores them in VRAM.
                
                Also initializes proxy.

  Arguments:    pImgProxy:  Image proxy that takes texture VRAM information.
                pPltProxy:  Palette proxy that takes texture palette VRAM information.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources(NNSG2dImageProxy* pImgProxy, NNSG2dImagePaletteProxy* pPltProxy)
{
    void* pBuf;
    SDK_NULL_ASSERT( pImgProxy );
    SDK_NULL_ASSERT( pPltProxy );

    // Initialize proxy
    NNS_G2dInitImageProxy( pImgProxy );
    NNS_G2dInitImagePaletteProxy( pPltProxy );

    //------------------------------------------------------------------------------
    // load character data
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/SoftwareSprite2.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        NNS_G2dLoadImage2DMapping(
            pCharData,
            TEX_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            pImgProxy );

        // Deallocate the buffer because the character data was copied into VRAM.
        // 
        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPltData;

        pBuf = G2DDemo_LoadNCLR( &pPltData, "data/SoftwareSprite2.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        NNS_G2dLoadPalette(
            pPltData,
            TEX_PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            pPltProxy);

        G2DDemo_Free( pBuf );
    }
}



/*---------------------------------------------------------------------------*
  Name:         SetSpriteDefaultSetting

  Description:  Sets default value for sprite.

  Arguments:    pImgProxy:  Image proxy for setting sprite default.
                pPltProxy:  Palette proxy for setting sprite default.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetSpriteDefaultSetting(
                NNSG2dImageProxy* pImgProxy,
                const NNSG2dImagePaletteProxy* pPltProxy)
{
    NNSG2dExtendedSprite      defaultSetting;
    SDK_NULL_ASSERT( pImgProxy );
    SDK_NULL_ASSERT( pPltProxy );

    // Disable UV, flip
    defaultSetting.uvUL.x = 0 << FX32_SHIFT;
    defaultSetting.uvLR.x = 0 << FX32_SHIFT;
    defaultSetting.uvUL.y = 0 << FX32_SHIFT;
    defaultSetting.uvLR.y = 0 << FX32_SHIFT;
    defaultSetting.flipH = FALSE;
    defaultSetting.flipV = FALSE;

    // No rotation
    defaultSetting.rotO.x = 0;
    defaultSetting.rotO.y = 0;
    defaultSetting.rotX = 0;
    defaultSetting.rotY = 0;

    // Texture settings
    defaultSetting.basicParams.pTextureAttr = &(pImgProxy->attr);
    defaultSetting.basicParams.texAddr      = NNS_G2dGetImageLocation(pImgProxy, NNS_G2D_VRAM_TYPE_3DMAIN);
    defaultSetting.basicParams.plttAddr     = NNS_G2dGetImagePaletteLocation(pPltProxy, NNS_G2D_VRAM_TYPE_3DMAIN);
    defaultSetting.basicParams.plttOffset   = TEX_PLTT_NO;
    defaultSetting.basicParams.color        = GX_RGB( 0, 0, 0 );

    // Specify default value.
    // Values that are set here will be the defaults to be references when rendering subsequent NNSG2dSimpleSprite and NNSG2dBasicSprite objects that lack information.
    // 
    NNS_G2dSetSpriteDefaultAttr( &defaultSetting );
}



/*---------------------------------------------------------------------------*
  Name:         InitSoftwareSpriteEnv

  Description:  Specifies settings for software sprite display.

  Arguments:    pImgProxy:  Image proxy for setting sprite default.
                pPltProxy:  Palette proxy for setting sprite default.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitSoftwareSpriteEnv(
                NNSG2dImageProxy* pImgProxy,
                const NNSG2dImagePaletteProxy* pPltProxy)
{
    SDK_NULL_ASSERT( pImgProxy );
    SDK_NULL_ASSERT( pPltProxy );

    NNS_G2dSetSpriteAttrEnable(
        NNS_G2D_SPRITEATTR_TEXTURE |
        NNS_G2D_SPRITEATTR_UV      |
        NNS_G2D_SPRITEATTR_COLOR
    );

    SetSpriteDefaultSetting(pImgProxy, pPltProxy);
}



/*---------------------------------------------------------------------------*
  Name:         InitSprite

  Description:  Initialize sprite for demo.

  Arguments:    pSprite:    Pointer to sprite to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitSprite( NNSG2dExtendedSprite* pSprite )
{
    NNSG2dSimpleSprite *simpleParams = &(pSprite->basicParams.simpleParams);
    SDK_NULL_ASSERT( pSprite );

    // Initialization using the default values.
    *pSprite = *NNS_G2dGetSpriteDefaultAttr();

    pSprite->uvUL.x = MOPH_CENTER;
    pSprite->uvLR.x = MOPH_CENTER;
    pSprite->uvUL.y = MOPH_CENTER;
    pSprite->uvLR.y = MOPH_CENTER;


    simpleParams->pos.x     = (SCREEN_WIDTH - SPRITE_SIZE)/2;
    simpleParams->pos.y     = (SCREEN_HEIGHT - SPRITE_SIZE)/2;
    simpleParams->size.x    = SPRITE_SIZE;
    simpleParams->size.y    = SPRITE_SIZE;
    simpleParams->rotZ      = 0;
    simpleParams->priority  = 0;
    simpleParams->alpha     = SPRITE_ALPHA_MAX;
}



/*---------------------------------------------------------------------------*
  Name:         InitBar

  Description:  Initializes sprite to be displayed as color bar.

  Arguments:    pBar:   Pointer to sprite to be initialized.
                color:  Sprite color.
                x:      x-coordinate of position for sprite display.
                y:      y-coordinate of position for sprite display.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitBar( NNSG2dBasicSprite* pBar, GXRgb color, s16 x, s16 y )
{
    NNSG2dSimpleSprite *simpleParams = &(pBar->simpleParams);
    SDK_NULL_ASSERT( pBar );

    // Initialization using the default values.
    *pBar = NNS_G2dGetSpriteDefaultAttr()->basicParams;

    pBar->color = color;

    simpleParams->pos.x     = x;
    simpleParams->pos.y     = y;
    simpleParams->size.x    = 0;        // Specify with SetColorBarValue()
    simpleParams->size.y    = BAR_HEIGHT;
    simpleParams->rotZ      = 0;
    simpleParams->priority  = 1;        // Display before sprite for demo
    simpleParams->alpha     = SPRITE_ALPHA_MAX;
}



/*---------------------------------------------------------------------------*
  Name:         SetColorBarValue

  Description:  Change length of sprite displayed as color bar.

  Arguments:    pBar:   Pointer to sprite whose length is to be changed.
                value:  Size of color component corresponding to pBar.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetColorBarValue( NNSG2dBasicSprite* pBar, u8 value )
{
    SDK_NULL_ASSERT( pBar );
    SDK_ASSERT( 0 <= value && value <= 31 );

    pBar->simpleParams.size.x = (s16)(BAR_WIDTH_UNIT * value);
}



/*---------------------------------------------------------------------------*
  Name:         BounceValue

  Description:  When *dv is added to *v, reverses the sign of *dv if min <= *v <= max is not true.
                
                Calling this function every frame causes *v to go back and forth within the range defined by min and max.

  Arguments:    v:      Pointer to value.
                dv:     Pointer to value increment.
                min:    Minimum value.
                max:    Maximum value.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BounceValue( fx32* v, fx32* dv, fx32 min, fx32 max )
{
    SDK_NULL_ASSERT( v );
    SDK_NULL_ASSERT( dv );
    SDK_MINMAX_ASSERT(*v, min, max);

    *v += *dv;

    if( *v < min )
    {
        *dv = - *dv;
        *v = min;
    }
    if( max < *v )
    {
        *dv = - *dv;
        *v = max;
    }
}



/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Changes modulation color in response to key input.

  Arguments:    color:  Pointer to modulation color.
                pos:    Pointer to number of color bar being manipulated.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(RGB* color, s8* pos)
{
    G2DDemo_ReadGamePad();
    SDK_NULL_ASSERT( color );
    SDK_NULL_ASSERT( pos );

    // Change the color bar to manipulate
    if( G2DDEMO_IS_TRIGGER(PAD_KEY_UP) )
    {
        (*pos)--;

        if( *pos < 0 )
        {
            *pos = NUM_OF_BAR - 1;
        }
    }
    if( G2DDEMO_IS_TRIGGER(PAD_KEY_DOWN) )
    {
        (*pos)++;

        if( *pos >= NUM_OF_BAR )
        {
            *pos = 0;
        }
    }

    // Color bar operation
    {
        u8* value;

        switch( *pos ){
        case 0: value = &(color->r); break;
        case 1: value = &(color->g); break;
        case 2: value = &(color->b); break;
        default: return;
        }

        if( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
        {
            if( *value < 31 )
            {
	            (*value)++;
            }
        }
        if( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
        {
            if( *value > 0 )
            {
	            (*value)--;
            }
        }
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
    NNSG2dExtendedSprite    sprite;             // Sprite for demo
    NNSG2dBasicSprite       barR, barG, barB;   // Color bar
    NNSG2dImageProxy        imgProxy;           // Image proxy
    NNSG2dImagePaletteProxy pltProxy;           // Palette proxy
    GXTexFmt                orgTexFmt;          // Original format of texture

    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();

        LoadResources(&imgProxy, &pltProxy);
        InitSoftwareSpriteEnv(&imgProxy, &pltProxy);
        InitSprite(&sprite);
        InitBar(&barR, COLOR_RED, BAR_POS_RX, BAR_POS_RY);
        InitBar(&barG, COLOR_GREEN, BAR_POS_GX, BAR_POS_GY);
        InitBar(&barB, COLOR_BLUE, BAR_POS_BX, BAR_POS_BY);

        // Record original texture format
        orgTexFmt = imgProxy.attr.fmt;
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
        static RGB modColor = { 31, 31, 31 };   // Modulation color
        static s8 pointerPos = 0;               // Color being manipulated
        static fx32 lx = MOPH_SPEED1;           // Speed of uvUL.x change
        static fx32 ly = MOPH_SPEED2;           // Speed of uvUL.y change
        static fx32 rx = MOPH_SPEED3;           // Speed of uvLR.x  change
        static fx32 ry = MOPH_SPEED4;           // Speed of uvLR.y  change


        // input handling
        {
            ProcessInput(&modColor, &pointerPos);

            // update color bar length
            SetColorBarValue(&barR, modColor.r);
            SetColorBarValue(&barG, modColor.g);
            SetColorBarValue(&barB, modColor.b);
        }

        // update SoftwareSprite parameter
        {
            // Set modulation color
            sprite.basicParams.color = GX_RGB(modColor.r, modColor.g, modColor.b);

            // Change UV value
            BounceValue( &(sprite.uvUL.x), &lx, MOPH_MIN, MOPH_CENTER );
            BounceValue( &(sprite.uvUL.y), &ly, MOPH_MIN, MOPH_CENTER );
            BounceValue( &(sprite.uvLR.x), &rx, MOPH_CENTER, MOPH_MAX );
            BounceValue( &(sprite.uvLR.y), &ry, MOPH_CENTER, MOPH_MAX );

            // rotation
            sprite.basicParams.simpleParams.rotZ += ROT_UNIT;
        }

        // draw SoftwareSprite
        {
            // Set texture format to original format.
            // Enable texture.
            imgProxy.attr.fmt = orgTexFmt;
            NNS_G2dDrawSpriteExtended( &sprite );

            // Set texture format to none.
            // Disable texture.
            // We can't disable texture with NNS_G2dSetSpriteAttrEnable.
            imgProxy.attr.fmt = GX_TEXFMT_NONE;
            NNS_G2dDrawSpriteBasic( &barR );
            NNS_G2dDrawSpriteBasic( &barG );
            NNS_G2dDrawSpriteBasic( &barB );
        }

        // Render display information
        {
            G2DDemo_PrintOutf(0, 0, "UL: (%4d, %4d)",
                sprite.uvUL.x >> FX32_SHIFT, sprite.uvUL.y >> FX32_SHIFT);
            G2DDemo_PrintOutf(0, 1, "LR: (%4d, %4d)",
                sprite.uvLR.x >> FX32_SHIFT, sprite.uvLR.y >> FX32_SHIFT);

            G2DDemo_PrintOutf(19, 0, "rot: %5d",
                sprite.basicParams.simpleParams.rotZ);
            G2DDemo_PrintOutf(19, 1, "     (%3.f)",
                (float)(sprite.basicParams.simpleParams.rotZ * 360) / 0x10000);

            G2DDemo_PrintOutf(0, 21, " R:%2d", modColor.r);
            G2DDemo_PrintOutf(0, 22, " G:%2d", modColor.g);
            G2DDemo_PrintOutf(0, 23, " B:%2d", modColor.b);
            G2DDemo_PrintOut(0, 21 + pointerPos, "*");
        }


        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();


        // Reflect BG
        {
            G2DDemo_PrintApplyToHW();
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
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );                   // Checking VBlank interrupt
}


