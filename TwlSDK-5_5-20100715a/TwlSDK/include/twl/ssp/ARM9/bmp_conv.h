/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     bmp_conv.h

  Copyright 2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-01-09#$
  $Rev: 9810 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_BMP_H_
#define TWL_SSP_BMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

typedef enum
{
    SSP_BMP_YUV422 = 1,
    SSP_BMP_RGB555
}
SSPConvertBmpType;

/*---------------------------------------------------------------------------*
  Name:         SSP_YUV422ToRGB888b

  Description:  Converts data from the YUV422 format to the RGB888 (bottom-up) format.

  Arguments:    src: Input (YUV422) data
                dst: Output (RGB888) data
                width: Image width
                height: Image height

  Returns:      Returns the size.
 *---------------------------------------------------------------------------*/
u32 SSP_YUV422ToRGB888b( const void* src, void* dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_RGB555ToRGB888b

  Description:  Converts data from the RGB555 format to the RGB888 (bottom-up) format.

  Arguments:    src: Input (YUV422) data
                dst: Output (RGB888) data
                width: Image width
                height: Image height

  Returns:      Returns the size.
 *---------------------------------------------------------------------------*/
u32 SSP_RGB555ToRGB888b( const void* src, void* dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_GetRGB888Size

  Description:  Gets the size of data in the RGB88 format.

  Arguments:    width: Image width
                height: Image height

  Returns:      Returns the size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetRGB888Size( u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_CreateBmpHeader

  Description:  Creates a BMP file header.

  Arguments:    dst: Output data
                width: Image width
                height: Image height

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_CreateBmpHeader( u8 *dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_GetBmpHeaderSize

  Description:  Gets the header size for a BMP file.

  Arguments:    None.

  Returns:      Returns the size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetBmpHeaderSize();

/*---------------------------------------------------------------------------*
  Name:         SSP_GetBmpFileSize

  Description:  Gets the size of a BMP file.

  Arguments:    width: Image width
                height: Image height

  Returns:      Returns the size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetBmpFileSize(u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         SSP_CreateBmpFile

  Description:  Creates a BMP file.

  Arguments:    src: Input (YUV422 or RGB555) data
                dst: Buffer to store the BMP file
                width: Image width
                height: Image height
                type: Input data type

  Returns:      Returns the size of the created BMP file.
 *---------------------------------------------------------------------------*/
u32 SSP_CreateBmpFile(const void* src, void* dst, u16 width, u16 height, SSPConvertBmpType type);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_BMP_H_ */
#endif
