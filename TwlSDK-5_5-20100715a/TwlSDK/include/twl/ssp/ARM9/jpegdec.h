/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     jpegdec.h

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

#ifndef TWL_SSP_JPEGDEC_H_
#define TWL_SSP_JPEGDEC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>
#include <twl/ssp/ARM9/exifdec.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>


// Define errorCode with a range of s8.
// The cause of "Could not process data." is either that the data format is invalid or unsupported. It is also possible that the data is incomplete.
// 
typedef enum SSPJpegDecoderErrorCode
{
    SSP_JPEG_DECODER_OK = 0,       // Success
    SSP_JPEG_DECODER_ERROR_ARGUMENT = (-1),   // There is a mistake in an argument other than the 'option' argument
    SSP_JPEG_DECODER_ERROR_WORK_ALIGN = (-2),   // pCtx is not in 4-byte alignment
    SSP_JPEG_DECODER_ERROR_OPTION = (-3),   // There is a mistake in the 'option' argument
    SSP_JPEG_DECODER_ERROR_SIGN = (-10),

    SSP_JPEG_DECODER_ERROR_WIDTH_HEIGHT = (-20),  // Either the width or the height exceeds the specified maximum value. The actual size is in pCtx->width and pCtx->height

    SSP_JPEG_DECODER_ERROR_EXIF_0 = (-30),   // Could not process data
// If decoding of a thumbnail is specified for an image for which no thumbnail exists, this error code might be returned
// 

    SSP_JPEG_DECODER_ERROR_MARKER_COMBINATION = (-50),   // Could not process data
// Marker combination is incorrect.
// For example, this error code might be returned if the SOS marker corresponding to the SOF marker was not found.
// 

    SSP_JPEG_DECODER_ERROR_SOI = (-60),   // Could not process data (or marker)
    SSP_JPEG_DECODER_ERROR_SOF = (-61),
    SSP_JPEG_DECODER_ERROR_SOF_BLOCK_ID = (-62),
    SSP_JPEG_DECODER_ERROR_DHT = (-63),
    SSP_JPEG_DECODER_ERROR_SOS = (-64),
    SSP_JPEG_DECODER_ERROR_DQT = (-65),
    SSP_JPEG_DECODER_ERROR_DRI = (-66),

    SSP_JPEG_DECODER_ERROR_UNDERRUN_0 = (-90),   // Could not process data
    SSP_JPEG_DECODER_ERROR_UNDERRUN_1 = (-91),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_2 = (-92),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_3 = (-93),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_4 = (-94),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_5 = (-95),

    SSP_JPEG_DECODER_ERROR_RANGE_0 = (-110),  // Could not process data
    SSP_JPEG_DECODER_ERROR_RANGE_1 = (-111),
    SSP_JPEG_DECODER_ERROR_RANGE_2 = (-112),
    SSP_JPEG_DECODER_ERROR_RANGE_3 = (-113),
    SSP_JPEG_DECODER_ERROR_RANGE_4 = (-114),
    SSP_JPEG_DECODER_ERROR_RANGE_5 = (-115),

    SSP_JPEG_DECODER_ERROR_HLB_0 = (-120),  // Could not process data

    SSP_JPEG_DECODER_ERROR_INTERNAL_CTX_SIZE = (-128)  // Internal error. Normally, this does not occur
} SSPJpegDecoderErrorCode;

// For Fast versions, work memory to call a context structure is required.
// Initialization before and after calling the function is unnecessary. After the function completes, the library will not use it again.
// The context structure is large (8 KB), so do not allocate it from a stack in the same way as the local variables.
// 

// Context structure
typedef struct SSPJpegDecoderFastContext {
    u8*     pSrc;
    u32     inputDataSize;
    void*   pDst;
    u32     option;
    u16     maxWidth;       // Maximum allowable image width (less than 65536). The argument maxWidth of the SSP_StartJpegDecoderFast function is copied here
    u16     maxHeight;      // Maximum allowable image height (less than 65536). The argument maxHeight of the SSP_StartJpegDecoderFast function is copied here
    u16     width;          // Actual image width
    u16     height;         // Actual image height
    SSPJpegDecoderErrorCode errorCode; // No error if 0
    u8      reserved[4];
    u32     work[0x7f8];
} SSPJpegDecoderFastContext;

