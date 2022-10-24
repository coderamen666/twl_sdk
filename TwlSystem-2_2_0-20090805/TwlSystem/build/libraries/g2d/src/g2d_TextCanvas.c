/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_TextCanvas.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <string.h>

#undef NNS_G2D_UNICODE
#include <nnsys/g2d/g2d_TextCanvas.h>






//****************************************************************************
// global functions
//****************************************************************************

//----------------------------------------------------------------------------
// Render
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawString

  Description:  Renders up to the first line feed character or a terminating character.

  Arguments:    pTxn:   Pointer to the TextCanvas to be rendered.
                x:      Render starting position X coordinate.
                y:      Render starting position Y coordinate.
                cl:     Color number of character color.
                str:    String to be rendered.
                pPos:   Pointer to character after a carriage return when rendering has occurred to a carriage return, or pointer to a buffer storing NULL when rendered to the terminating character.
                        
                        
                        NULL can be specified here if unnecessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawString(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    const void* str,
    const void** pPos
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
)
{
    const void* pos;        // pointer to the character to be rendered
    int charSpace;          // space between characters (the amount of space between the right edge of the 1st character and the left edge of the 2nd character)
    const NNSG2dFont* pFont;
    u16 c;
    NNSiG2dSplitCharCallback getNextChar;

    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_POINTER_ASSERT( str );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))
                    <= NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp)
        - (1 << NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))) + 1 );

    charSpace   = pTxn->hSpace;
    pFont       = pTxn->pFont;
    pos         = str;
    getNextChar = NNSi_G2dFontGetSpliter(pFont);


    while( (c = getNextChar((const void**)&pos)) != 0 )
    {
        if( c == '\n' )
        {
            break;
        }

        // render one character
        {
            const int w = NNS_G2dCharCanvasDrawChar(pTxn->pCanvas, pFont, x, y, cl, c) + charSpace;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += w * d.x;
            y += w * d.y;
#else
            x += w;
#endif
        }
    }

    if( pPos != NULL )
    {
        *pPos = (c == '\n') ? pos: NULL;
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dTextCanvasDrawTextAlign

  Description:  Renders left-, right-, and center-aligned inside the width of areaWidth.

  Arguments:    pTxn:       Pointer to the TextCanvas to be rendered.
                x:          Render starting position X coordinate.
                y:          Render starting position Y coordinate.
                cl:         Color number of character color.
                flags:      Flag that designates the rendering position.
                txt:        String to be rendered.
                areaWidth:  Render region width referenced when right-aligning or centering.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTextAlign(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int areaWidth,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
)
{
    const void* str;    // pointer to the character string to render
    int linefeed;       // line spacing (the amount of space between the upper edge of the 1st line and the upper edge of the 2nd line)
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    int linefeedx;
    int linefeedy;
    int line;
#endif
    int charSpace;      // space between characters (the amount of space between the right edge of the 1st character and the left edge of the 2nd character)
    const NNSG2dFont* pFont;
    int px, py;

    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_POINTER_ASSERT( txt );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))
                    <= NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp)
        - (1 << NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))) + 1 );

    charSpace = pTxn->hSpace;
    linefeed = NNS_G2dFontGetLineFeed(pTxn->pFont) + pTxn->vSpace;
    pFont = pTxn->pFont;
    str = txt;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    linefeedx = linefeed * -d.y;
    linefeedy = linefeed * d.x;
    line = 0;
#else
    py = y;
