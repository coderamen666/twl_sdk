/*---------------------------------------------------------------------------*
  Project:  TwlSDK - cht - include
  File:     pictocatch.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CHT_ARM9_PICTOCATCH_H_
#define TWL_CHT_ARM9_PICTOCATCH_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro/types.h>
#include <nitro/wm.h>


/*---------------------------------------------------------------------------*
  Name:         CHT_IsPictochatParent

  Description:  Checks whether parent's beacon information indicates it is a PictoChat beacon.
                

  Arguments:    pWmBssDesc:     Pointer to beacon information being checked.

  Returns:      BOOL - Returns TRUE for PictoChat beacons and FALSE otherwise.
                                
 *---------------------------------------------------------------------------*/
BOOL    CHT_IsPictochatParent( const WMBssDesc* pWmBssDesc );

/*---------------------------------------------------------------------------*
  Name:         CHT_GetPictochatClientNum

  Description:  Checks the PictoChat beacon information to see how many children are participating in the group.
                
                If the beacon is not a PictoChat beacon this function returns an undefined value, so before running this execute CHT_IsPictochatParent to verify it is a PictoChat beacon.
                

  Arguments:    pWmBssDesc:     Pointer to beacon information being checked.

  Returns:      int:           The number of children participating in group.
                                Returns -1 if beacon information is abnormal.
 *---------------------------------------------------------------------------*/
int     CHT_GetPictochatClientNum( const WMBssDesc* pWmBssDesc );

/*---------------------------------------------------------------------------*
  Name:         CHT_GetPictochatRoomNumber

  Description:  Gets the room number from the PictoChat beacon information.
                If the beacon is not a PictoChat beacon this function returns an undefined value, so before running this execute CHT_IsPictochatParent to verify it is a PictoChat beacon.
                

  Arguments:    pWmBssDesc:     Pointer to beacon information being checked.

  Returns:      int:           Returns the room number.
                                Returns -1 if beacon information is abnormal.
 *---------------------------------------------------------------------------*/
int     CHT_GetPictochatRoomNumber( const WMBssDesc* pWmBssDesc );


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif  /* TWL_CHT_ARM9_PICTOCATCH_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
