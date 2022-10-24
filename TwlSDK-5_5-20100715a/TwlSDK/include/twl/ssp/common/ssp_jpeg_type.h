/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ARMJPEG - include
  File:     jpegdec.h

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

#ifndef TWL_SSP_JPEG_TYPE_H_
#define TWL_SSP_JPEG_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define SSP_JPEG_THUMBNAIL   0x0001
//#define SSP_JPEG_BILINEAR  0x0002 // Not supported yet
#define SSP_JPEG_RGB555      0x0004
//#define SSP_JPEG_RGB888    0x0008 // Not supported yet
#define SSP_JPEG_EXIF        0x0010

// These types are not compatible with DSP JPEG library
// SSP_JPEG_RGB422: (8bits x 4)/2pixels. { Y0,U0,Y1,V0 }, { Y2,U1,Y3,V1 }, ...
// SSP_JPEG_YUV888: (8 bits/pixel) x 3. Not interleaved. (internal format for encoder)
// {Y0,Y1,...,Yn, U0,U1,...,Un, V0,V1,...,Vn}
#define SSP_JPEG_YUV422      0x0100
//#define SSP_JPEG_YUV888    0x0200 // Partly supported

#define SSP_JPEG_FORMAT_MASK (SSP_JPEG_RGB555 | SSP_JPEG_YUV422)


#define SSP_JPEG_THUMBNAIL_WIDTH  160
#define SSP_JPEG_THUMBNAIL_HEIGHT 120
#define SSP_JPEG_THUMBNAIL_BUFFER_WIDTH  160
#define SSP_JPEG_THUMBNAIL_BUFFER_HEIGHT 128


#define SSP_JPEG_OUTPUT_YUV444 1
#define SSP_JPEG_OUTPUT_YUV420 2
#define SSP_JPEG_OUTPUT_YUV422 3


typedef enum
{
    SSP_MAKERNOTE_PHOTO = 0x1001,
    SSP_MAKERNOTE_USER = 0x1002
}
SSPJpegMakernote;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_SSP_JPEG_TYPE_H_ */