/*---------------------------------------------------------------------------*
  Name:         SSP_CheckJpegDecoderSign

  Description:  Only checks a JPEG file's signature.

  Arguments:    data: JPEG file
                size: JPEG file size

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL SSP_CheckJpegDecoderSign(u8* data, u32 size);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoder

  Description:  Decodes a JPEG file.

  Arguments:    data: JPEG file
                size: JPEG file size
                dst: Destination buffer for decoding (now only RGB555)
                width: Gives the maximum image width as an argument.
                         When the image is decoded successfully, its width will be returned.
                height: Gives the maximum image height as an argument.
                         When the image is decoded successfully, its height will be returned.
                option: Options (bitwise logical OR)
                         SSP_JPEG_RGB555: Decode in the RGB555 format.
                                Currently, even if this is not specified, data will be decoded in the RGB555 format.
                         SSP_JPEG_THUMBNAIL: Decode thumbnails. When this is unspecified, the main image will be decoded.
                         SSP_JPEG_EXIF: Get only EXIF information

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoder(u8* data, u32 size, void* dst, s16* width, s16* height, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderEx

  Description:  Specifies whether to use a signature and decode a JPEG file.

  Arguments:    data: JPEG file
                size: JPEG file size
                dst: Destination buffer for decoding (now only RGB555)
                width: Gives the maximum image width as an argument.
                         When the image is decoded successfully, its width will be returned.
                height: Gives the maximum image height as an argument.
                         When the image is decoded successfully, its height will be returned.
                option: Options (bitwise logical OR)
                         SSP_JPEG_RGB555: Decode in the RGB555 format.
                                Currently, even if this is not specified, data will be decoded in the RGB555 format.
                         SSP_JPEG_THUMBNAIL: Decode thumbnails. When this is unspecified, the main image will be decoded.
                         SSP_JPEG_EXIF: Get only EXIF information
                sign: Specify TRUE to check the signature.

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
static inline BOOL SSP_StartJpegDecoderEx(u8* data, u32 size, void* dst, s16* width, s16* height, u32 option, BOOL sign)
{
    BOOL result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegDecoderSignMode(sign);
    result = SSP_StartJpegDecoder(data, size, dst, width, height, option);
    (void)SSP_SetJpegDecoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_ExtractJpegDecoderExif

  Description:  Extracts only EXIF information from a JPEG file.
                Use the various get functions to get the actual extracted values.

  Arguments:    data: JPEG file
                size: JPEG file size
                
  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
static inline BOOL SSP_ExtractJpegDecoderExif(u8* data, u32 size)
{
    return SSP_StartJpegDecoder(data, size, 0, 0, 0, SSP_JPEG_EXIF);
}

 /*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderFast

  Description:  Decodes a JPEG file (Fast version).

                If the image size is a multiple of 8 or 16 pixels (whether 8 or 16 depends on the data format), this function can decode it with the fastest possible speed.
                

  Arguments:    pCtx: Pointer to the context structure.
                         Initialization before and after calling the function is unnecessary. After the function completes, the library will not use it again.
                         The context structure is large, so do not allocate it from a stack in the same way as the local variables.
                         Currently, you cannot call this function simultaneously from multiple strings.
                data: JPEG file
                size: JPEG file size
                dst: Destination buffer for decoding (now only RGB555)
                         Regardless of the output format, 4-byte alignment is required.
                maxWidth: Gives the maximum image width as an argument. (Up to 65535)
                            When the image is decoded successfully, the decoded image's width is returned to pCtx->width.
                maxHeight: Gives the maximum image height as an argument. (Up to 65535)
                            When the image is decoded successfully, the decoded image's height is returned to pCtx->height.
                option: Options (bitwise logical OR)
                         SSP_JPEG_RGB555: Decode in the RGB555 format.
                                Right now only RGB555 is available, but this specification cannot be omitted.
                         SSP_JPEG_THUMBNAIL: Decode thumbnails. When this is unspecified, the main image will be decoded.

  Returns:      TRUE if successful.
                If failed, a detailed error code is stored in pCtx->errorCode.
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoderFast(SSPJpegDecoderFastContext* pCtx, u8* data, u32 size, void* dst, u32 maxWidth, u32 maxHeight, u32 option);

 /*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderFastEx

  Description:  Decodes a JPEG file, specifying whether signature features are enabled (Fast version).

                If the image size is a multiple of 8 or 16 pixels (whether 8 or 16 depends on the data format), this function can decode it with the fastest possible speed.
                

  Arguments:    pCtx: Pointer to the context structure.
                         Initialization before and after calling the function is unnecessary. After the function completes, the library will not use it again.
                         The context structure is large, so do not allocate it from a stack in the same way as the local variables.
                         Currently, you cannot call this function simultaneously from multiple strings.
                data: JPEG file
                size: JPEG file size
                dst: Destination buffer for decoding (now only RGB555)
                         Regardless of the output format, 4-byte alignment is required.
                maxWidth: Gives the maximum image width as an argument. (Up to 65535)
                            When the image is decoded successfully, the decoded image's width is returned to pCtx->width.
                maxHeight: Gives the maximum image height as an argument. (Up to 65535)
                            When the image is decoded successfully, the decoded image's height is returned to pCtx->height.
                option: Options (bitwise logical OR)
                         SSP_JPEG_RGB555: Decode in the RGB555 format.
                                Right now only RGB555 is available, but this specification cannot be omitted.
                         SSP_JPEG_THUMBNAIL: Decode thumbnails. When this is unspecified, the main image will be decoded.
                sign: Specify TRUE to check the signature.

  Returns:      TRUE if successful.
                If failed, a detailed error code is stored in pCtx->errorCode.
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoderFastEx(SSPJpegDecoderFastContext* pCtx, u8* data, u32 size, void* dst, u32 maxWidth, u32 maxHeight, u32 option, BOOL sign);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_JPEGDEC_H_ */
#endif