#endif


    while( str != NULL )
    {
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
        px = x + line * linefeedx;
        py = y + line * linefeedy;
#else
        px = x;
#endif

        if( flags & NNS_G2D_HORIZONTALALIGN_RIGHT )
        {
            const int width = NNS_G2dTextCanvasGetStringWidth(pTxn, str, NULL);
            const int offset = areaWidth - width;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            px += offset * d.x;
            py += offset * d.y;
#else
            py += offset;
#endif
        }
        else if( flags & NNS_G2D_HORIZONTALALIGN_CENTER )
        {
            const int width = NNS_G2dTextCanvasGetStringWidth(pTxn, str, NULL);
            const int offset = (areaWidth + 1) / 2 - (width + 1) / 2;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            px += offset * d.x;
            py += offset * d.y;
#else
            py += offset;
#endif
        }

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
        NNSi_G2dTextCanvasDrawString(pTxn, px, py, cl, str, &str, d);
        line++;
#else
        NNSi_G2dTextCanvasDrawString(pTxn, px, py, cl, str, &str);
        py += linefeed;
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawText

  Description:  Renders a character string at the position designated with flags for (x,y).

  Arguments:    pTxn:   pointer to the TextCanvas
                x:      render baseline coordinates
                y:      render baseline coordinates
                cl:     character render color number
                flags:  position designation flag
                txt:    String to be rendered.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawText(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
)
{
    NNSG2dTextRect area;

    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_POINTER_ASSERT( txt );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))
                    <= NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp)
        - (1 << NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))) + 1 );

    // find the upper-left coordinate
    {
        area = NNS_G2dTextCanvasGetTextRect(pTxn, txt);

        if( flags & NNS_G2D_HORIZONTALORIGIN_CENTER )
        {
            const int offset = - (area.width + 1) / 2;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * d.x;
            y += offset * d.y;
#else
            x += offset;
#endif
        }
        else if( flags & NNS_G2D_HORIZONTALORIGIN_RIGHT )
        {
            const int offset = - area.width;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * d.x;
            y += offset * d.y;
#else
            x += offset;
#endif
        }

        if( flags & NNS_G2D_VERTICALORIGIN_MIDDLE )
        {
            const int offset = - (area.height + 1) / 2;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * -d.y;
            y += offset * d.x;
#else
            y += offset;
#endif
        }
        else if( flags & NNS_G2D_VERTICALORIGIN_BOTTOM )
        {
            const int offset = - area.height;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * -d.y;
            y += offset * d.x;
#else
            y += offset;
#endif
        }
    }

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    NNSi_G2dTextCanvasDrawTextAlign(pTxn, x, y, area.width, cl, flags, txt, d);
#else
    NNSi_G2dTextCanvasDrawTextAlign(pTxn, x, y, area.width, cl, flags, txt);
#endif
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawTextRect

  Description:  Positions and renders a character string with a rectangle as the base.
                This does not try to render within the bounds of the rectangle.

  Arguments:    pTxn:   pointer to the TextCanvas
                x:      rectangle upper left coordinate
                y:      rectangle upper left coordinate
                cl:     character render color number
                flags:  position designation flag
                txt:    String to be rendered.
                w:      rectangle width
                h:      rectangle height

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTextRect(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int w,
    int h,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
)
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_POINTER_ASSERT( txt );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))
                    <= NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp)
        - (1 << NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))) + 1 );

    // Compensate the y coordinate according to the flag
    {
        if( flags & NNS_G2D_VERTICALALIGN_BOTTOM )
        {
            const int height = NNS_G2dTextCanvasGetTextHeight(pTxn, txt);
            const int offset = h - height;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * -d.y;
            y += offset * d.x;
#else
            y += offset;
#endif
        }
        else if( flags & NNS_G2D_VERTICALALIGN_MIDDLE )
        {
            const int height = NNS_G2dTextCanvasGetTextHeight(pTxn, txt);
            const int offset = (h + 1)/2 - (height + 1) / 2;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            x += offset * -d.y;
            y += offset * d.x;
#else
            y += offset;
#endif
        }
    }

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    NNSi_G2dTextCanvasDrawTextAlign(pTxn, x, y, w, cl, flags, txt, d);
#else
    NNSi_G2dTextCanvasDrawTextAlign(pTxn, x, y, w, cl, flags, txt);
