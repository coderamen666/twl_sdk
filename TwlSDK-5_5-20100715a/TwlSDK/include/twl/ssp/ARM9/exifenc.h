/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     exifenc.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-02-15#$
  $Rev: 11293 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_EXIFENC_H_
#define TWL_SSP_EXIFENC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderMakerNoteEx

  Description:  Data to add to a MakerNote when encoding a JPEG.

  Arguments:    tag: The type of data to fill in
                buffer: Data buffer
                size: Data size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_SetJpegEncoderMakerNoteEx(SSPJpegMakernote tag, const u8* buffer, u32 size);

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderMakerNote

  Description:  DSi camera data to add to a MakerNote when encoding a JPEG.

  Arguments:    buffer: Buffer for DSi camera data
                size: Data size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void SSP_SetJpegEncoderMakerNote(const u8* buffer, u32 size)
{
    SSP_SetJpegEncoderMakerNoteEx(SSP_MAKERNOTE_PHOTO, buffer, size);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderDateTime

  Description:  Specifies the date and time to set in the Exif IFD tags DateTimeOriginal and DateTimeDigitized when encoding a JPEG.
                

  Arguments:    buffer: A string that shows the date and time in the following format: YYYY:MM:DD HH:MM:SS

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_SetJpegEncoderDateTime(const u8* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetDateTime

  Description:  This function simply calls RTC_GetDateTime internally.

  Arguments:    None.

  Returns:      Returns TRUE if it was successful.
 *---------------------------------------------------------------------------*/
BOOL SSP_GetDateTime( RTCDate* date , RTCTime* time );

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderDateTimeNow

  Description:  Sets the current time in the Exif IFD tags DateTimeOriginal and DateTimeDigitized when encoding a JPEG.
                

  Arguments:    None.

  Returns:      Returns TRUE if the current time was set successfully.
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegEncoderDateTimeNow(void);

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderSignMode

  Description:  Enables or disables functionality that adds a signature when encoding a JPEG.

  Arguments:    If TRUE, it will be turned on. If FALSE, it will be turned off.

  Returns:      Returns the previously set mode.
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegEncoderSignMode(BOOL mode);

u32 SSP_ExifEncode(u8* l_dst, u32 width, u32 height, const u8* thumb_src, u32 thumb_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_EXIFENC_H_ */
#endif
