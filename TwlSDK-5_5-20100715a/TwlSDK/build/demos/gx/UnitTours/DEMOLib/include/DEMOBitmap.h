/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOBitmap.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef DEMO_BITMAP_H_
#define DEMO_BITMAP_H_

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

#define DEMO_RGB_NONE   GX_RGBA(31, 31, 31, 0)
#define DEMO_RGB_CLEAR  GX_RGBA( 0,  0,  0, 0)

extern const u32 DEMOAsciiChr[8 * 0x100];


/*---------------------------------------------------------------------------*/
/* Functions */

// Simplified versions that have taken conditional operators into account
#define DEMOSetBitmapTextColor(color)       DEMOiSetBitmapTextColor(DEMOVerifyGXRgb(color))
#define DEMOSetBitmapGroundColor(color)     DEMOiSetBitmapGroundColor(DEMOVerifyGXRgb(color))
#define DEMOFillRect(x, y, wx, wy, color)   DEMOiFillRect(x, y, wx, wy, DEMOVerifyGXRgb(color))
#define DEMODrawLine(sx, sy, tx, ty, color) DEMOiDrawLine(sx, sy, tx, ty, DEMOVerifyGXRgb(color))
#define DEMODrawFrame(x, y, wx, wy, color)  DEMOiDrawFrame(x, y, wx, wy, DEMOVerifyGXRgb(color))


/*---------------------------------------------------------------------------*
  Name:         DEMOInitDisplayBitmap

  Description:  Initializes DEMO in bitmap render mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOInitDisplayBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         DEMO_DrawFlip

  Description:  Applies the current rendering content to VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMO_DrawFlip();

/*---------------------------------------------------------------------------*
  Name:         DEMOHookConsole

  Description:  Configure output to be sent to the log, as well, using the OS_PutString hook function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOHookConsole(void);

/*---------------------------------------------------------------------------*
  Name:         DEMOVerifyGXRgb

  Description:  Checks for a valid color value.

  Arguments:    color: Color value that is thought to be within the GXRgb range.

  Returns:      If color is a valid GXRgb value, it will be returned unchanged. If it is out of range, 0xFFFF will be returned.
 *---------------------------------------------------------------------------*/
GXRgb DEMOVerifyGXRgb(int color);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetBitmapTextColor

  Description:  Sets the text color for rendering bitmaps.

  Arguments:    color: Color to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiSetBitmapTextColor(GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetBitmapGroundColor

  Description:  Sets the background color for rendering bitmaps.

  Arguments:    color: the color to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiSetBitmapGroundColor(GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOFillRect

  Description:  Renders a rectangle to the bitmap.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                wx: X width for rendering
                wy: Y width for rendering
                color: Color to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiFillRect(int x, int y, int wx, int wy, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOBlitRect

  Description:  Transfers a rectangle to the bitmap.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                wx: X width for rendering
                wy: Y width for rendering
                image: Image to transfer
                stroke: Number of pixels per line to transfer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOBlitRect(int x, int y, int wx, int wy, const GXRgb *image, int stroke);

/*---------------------------------------------------------------------------*
  Name:         DEMOBlitTex16

  Description:  Transfers a 16-color texture to the bitmap.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                wx: Rendering width in the x-direction (must be an integer multiple of 8 pixels)
                wy: Rendering width in the y-direction (must be an integer multiple of 8 pixels)
                chr: Character image (4x8x8 1-D character format)
                plt: Palette image (index 0 will be considered transparent)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOBlitTex16(int x, int y, int wx, int wy, const void *chr, const GXRgb *plt);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawLine

  Description:  Renders a line on the bitmap.

  Arguments:    sx: X-coordinate of the starting point
                sy: Y-coordinate of the starting point
                tx: Distance along the x-axis to the ending point
                ty: Distance along the y-axis to the ending point
                color: Color to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiDrawLine(int sx, int sy, int tx, int ty, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawFrame

  Description:  Renders a frame to the bitmap.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                wx: X width for rendering
                wy: Y width for rendering
                color: Color to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiDrawFrame(int x, int y, int wx, int wy, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawText

  Description:  Renders a character string on the bitmap.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                format: Formatted string

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMODrawText(int x, int y, const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOClearString

  Description:  Deletes all background text.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOClearString(void);

/*---------------------------------------------------------------------------*
  Name:         DEMOPutString

  Description:  Renders the background text.

  Arguments:    x: X-coordinate for rendering
                y: Y-coordinate for rendering
                format: Formatted character string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOPutString(int x, int y, const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOPutLog

  Description:  Displays a log string on the lower screen.

  Arguments:    format: Formatted string

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOPutLog(const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetViewPort

  Description:  Sets the viewport and projection.

  Arguments:    x: Upper-left x-coordinate
                y: Upper-left y-coordinate
                wx: Width of the viewport in the x direction
                wy: Width of the viewport in the y direction

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOSetViewPort(int x, int y, int wx, int wy);


#ifdef __cplusplus
}/* extern "C" */
#endif

/* DEMO_BITMAP_H_ */
#endif
