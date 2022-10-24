/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     exifdec.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_EXIFDEC_H_
#define TWL_SSP_EXIFDEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderDateTime

  Description:  Gets the date and time information as they exist when a JPEG is decoded.

  Arguments:    buffer: Buffer in which to put the date and time information

  Returns:      Size
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderDateTime(u8* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderSoftware

  Description:  Gets the lower 4 bytes of the title ID registered with the JPEG Software tag.

  Arguments:    buffer: Buffer in which to put the Software information

  Returns:      Size
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderSoftware(char* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteAddrEx

  Description:  Gets the starting address of a MakerNote when decoding a JPEG.

  Arguments:    tag: The type of data to get

  Returns:      Address
 *---------------------------------------------------------------------------*/
u8* SSP_GetJpegDecoderMakerNoteAddrEx(SSPJpegMakernote tag);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteSizeEx

  Description:  Gets the size of a MakerNote when decoding a JPEG.

  Arguments:    tag: The type of data to get

  Returns:      Size
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderMakerNoteSizeEx(SSPJpegMakernote tag);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteAddr

  Description:  Gets the starting address of the MakerNote's DSi camera data when decoding a JPEG.

  Arguments:    None.

  Returns:      Address
 *---------------------------------------------------------------------------*/
static inline u8* SSP_GetJpegDecoderMakerNoteAddr(void)
{
    return SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_PHOTO);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteSize

  Description:  Gets the size of the MakerNote's DSi camera data when decoding a JPEG.

  Arguments:    None.

  Returns:      Size
 *---------------------------------------------------------------------------*/
static inline u16 SSP_GetJpegDecoderMakerNoteSize(void)
{
    return SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_PHOTO);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegDecoderSignMode

  Description:  Enables or disables functionality that checks a signature when decoding a JPEG.

  Arguments:    If TRUE, it will be turned on. If FALSE, it will be turned off.

  Returns:      Returns the previously set mode.
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegDecoderSignMode(BOOL mode);

s32 SSP_ExifDecode(u8* src, u32 src_size, int* cur, int option);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_EXIFDEC_H_ */
#endif
