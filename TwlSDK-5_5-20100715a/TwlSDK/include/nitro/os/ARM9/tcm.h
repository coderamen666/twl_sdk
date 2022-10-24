/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     tcm.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_TCM_H_
#define NITRO_OS_TCM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/hw/ARM9/mmap_global.h>
#else
#include <twl/hw/ARM9/mmap_global.h>
#endif

//=======================================================================
//           ITCM
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableITCM

  Description:  enable ITCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_EnableITCM(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableITCM

  Description:  disable ITCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_DisableITCM(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetITCMParam

  Description:  set parameter to ITCM

  Arguments:    param :  parameter to be set to ITCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetITCMParam(u32 param);

/*---------------------------------------------------------------------------*
  Name:         OS_GetITCMParam

  Description:  get parameter to ITCM

  Arguments:    None

  Returns:      parameter about ITCM
 *---------------------------------------------------------------------------*/
u32     OS_GetITCMParam(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetITCMAddress

  Description:  Get start address of ITCM
                this function always returns HW_ITCM.

  Arguments:    None.

  Returns:      start address of ITCM
 *---------------------------------------------------------------------------*/
static inline u32 OS_GetITCMAddress(void)
{
    return (u32)HW_ITCM;
}


//=======================================================================
//           DTCM
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableDTCM

  Description:  enable DTCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_EnableDTCM(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableDTCM

  Description:  disable DTCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_DisableDTCM(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetDTCMParam

  Description:  set parameter to DTCM

  Arguments:    param :  parameter to be set to DTCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetDTCMParam(u32 param);

/*---------------------------------------------------------------------------*
  Name:         OS_GetDTCMParam

  Description:  get parameter to DTCM

  Arguments:    None

  Returns:      parameter about DTCM
 *---------------------------------------------------------------------------*/
u32     OS_GetDTCMParam(void);



//================================================================================
//          SET TCM ADDRESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetDTCMAddress

  Description:  Set start address of DTCM

  Arguments:    start address of DTCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetDTCMAddress(u32 address);



//================================================================================
//          GET TCM ADDRESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetDTCMAddress

  Description:  Get start address of DTCM

  Arguments:    None.

  Returns:      start address of DTCM
 *---------------------------------------------------------------------------*/
u32     OS_GetDTCMAddress(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_TCM_H_ */
#endif
