/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_tcm.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/code32.h>

//=======================================================================
//           ITCM
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableITCM

  Description:  enable ITCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_EnableITCM( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #HW_C1_ITCM_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_DisableITCM

  Description:  disable ITCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_DisableITCM( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #HW_C1_ITCM_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetITCMParam

  Description:  set parameter to ITCM

  Arguments:    param :  parameter to be set to ITCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_SetITCMParam( register u32 param )
{
    and     r0, r0, #HW_C9_TCMR_SIZE_MASK
    mcr     p15, 0, r0, c9, c1, 1
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetITCMParam

  Description:  get parameter to ITCM

  Arguments:    None

  Returns:      parameter about ITCM
 *---------------------------------------------------------------------------*/
asm u32 OS_GetITCMParam( void )
{
    mrc     p15, 0, r0, c9, c1, 1
    and     r0, r0, #HW_C9_TCMR_SIZE_MASK
    bx      lr
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
asm void OS_EnableDTCM( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #HW_C1_DTCM_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_DisableDTCM

  Description:  disable DTCM

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_DisableDTCM( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #HW_C1_DTCM_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetDTCMParam

  Description:  set parameter to DTCM

  Arguments:    param :  parameter to be set to DTCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_SetDTCMParam( register u32 param )
{
    ldr     r1, =HW_C9_TCMR_BASE_MASK | HW_C9_TCMR_SIZE_MASK
    and     r0, r0, r1
    mcr     p15, 0, r0, c9, c1, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDTCMParam

  Description:  get parameter to DTCM

  Arguments:    None

  Returns:      parameter about DTCM
 *---------------------------------------------------------------------------*/
asm u32 OS_GetDTCMParam( void )
{
    mrc     p15, 0, r0, c9, c1, 0
    ldr     r1, =HW_C9_TCMR_BASE_MASK | HW_C9_TCMR_SIZE_MASK
    and     r0, r0, r1
    bx      lr
}

//================================================================================
//          SET TCM ADDRESS
//================================================================================
#define OSi_TCM_REGION_SIZE_MASK  HW_C9_TCMR_SIZE_MASK  // 0x0000003e
#define OSi_TCM_REGION_BASE_MASK  HW_C9_TCMR_BASE_MASK  // 0xfffff000

/*---------------------------------------------------------------------------*
  Name:         OS_SetDTCMAddress

  Description:  Set start address of DTCM

  Arguments:    start address of DTCM

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_SetDTCMAddress( register u32 address )
{
    mrc      p15, 0, r2, c9, c1, 0
    and      r2, r2, #OSi_TCM_REGION_SIZE_MASK

    ldr      r1, =OSi_TCM_REGION_BASE_MASK
    and      r0, r0, r1
    orr      r0, r0, r2
    mcr      p15, 0, r0, c9, c1, 0

    bx       lr
}

//================================================================================
//          GET TCM ADDRESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetDTCMAddress

  Description:  Get start address of DTCM

  Arguments:    None.

  Returns:      start address of DTCM
 *---------------------------------------------------------------------------*/
asm u32 OS_GetDTCMAddress( void )
{
    mrc      p15, 0, r0, c9, c1, 0
    ldr      r1, =OSi_TCM_REGION_BASE_MASK
    and      r0, r0, r1

    bx       lr
}

#include <nitro/codereset.h>
