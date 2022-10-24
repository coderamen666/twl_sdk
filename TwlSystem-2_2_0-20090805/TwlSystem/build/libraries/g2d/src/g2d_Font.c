/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Font.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/load/g2d_NFT_load.h>
#include <nnsys/g2d/fmt/g2d_Font_data.h>
#include <nnsys/g2d/g2d_config.h>



//****************************************************************************
// Local Functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         GetGlyphIndex

  Description:  Gets the glyph index that corresponds to c from the character code map block.
                

  Arguments:    pMap:   character code map block of the font
                c:      character code

  Returns:      Glyph index that corresponds to c.
                Returns NNS_G2D_GLYPH_INDEX_NOT_FOUND if one does not exist.
 *---------------------------------------------------------------------------*/
static u16 GetGlyphIndex(const NNSG2dFontCodeMap* pMap, u16 c)
{
    u16 index = NNS_G2D_GLYPH_INDEX_NOT_FOUND;

    NNS_G2D_POINTER_ASSERT( pMap );
    NNS_G2D_ASSERT(pMap->ccodeBegin <= c && c <= pMap->ccodeEnd);

    switch( pMap->mappingMethod )
    {
    //-----------------------------------------------------------
    // index = character code - offset
    case NNS_G2D_MAPMETHOD_DIRECT:
        {
            u16 offset = pMap->mapInfo[0];
            index = (u16)(c - pMap->ccodeBegin + offset);
        }
        break;

    //-----------------------------------------------------------
    // index  = table[character code - character code offset]
    case NNS_G2D_MAPMETHOD_TABLE:
        {
            const int table_index = c - pMap->ccodeBegin;

            index = pMap->mapInfo[table_index];
        }
        break;

    //-----------------------------------------------------------
    // index  = binary search (character code)
    case NNS_G2D_MAPMETHOD_SCAN:
        {
            const NNSG2dCMapInfoScan* const ws = (NNSG2dCMapInfoScan*)(pMap->mapInfo);
            const NNSG2dCMapScanEntry* st = &(ws->entries[0]);
            const NNSG2dCMapScanEntry* ed = &(ws->entries[ws->num - 1]);

            while( st <= ed )
            {
                const NNSG2dCMapScanEntry* md = st + (ed - st) / 2;

                if( md->ccode < c )
                {
                    st = md + 1;
                }
                else if( c < md->ccode )
                {
                    ed = md - 1;
                }
                else
                {
                    index = md->index;
                    break;
                }
            }
        }
        break;

    //-----------------------------------------------------------
    // unknown
    default:
        NNS_G2D_ASSERTMSG(FALSE, "unknwon MAPMETHOD");
    }

    return index;
}



/*---------------------------------------------------------------------------*
  Name:         GetCharWidthsFromIndex

  Description:  Gets the character width information corresponding to glyph index from the character width block.
                

  Arguments:    pWidth: Pointer to the character width block of the font.
                idx:    glyph index

  Returns:      character width information corresponding to idx
                The operations are undefined when idx is an index not included in pWidth.
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE const NNSG2dCharWidths* GetCharWidthsFromIndex( const NNSG2dFontWidth* const pWidth, int idx )
{
    NNS_G2D_POINTER_ASSERT( pWidth );
    NNS_G2D_ASSERT(pWidth->indexBegin <= idx && idx <= pWidth->indexEnd);

    return (NNSG2dCharWidths*)(pWidth->widthTable) + (idx - pWidth->indexBegin);
}





//*******************************************************************************
// Global Functions
//*******************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontInit*

  Description:  Builds the font from the font resource.

  Arguments:    pFont:      Pointer to the font.
                pNftrFile:  Pointer to the font resource.

  Returns:      Returns a value other than FALSE if the font build is successful.
 *---------------------------------------------------------------------------*/

