/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - textdemolib - include - g2d_textdemolib
  File:     txt.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef TEXT_H_
#define TEXT_H_

#include <nnsys/g2d/g2d_Font.h>

#ifdef __cplusplus
extern "C" {
#endif

// Upper left alignment on DrawText
#define TXT_DRAWTEXT_FLAG_DEFAULT   (NNS_G2D_VERTICALORIGIN_TOP | NNS_G2D_HORIZONTALORIGIN_LEFT | NNS_G2D_HORIZONTALALIGN_LEFT)

// Font resource names for the demo
#define TXT_FONTRESOURCE_NAME               "/data/fonts.NFTR"
#define TXT_SJIS_FONTRESOURCE_NAME          "/data/fonts.NFTR"
#define TXT_UTF8_FONTRESOURCE_NAME          "/data/fontu8.NFTR"
#define TXT_UTF16_FONTRESOURCE_NAME         "/data/fontu16.NFTR"
#define TXT_CP1252_FONTRESOURCE_NAME        "/data/font1252.NFTR"



// TXTColorPalette color name, assume loading it to the 16-color palette
enum
{
    // palette 0 TXT_CPALETTE_MAIN
    TXT_COLOR_NULL=0,
    TXT_COLOR_WHITE,
    TXT_COLOR_BLACK,
    TXT_COLOR_RED,
    TXT_COLOR_GREEN,
    TXT_COLOR_BLUE,
    TXT_COLOR_CYAN,
    TXT_COLOR_MAGENTA,
    TXT_COLOR_YELLOW,

    // palette 1 TXT_CPALETTE_USERCOLOR
    TXT_UCOLOR_NULL=0,
    TXT_UCOLOR_GRAY,
    TXT_UCOLOR_BROWN,
    TXT_UCOLOR_RED,
    TXT_UCOLOR_PINK,
    TXT_UCOLOR_ORANGE,
    TXT_UCOLOR_YELLOW,
    TXT_UCOLOR_LIMEGREEN,
    TXT_UCOLOR_DARKGREEN,
    TXT_UCOLOR_SEAGREEN,
    TXT_UCOLOR_TURQUOISE,
    TXT_UCOLOR_BLUE,
    TXT_UCOLOR_DARKBLUE,
    TXT_UCOLOR_PURPLE,
    TXT_UCOLOR_VIOLET,
    TXT_UCOLOR_MAGENTA,

    // palette TXT_CPALETTE_4BPP
    TXT_COLOR_4BPP_NULL=0,
    TXT_COLOR_4BPP_BG=1,
    TXT_COLOR_4BPP_TEXT=1
};

// Assumed to be loaded to the palette name 16-color palette of TXTColorPalette
enum
{
    TXT_CPALETTE_MAIN,
    TXT_CPALETTE_USERCOLOR,
    TXT_CPALETTE_4BPP,
    TXT_NUM_CPALEETE
};

// Shared color palette data
extern GXRgb TXTColorPalette[TXT_NUM_CPALEETE * 16];



//****************************************************************************
//
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         TXT_Init

  Description:  Shared initialization of the sample demos.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_Init(void);



/*---------------------------------------------------------------------------*
  Name:         TXT_SetupBackground

  Description:  Loads and displays the background image to the main screen BG0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_SetupBackground( void );



/*---------------------------------------------------------------------------*
  Name:         TXT_Alloc

  Description:  Allocates memory.

  Arguments:    size:   The size of the memory to allocate.

  Returns:      Pointer to the allocated memory region.
 *---------------------------------------------------------------------------*/
void* TXT_Alloc(u32 size);



/*---------------------------------------------------------------------------*
  Name:         TXT_Free

  Description:  Deallocates the memory allocated with TXT_Alloc().

  Arguments:    ptr:    Pointer to the memory region to deallocate.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_Free(void* ptr);



/*---------------------------------------------------------------------------*
  Name:         TXT_SetCharCanvasOBJAttrs

  Description:  Configures all the parameters not configured by NNS_G2dArrangeOBJ* of successive OAMs.
                

  Arguments:    oam:        Top of target OAMs
                num:        Number of target OAMs
                priority:   Display priority (0 to 3)
                mode:       OBJ Mode
                mosaic:     Whether the OAM has mosaic.
                effect:     Effect type.
                cParam:     Color parameter
                rsParam:    Affine transformation parameter index

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_SetCharCanvasOBJAttrs(
    GXOamAttr * oam,
    int         num,
    int         priority,
    GXOamMode   mode,
    BOOL        mosaic,
    GXOamEffect effect,
    int         cParam,
    int         rsParam
);



/*---------------------------------------------------------------------------*
  Name:         TXT_LoadFont

  Description:  Loads a font from a file to memory.

  Arguments:    pFname: The font resource path.

  Returns:      The pointer to the loaded font.
 *---------------------------------------------------------------------------*/
void TXT_LoadFont( NNSG2dFont* pFont, const char* pFname );



/*---------------------------------------------------------------------------*
  Name:         TXT_LoadFile

  Description:  Loads a file to memory.

  Arguments:    ppFile: Pointer to the buffer that receives the memory address where the file was loaded.
                        
                fpath:  The path of the file to load.

  Returns:      Returns the file size of the loaded file.
                If the file size is 0, this indicates that the file failed to load.
                In that case, the *ppFile value is invalid.
 *---------------------------------------------------------------------------*/
u32 TXT_LoadFile(void** ppFile, const char* fpath);





#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // TEXT_H_

