/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_CharCanvas.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/fmt/g2d_Oam_data.h>
#include <nnsys/g2d/g2d_config.h>
#include "g2di_BitReader.h"


#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8


#define MAX_TEXT_BG_CHEIGHT         64
#define MAX_AFFINE_BG_CHEIGHT       128
#define MAX_256x16PLTT_BG_CHEIGHT   128

#define MAX_TEXT_BG_CHARACTER       1024
#define MAX_AFFINE_BG_CHARACTER     256
#define MAX_256x16PLTT_BG_CHARACTER 1024

#define MAX_OBJ_CHARA_WIDTH         8
#define MAX_OBJ_CHARA_HEIGHT        8

#define MAX_OBJ1D_CHARACTER         8192
#define MAX_OBJ2DRECT_HEIGHT        32


/*---------------------------------------------------------------------------*
  Name:         LC_INFO

  Description:  These are the parameters passed to the LetterChar function.
 *---------------------------------------------------------------------------*/
typedef struct LC_INFO
{
    const u8* dst;  // pointer to the render target character
    const u8* src;  // Pointer to the glyph image
    int ofs_x;      // render offset
    int ofs_y;      // render offset
    int width;      // glyph width
    int height;     // glyph height
    int dsrc;       // number of bits in 1 line of the glyph image
    int srcBpp;     // number of bits in 1 dot of the glyph image
    int dstBpp;     // number of bits in 1 dot of the render target character
    u32 cl;         // color number
}
LC_INFO;



/*---------------------------------------------------------------------------*
  Name:         OBJ1DParam

  Description:  These are the parameters for rendering the 1D OBJ CharCanvas.
 *---------------------------------------------------------------------------*/
typedef union OBJ1DParam
{
    u32 packed;
    struct
    {
        unsigned baseWidthShift:    8;  // size of the OBJ in the upper left corner of CharCanvas
        unsigned baseHeightShift:   8;  // size of the OBJ in the upper left corner of CharCanvas
    };
}
OBJ1DParam;



/*---------------------------------------------------------------------------*
  Name:         ObjectSize

  Description:  This is the structure for the OBJ size table.
                Each value is the base-2 logarithm of the size in character units.
                In other words, (1<<widthShift)*8 shows the width in pixel units.
 *---------------------------------------------------------------------------*/
typedef struct ObjectSize
{
    u8 widthShift;      // width
    u8 heightShift;     // height
}
ObjectSize;




// virtual function table definition
static void DrawGlyphLine  (const NNSG2dCharCanvas* pCC, const NNSG2dFont* pFont, int x, int y, int cl, const NNSG2dGlyph* pGlyph);
static void DrawGlyph1D    (const NNSG2dCharCanvas* pCC, const NNSG2dFont* pFont, int x, int y, int cl, const NNSG2dGlyph* pGlyph);
static void ClearContinuous(const NNSG2dCharCanvas* pCC, int cl);
static void ClearLine      (const NNSG2dCharCanvas* pCC, int cl);
static void ClearAreaLine  (const NNSG2dCharCanvas* pCC, int cl, int x, int y, int w, int h);
static void ClearArea1D    (const NNSG2dCharCanvas* pCC, int cl, int x, int y, int w, int h);

static const NNSiG2dCharCanvasVTable VTABLE_BG =
    { DrawGlyphLine, ClearContinuous, ClearAreaLine };
static const NNSiG2dCharCanvasVTable VTABLE_OBJ1D =
    { DrawGlyph1D, ClearContinuous, ClearArea1D };
static const NNSiG2dCharCanvasVTable VTABLE_OBJ2DRECT =
    { DrawGlyphLine, ClearLine, ClearAreaLine };




//****************************************************************************
// static inline functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         GetCharacterSize

  Description:  Calculates the data size of 1 character that is the target of CharCanvas.

  Arguments:    pCC:    Pointer to CharCanvas.

  Returns:      Size of 1 character in bytes.
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE int GetCharacterSize(const NNSG2dCharCanvas* pCC)
{
    NNS_G2D_CHARCANVAS_ASSERT(pCC);

    // width  x height x bpp / (byte/bit)
    return CHARACTER_HEIGHT * CHARACTER_WIDTH * pCC->dstBpp / 8;
}



/*---------------------------------------------------------------------------*
  Name:         SpreadColor32

  Description:  Creates a 32-bit value associated with a color number corresponding to the character bpp targeted by pCC.
                

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     color number

  Returns:      A color number value spread over 4 or 8 times
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE u32 SpreadColor32(const NNSG2dCharCanvas* pCC, int cl)
{
    u32 val = (u32)cl;

    NNS_G2D_CHARCANVAS_ASSERT(pCC);

    if( pCC->dstBpp == 4 )
    {
        NNS_G2D_ASSERT( (cl & ~0xF) == 0 );

        val = (val << 4) | val;
        val |= val << 8;
        val |= val << 16;
    }
    else
    {
        NNS_G2D_ASSERT( (cl & ~0xFF) == 0 );

        val = (val << 8) | val;
        val |= val << 16;
    }

    return val;
}



/*---------------------------------------------------------------------------*
  Name:         GetMaxObjectSize

  Description:  Gets the maximum OBJ size below the specified size.

  Arguments:    w:  width in characters
                h:  height in characters

  Returns:      pointer to the structure that stores the maximum size of the OBJ
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE const ObjectSize* GetMaxObjectSize(int w, int h)
{
    const static ObjectSize objs[4][4] =
    {
        { {0, 0}, {1, 0}, {2, 0}, {2, 0} },
        { {0, 1}, {1, 1}, {2, 1}, {2, 1} },
        { {0, 2}, {1, 2}, {2, 2}, {3, 2} },
        { {0, 2}, {1, 2}, {2, 3}, {3, 3} },
    };

    int log_w = (w >= 8) ? 3: MATH_ILog2((unsigned long)w);
    int log_h = (h >= 8) ? 3: MATH_ILog2((unsigned long)h);

    NNS_G2D_MINMAX_ASSERT(log_w, 0, 3);
    NNS_G2D_MINMAX_ASSERT(log_h, 0, 3);

    return &objs[log_h][log_w];
}





//****************************************************************************
// Static Functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         ISqrt

  Description:  Calculates square root using integer values.

  Arguments:    x:  value to calculate square root for

  Returns:      square root of x
 *---------------------------------------------------------------------------*/
static int ISqrt(int x)
{
    u32 min = 1;
    u32 max = (u32)x;

    if( x <= 0 ) return 0;

    for(;;)
    {
        const u32 mid = (min + max) / 2;
        const u32 mid2 = mid * mid;

        if( mid2 < x )
        {
            if( mid == min )
            {
                break;
            }
            min = mid;
        }
        else
        {
            max = mid;
        }
    }

    return (int)min;
}


/*---------------------------------------------------------------------------*
  Name:         GetCharIndex1D

  Description:  Calculates the character number at the designated position in the 1D OBJ CharCanvas.
                Assumes the character number for the first character associated with CharCanvas is 0.
                

  Arguments:    cx:         position of the character that gets the character number
                cy:         position of the character that gets the character number
                areaWidth:  CharCanvas width
                areaHeight: CharCanvas height
                objWidth:   maximum OBJ size in the CharCanvas (logarithm)
                objHeight:  maximum OBJ size in the CharCanvas (logarithm)

  Returns:

 *---------------------------------------------------------------------------*/
