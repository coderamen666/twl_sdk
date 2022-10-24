/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera.c

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
#include <twl.h>
#include <twl/camera.h>


/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToWidth

  Description:  retrieve width from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      width in pixels
 *---------------------------------------------------------------------------*/
u16 CAMERA_SizeToWidthCore( CAMERASize size )
{
    switch (size)
    {
    case CAMERA_SIZE_VGA:       return 640;
    case CAMERA_SIZE_QVGA:      return 320;
    case CAMERA_SIZE_QQVGA:     return 160;
    case CAMERA_SIZE_CIF:       return 352;
    case CAMERA_SIZE_QCIF:      return 176;
    case CAMERA_SIZE_DS_LCD:    return 256;
    case CAMERA_SIZE_DS_LCDx4:  return 512;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToHeight

  Description:  retrieve height from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      height in pixels
 *---------------------------------------------------------------------------*/
u16 CAMERA_SizeToHeightCore( CAMERASize size )
{
    switch (size)
    {
    case CAMERA_SIZE_VGA:       return 480;
    case CAMERA_SIZE_QVGA:      return 240;
    case CAMERA_SIZE_QQVGA:     return 120;
    case CAMERA_SIZE_CIF:       return 288;
    case CAMERA_SIZE_QCIF:      return 144;
    case CAMERA_SIZE_DS_LCD:    return 192;
    case CAMERA_SIZE_DS_LCDx4:  return 384;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SizeToPixels

  Description:  retrieve total pixels from CAMERASize enum.

  Arguments:    size    one of CAMERASize

  Returns:      pixels
 *---------------------------------------------------------------------------*/
u32 CAMERA_SizeToPixelsCore( CAMERASize size )
{
    switch (size)
    {
    case CAMERA_SIZE_VGA:       return 640*480;
    case CAMERA_SIZE_QVGA:      return 320*240;
    case CAMERA_SIZE_QQVGA:     return 160*120;
    case CAMERA_SIZE_CIF:       return 352*288;
    case CAMERA_SIZE_QCIF:      return 176*144;
    case CAMERA_SIZE_DS_LCD:    return 256*192;
    case CAMERA_SIZE_DS_LCDx4:  return 512*384;
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
CAMERASize CAMERA_WidthHeightToSizeCore( u16 width, u16 height )
{
    int size;
    for (size = 0; size < CAMERA_SIZE_MAX; size++)
    {
        if (width == CAMERA_SizeToWidthCore((CAMERASize)size))
        {
            if (height == CAMERA_SizeToHeightCore((CAMERASize)size))
            {
                break;
            }
            return CAMERA_SIZE_MAX;
        }
    }
    return (CAMERASize)size;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WidthToSize

  Description:  retrieve CAMERASize enum from width.

  Arguments:    width       frame width

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_WidthToSizeCore( u16 width )
{
    int size;
    for (size = 0; size < CAMERA_SIZE_MAX; size++)
    {
        if (width == CAMERA_SizeToWidthCore((CAMERASize)size))
        {
            break;
        }
    }
    return (CAMERASize)size;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_HeightToSize

  Description:  retrieve CAMERASize enum from height.

  Arguments:    height      frame height

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_HeightToSizeCore( u16 height )
{
    int size;
    for (size = 0; size < CAMERA_SIZE_MAX; size++)
    {
        if (height == CAMERA_SizeToHeightCore((CAMERASize)size))
        {
            break;
        }
    }
    return (CAMERASize)size;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PixelsToSize

  Description:  retrieve CAMERASize enum from total pixels.

  Arguments:    pixels      total pixels

  Returns:      one of CAMERASize
 *---------------------------------------------------------------------------*/
CAMERASize CAMERA_PixelsToSizeCore( u32 pixels )
{
    int size;
    for (size = 0; size < CAMERA_SIZE_MAX; size++)
    {
        if (pixels == CAMERA_SizeToPixelsCore((CAMERASize)size))
        {
            break;
        }
    }
    return (CAMERASize)size;
}