void NNS_G2dFontInitAuto(NNSG2dFont* pFont, void* pNftrFile)
{
    const static NNSiG2dSplitCharCallback spliterTable[] =
    {
        NNSi_G2dSplitCharUTF8,
        NNSi_G2dSplitCharUTF16,
        NNSi_G2dSplitCharShiftJIS,
        NNSi_G2dSplitChar1Byte
    };
    NNSG2dFontEncoding encoding;
    BOOL result;

    NNS_G2D_POINTER_ASSERT( pFont );
    NNS_G2D_POINTER_ASSERT( pNftrFile );

    result = NNSi_G2dGetUnpackedFont(pNftrFile, &(pFont->pRes));
    NNS_G2D_ASSERTMSG(result, "Faild to unpack font.");
    encoding = (NNSG2dFontEncoding)pFont->pRes->encoding;
    NNS_G2D_MINMAX_ASSERT( encoding, 0, NNS_G2D_NUM_OF_ENCODING - 1 );

    pFont->cbCharSpliter = spliterTable[encoding];
}


#define NNS_G2D_DEFINE_FONT_INIT(name, enc, spliter)                        \
    void NNS_G2dFontInit##name(NNSG2dFont* pFont, void* pNftrFile)          \
    {                                                                       \
        BOOL result;                                                        \
        NNS_G2D_POINTER_ASSERT( pFont );                                    \
        NNS_G2D_POINTER_ASSERT( pNftrFile );                                \
                                                                            \
        result = NNSi_G2dGetUnpackedFont(pNftrFile, &(pFont->pRes));        \
        NNS_G2D_ASSERTMSG(result, "Faild to unpack font.");                 \
        NNS_G2D_ASSERT(  (NNSG2dFontEncoding)pFont->pRes->encoding          \
                                    == NNS_G2D_FONT_ENCODING_##enc );       \
                                                                            \
        pFont->cbCharSpliter = NNSi_G2dSplitChar##spliter;                  \
    }

NNS_G2D_DEFINE_FONT_INIT(UTF8, UTF8, UTF8)
NNS_G2D_DEFINE_FONT_INIT(UTF16, UTF16, UTF16)
NNS_G2D_DEFINE_FONT_INIT(ShiftJIS, SJIS, ShiftJIS)
NNS_G2D_DEFINE_FONT_INIT(CP1252, CP1252, 1Byte)



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontFindGlyphIndex

  Description:  Gets the supported glyph index from a character code.

  Arguments:    pFont:  Pointer to the font.
                c:      Character code that gets the glyph index.

  Returns:      If a glyph corresponding to c exists, returns its index.
                Returns NNS_G2D_GLYPH_INDEX_NOT_FOUND if one does not exist.
 *---------------------------------------------------------------------------*/
u16 NNS_G2dFontFindGlyphIndex( const NNSG2dFont* pFont, u16 c )
{
    const NNSG2dFontCodeMap* pMap;

    NNS_G2D_FONT_ASSERT(pFont);

    pMap = pFont->pRes->pMap;

    // linear search for the CMAP block list
    while( pMap != NULL )
    {
        if( (pMap->ccodeBegin <= c) && (c <= pMap->ccodeEnd) )
        {
            return GetGlyphIndex(pMap, c);
        }

        pMap = pMap->pNext;
    }

    // not found
    return NNS_G2D_GLYPH_INDEX_NOT_FOUND;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontGetCharWidthsFromIndex

  Description:  Gets the character width information from the glyph index.

  Arguments:    pFont:  Pointer to the font.
                idx:    glyph index

  Returns:      When character width information corresponding to idx exists, a pointer to that information stored within the font is returned.
                
                Returns the pointer to the default width information if it doesn't exist.
 *---------------------------------------------------------------------------*/
const NNSG2dCharWidths* NNS_G2dFontGetCharWidthsFromIndex( const NNSG2dFont* pFont, u16 idx )
{
    const NNSG2dFontWidth* pWidth;

    NNS_G2D_FONT_ASSERT(pFont);

    pWidth = pFont->pRes->pWidth;

    // linear search width information block list
    while( pWidth != NULL )
    {
        if( (pWidth->indexBegin <= idx) && (idx <= pWidth->indexEnd) )
        {
            return GetCharWidthsFromIndex(pWidth, idx);

        }

        pWidth = pWidth->pNext;
    }

    // Return default if not found.
    return &(pFont->pRes->defaultWidth);
}















//----------------------------------------------------------------------------
// Get size.
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontGetStringWidth

  Description:  Gets the width when one line of a character string is rendered.
                I.e., looks for the text string width when rendering from txt or up to the terminal character.
                

  Arguments:    pTxn:   pointer to the text screen structure
                txt:    pointer to the NULL terminated character string
                pPos:   A pointer to the buffer storing the pointer to the character after the first carriage return, when txt includes a carriage return.
                        
                        NULL is stored when txt does not contain a line feed character.
                        NULL can be specified here if unnecessary.

  Returns:      txt width
 *---------------------------------------------------------------------------*/
int NNSi_G2dFontGetStringWidth(
    const NNSG2dFont* pFont,
    int hSpace,
    const void* str,
    const void** pPos
)
{
    int width = 0;
    const void* pos = str;
    u16 c;
    NNSiG2dSplitCharCallback getNextChar;

    NNS_G2D_FONT_ASSERT( pFont );
    NNS_G2D_POINTER_ASSERT( str );

    getNextChar = NNSi_G2dFontGetSpliter(pFont);

    while( (c = getNextChar((const void**)&pos)) != 0 )
    {
        if( c == '\n' )
        {
            break;
        }

        width += NNS_G2dFontGetCharWidth(pFont, c) + hSpace;
    }

    if( pPos != NULL )
    {
        *pPos = (c == '\n') ? pos: NULL;
    }
    if( width > 0 )
    {
        width -= hSpace;
    }
    return width;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontGetTextHeight

  Description:  Calculates the height when a character string containing a line feed is rendered.

  Arguments:    pTxn:   pointer to the text screen structure
                txt:    pointer to the NULL terminated character string

  Returns:      txt height
 *---------------------------------------------------------------------------*/
int NNSi_G2dFontGetTextHeight(
    const NNSG2dFont* pFont,
    int vSpace,
    const void* txt
)
{
    const void* pos = txt;
    int lines = 1;
    NNSG2dTextRect rect = {0, 0};
    u16 c;
    NNSiG2dSplitCharCallback getNextChar;

    NNS_G2D_FONT_ASSERT( pFont );
    NNS_G2D_POINTER_ASSERT( txt );

    getNextChar = NNSi_G2dFontGetSpliter(pFont);

    while( (c = getNextChar((const void**)&pos)) != 0 )
    {
        if( c == '\n' )
        {
            lines++;
        }
    }

    return lines * (NNS_G2dFontGetLineFeed(pFont) + vSpace) - vSpace;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontGetTextWidth

  Description:  Calculates the width when a character string is rendered.
                This value will become the maximum width of each line.

  Arguments:    pTxn:   pointer to the font that is the basis for the calculation
                txt:    String.

  Returns:      The width when the character string is rendered.
 *---------------------------------------------------------------------------*/
int NNSi_G2dFontGetTextWidth(
    const NNSG2dFont* pFont,
    int hSpace,
    const void* txt
)
{
    int width = 0;

    NNS_G2D_FONT_ASSERT( pFont );
    NNS_G2D_POINTER_ASSERT( txt );

    while( txt != NULL )
    {
        const int line_width = NNSi_G2dFontGetStringWidth(pFont, hSpace, txt, &txt);
        if( line_width > width )
        {
            width = line_width;
        }
    }

    return width;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFontGetTextRect

  Description:  Calculates the maximum width and height when a character string containing a line feed is rendered.

  Arguments:    pTxn:   pointer to the text screen structure
                txt:    pointer to the NULL terminated character string

  Returns:      width and height of the smallest rectangle that can surround txt
 *---------------------------------------------------------------------------*/
NNSG2dTextRect NNSi_G2dFontGetTextRect(
    const NNSG2dFont* pFont,
    int hSpace,
    int vSpace,
    const void* txt
)
{
    int lines = 1;
    NNSG2dTextRect rect = {0, 0};

    NNS_G2D_FONT_ASSERT( pFont );
    NNS_G2D_POINTER_ASSERT( txt );

    while( txt != NULL )
    {
        const int width = NNSi_G2dFontGetStringWidth(pFont, hSpace, txt, &txt);
        if( width > rect.width )
        {
            rect.width = width;
        }
        lines++;
    }

    // height  = number of lines x (line height + line space) - line space
    rect.height = ((lines - 1) * (NNS_G2dFontGetLineFeed(pFont) + vSpace) - vSpace);

    return rect;
}