#endif
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawTaggedText

  Description:  Renders a multi-line character string that has embedded tags.
                A callback is invoked when a control character (0x00&#8211;0x09, 0x0B&#8211;0x1F) is discovered within a text string.
                
                The tag processing is left to the callback.

  Arguments:    pTxn:       Pointer to the TextCanvas to be rendered.
                x:          Render base position X coordinate.
                y:          Render base position Y coordinate.
                cl:         Color number of character color.
                str:        String to be rendered.
                cbFunc:     Callback function invoked by the tag.
                cbParam:    User data passed to the callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTaggedText(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    const void* txt,
    NNSG2dTagCallback cbFunc,
    void* cbParam
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
)
{
    const void* pos;    // pointer to the character to be rendered
    int linefeed;       // line spacing (the amount of space between the upper edge of the 1st line and the upper edge of the 2nd line)
    int charSpace;      // space between characters (the amount of space between the right edge of the 1st character and the left edge of the 2nd character)
    const NNSG2dFont* pFont;
    NNSG2dTagCallbackInfo cbInfo;
    u16 c;
    NNSiG2dSplitCharCallback getNextChar;

    int px = x;
    int py = y;

    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_FONT_ASSERT( pTxn->pFont );
    NNS_G2D_POINTER_ASSERT( txt );
    NNS_G2D_ASSERT( NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))
                    <= NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp );
    NNS_G2D_MINMAX_ASSERT( cl, 0,
        (1 << NNS_G2dTextCanvasGetCharCanvas(pTxn)->dstBpp)
        - (1 << NNS_G2dFontGetBpp(NNS_G2dTextCanvasGetFont(pTxn))) + 1 );
    NNS_G2D_POINTER_ASSERT( cbFunc );

    cbInfo.txn      = *pTxn;
    cbInfo.cbParam  = cbParam;

    charSpace   = cbInfo.txn.hSpace;
    pFont       = cbInfo.txn.pFont;
    linefeed    = NNS_G2dFontGetLineFeed(pFont) + cbInfo.txn.vSpace;
    pos         = txt;
    getNextChar = NNSi_G2dFontGetSpliter(pFont);

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    linefeed   *= (d.x != 0) ? d.x: -d.y;
#endif

    while( (c = getNextChar((const void**)&pos)) != 0 )
    {
        if( c < ' ' )
        {
            if( c == '\n' )
                // carriage return processing
            {
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
                if( d.x == 0 )
                {
                    px += linefeed;
                    py = y;
                }
                else
#endif
                {
                    px = x;
                    py += linefeed;
                }
            }
            else
                // callback call
            {
                cbInfo.str = (const NNSG2dChar*)pos;
                cbInfo.x = px;
                cbInfo.y = py;
                cbInfo.clr = cl;

                cbFunc(c, &cbInfo);
                NNS_G2D_TEXTCANVAS_ASSERT( &(cbInfo.txn) );
                NNS_G2D_FONT_ASSERT( cbInfo.txn.pFont );
                NNS_G2D_POINTER_ASSERT( cbInfo.str );
                NNS_G2D_MINMAX_ASSERT( cl, 0, 255 );

                pos = (const void*)cbInfo.str;
                px = cbInfo.x;
                py = cbInfo.y;
                cl = cbInfo.clr;

                pFont       = cbInfo.txn.pFont;
                charSpace   = cbInfo.txn.hSpace;
                linefeed    = NNS_G2dFontGetLineFeed(pFont) + cbInfo.txn.vSpace;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
                linefeed   *= (d.x != 0) ? d.x: -d.y;
#endif
            }

            continue;
        }
        else
            // render one character
        {
            const int w = NNS_G2dCharCanvasDrawChar(cbInfo.txn.pCanvas, cbInfo.txn.pFont, px, py, cl, c) + charSpace;
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
            px += w * d.x;
            py += w * d.y;
#else
            px += w;
#endif
        }
    }
}

//--------------------------------------------------------------------
