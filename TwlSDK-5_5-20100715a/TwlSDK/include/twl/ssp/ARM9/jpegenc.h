/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     jpegenc.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-06#$
  $Rev: 10865 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_JPEGENC_H_
#define TWL_SSP_JPEGENC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>
#include <twl/ssp/ARM9/exifenc.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegEncoderBufferSize

  Description:  Returns the work size required for encoding.

  Arguments:    width: Image width
                height: Image height
                sampling: Sampling (1 or 2)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      The required memory size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetJpegEncoderBufferSize(u32 width, u32 height, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoder

  Description:  Encodes a JPEG.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoder(const void* src, u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderEx

  Description:  Specifies whether to use a signature and encodes a JPEG.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.
                sign: Specify TRUE to add a signature

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderEx(const void* src, u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoder(src, dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_ConvertJpegEncodeData

  Description:  Converts input data into a format used to perform JPEG encoding.
                The input data can be cleared after this function has finished.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                wrk: Work area
                width: Image width
                height: Image height
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL SSP_ConvertJpegEncodeData(const void* src, u8 *wrk,
                         u32 width, u32 height, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderWithEncodeData

  Description:  Encodes a JPEG from data that was converted in advance for encoding.
                You must call the SSP_ConvertJpegEncodeData function before this one.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The dst and wrk values must be 4-byte aligned.

  Arguments:    dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderWithEncodeData(u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderWithEncodeDataEx

  Description:  Specifies whether to use a signature and encodes a JPEG from data that was converted in advance for encoding.
                You must call the SSP_ConvertJpegEncodeData function before this one.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The dst and wrk values must be 4-byte aligned.

  Arguments:    dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.
                sign: Specify TRUE to add a signature

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderWithEncodeDataEx(u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoderWithEncodeData(dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegEncoderLiteBufferSize

  Description:  Returns the work size required for encoding (reduced-memory version).
                This work size is used by SSP_StartJpegEncoderLite.

  Arguments:    option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      The required memory size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetJpegEncoderLiteBufferSize(u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderLite

  Description:  Encodes a JPEG (reduced-memory version).
                Note: This is unrelated to SSP_StartJpegEncoderEx.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The width value must be a multiple of 16, and the height value must be a multiple of 8 if sampling is 3.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  Failure to specify a format will result in an error.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderLite(const void* src, u8 *dst, u32 limit, u8 *wrk,
                             u32 width, u32 height,
                             u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderLiteEx

  Description:  Encodes a JPEG (reduced-memory version).
                Note: This is unrelated to SSP_StartJpegEncoderEx.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The width value must be a multiple of 16, and the height value must be a multiple of 8 if sampling is 3.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  Failure to specify a format will result in an error.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.
                sign: Specify TRUE to add a signature

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderLiteEx(const void* src, u8 *dst, u32 limit, u8 *wrk,
                             u32 width, u32 height,
                             u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoderLite(src, dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegEncoderFastBufferSize

  Description:  Returns the work size required for encoding (High-speed version).
                This work size is used by SSP_StartJpegEncoderFast.

  Arguments:    option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  If unspecified, the format is considered to be RGB555.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      The required memory size.
 *---------------------------------------------------------------------------*/
u32 SSP_GetJpegEncoderFastBufferSize(u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderFast

  Description:  Encodes a JPEG (High-speed version).
                Note: This is unrelated to SSP_StartJpegEncoderEx.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The width value must be a multiple of 16, and the height value must be a multiple of 8 if sampling is 3.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  Failure to specify a format will result in an error.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderFast(const void* src, u8 *dst, u32 limit, u8 *wrk,
                             u32 width, u32 height,
                             u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderFastEx

  Description:  Encodes a JPEG (High-speed version).
                Note: This is unrelated to SSP_StartJpegEncoderEx.
                When sampling is 1, the width and height must be multiples of 8.
                When sampling is 2, the width and height must be multiples of 16.
                The width value must be a multiple of 16, and the height value must be a multiple of 8 if sampling is 3.
                src, dst, and wrk must be 4-byte aligned.

  Arguments:    src: Image data (RGB555/YUV422 data from the upper left to the lower right)
                dst: Location to put the encoded JPEG data
                limit: Size limit for dst (encoding will fail if this is exceeded)
                wrk: Work area
                width: Image width
                height: Image height
                quality: Encoding quality (1-100)
                sampling: Main image sampling (1=YUV444, 2=YUV420, 3=YUV422. Thumbnails are internally fixed to 3)
                option: Options (bitwise logical OR)
                           SSP_JPEG_RGB555: Encode from the RGB555 format.
                           SSP_JPEG_YUV422: Encode from the YUV422 format.
                                  Failure to specify a format will result in an error.
                           SSP_JPEG_THUMBNAIL: Encode with a thumbnail.
                sign: Specify TRUE to add a signature

  Returns:      Encoded JPEG size (when this is 0, encoding failed).
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderFastEx(const void* src, u8 *dst, u32 limit, u8 *wrk,
                               u32 width, u32 height,
                               u32 quality, u32 sampling, u32 option, BOOL sign);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_JPEGENC_H_ */
#endif