static u32 GetCharIndex1D( u32 cx, u32 cy, u32 areaWidth, u32 areaHeight, u32 objWidth, u32 objHeight)
{
    const u32 fullbits = (u32)(~0);
    u32 idx = 0;

    NNS_G2D_ASSERT(cx < areaWidth);
    NNS_G2D_ASSERT(cy < areaHeight);
    NNS_G2D_MINMAX_ASSERT(objWidth, 0, 3);
    NNS_G2D_MINMAX_ASSERT(objHeight, 0, 3);

    for(;;)
    {
        const u32 objWidthMaskInv = fullbits << objWidth;
        const u32 objHeightMaskInv = fullbits << objHeight;
        const u32 areaAWidth = areaWidth & objWidthMaskInv;
        const u32 areaAHeight = areaHeight & objHeightMaskInv;

        if( areaAHeight <= cy )
        {
            idx += areaWidth * areaAHeight;

            if( areaAWidth <= cx )
            // area D
            {
                idx += (areaHeight - areaAHeight) * areaAWidth;

                cx -= areaAWidth;
                cy -= areaAHeight;
                areaWidth -= areaAWidth;
                areaHeight -= areaAHeight;
            }
            else
            // area C
            {
                cy -= areaAHeight;
                areaWidth = areaAWidth;
                areaHeight -= areaAHeight;
            }
        }
        else
        {
            const u32 objHeightMask = ~objHeightMaskInv;

            if ( areaAWidth <= cx )
            // area B
            {
                idx += (u32)(areaAWidth * areaAHeight);

                cx -= areaAWidth;
                areaWidth -= areaAWidth;
                areaHeight = areaAHeight;
            }
            else
            // area A
            {
                const u32 objWidthMask = ~objWidthMaskInv;
                idx += (u32)(cy & objHeightMaskInv) * areaAWidth;
                idx += (u32)(cx & objWidthMaskInv) << objHeight;
                idx += (u32)(cy & objHeightMask) << objWidth;
                idx += (u32)(cx & objWidthMask);
                return idx;
            }
        }

        {
            const ObjectSize* pobj = GetMaxObjectSize((int)areaWidth, (int)areaHeight);

            objWidth = pobj->widthShift;
            objHeight = pobj->heightShift;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         OBJSizeToShape

  Description:  Converts ObjectSize to GXOamShape.

  Arguments:    pSize:  ObjectSize before conversion

  Returns:      value of the GXOamShape that corresponds to pSize
 *---------------------------------------------------------------------------*/
static GXOamShape OBJSizeToShape(const ObjectSize* pSize)
{
    const static GXOamShape shape[4][4] =
    {
        {GX_OAM_SHAPE_8x8,  GX_OAM_SHAPE_16x8,  GX_OAM_SHAPE_32x8,  (GXOamShape)NULL},
        {GX_OAM_SHAPE_8x16, GX_OAM_SHAPE_16x16, GX_OAM_SHAPE_32x16, (GXOamShape)NULL},
        {GX_OAM_SHAPE_8x32, GX_OAM_SHAPE_16x32, GX_OAM_SHAPE_32x32, GX_OAM_SHAPE_64x32},
        {(GXOamShape)NULL,  (GXOamShape)NULL,   GX_OAM_SHAPE_32x64, GX_OAM_SHAPE_64x64},
    };

    NNS_G2D_POINTER_ASSERT(pSize);
    NNS_G2D_MINMAX_ASSERT(pSize->heightShift, 0, 3);
    NNS_G2D_MINMAX_ASSERT(pSize->widthShift, 0, 3);

    return shape[pSize->heightShift][pSize->widthShift];
}



/*---------------------------------------------------------------------------*
  Name:         ClearChar

  Description:  Fills in 1 character.

  Arguments:    pChar:  pointer to the character to fill
                x:      fill starting point x coordinate (character coordinate system)
                y:      fill starting point y coordinate (character coordinate system)
                w:      fill width  (character coordinate system)
                h:      fill height (character coordinate system)
                cl8:    fill color's 4- or 8-bit color number is repeated 8 or 4 times to create a 32-bit value
                        

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearChar(void* pChar, int x, int y, int w, int h, u32 cl8, int bpp)
{
    NNS_G2D_POINTER_ASSERT( pChar );
    NNS_G2D_MINMAX_ASSERT(x, 0, CHARACTER_WIDTH - 1);
    NNS_G2D_MINMAX_ASSERT(y, 0, CHARACTER_HEIGHT - 1);
    NNS_G2D_MINMAX_ASSERT(w, 1, CHARACTER_WIDTH);
    NNS_G2D_MINMAX_ASSERT(h, 1, CHARACTER_HEIGHT);
    NNS_G2D_ASSERT( x + w <= CHARACTER_WIDTH );
    NNS_G2D_ASSERT( y + h <= CHARACTER_HEIGHT );

    if( (w == CHARACTER_WIDTH) && (h == CHARACTER_HEIGHT) )
    {
        // 8 * bpp = CHARACTER_HEIGHT * CHARACTER_WIDTH * bpp / 8
        MI_CpuFillFast(pChar, cl8, (u32)(8 * bpp));
    }
    else
    {
        if( bpp == 4 )
        {
            u32 mask;
            u32 data;
            u32* pLine;
            u32* pLineEnd;

            {
                u32 x4 = (unsigned int)x * 4;
                u32 rw4 = 32 - (w * 4 + x4);

                mask = (u32)(~0) >> x4;
                mask <<= x4 + rw4;
                mask >>= rw4;

                data = cl8 & mask;
                mask = ~mask;
            }

            pLine = (u32*)pChar + y;
            pLineEnd = pLine + h;
            for( ; pLine < pLineEnd; pLine++ )
            {
                *pLine = (*pLine & mask) | data;
            }
        }
        else
        {
            u32 mask_0, mask_1;
            u32 data_0, data_1;

            {
                u32 x8 = (unsigned int)x * 8;
                u32 rw8 = 64 - (w * 8 + x8);

                mask_0 = (u32)(~0) >> x8;
                if( rw8 >= 32 )
                {
                    const u32 rw32 = rw8 - 32;
                    mask_0 <<= ( x8 + rw32 );
                    mask_0 >>= rw32;
                }
                else
                {
                    mask_0 <<= x8;
                }

                mask_1 = (u32)(~0) << rw8;
                if( x8 >= 32 )
                {
                    const u32 x32 = x8 - 32;
                    mask_1 >>= ( x32 + rw8 );
                    mask_1 <<= x32;
                }
                else
                {
                    mask_1 >>= rw8;
                }

                data_0 = cl8 & mask_0;
                data_1 = cl8 & mask_1;

                mask_0 = ~mask_0;
                mask_1 = ~mask_1;
            }

            {
                u32* pLine = (u32*)( (u64*)pChar + y );
                u32* const pLineEnd = (u32*)( (u64*)pLine + h );

                while( pLine < pLineEnd )
                {
                    *pLine = (*pLine & mask_0) | data_0;
                    pLine++;
                    *pLine = (*pLine & mask_1) | data_1;
                    pLine++;
                }
            }
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         LetterChar

  Description:  Renders a single character worth of the character rendering.

  Arguments:    i:  pointer to the render information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LetterChar(LC_INFO* i)
{
    const u8* pSrc;     // load source == font data
    u32 x_st;           // begin character rendering X coordinate
    u32 x_ed;           // character rendering end X coordinate
    u32 y_st;         // begin character rendering Y rendering
    u32 y_ed;         // character rendering end Y coordinate
    u32 offset;

    // ASSERT required
    NNS_G2D_POINTER_ASSERT( i );
    NNS_G2D_POINTER_ASSERT( i->dst );
    NNS_G2D_POINTER_ASSERT( i->src );
    NNS_G2D_ASSERT( i->ofs_x < CHARACTER_WIDTH );
    NNS_G2D_ASSERT( i->ofs_y < CHARACTER_HEIGHT );
    NNS_G2D_ASSERT( i->srcBpp * i->width <= i->dsrc );
    NNS_G2D_ASSERT( 0 < i->srcBpp <= 8 );
    NNS_G2D_ASSERT( i->dstBpp == 4 || i->dstBpp == 8 );
    NNS_G2D_ASSERT( i->cl + (1 << i->srcBpp) <= (1 << i->dstBpp) );

    // unnecessary confirmation ASSERT for the process
    NNS_G2D_ASSERT( - i->width < i->ofs_x );
    NNS_G2D_ASSERT( - i->height < i->ofs_y );
    NNS_G2D_ASSERT( 0 < i->width );
    NNS_G2D_ASSERT( 0 < i->height );

    {
        u32 bit_y_begin;  // font data offset y

        // Render area
        x_st = (unsigned int)MATH_IMax(i->ofs_x, 0);
        y_st = (unsigned int)MATH_IMax(i->ofs_y, 0);
        x_ed = (unsigned int)MATH_IMin(CHARACTER_WIDTH, i->ofs_x + i->width);
        y_ed = (unsigned int)MATH_IMin(CHARACTER_HEIGHT, i->ofs_y + i->height);

        bit_y_begin = (unsigned int)- MATH_IMin(i->ofs_y, 0);
        offset = - MATH_IMin(i->ofs_x, 0) * i->srcBpp + bit_y_begin * i->dsrc;

        pSrc = i->src;// + (x_st / CHARACTER_WIDTH);
    }

    NNS_G2D_ASSERT( x_st < x_ed );
    NNS_G2D_ASSERT( y_st < y_ed );
    NNS_G2D_ASSERT( x_ed <= CHARACTER_WIDTH );
    NNS_G2D_ASSERT( y_ed <= CHARACTER_HEIGHT );

    // begin character rendering
    {
        u32 x;           //
        const int dsrc = i->dsrc;
        const int srcBpp = i->srcBpp;
        const int dstBpp = i->dstBpp;

        x_st *= dstBpp;
        x_ed *= dstBpp;

        if( dstBpp == 4 )
        {
            u32* pDst = (u32*)i->dst + y_st;
            u32* pDstEnd = (u32*)i->dst + y_ed;
            u32 cl = i->cl;

            for( ; pDst < pDstEnd; pDst++ )
            {
                NNSiG2dBitReader reader;
                u32 out_line = *pDst;

                NNSi_G2dBitReaderInit(&reader, pSrc + offset/8);
                (void)NNSi_G2dBitReaderRead(&reader, (int)offset%8);

                for( x = x_st; x < x_ed; x += 4 )
                {
                    u32 bits = NNSi_G2dBitReaderRead(&reader, srcBpp);

                    if( bits != 0 )
                    {
                        out_line = (out_line & ~(0xF << x)) | ((cl + bits) << x);
                    }
                }

                *pDst = out_line;

                offset += dsrc;
            }
        }
        else
        {
            u32* pDst = (u32*)( (u64*)i->dst + y_st );
            u32* const pDstEnd = (u32*)( (u64*)i->dst + y_ed );
            u32 cl = i->cl;

            NNS_G2D_ASSERT( dstBpp == 8 );

            for( ; pDst < pDstEnd; pDst +=2 )
            {
                NNSiG2dBitReader reader;
                u32 out_line_0 = *pDst;
                u32 out_line_1 = *(pDst + 1);

                NNSi_G2dBitReaderInit(&reader, pSrc + offset/8);
                (void)NNSi_G2dBitReaderRead(&reader, (int)offset%8);

                for( x = x_st; x < x_ed; x += 8 )
                {
                    u32 bits = NNSi_G2dBitReaderRead(&reader, srcBpp);

                    if( bits != 0 )
                    {
                        if( x < 32 )
                        {
                            out_line_0 = (out_line_0 & ~((u32)0xFF << x)) | ((u32)(cl + bits) << x);
                        }
                        else
                        {
                            const u32 x_32 =  x - 32;
                            out_line_1 = (out_line_1 & ~((u32)0xFF << x_32)) | ((u32)(cl + bits) << x_32);
                        }
                    }
                }

                *pDst = out_line_0;
                *(pDst + 1) = out_line_1;

                offset += dsrc;
            }
        }
    }
}




/*---------------------------------------------------------------------------*
  Name:         DrawGlyphLine

  Description:  Renders the specified glyph to CharCanvas.
                This is for a CharCanvas where a fixed number of characters are empty in each line.

  Arguments:    pCC:    Pointer to CharCanvas.
                pFont:  Pointer to the font used for rendering.
                x:      Upper-left character coordinate.
                y:      Upper-left character coordinate.
                cl:     Color number of character color.
                pGlyph: Pointer to the glyph to render.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawGlyphLine(
    const NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
    const NNSG2dGlyph* pGlyph
)
{
    int ofs_x_base;
    int ofs_x;
    int ofs_y;
    int ofs_x_end;
    int ofs_y_end;
    unsigned int nextLineOffset;
    u8 *pChar;
    u8 glyphWidth;
    u8 charHeight;
    int charSize;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_FONT_ASSERT(pFont);
    NNS_G2D_GLYPH_ASSERT( pGlyph );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(pFont) <= pCC->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << pCC->dstBpp)
        - (1 << NNS_G2dFontGetBpp(pFont)) + 1 );

    charSize = GetCharacterSize(pCC);

    // calculate the processing boundary
    {
        int chara_x_num;
        int chara_y_num;
        const unsigned int areaWidth = (unsigned int)pCC->areaWidth;
        const unsigned int areaHeight = (unsigned int)pCC->areaHeight;
        u8* const charBase = pCC->charBase;
        const NNSG2dCharWidths* const pWidth = pGlyph->pWidths;

        unsigned int chara_x_begin;
        unsigned int chara_x_last;
        unsigned int chara_y_begin;
        unsigned int chara_y_last;

        // glyph width and height
        glyphWidth = pWidth->glyphWidth;
        charHeight = NNS_G2dFontGetCellHeight(pFont);

        // no width == rendering unnecessary
        if( glyphWidth <= 0 )
        {
            return;
        }

        // outside of region
        if( (x + glyphWidth < 0) || (y + charHeight) < 0 )
        {
            return;
        }

        // render starting character coordinates
        chara_x_begin = (x <= 0) ? 0: ((u32)x / CHARACTER_WIDTH);
        chara_y_begin = (y <= 0) ? 0: ((u32)y / CHARACTER_HEIGHT);

        // render ending character coordinates
        chara_x_last = (u32)(x + glyphWidth + (CHARACTER_WIDTH - 1)) / CHARACTER_WIDTH;
        if( chara_x_last >= areaWidth )
        {
            chara_x_last = areaWidth;
        }
        chara_y_last = (u32)(y + charHeight + (CHARACTER_HEIGHT - 1)) / CHARACTER_HEIGHT;
        if( chara_y_last >= areaHeight )
        {
            chara_y_last = areaHeight;
        }

        // amount of vertical and horizontal characters -1
        chara_x_num = (int)(chara_x_last - chara_x_begin);
        chara_y_num = (int)(chara_y_last - chara_y_begin);

        // number of characters that should be rendered is 0
        if( (chara_x_num < 0) || (chara_y_num < 0) )
        {
            return;
        }

        // pointer to the first character
        pChar = charBase + (pCC->param * chara_y_begin + chara_x_begin) * charSize;

        // difference from the last character that should be rendered in each line to the last character that should be rendered in the next line
        // pCC->param == number of characters in one line
        nextLineOffset = (pCC->param - chara_x_num) * charSize;

        // for calculating the render position inside each character
        ofs_x_base = (x < 0) ? x: x & 0x7;
        ofs_y = (y < 0) ? y: y & 0x7;
        ofs_x_end = ofs_x_base - CHARACTER_WIDTH * chara_x_num;
        ofs_y_end = ofs_y - CHARACTER_HEIGHT * chara_y_num;
    }

    // render to each character
    {
        LC_INFO i;

        i.src       = pGlyph->image;
        i.width     = glyphWidth;
        i.height    = charHeight;
        i.cl        = (u32)(cl - 1);
        i.srcBpp    = NNS_G2dFontGetBpp(pFont);
        i.dstBpp    = pCC->dstBpp;
        i.dsrc      = NNS_G2dFontGetCellWidth(pFont) * i.srcBpp;

        for( ; ofs_y > ofs_y_end; ofs_y -= CHARACTER_HEIGHT )
        {
            i.ofs_y = ofs_y;
            for( ofs_x = ofs_x_base; ofs_x > ofs_x_end; ofs_x -= CHARACTER_WIDTH )
            {
                i.dst = pChar;
                i.ofs_x = ofs_x;
                LetterChar(&i);
                pChar += charSize;
            }
            pChar += nextLineOffset;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         DrawGlyph1D

  Description:  Renders the specified glyph to CharCanvas.
                This is for the 1D OBJ CharCanvas.

  Arguments:    pCC:    Pointer to CharCanvas.
                pFont:  Pointer to the font used for rendering.
                x:      Upper-left character coordinate.
                y:      Upper-left character coordinate.
                cl:     Color number of character color.
                pGlyph: Pointer to the glyph to render.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawGlyph1D(
    const NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
    const NNSG2dGlyph* pGlyph
)
{
    int ofs_x_base;
    int ofs_x;
    int ofs_y;
    int ofs_x_end;
    int ofs_y_end;
    int cx, cx_base;
    int cy;
    u8 glyphWidth;
    u8 charHeight;
    int charSize;
    u16* mapTable;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_FONT_ASSERT(pFont);
    NNS_G2D_GLYPH_ASSERT( pGlyph );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(pFont) <= pCC->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << pCC->dstBpp)
        - (1 << NNS_G2dFontGetBpp(pFont)) + 1 );

    charSize = GetCharacterSize(pCC);
    mapTable = (u16*)(pCC->param);

    // calculate the processing boundary
    {
        int chara_x_num;
        int chara_y_num;
        const unsigned int areaWidth = (unsigned int)pCC->areaWidth;
        const unsigned int areaHeight = (unsigned int)pCC->areaHeight;
        const NNSG2dCharWidths* const pWidth = pGlyph->pWidths;

        u32 chara_x_begin;
        u32 chara_x_last;
        u32 chara_y_begin;
        u32 chara_y_last;

        // glyph width and height
        glyphWidth = pWidth->glyphWidth;
        charHeight = NNS_G2dFontGetCellHeight(pFont);

        // rendering unnecessary
        if( glyphWidth <= 0 )
        {
            return;
        }

        // outside of region
        if( (x + glyphWidth < 0) || (y + charHeight) < 0 )
        {
            return;
        }

        // render starting character coordinates
        chara_x_begin = (x <= 0) ? 0: ((u32)x / CHARACTER_WIDTH);
        chara_y_begin = (y <= 0) ? 0: ((u32)y / CHARACTER_HEIGHT);

        // render ending character coordinates
        chara_x_last = (u32)(x + glyphWidth + (CHARACTER_WIDTH - 1)) / CHARACTER_WIDTH;
        if( chara_x_last >= areaWidth )
        {
            chara_x_last = areaWidth;
        }
        chara_y_last = (u32)(y + charHeight + (CHARACTER_HEIGHT - 1)) / CHARACTER_HEIGHT;
        if( chara_y_last >= areaHeight )
        {
            chara_y_last = areaHeight;
        }

        // amount of vertical and horizontal characters -1
        chara_x_num = (int)(chara_x_last - chara_x_begin);
        chara_y_num = (int)(chara_y_last - chara_y_begin);

        // rendering unnecessary, since outside of region
        if( (chara_x_num < 0) || (chara_y_num < 0) )
        {
            return;
        }

        cx_base = (int)chara_x_begin;
        cy      = (int)chara_y_begin;

        // for calculating the render position inside each character
        ofs_x_base = (x < 0) ? x: x & 0x7;
        ofs_y = (y < 0) ? y: y & 0x7;
        ofs_x_end = ofs_x_base - CHARACTER_WIDTH * chara_x_num;
        ofs_y_end = ofs_y - CHARACTER_HEIGHT * chara_y_num;
    }

    // render to each character
    {
        LC_INFO i;
        u8* const pCharBase = pCC->charBase;
        OBJ1DParam p;

        i.src       = pGlyph->image;
        i.width     = glyphWidth;
        i.height    = charHeight;
        i.cl        = (u32)(cl - 1);
        i.srcBpp    = NNS_G2dFontGetBpp(pFont);
        i.dstBpp    = pCC->dstBpp;
        i.dsrc      = NNS_G2dFontGetCellWidth(pFont) * i.srcBpp;

        p.packed = pCC->param;

        {
            const u32 areaWidth         = (u32)pCC->areaWidth;
            const u32 areaHeight        = (u32)pCC->areaHeight;
            const u32 baseWidthShift    = p.baseWidthShift;
            const u32 baseHeightShift   = p.baseHeightShift;

            for( ; ofs_y > ofs_y_end; ofs_y -= CHARACTER_HEIGHT )
            {
                i.ofs_y = ofs_y;
                cx = cx_base;
                for( ofs_x = ofs_x_base; ofs_x > ofs_x_end; ofs_x -= CHARACTER_WIDTH )
                {
                    const unsigned int iChar = GetCharIndex1D((u32)cx, (u32)cy, areaWidth, areaHeight, baseWidthShift, baseHeightShift);

                    i.ofs_x = ofs_x;
                    i.dst = pCharBase + iChar * charSize;

                    LetterChar(&i);
                    cx++;
                }
                cy++;
            }
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         ClearContinuous

  Description:  Fills all characters associated with CharCanvas with the specified color.
                
                This is for a CharCanvas where all characters are continuous.

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number of the fill color.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearContinuous(const NNSG2dCharCanvas* pCC, int cl)
{
    u32 data;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );

    // prepare fill data
    data = SpreadColor32(pCC, cl);

    // fill
    MI_CpuFillFast(
        pCC->charBase,
        data,
        (u32)pCC->areaWidth * pCC->areaHeight * GetCharacterSize(pCC)
    );
}



/*---------------------------------------------------------------------------*
  Name:         ClearLine

  Description:  Fills all characters associated with CharCanvas with the specified color.
                
                This is for a CharCanvas where a fixed number of characters are empty in each line.

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number of the fill color.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearLine(const NNSG2dCharCanvas* pCC, int cl)
{
    u32 data;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );

    // prepare fill data
    data = SpreadColor32(pCC, cl);

    // fill
    {
        const int charSize = GetCharacterSize(pCC);
        const int lineSize = (int)(charSize * pCC->param);
        const u32 blockSize = (u32)(charSize * pCC->areaWidth);
        int y;
        u8* pChar = pCC->charBase;

        for( y = 0; y < pCC->areaHeight; y++ )
        {
            MI_CpuFillFast(pChar, data, blockSize);
            pChar += lineSize;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         ClearAreaLine

  Description:  
                Fills the specified regions of the character associated with CharCanvas with the specified color.
                This is for a CharCanvas where a fixed number of characters are empty in each line.

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number used in filling
                x:      x-coordinate of the upper-left corner of the rectangle
                y:      y-coordinate of the upper-left corner of the rectangle
                w:      rectangle width
                h:      rectangle height

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearAreaLine(
    const NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
)
{
    int ix, iy;
    int cx, cy, cw, ch;
    const int xw = x + w;
    const int yh = y + h;
    u32 cl8;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_ASSERT( xw <= pCC->areaWidth * 8 );
    NNS_G2D_ASSERT( yh <= pCC->areaHeight * 8 );

    // prepare fill data
    cl8 = SpreadColor32(pCC, cl);

    {
        // apply the alignment in characters to the pixel coordinates
        const int left    = MATH_ROUNDDOWN(x, CHARACTER_WIDTH);
        const int top     = MATH_ROUNDDOWN(y, CHARACTER_HEIGHT);
        const int right   = MATH_ROUNDUP(xw, CHARACTER_WIDTH);
        const int bottom  = MATH_ROUNDUP(yh, CHARACTER_HEIGHT);

        const int charSize = GetCharacterSize(pCC);
        const int charBaseLineOffset = (int)(pCC->param * charSize);
        const int bpp = pCC->dstBpp;
        u8* pCharBase;
        u8* pChar;

        // calculate the pointer to the upper left corner of the rectangle
        pCharBase = pCC->charBase + ((top / CHARACTER_HEIGHT) * pCC->param + (left / CHARACTER_WIDTH)) * charSize;

        for( iy = top; iy < bottom; iy += CHARACTER_HEIGHT )
        {
            cy = (iy < y) ? y - iy: 0;
            ch = ((yh - iy > CHARACTER_HEIGHT) ? CHARACTER_HEIGHT: yh - iy) - cy;
            pChar = pCharBase;

            for( ix = left; ix < right; ix += CHARACTER_WIDTH )
            {
                cx = (ix < x) ? x - ix: 0;
                cw = ((xw - ix > CHARACTER_WIDTH) ? CHARACTER_WIDTH: xw - ix) - cx;

                ClearChar(pChar, cx, cy, cw, ch, cl8, bpp);
                pChar += charSize;
            }

            pCharBase += charBaseLineOffset;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         ClearArea1D

  Description:  
                Fills the specified regions of characters associated with CharCanvas with the specified color.
                This is for the 1D OBJ CharCanvas.

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number used in filling
                x:      x-coordinate of the upper-left corner of the rectangle
                y:      y-coordinate of the upper-left corner of the rectangle
                w:      rectangle width
                h:      rectangle height

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearArea1D(
    const NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
)
{
    int ix, iy;
    int pcx, pcy, pcw, pch;
    int cx, cy;
    const int xw = x + w;
    const int yh = y + h;
    u32 cl8;
    OBJ1DParam p;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_ASSERT( xw <= pCC->areaWidth * 8 );
    NNS_G2D_ASSERT( yh <= pCC->areaHeight * 8 );

    // prepare fill data
    cl8 = SpreadColor32(pCC, cl);

    p.packed = pCC->param;

    {
        // apply the alignment in characters to the pixel coordinates
        const int left    = MATH_ROUNDDOWN(x, CHARACTER_WIDTH);
        const int top     = MATH_ROUNDDOWN(y, CHARACTER_HEIGHT);
        const int right   = MATH_ROUNDUP(xw, CHARACTER_WIDTH);
        const int bottom  = MATH_ROUNDUP(yh, CHARACTER_HEIGHT);

        const int areaWidth         = pCC->areaWidth;
        const int areaHeight        = pCC->areaHeight;
        const int baseWidthShift    = (int)p.baseWidthShift;
        const int baseHeightShift   = (int)p.baseHeightShift;

        const int charSize = GetCharacterSize(pCC);
        const int charBaseLineOffset = (int)pCC->param * charSize;
        const int bpp = pCC->dstBpp;
        const int cx_base = left / CHARACTER_WIDTH;
        u8* const pCharBase = pCC->charBase;
        u8* pChar;

        cy = top / CHARACTER_HEIGHT;

        for( iy = top; iy < bottom; iy += CHARACTER_HEIGHT )
        {
            pcy = (iy < y) ? y - iy: 0;
            pch = ((yh - iy > CHARACTER_HEIGHT) ? CHARACTER_HEIGHT: yh - iy) - pcy;
            pChar = pCharBase;

            cx = cx_base;
            for( ix = left; ix < right; ix += CHARACTER_WIDTH )
            {
                const u32 iChar = GetCharIndex1D((u32)cx, (u32)cy, (u32)areaWidth, (u32)areaHeight, (u32)baseWidthShift, (u32)baseHeightShift);
                pcx = (ix < x) ? x - ix: 0;
                pcw = ((xw - ix > CHARACTER_WIDTH) ? CHARACTER_WIDTH: xw - ix) - pcx;

                pChar = pCharBase + iChar * charSize;

                ClearChar(pChar, pcx, pcy, pcw, pch, cl8, bpp);
                cx++;
            }
            cy++;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         InitCharCanvas

  Description:  Initializes CharCanvas by directly designating its members.

  Arguments:    pCC:        Pointer to the CharCanvas to initialize.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.
                charBase:   Pointer to the top of the character that is the render target
                pDrawGlyph: Pointer to the function used in glyph rendering
                pDrawGlyph: Pointer to the function used in clearing
                pDrawGlyph: Pointer to the function used in sectional clearing
                param:      Parameters used by the character rendering function

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitCharCanvas(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode,
    const NNSiG2dCharCanvasVTable* vtable,
    u32 param
)
{
    NNS_G2D_POINTER_ASSERT(pCC);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_COLORMODE_ASSERT( colorMode );
    NNS_G2D_POINTER_ASSERT( charBase );
    NNS_G2D_ALIGN_ASSERT(charBase, 4);
    NNS_G2D_POINTER_ASSERT( vtable );
    NNS_G2D_POINTER_ASSERT( vtable->pDrawGlyph );
    NNS_G2D_POINTER_ASSERT( vtable->pClear );
    NNS_G2D_POINTER_ASSERT( vtable->pClearArea );

    pCC->areaWidth  = areaWidth;
    pCC->areaHeight = areaHeight;
    pCC->dstBpp     = colorMode;
    pCC->charBase   = charBase;
    pCC->vtable     = vtable;
    pCC->param      = param;
}

/*---------------------------------------------------------------------------*
  Name:         SetCharCanvasOBJAttrs

  Description:  Configures all the parameters not configured by NNS_G2dArrangeOBJ* of successive OAMs.
                

  Arguments:    pOam:           Pointer to the OAM array.
                numOBJ:         pOam array element count
                priority:       Cell priority
                mode:           Cell mode
                mosaic:         Cell mosaic
                effect:         Cell effects
                cParam:         Color palette number

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetCharCanvasOBJAttrs(
    GXOamAttr* pOam,
    u16 numObj,
    int priority,
    GXOamMode mode,
    BOOL mosaic,
    GXOamEffect effect,
    int cParam
)
{
	int i;

    for( i = 0; i < numObj; i++ )
    {
        G2_SetOBJPriority(&pOam[i], priority);
        G2_SetOBJMode(&pOam[i], mode, cParam);
        G2_SetOBJEffect(&pOam[i], effect, 0);
        G2_OBJMosaic(&pOam[i], mosaic);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MakeCell

  Description:  Creates the cell for displaying the CharCanvas initialized with the NNS_G2dCharCanvasInitForOBJ1D function.
                

  Arguments:    pCell:          Buffer that stores generated cell data.
                pOam:           pointer to the arranged OAM array
                numOBJ:         pOam array element count
                x:              cell's rotation center coordinates (based on CharCanvas coordinates)
                y:              cell's rotation center coordinates (based on CharCanvas coordinates)
                areaPWidth:     1/2 CharCanvas width (in pixels)
                areaPHeight:    1/2 CharCanvas height (in pixels)
                effect:         Cell effects
                makeBR:         Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeCell(
    NNSG2dCellData* pCell,
    GXOamAttr* pOam,
    u16 numObj,
    int x,
    int y,
    int areaPWidth,
    int areaPHeight,
    GXOamEffect effect,
    BOOL makeBR
)
{
    NNSG2dCellOAMAttrData* oamArray = NULL;
    NNSG2dCellBoundingRectS16* pBoundingRect = NULL;
    u16 attr = 0;
    int i;

    NNS_G2D_POINTER_ASSERT( pCell );
    NNS_G2D_POINTER_ASSERT( pOam );


    //---- Create BoundingRect information
    if( makeBR )
    {
        pBoundingRect = (NNSG2dCellBoundingRectS16*)(pCell + 1);
        oamArray = (NNSG2dCellOAMAttrData*)(pBoundingRect + 1);
        attr |= NNSi_G2dSetCellAttrHasBR(1);

        pBoundingRect->minX = (s16)(- x);
        pBoundingRect->maxX = (s16)(areaPWidth  - x);
        pBoundingRect->minY = (s16)(- y);
        pBoundingRect->maxY = (s16)(areaPHeight - y);
    }
    else
    {
        oamArray = (NNSG2dCellOAMAttrData*)(pCell + 1);
    }

    //---- apply parameters to OBJ
    for( i = 0; i < numObj; i++ )
    {
        //---- adjust the position for double display
        if( effect == GX_OAM_EFFECT_AFFINE_DOUBLE )
        {
            const GXOamShape shape = G2_GetOBJShape(&pOam[i]);
            const int w = NNS_G2dGetOamSizeX(&shape);
            const int h = NNS_G2dGetOamSizeY(&shape);
            u32 x, y;
            int mx, my;

            G2_GetOBJPosition(&pOam[i], &x, &y);
            mx = NNS_G2dRepeatXinCellSpace((s16)(x - w/2));
            my = NNS_G2dRepeatYinCellSpace((s16)(y - h/2));
            G2_SetOBJPosition(&pOam[i], mx, my);
        }
    }

    //---- copy OAM to cell
    for( i = 0; i < numObj; i++ )
    {
        oamArray[i].attr0 = pOam[i].attr0;
        oamArray[i].attr1 = pOam[i].attr1;
        oamArray[i].attr2 = pOam[i].attr2;
    }

    //---- create cell data
    attr |= NNSi_G2dSetCellAttrFlipFlag(
        (effect == GX_OAM_EFFECT_FLIP_H ) ? 1: 0,
        (effect == GX_OAM_EFFECT_FLIP_V ) ? 1: 0,
        (effect == GX_OAM_EFFECT_FLIP_HV) ? 1: 0
    );

    pCell->numOAMAttrs   = numObj;
    pCell->cellAttr      = attr;
    pCell->pOamAttrArray = oamArray;

    //---- add bounding sphere radius
    {
        const u8 bsr = (u8)(ISqrt(areaPWidth * areaPWidth + areaPHeight * areaPHeight) / 2);
        NNSi_G2dSetCellBoundingSphereR(pCell, bsr);
    }
}





//****************************************************************************
// CharCanvas functions
//****************************************************************************

//----------------------------------------------------------------------------
// Render
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasDrawChar

  Description:  Renders characters.

  Arguments:    pCC:    Pointer to CharCanvas.
                x:      Character's upper-left x-coordinate (text screen coordinate system)
                y:      Character's upper-left y-coordinate (text screen coordinate system)
                cl:     Color number of the character color
                ccode:  Character code

  Returns:      Returns the width in pixels of the rendered character.
 *---------------------------------------------------------------------------*/

int NNS_G2dCharCanvasDrawChar(const NNSG2dCharCanvas* pCC, const NNSG2dFont* pFont, int x, int y, int cl, u16 ccode)
{
    NNSG2dGlyph glyph;

    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_FONT_ASSERT( pFont );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(pFont) <= pCC->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << pCC->dstBpp)
        - (1 << NNS_G2dFontGetBpp(pFont)) + 1 );

    NNS_G2dFontGetGlyph(&glyph, pFont, ccode);

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    switch( NNS_G2dFontGetFlags(pFont) )
    {
    case (NNS_G2D_FONT_FLAG_ROT_0):
    case (NNS_G2D_FONT_FLAG_ROT_270|NNS_G2D_FONT_FLAG_TBRL):
#endif
        x += glyph.pWidths->left;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
        break;
    case (NNS_G2D_FONT_FLAG_ROT_90):
    case (NNS_G2D_FONT_FLAG_ROT_0|NNS_G2D_FONT_FLAG_TBRL):
        x -= NNS_G2dFontGetCellWidth(pFont);
        y += glyph.pWidths->left;
        break;
    case (NNS_G2D_FONT_FLAG_ROT_180):
    case (NNS_G2D_FONT_FLAG_ROT_90|NNS_G2D_FONT_FLAG_TBRL):
        x -= glyph.pWidths->left + glyph.pWidths->glyphWidth;
        y -= NNS_G2dFontGetCellHeight(pFont);
        break;
    case (NNS_G2D_FONT_FLAG_ROT_270):
    case (NNS_G2D_FONT_FLAG_ROT_180|NNS_G2D_FONT_FLAG_TBRL):
        y -= glyph.pWidths->left + NNS_G2dFontGetCellHeight(pFont);
        break;
    }
#endif

    NNS_G2dCharCanvasDrawGlyph(pCC, pFont, x, y, cl, &glyph);

    return glyph.pWidths->charWidth;
}





//----------------------------------------------------------------------------
// Build
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForBG

  Description:  Initializes the CharCanvas for use by the background.

  Arguments:    pCC:        Pointer to the CharCanvas to initialize.
                charBase:   Pointer to the top of the character that is the render target
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForBG(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
)
{
    NNS_G2D_POINTER_ASSERT( pCC );
    NNS_G2D_POINTER_ASSERT( charBase );
    NNS_G2D_ALIGN_ASSERT(charBase, 4);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_COLORMODE_ASSERT( colorMode );

    InitCharCanvas(
        pCC,
        charBase, areaWidth, areaHeight, colorMode,
        &VTABLE_BG, (unsigned int)areaWidth
    );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForOBJ1D

  Description:  Initializes the CharCanvas for use by the 1D mapping OBJ.

  Arguments:    pCC:        Pointer to the CharCanvas to initialize.
                charBase:   Pointer to the starting point of the character allocated to CharCanvas.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForOBJ1D(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
)
{
    OBJ1DParam p;
    const ObjectSize* pObj = GetMaxObjectSize(areaWidth, areaHeight);

    NNS_G2D_POINTER_ASSERT( pCC );
    NNS_G2D_POINTER_ASSERT( charBase );
    NNS_G2D_ALIGN_ASSERT(charBase, 4);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_ASSERT( areaWidth * areaHeight <= MAX_OBJ1D_CHARACTER );
    NNS_G2D_COLORMODE_ASSERT( colorMode );

    p.baseWidthShift    = pObj->widthShift;
    p.baseHeightShift   = pObj->heightShift;

    InitCharCanvas(
        pCC,
        charBase, areaWidth, areaHeight, colorMode,
        &VTABLE_OBJ1D, p.packed
    );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForOBJ2DRect

  Description:  Initializes the CharCanvas for use by the 2D mapping OBJ.
                Sets a rectangular region from the 2D mapping character region as the CharCanvas.
                
                The size and positioning of the OBJ is not taken into consideration.

  Arguments:    pCC:        Pointer to the CharCanvas to initialize.
                charBase:   pointer to the upper left character in the character rectangle region
                areaWidth:  width of the rectangular region in character units
                areaHeight: height of the rectangular region in character units
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForOBJ2DRect(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
)
{
    int lineWidth = (colorMode == NNS_G2D_CHARA_COLORMODE_16) ? 32: 16;

    NNS_G2D_POINTER_ASSERT( pCC );
    NNS_G2D_POINTER_ASSERT( charBase );
    NNS_G2D_ALIGN_ASSERT(charBase, 4);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( areaWidth <= lineWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_ASSERT( areaHeight <= MAX_OBJ2DRECT_HEIGHT );
    NNS_G2D_COLORMODE_ASSERT( colorMode );


    InitCharCanvas(
        pCC,
        charBase, areaWidth, areaHeight, colorMode,
        &VTABLE_OBJ2DRECT, (unsigned int)lineWidth
    );
}





//****************************************************************************
// BG Screen Structure Functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToChar256x16Pltt

  Description:  Configures the screen so that it references CharCanvas characters.
                 This is for the text BG.

  Arguments:    scnBase:    A pointer to the background screen's screen base used by CharCanvas.
                            
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                areaLeft:   The screen X coordinate corresponding to areaBase when the upper left of the background screen is (0, 0).
                            
                areaLeft:   The screen Y coordinate corresponding to areaBase when the upper left of the background screen is (0, 0).
                            
                scnWidth:   screen width in character width units
                charNo:     character number of starting character
                cplt:       color palette number configured to the screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToCharText(
    void* scnBase,
    int areaWidth,
    int areaHeight,
    int areaLeft,
    int areaTop,
    NNSG2dTextBGWidth scnWidth,
    int charNo,
    int cplt
)
{
    NNS_G2D_POINTER_ASSERT( scnBase );
    NNS_G2D_ALIGN_ASSERT(scnBase, 2);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_ASSERT( 0 <= areaLeft );
    NNS_G2D_ASSERT( 0 <= areaTop );
    NNS_G2D_ASSERT( areaLeft + areaWidth <= scnWidth );
    NNS_G2D_ASSERT( areaTop + areaHeight <= MAX_TEXT_BG_CHEIGHT );
    NNS_G2D_TEXT_BG_WIDTH_ASSERT( scnWidth );
    NNS_G2D_ASSERT( 0 <= charNo );
    NNS_G2D_ASSERT( charNo + (areaHeight * areaWidth) <= MAX_TEXT_BG_CHARACTER );
    NNS_G2D_MINMAX_ASSERT( cplt, 0, 15 );

    if( scnWidth <= 32 )
    {
        void* areaBase = (u16*)scnBase + (scnWidth * areaTop + areaLeft);
        NNS_G2dMapScrToChar256x16Pltt(areaBase, areaWidth, areaHeight,
            (NNSG2d256x16PlttBGWidth)scnWidth, charNo, cplt);
    }
    else
    {
        const int areaRight = areaLeft + areaWidth;
        const int areaBottom = areaTop + areaHeight;
        const u16 cplt_sft = (u16)(cplt << 12);
        u16* const pScrBase = (u16*)scnBase;
        int x, y;

        for( y = areaTop; y < areaBottom; ++y )
        {
            const int py = (y < 32) ? y: y + 32;
            u16* pScr = pScrBase + py * 32;

            for( x = areaLeft; x < areaRight; ++x )
            {
                const int px = (x < 32) ? x: x + 32 * 31;
                *( pScr + px ) = (u16)(cplt_sft | charNo++);
            }
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToCharAffine

  Description:  Configures the screen so that it references CharCanvas characters.
                 This is for the affine BG.
                Note that the affine background can only handle 256 characters.
                It can handle a maximum region of 128 x 128.

  Arguments:    areaBase:   Pointer to the screen that corresponds to the CharCanvas upper left.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                scnWidth:   screen width in character width units
                charNo:     character number of starting character

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToCharAffine(
    void* areaBase,
    int areaWidth,
    int areaHeight,
    NNSG2dAffineBGWidth scnWidth,
    int charNo
)
{
    NNS_G2D_POINTER_ASSERT( areaBase );
    NNS_G2D_ASSERT( areaHeight <= MAX_AFFINE_BG_CHEIGHT );
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_AFFINE_BG_WIDTH_ASSERT( scnWidth );
    NNS_G2D_ASSERT( areaWidth <= scnWidth );
    NNS_G2D_ASSERT( 0 <= charNo );
    NNS_G2D_ASSERT( charNo + (areaHeight * areaWidth) <= MAX_AFFINE_BG_CHARACTER );

    {
        const BOOL bOddBase = (((u32)(areaBase) % 2) == 1);
        const BOOL bOddWidth = ((areaWidth % 2) == 1);
        const BOOL bOddLast = bOddBase ^ bOddWidth;
        int wordWidth = areaWidth / 2;
        u16* pScrBase = (u16*)((u32)(areaBase) & ~1);
        int y;

        if( bOddBase && !bOddWidth )
        {
            wordWidth--;
        }

        scnWidth /= 2;

        for( y = 0; y < areaHeight; ++y )
        {
            int x = 0;
            u16* pScr = pScrBase;

            // Separate processing by evens because the affine background screen data is 8 bits but VRAM can only be accessed in 16-bit units.
            // 
            if( bOddBase )
            {
                *pScr++ = (u16)( (charNo++ << 8) | (*pScr & 0xFF) );
            }
            for( ; x < wordWidth; ++x )
            {
                *pScr++ = (u16)( ((charNo + 1) << 8) | charNo );
                charNo += 2;
            }
            if( bOddLast )
            {
                *pScr++ = (u16)( (*pScr & 0xFF00) | charNo++ );
            }
            pScrBase += scnWidth;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToChar256x16Pltt

  Description:  Configures the screen so that it references CharCanvas characters.
                
                This is for the affine expansion background 256x16 palette.

  Arguments:    areaBase:   Pointer to the screen that corresponds to the CharCanvas upper left.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                scnWidth:   screen width in character width units
                charNo:     character number of starting character
                cplt:         color palette number configured to the screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToChar256x16Pltt(
    void* areaBase,
    int areaWidth,
    int areaHeight,
    NNSG2d256x16PlttBGWidth scnWidth,
    int charNo,
    int cplt
)
{
    u16* pScrBase;
    int x, y;
    const u16 cplt_sft = (u16)(cplt << 12);

    NNS_G2D_POINTER_ASSERT( areaBase );
    NNS_G2D_ALIGN_ASSERT(areaBase, 2);
    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );
    NNS_G2D_ASSERT( areaHeight <= MAX_256x16PLTT_BG_CHEIGHT );
    NNS_G2D_256x16PLTT_BG_WIDTH_ASSERT( scnWidth );
    NNS_G2D_ASSERT( areaWidth <= scnWidth );
    NNS_G2D_ASSERT( 0 <= charNo );
    NNS_G2D_ASSERT( charNo + (areaHeight * areaWidth) <= MAX_256x16PLTT_BG_CHARACTER );
    NNS_G2D_MINMAX_ASSERT( cplt, 0, 15 );

    pScrBase = areaBase;

    for( y = 0; y < areaHeight; ++y )
    {
        u16* pScr = pScrBase;
        for( x = 0; x < areaWidth; ++x )
        {
            *pScr++ = (u16)(cplt_sft | charNo++);
        }
        pScrBase += scnWidth;
    }
}





//****************************************************************************
// OBJ structure functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dCalcRequiredOBJ

  Description:  Calculates the number of OBJ needed by NNS_G2dCharCanvasInitForOBJ*.

  Arguments:    areaWidth:  CharCanvas width in characters for which to calculate the number of OBJs.
                areaHeight: CharCanvas height in characters for which to calculate the number of OBJs.

  Returns:      The number of OBJs needed.
 *---------------------------------------------------------------------------*/
int NNSi_G2dCalcRequiredOBJ(int areaWidth, int areaHeight)
{
    const u32 w = (unsigned int)areaWidth;
    const u32 h = (unsigned int)areaHeight;
    const u32 w8 = w / 8;
    const u32 h8 = h / 8;
    const u32 w4 = (w & 4) >> 2;
    const u32 h4 = (h & 4) >> 2;
    const u32 wcp = ((w & 2) >> 1) + (w & 1);
    const u32 hcp = ((h & 2) >> 1) + (h & 1);
    int ret = 0;

    NNS_G2D_ASSERT( 0 < areaWidth );
    NNS_G2D_ASSERT( 0 < areaHeight );

    // area A
    ret += w8 * h8;

    // area B
    ret += (wcp * 2 + w4) * h8;

    // area C
    ret += (hcp * 2 + h4) * w8;

    // area D
    ret += (wcp + w4) * (hcp + h4);

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dArrangeOBJ1D

  Description:  Arranges the objects rendered with NNS_G2dCharCanvasInitForOBJ1D so that they can be displayed appropriately.
                
                
                With OAM as the starting point, uses (NNS_G2dCalcRequireOBJ1D(areaWidth, areaHeight)) number of objects.

  Arguments:    oam:        Pointer to the starting point of the OAM string to be used.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                x:          CharCanvas upper left corner display position.
                y:          CharCanvas upper left corner display position.
                color:      CharCanvas color mode.
                charName:   Character name at the top of the character string that the OBJ string uses in display.
                vramMode:   OBJ VRAM capacity

  Returns:      Returns the number of OBJ that were re-written.
 *---------------------------------------------------------------------------*/
int NNS_G2dArrangeOBJ1D(
    GXOamAttr * oam,
    int areaWidth,
    int areaHeight,
    int x,
    int y,
    GXOamColorMode color,
    int charName,
    NNSG2dOBJVramMode vramMode
)
{
    const u32 fullbits = (u32)(~0);
    const ObjectSize* pSize = GetMaxObjectSize(areaWidth, areaHeight);
    const int objWidthShift = pSize->widthShift;
    const int objHeightShift = pSize->heightShift;
    const u32 objWidthMaskInv = fullbits << objWidthShift;
    const u32 objHeightMaskInv = fullbits << objHeightShift;
    const u32 areaAWidth = areaWidth & objWidthMaskInv;
    const u32 areaAHeight =  areaHeight & objHeightMaskInv;
    const int charNameUnit = (color == GX_OAM_COLORMODE_16) ? 1: 2;
    int usedObjs = 0;

    NNS_G2D_POINTER_ASSERT( oam );
    NNS_G2D_MINMAX_ASSERT( areaWidth, 1, 1024 );
    NNS_G2D_MINMAX_ASSERT( areaHeight, 1, 1024 );
    GX_OAM_COLORMODE_ASSERT(color);
    NNS_G2D_OBJVRAMMODE_ASSERT(vramMode);
    NNS_G2D_ASSERT( 0 <= charName );
    NNS_G2D_ASSERT( (charName + ((areaWidth * areaHeight) >> vramMode)) <= 1024 );

    // area A
    {
        const GXOamShape shape = OBJSizeToShape(pSize);
        const int xNum = areaWidth >> objWidthShift;
        const int yNum = areaHeight >> objHeightShift;
        const int charNameSize = ((charNameUnit << objWidthShift) << objHeightShift) >> vramMode;
        int ox, oy;

        for( oy = 0; oy < yNum; ++oy )
        {
            const int py = y + (oy << objHeightShift) * CHARACTER_HEIGHT;

            for( ox = 0; ox < xNum; ++ox )
            {
                const int px = x + (ox << objWidthShift) * CHARACTER_WIDTH;

                G2_SetOBJPosition(oam, px, py);
                G2_SetOBJShape(oam, shape);
                G2_SetOBJCharName(oam, charName);
                G2_SetOBJColorMode(oam, color);

                oam++;
                charName += charNameSize;
            }
        }

        usedObjs += xNum * yNum;
    }

    // area B
    if( areaAWidth < areaWidth )
    {
        const unsigned int areaBWidth = areaWidth - areaAWidth;
        const unsigned int areaBHeight = areaAHeight;
        const int px = x + (int)areaAWidth * CHARACTER_WIDTH;
        const int py = y;

        const int areaBObjs =
            NNS_G2dArrangeOBJ1D(oam, (int)areaBWidth, (int)areaBHeight, px, py, color, charName, vramMode);

        oam += areaBObjs;
        usedObjs += areaBObjs;
        charName += (charNameUnit * areaBWidth * areaBHeight) >> vramMode;
    }

    // area C
    if( areaAHeight < areaHeight )
    {
        const unsigned int areaCWidth = areaAWidth;
        const unsigned int areaCHeight = areaHeight - areaAHeight;
        const int px = x;
        const int py = y + (int)areaAHeight * CHARACTER_HEIGHT;

        const int areaCObjs =
            NNS_G2dArrangeOBJ1D(oam, (int)areaCWidth, (int)areaCHeight, px, py, color, charName, vramMode);

        oam += areaCObjs;
        usedObjs += areaCObjs;
        charName += (charNameUnit * areaCWidth * areaCHeight) >> vramMode;
    }

    // area D
    if( areaAWidth < areaWidth && areaAHeight < areaHeight )
    {
        const unsigned int areaDWidth = areaWidth - areaAWidth;
        const unsigned int areaDHeight = areaHeight - areaAHeight;
        const int px = x + (int)areaAWidth * CHARACTER_WIDTH;
        const int py = y + (int)areaAHeight * CHARACTER_HEIGHT;

        const int areaDObjs =
            NNS_G2dArrangeOBJ1D(oam, (int)areaDWidth, (int)areaDHeight, px, py, color, charName, vramMode);

        usedObjs += areaDObjs;
    }

    return usedObjs;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dArrangeOBJ2DRect

  Description:  Arranges the objects rendered with NNS_G2dCharCanvasInitForOBJ2DRect so that they can be displayed appropriately.
                
                With OAM as the starting point, uses (NNS_G2dCalcRequireOBJ2DRect(areaWidth, areaHeight)) number of objects.
                

  Arguments:    oam:        Pointer to the starting point of the OAM string to be used.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                x:          CharCanvas upper left corner display position.
                y:          CharCanvas upper left corner display position.
                color:      CharCanvas color mode.
                charName:   Character name at the top of the character string that the OBJ string uses in display.

  Returns:      Returns the number of OBJ that were re-written.
 *---------------------------------------------------------------------------*/
int NNS_G2dArrangeOBJ2DRect(
    GXOamAttr * oam,
    int areaWidth,
    int areaHeight,
    int x,
    int y,
    GXOamColorMode color,
    int charName
)
{
    const u32 fullbits = (u32)(~0);
    const ObjectSize* pSize = GetMaxObjectSize(areaWidth, areaHeight);
    const int objWidthShift = pSize->widthShift;
    const int objHeightShift = pSize->heightShift;
    const u32 objWidthMaskInv = fullbits << objWidthShift;
    const u32 objHeightMaskInv = fullbits << objHeightShift;
    const u32 areaAWidth = areaWidth & objWidthMaskInv;
    const u32 areaAHeight =  areaHeight & objHeightMaskInv;
    const int charNameUnit = (color == GX_OAM_COLORMODE_16) ? 1: 2;
    int usedObjs = 0;

    NNS_G2D_POINTER_ASSERT( oam );
    NNS_G2D_MINMAX_ASSERT( areaWidth, 1, 32 );
    NNS_G2D_MINMAX_ASSERT( areaHeight, 1, 32 );
    GX_OAM_COLORMODE_ASSERT(color);
    NNS_G2D_ASSERT( 0 <= charName );
    NNS_G2D_ASSERT( charName + areaWidth * areaHeight <= 1024 );

    // area A
    {
        const GXOamShape shape = OBJSizeToShape(pSize);
        const unsigned int charNameSize = (u32)(charNameUnit << objWidthShift) << objHeightShift;
        const int xNum = areaWidth >> objWidthShift;
        const int yNum = areaHeight >> objHeightShift;
        int ox, oy;

        for( oy = 0; oy < yNum; ++oy )
        {
            const int cy = oy << objHeightShift;
            const int py = y + cy * CHARACTER_HEIGHT;

            for( ox = 0; ox < xNum; ++ox )
            {
                const int cx = ox << objWidthShift;
                const int px = x + cx * CHARACTER_WIDTH;
                const int cName = charName + cy * 32 + cx * charNameUnit;

                G2_SetOBJPosition(oam, px, py);
                G2_SetOBJShape(oam, shape);
                G2_SetOBJCharName(oam, cName);
                G2_SetOBJColorMode(oam, color);

                oam++;
            }
        }

        usedObjs += xNum * yNum;
    }

    // area B
    if( areaAWidth < areaWidth )
    {
        const unsigned int areaBWidth = areaWidth - areaAWidth;
        const unsigned int areaBHeight = areaAHeight;
        const int px = x + (int)areaAWidth * CHARACTER_WIDTH;
        const int py = y;
        const unsigned int cName = charName + areaAWidth * charNameUnit;

        const int areaBObjs =
            NNS_G2dArrangeOBJ2DRect(oam, (int)areaBWidth, (int)areaBHeight, px, py, color, (int)cName);

        oam += areaBObjs;
        usedObjs += areaBObjs;
    }

    // area C
    if( areaAHeight < areaHeight )
    {
        const unsigned int areaCWidth = areaAWidth;
        const unsigned int areaCHeight = areaHeight - areaAHeight;
        const int px = x;
        const int py = y + (int)areaAHeight * CHARACTER_HEIGHT;
        const unsigned int cName = charName + areaAHeight * 32;

        const int areaCObjs =
            NNS_G2dArrangeOBJ2DRect(oam, (int)areaCWidth, (int)areaCHeight, px, py, color, (int)cName);

        oam += areaCObjs;
        usedObjs += areaCObjs;
    }

    // area D
    if( areaAWidth < areaWidth && areaAHeight < areaHeight )
    {
        const unsigned int areaDWidth = areaWidth - areaAWidth;
        const unsigned int areaDHeight = areaHeight - areaAHeight;
        const int px = x + (int)areaAWidth * CHARACTER_WIDTH;
        const int py = y + (int)areaAHeight * CHARACTER_HEIGHT;
        const unsigned int cName = charName + areaAHeight * 32 + areaAWidth * charNameUnit;

        const int areaDObjs =
            NNS_G2dArrangeOBJ2DRect(oam, (int)areaDWidth, (int)areaDHeight, px, py, color, (int)cName);

        usedObjs += areaDObjs;
    }

    return usedObjs;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasMakeCell1D

  Description:  Creates the cell for displaying the CharCanvas initialized with the NNS_G2dCharCanvasInitForOBJ1D function.
                

  Arguments:    pCell:      Buffer that stores generated cell data.
                pCC:        Pointer to CharCanvas
                x:          Cell's center coordinates (based on CharCanvas coordinates)
                y:          Cell's center coordinates (based on CharCanvas coordinates)
                priority:   Cell priority
                mode:       Cell mode
                mosaic:     Cell mosaic
                effect:     Cell effects
                color:      CharCanvas color mode.
                charName:   Starting character name
                cParam:     Color palette number
                vramMode:   Specifies OBJ VRAM capacity setting.
                makeBR:     Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasMakeCell1D(
    NNSG2dCellData*         pCell,
    const NNSG2dCharCanvas* pCC,
    int                     x,
    int                     y,
    int                     priority,
    GXOamMode               mode,
    BOOL                    mosaic,
    GXOamEffect             effect,
    GXOamColorMode          color,
    int                     charName,
    int                     cParam,
    NNSG2dOBJVramMode       vramMode,
    BOOL                    makeBR
)
{
    NNS_G2D_POINTER_ASSERT( pCell );
    NNS_G2D_CHARCANVAS_ASSERT( pCC );

    {
        const int areaCWidth  = (u16)pCC->areaWidth;
        const int areaCHeight = (u16)pCC->areaHeight;

        const u16 numObj = (u16)NNS_G2dCalcRequiredOBJ1D(areaCWidth, areaCHeight);
        GXOamAttr* pTmpBuffer = (GXOamAttr*)__alloca( sizeof(GXOamAttr) * numObj );

		SetCharCanvasOBJAttrs(
			pTmpBuffer,
			numObj,
			priority,
			mode,
			mosaic,
			effect,
			cParam
			);
			

        //---- arranges OBJ
        (void)NNS_G2dArrangeOBJ1D(
            pTmpBuffer,
            areaCWidth,
            areaCHeight,
            -x, -y,
            color,
            charName,
            vramMode
        );

        //---- build cell from OAM
        MakeCell(
            pCell,
            pTmpBuffer,
            numObj,
            x, y,
            areaCWidth  * CHARACTER_WIDTH,
            areaCHeight * CHARACTER_HEIGHT,
            effect,
            makeBR
        );
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasMakeCell2DRect

  Description:  Creates the cell for displaying the CharCanvas initialized with the NNS_G2dCharCanvasInitForOBJ2DRect function.
                

  Arguments:    pCell:      Buffer that stores generated cell data.
                pCC:        Pointer to CharCanvas
                x:          Cell's center coordinates (based on CharCanvas coordinates)
                y:          Cell's center coordinates (based on CharCanvas coordinates)
                priority:   Cell priority
                mode:       Cell mode
                mosaic:     Cell mosaic
                effect:     Cell effects
                color:      CharCanvas color mode.
                charName:   Starting character name
                cParam:     Color palette number
                makeBR:     Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasMakeCell2DRect(
    NNSG2dCellData*         pCell,
    const NNSG2dCharCanvas* pCC,
    int                     x,
    int                     y,
    int                     priority,
    GXOamMode               mode,
    BOOL                    mosaic,
    GXOamEffect             effect,
    GXOamColorMode          color,
    int                     charName,
    int                     cParam,
    BOOL                    makeBR
)
{
    NNS_G2D_POINTER_ASSERT( pCell );
    NNS_G2D_CHARCANVAS_ASSERT( pCC );

    {
        const int areaCWidth  = (u16)pCC->areaWidth;
        const int areaCHeight = (u16)pCC->areaHeight;

        const u16 numObj = (u16)NNS_G2dCalcRequiredOBJ1D(areaCWidth, areaCHeight);
        GXOamAttr* pTmpBuffer = (GXOamAttr*)__alloca( sizeof(GXOamAttr) * numObj );

		SetCharCanvasOBJAttrs(
			pTmpBuffer,
			numObj,
			priority,
			mode,
			mosaic,
			effect,
			cParam
			);
			
        //---- arranges OBJ
        (void)NNS_G2dArrangeOBJ2DRect(
            pTmpBuffer,
            areaCWidth,
            areaCHeight,
            -x, -y,
            color,
            charName
        );

        //---- build cell from OAM
        MakeCell(
            pCell,
            pTmpBuffer,
            numObj,
            x, y,
            areaCWidth  * CHARACTER_WIDTH,
            areaCHeight * CHARACTER_HEIGHT,
            effect,
            makeBR
        );
    }
}

