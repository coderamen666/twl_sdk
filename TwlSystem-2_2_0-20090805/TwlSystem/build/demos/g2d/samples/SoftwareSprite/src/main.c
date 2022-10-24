/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - SoftwareSprite
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
//      Software sprite render sample
//      The render method can be switched and the change in process efficiency can be verified.
//       
//      The displayed process time is the time it takes to render a small sprite.
//       (ignoring large central sprites)
//
//  Using the Demo
//      B Button: Change the render method
// ============================================================================

#include <stdlib.h>
#include <nitro.h>
#include <nnsys/g2d/g2d_Softsprite.h>
#include <nnsys/g2d/g2d_Load.h>

#include "g2d_demolib.h"


#define SPRITE_ROT_2PI          0x10000 // Software sprite rotation corresponding to 2 pi radians.
#define SPRITE_ALPHA_MAX        31      // Maximum alpha value for software sprite
#define SCREEN_WIDTH            256     // Screen width
#define SCREEN_HEIGHT           192     // Screen height

#define NUM_OF_SIMPLE_SPRITE    512     // Number of simple sprites
#define SPL_SPRITE_SIZE         8       // Size of simple sprite
#define EXT_SPRITE_SIZE         96      // Size of extended sprite
#define TEXTURE_SIZE            8       // Size of texture

#define ROT_DX                  0xFF    // Increment in x direction rotation
#define ROT_DY                  0x7F    // Increment in y direction rotation
#define ROT_DZ                  0xBF    // Increment in z direction rotation

#define SPRITE_ROT_PI_PER_2     (SPRITE_ROT_2PI / 4)

#define TEX_BASE                0x0     // Texture base address
#define TEX_PLTT_BASE           0x0     // Texture palette base address
#define TEX_PLTT_NO             0x2     // Base palette texture number


//------------------------------------------------------------------------------
// Render Method
typedef enum MyDrawType
{
    MyDrawType_DrawDirect = 0,  // Function to render while directly specifying parameters
    MyDrawType_DrawSprite,      // Normal rendering
    MyDrawType_DrawSpriteFast,  // Fast rendering without storing the current matrix
    MyDrawType_MAX

}MyDrawType;

const char* MyDrawTypeStr [] =
{
    "DirectParams",
    "Normal      ",
    "Fast        ",
};


//------------------------------------------------------------------------------
// Sprite object with speed data

typedef struct SimpleSpriteObject
{
    NNSG2dSimpleSprite sprite;

    struct Speed
    {
        short x;
        short y;
    } speed;
} SimpleSpriteObject;



//------------------------------------------------------------------------------
// Prototype Declarations

void VBlankIntr(void);

static inline BOOL IsInvisibleRot(u16 rot, u16 margin);
static inline short GetSpeedRandom(void);
static inline u8 GetAlphaRandom(void);

static void LoadResources(void);
static void MoveSimpleSpriteObject(SimpleSpriteObject *pSSObj);

static void SetSpriteDefaultSetting(void);
static void InitBallObject(SimpleSpriteObject objects[]);
static void InitSquareSprite(NNSG2dExtendedSprite *pes);
static void InitSoftwareSprite(void);


//------------------------------------------------------------------------------
// Global Variables

static SimpleSpriteObject          sBalls[NUM_OF_SIMPLE_SPRITE];    // Simple sprite
static NNSG2dExtendedSprite        sSquare;                         // Extended sprite

static NNSG2dImageAttr texAttr_ =       // Common texture settings
{
    GX_TEXSIZE_S256,        // S size
    GX_TEXSIZE_T256,        // T size
    GX_TEXFMT_PLTT16,       // Texture format
    FALSE,                  // Whether extended palette used
    GX_TEXPLTTCOLOR0_TRNS,  // Enable transparent color
    GX_OBJVRAMMODE_CHAR_2D  // Character mapping type
};


MyDrawType                      sDrawType = MyDrawType_DrawDirect; // Render Method

