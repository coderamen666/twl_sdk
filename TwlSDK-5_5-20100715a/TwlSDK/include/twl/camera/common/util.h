/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     util.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_UTIL_H_
#define TWL_CAMERA_UTIL_H_

#include <twl/types.h>
#include <twl/camera/common/types.h>
#include <nitro/os/common/emulator.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToWidth

  Description:  retrieve width from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      width in pixels
 *---------------------------------------------------------------------------*/
u16 CAMERA_SizeToWidthCore( CAMERASize size );
SDK_INLINE u16 CAMERA_SizeToWidth( CAMERASize size )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SizeToWidthCore(size);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToHeight

  Description:  retrieve height from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      height in pixels
 *---------------------------------------------------------------------------*/
u16 CAMERA_SizeToHeightCore( CAMERASize size );
SDK_INLINE u16 CAMERA_SizeToHeight( CAMERASize size )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SizeToHeightCore(size);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToPixels

  Description:  retrieve total pixels from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      pixels
 *---------------------------------------------------------------------------*/
u32 CAMERA_SizeToPixelsCore( CAMERASize size );
SDK_INLINE u32 CAMERA_SizeToPixels( CAMERASize size )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SizeToPixelsCore(size);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WidthHeightToSize

  Description:  retrieve CAMERASize enum from width and height.

  Arguments:    width       frame width
                height      frame height

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_WidthHeightToSizeCore( u16 width, u16 height );
SDK_INLINE CAMERASize CAMERA_WidthHeightToSize( u16 width, u16 height )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_WidthHeightToSizeCore(width, height);
    }
    return (CAMERASize)0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WidthToSize

  Description:  retrieve CAMERASize enum from width.

  Arguments:    width       frame width

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_WidthToSizeCore( u16 width );
SDK_INLINE CAMERASize CAMERA_WidthToSize( u16 width )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_WidthToSizeCore(width);
    }
    return (CAMERASize)0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_HeightToSize

  Description:  retrieve CAMERASize enum from height.

  Arguments:    height      frame height

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_HeightToSizeCore( u16 height );
SDK_INLINE CAMERASize CAMERA_HeightToSize( u16 height )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_HeightToSizeCore(height);
    }
    return (CAMERASize)0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PixelsToSize

  Description:  retrieve CAMERASize enum from total pixels.

  Arguments:    pixels      total pixels

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_PixelsToSizeCore( u32 pixels );
SDK_INLINE CAMERASize CAMERA_PixelsToSize( u32 pixels )
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_PixelsToSizeCore(pixels);
    }
    return (CAMERASize)0;
}

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_CAMERA_UTIL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