/*---------------------------------------------------------------------------*
  Name:         IsInvisibleRot

  Description:  Considers the difference to determine whether the rotation angle, rot, will result in the software sprite facing directly sideways, either at 90 or 270 degrees.
                
                

  Arguments:    rot:    The angle of rotation to be assessed
                margin: Error allowed in evaluation.

  Returns:      Returns TRUE if rotation angle makes invisible; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static inline BOOL IsInvisibleRot(u16 rot, u16 margin)
{
    return ( (SPRITE_ROT_PI_PER_2 - margin) < rot && rot < (SPRITE_ROT_PI_PER_2 + margin) )
        || ( (SPRITE_ROT_PI_PER_2 * 3 - margin) < rot && rot < (SPRITE_ROT_PI_PER_2* 3 + margin) );
}

/*---------------------------------------------------------------------------*
  Name:         GetSpeedRandom

  Description:  Gets a random speed.

  Arguments:    None.

  Returns:      A randomly determined speed.
 *---------------------------------------------------------------------------*/
static inline short GetSpeedRandom(void)
{
    return (short)((rand() & 0x3) + 1);
}

/*---------------------------------------------------------------------------*
  Name:         GetAlphaRandom

  Description:  Gets a random alpha value.

  Arguments:    None.

  Returns:      A randomly determined alpha value.
 *---------------------------------------------------------------------------*/
static inline u8 GetAlphaRandom(void)
{
    return (u8)(rand() & 0x1F);
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Reads character and palette data from a file and loads them to the VRAM.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources(void)
{
    NNSG2dImageProxy imgProxy;
    NNSG2dImagePaletteProxy pltProxy;
    void* pBuf;

    NNS_G2dInitImageProxy( &imgProxy );
    NNS_G2dInitImagePaletteProxy( &pltProxy );

    //------------------------------------------------------------------------------
    // Load character data
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/SoftwareSprite.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        NNS_G2dLoadImage2DMapping(
            pCharData,
            TEX_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &imgProxy );

        // The buffer is deallocated because the character data was copied into VRAM.
        // 
        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // Load palette data
    {
        NNSG2dPaletteData* pPltData;

        pBuf = G2DDemo_LoadNCLR( &pPltData, "data/SoftwareSprite.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        NNS_G2dLoadPalette(
            pPltData,
            TEX_PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &pltProxy);

        G2DDemo_Free( pBuf );
    }

}

/*---------------------------------------------------------------------------*
  Name:         SetSpriteDefaultSetting

  Description:  Sets the default attributes for the sprite.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetSpriteDefaultSetting(void)
{
    NNSG2dExtendedSprite      defaultSetting;

    defaultSetting.uvUL.x = 0 << FX32_SHIFT;
    defaultSetting.uvLR.x = TEXTURE_SIZE << FX32_SHIFT;
    defaultSetting.uvUL.y = 0 << FX32_SHIFT;
    defaultSetting.uvLR.y = TEXTURE_SIZE << FX32_SHIFT;
    defaultSetting.flipH = FALSE;
    defaultSetting.flipV = FALSE;

    defaultSetting.rotO.x = 0;
    defaultSetting.rotO.y = 0;
    defaultSetting.rotX = 0;
    defaultSetting.rotY = 0;

    defaultSetting.basicParams.pTextureAttr = &texAttr_;
    defaultSetting.basicParams.texAddr      = TEX_BASE;
    defaultSetting.basicParams.plttAddr     = TEX_PLTT_BASE;
    defaultSetting.basicParams.plttOffset   = TEX_PLTT_NO;
    defaultSetting.basicParams.color        = GX_RGB( 0, 0, 0 );

    // Specify default value.
    // Values that are set here will be the defaults to be references when rendering subsequent NNSG2dSimpleSprite and NNSG2dBasicSprite objects that lack information.
    // 
    NNS_G2dSetSpriteDefaultAttr( &defaultSetting );
}

/*---------------------------------------------------------------------------*
  Name:         InitBallObject

  Description:  Initializes SimpleSpriteObject.

  Arguments:    pObj:  Pointer to SimpleSpriteObject to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitBallObject(SimpleSpriteObject *pObj)
{
    NNSG2dSimpleSprite *sprite = &(pObj->sprite);

    pObj->speed.x = GetSpeedRandom();
    pObj->speed.y = GetSpeedRandom();

    sprite->pos.x     = 0;
    sprite->pos.y     = 0;
    sprite->size.x    = SPL_SPRITE_SIZE;
    sprite->size.y    = SPL_SPRITE_SIZE;
    sprite->rotZ      = 0;
    sprite->priority  = 0;
    sprite->alpha     = GetAlphaRandom();
}

/*---------------------------------------------------------------------------*
  Name:         InitSquareSprite

  Description:  Initialize extended sprite for quadrilateral display.

  Arguments:    pes:  Pointer to NNSG2dExtendedSprite to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitSquareSprite(NNSG2dExtendedSprite *pes)
{
    NNSG2dSimpleSprite *simpleParams = &(pes->basicParams.simpleParams);

    // Initialization using the default values.
    *pes = *NNS_G2dGetSpriteDefaultAttr();

    pes->uvUL.x = TEXTURE_SIZE << FX32_SHIFT;
    pes->uvLR.x = (TEXTURE_SIZE * 2) << FX32_SHIFT;
    pes->uvUL.y = 0 << FX32_SHIFT;
    pes->uvLR.y = TEXTURE_SIZE << FX32_SHIFT;

    pes->rotO.x = - EXT_SPRITE_SIZE/2;                  // 
    pes->rotO.y = - EXT_SPRITE_SIZE/2;                  // Sets the sprite's upper left corner as the center of rotation.

    simpleParams->pos.x     = SCREEN_WIDTH/2;
    simpleParams->pos.y     = SCREEN_HEIGHT/2;
    simpleParams->size.x    = EXT_SPRITE_SIZE;
    simpleParams->size.y    = EXT_SPRITE_SIZE;
    simpleParams->rotZ      = 0;
    simpleParams->priority  = 0;
    simpleParams->alpha     = SPRITE_ALPHA_MAX;
}

/*---------------------------------------------------------------------------*
  Name:         InitSoftwareSprite

  Description:  Initialize software sprite environment and data.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitSoftwareSprite(void)
{
    G2DDemo_CameraSetup();
    G2DDemo_MaterialSetup();

    // Specify attributes that are valid when rendering.
    // Performance can be improved by specifying unnecessary attributes as disabled.
    NNS_G2dSetSpriteAttrEnable(
        NNS_G2D_SPRITEATTR_ALPHA  |
        NNS_G2D_SPRITEATTR_UV     |
        NNS_G2D_SPRITEATTR_TEXTURE|
        NNS_G2D_SPRITEATTR_ROTXY  |
        NNS_G2D_SPRITEATTR_ROTO
    );
    
    SetSpriteDefaultSetting();

    {
        int i;
        for( i = 0; i < NUM_OF_SIMPLE_SPRITE; i++ )
        {
            InitBallObject(&sBalls[i]);
        }
    }
    InitSquareSprite(&sSquare);
}


/*---------------------------------------------------------------------------*
  Name:         MoveSimpleSpriteObject

  Description:  Moves the SimpleSpriteObject according to its speed.
                Also changes reflection at the screen edge and the alpha value for reflection.

  Arguments:    pSSObj:  Object to be moved.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MoveSimpleSpriteObject(SimpleSpriteObject *pSSObj)
{
    BOOL bReflected = FALSE;
    NNSG2dSVec2 *pPos = &(pSSObj->sprite.pos);
    struct Speed *pSpeed = &(pSSObj->speed);

    // Update position
    pPos->x += pSpeed->x;
    pPos->y += pSpeed->y;

    // Reflect at screen edge
    {
        if( pPos->x < 0 || (SCREEN_WIDTH - SPL_SPRITE_SIZE) <= pPos->x )
        {
            pPos->x -= pSpeed->x;
            pSpeed->x = (short)((pSpeed->x < 0) ? GetSpeedRandom(): - GetSpeedRandom());
            bReflected = TRUE;
        }

        if( pPos->y < 0 || (SCREEN_HEIGHT - SPL_SPRITE_SIZE) <= pPos->y )
        {
            pPos->y -= pSpeed->y;
            pSpeed->y = (short)((pSpeed->y < 0) ? GetSpeedRandom(): - GetSpeedRandom());
            bReflected = TRUE;
        }
    }

    // Change alpha value for reflection
    if( bReflected )
    {
        pSSObj->sprite.alpha = GetAlphaRandom();
    }
}


/*---------------------------------------------------------------------------*
  Name:         DrawDirect

  Description:  Rendering using a function for which the parameters are directly specified.
                Since the render function only specifies a UV value and renders a textured quad, the user must configure the 3D graphics engine.
                
                This is the fastest operation.
  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawDirect()
{
    G3_PushMtx();
    {
        int i;
        //------------------------------------------------------------------------------
        // 3D Graphics Engine: Set texture parameters
        {
            NNSG2dImageAttr*    pAttr = &texAttr_;
            
            G3_TexImageParam( pAttr->fmt,    // use alpha texture
                 GX_TEXGEN_TEXCOORD,         // use texcoord
                 pAttr->sizeS,               // 16 pixels
                 pAttr->sizeT,               // 16 pixels
                 GX_TEXREPEAT_NONE,          // no repeat
                 GX_TEXFLIP_NONE,            // no flip
                 pAttr->plttUse,             // use color 0 of the palette
                 TEX_BASE                    // the offset of the texture image
                 );
            
            if( NNS_G2dIsPaletteImageFmt( pAttr ) )
            {
                G3_TexPlttBase( (u32)( TEX_PLTT_BASE + 32 * TEX_PLTT_NO), pAttr->fmt );                                
            }
        }
        
        //------------------------------------------------------------------------------
        // Render: Function that takes parameters directly as arguments for rendering
        for( i = 0;i < NUM_OF_SIMPLE_SPRITE; i++ )
        {
            //
            // Does not render if the alpha value is zero
            //
            if( sBalls[i].sprite.alpha == 0 )
            {
                continue;
            }
            
            //
            // Alpha value setting
            //
            G3_PolygonAttr( GX_LIGHTMASK_NONE,       // disable lights
                            GX_POLYGONMODE_MODULATE, // modulation mode
                            GX_CULL_NONE,            // cull back
                            0,                       // polygon ID(0 - 63)
                            sBalls[i].sprite.alpha,  // alpha(0 - 31)
                            0                        // OR of GXPolygonAttrMisc's value
                            );
            
            
            G3_Identity();
            NNS_G2dDrawSpriteFast( sBalls[i].sprite.pos.x, sBalls[i].sprite.pos.y, -1, 
                                   SPL_SPRITE_SIZE, SPL_SPRITE_SIZE,
                                   0, 0, TEXTURE_SIZE , TEXTURE_SIZE );
        }
    }
    G3_PopMtx(1); 
}

/*---------------------------------------------------------------------------*
  Name:         DrawNormal

  Description:  Render using normal render functions
  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawNormal()
{
    //
    // Render using normal render functions
    //
    // Sets attributes
    NNS_G2dSetSpriteAttrEnable( NNS_G2D_SPRITEATTR_ALPHA | NNS_G2D_SPRITEATTR_TEXTURE );
    NNS_G2dDrawSpriteSimple( &(sBalls[0].sprite) );
    // Enables only the parameters that change for each sprite
    NNS_G2dSetSpriteAttrEnable( NNS_G2D_SPRITEATTR_ALPHA );
    
    {
        int i;
        for( i = 0;i < NUM_OF_SIMPLE_SPRITE; i++ )
        {
            G3_Identity();
            NNS_G2dDrawSpriteSimple( &(sBalls[i].sprite) );
        }
    }
    
    
}

/*---------------------------------------------------------------------------*
  Name:         DrawFast

  Description:  Renders with the NNS_G2dDrawSprite*Fast function.
                Operates quickly because the current matrix is not saved before or after calling the render function.
  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawFast()
{
    G3_PushMtx();
        
        {
            int i;
            
            NNS_G2dSetSpriteAttrEnable( NNS_G2D_SPRITEATTR_ALPHA | NNS_G2D_SPRITEATTR_TEXTURE );
            NNS_G2dDrawSpriteSimple( &(sBalls[0].sprite) );
            // Enables only the parameters that change for each sprite
            NNS_G2dSetSpriteAttrEnable( NNS_G2D_SPRITEATTR_ALPHA );
    
            for( i = 0;i < NUM_OF_SIMPLE_SPRITE; i++ )
            {
                G3_Identity();
                NNS_G2dDrawSpriteSimpleFast( &(sBalls[i].sprite) );
            }
        }
        
        
    
    G3_PopMtx(1);
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Initialize App.
    {
        G2DDemo_CommonInit();
        LoadResources();
        InitSoftwareSprite();
        G2DDemo_PrintInit();
    }

    // Start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    //
    // Main loop
    //
    while( TRUE )
    {
        static u16 rotX = 0x0;
        static u16 rotY = 0x0;
        static u16 rotZ = 0x0;
        static u16 plttNo = TEX_PLTT_NO;
        static OSTick time;
        

        //
        // Button-Based Controls
        //
        {
            G2DDemo_ReadGamePad();
            
            if ( G2DDEMO_IS_TRIGGER(PAD_BUTTON_B) )
            {
                sDrawType++;
                sDrawType %= MyDrawType_MAX;
            }
        }   
        
    
        // Draw
        {
            time = OS_GetTick();   
                switch( sDrawType )
                {
                case MyDrawType_DrawDirect:     DrawDirect();  break;
                case MyDrawType_DrawSprite:     DrawNormal();  break;
                case MyDrawType_DrawSpriteFast: DrawFast();    break;
                }
            time = OS_GetTick() - time;
            
            // draw extended sprite
            NNS_G2dSetSpriteAttrEnable(
                NNS_G2D_SPRITEATTR_ALPHA  |
                NNS_G2D_SPRITEATTR_UV     |
                NNS_G2D_SPRITEATTR_TEXTURE|
                NNS_G2D_SPRITEATTR_ROTXY  |
                NNS_G2D_SPRITEATTR_ROTO );
                
            sSquare.rotX = rotX;
            sSquare.rotY = rotY;
            sSquare.basicParams.simpleParams.rotZ = rotZ;
            sSquare.basicParams.plttOffset = plttNo;

            NNS_G2dDrawSpriteExtended( &sSquare );
        }
        
        {
            // Update sprite position
            int i;
            for( i = 0;i < NUM_OF_SIMPLE_SPRITE; i++ )
            {
                MoveSimpleSpriteObject(&sBalls[i]);
            }
        
            // update animation values
            rotX += ROT_DX;
            rotY += ROT_DY;
            rotZ += ROT_DZ;
            
            if( IsInvisibleRot(rotX, ROT_DX/2) || IsInvisibleRot(rotY, ROT_DY/2) )
            {
                plttNo = (u16)(rand() & 0x7);
            }
        }
        //
        // Display debug information
        // 
        {
            G2DDemo_PrintOutf( 0, 21, "TIME:%06ld usec\n", OS_TicksToMicroSeconds(time) );
            G2DDemo_PrintOut ( 0, 22, MyDrawTypeStr[sDrawType] );
        }
        
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();
        
        // Display information
        G2DDemo_PrintApplyToHW();
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

