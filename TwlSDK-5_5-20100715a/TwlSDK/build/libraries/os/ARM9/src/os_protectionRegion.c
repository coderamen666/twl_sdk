/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_protectionRegion.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $

 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/code32.h>

//================================================================================
//               Instruction Cache for each protection region
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableICacheForProtectionRegion
                OS_DisableICacheForProtectionRegion

  Description:  enable/disable instruction cache for each protection region

  Arguments:    flags :  Each bit shows the region to be enable/disable.
                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_EnableICacheForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c2, c0, 1
    orr     r1, r1, r0
    mcr     p15, 0, r1, c2, c0, 1
    bx      lr
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
asm void OS_DisableICacheForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c2, c0, 1
    bic     r1, r1, r0
    mcr     p15, 0, r1, c2, c0, 1
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetICacheEnableFlagsForProtectionRegion

  Description:  get instruction cache status for each protection region

  Arguments:    None

  Returns:      instruction cache status.
                bit for each protection region, means 1... enable, 0... disable

                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

 *---------------------------------------------------------------------------*/
asm u32 OS_GetICacheEnableFlagsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c2, c0, 1
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetIPermissionsForProtectionRegion

  Description:  set instruction access limitation for each protection region

  Arguments:    setMask : set mask bit, which is or-ed by mask.
                flags   : flag bit, which is or-ed by access limitation.

                   (mask)
                          OS_PRn_ACCESS_MASK (n=0-7)

                   (access limitation)
                          OS_PRn_ACCESS_NA    no permission to access
                          OS_PRn_ACCESS_RW    read and write permission
                          OS_PRn_ACCESS_RO    read only permission
                          (n=0-7)

                ex.
                    In case to set region3 readonly and region6 no permission, call
                    OS_SetIPermissionsForProtectionRegion(
                             OS_PR3_ACCESS_MASK | OS_PR6_ACCESS_MASK,
                             OS_PR3_ACCESS_RO | OS_PR6_ACCESS_NA );

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_SetIPermissionsForProtectionRegion( register u32 setMask, register u32 flags )
{
    mrc     p15, 0, r2, c5, c0, 3
    bic     r2, r2, r0
    orr     r2, r2, r1
    mcr     p15, 0, r2, c5, c0, 3
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetIPermissionsForProtectionRegion

  Description:  get instruction access limitation for each protection region

  Arguments:    None

  Returns:      Return value & OS_PRn_ACCESS_MASK (n=0-7) shows access limitation.

                   (access limitation)
                          OS_PRn_ACCESS_NA    no permission to access
                          OS_PRn_ACCESS_RW    read and write permission
                          OS_PRn_ACCESS_RO    read only permission
                          (n=0-7)

 *---------------------------------------------------------------------------*/
asm u32 OS_GetIPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 3
    bx      lr
}

//================================================================================
//               Data Cache for each protection region
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableDCacheForProtectionRegion
                OS_DisableDCacheForProtectionRegion

  Description:  enable/disable data cache for each protection region

  Arguments:    flags :  Each bit shows the region to be enable/disable.
                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_EnableDCacheForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c2, c0, 0
    orr     r1, r1, r0
    mcr     p15, 0, r1, c2, c0, 0
    bx      lr
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
asm void OS_DisableDCacheForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c2, c0, 0
    bic     r1, r1, r0
    mcr     p15, 0, r1, c2, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDCacheEnableFlagsForProtectionRegion

  Description:  get data cache status for each protection region

  Arguments:    None

  Returns:      data cache status.
                bit for each protection region, means 1... enable, 0... disable

                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

 *---------------------------------------------------------------------------*/
asm u32 OS_GetDCacheEnableFlagsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c2, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetDPermissionsForProtectionRegion

  Description:  set data access limitation for each protection region

  Arguments:    setMask : set mask bit, which is or-ed by mask.
                flags   : flag bit, which is or-ed by access limitation.

                   (mask)
                          OS_PRn_ACCESS_MASK (n=0-7)

                   (access limitation)
                          OS_PRn_ACCESS_NA    no permission to access
                          OS_PRn_ACCESS_RW    read and write permission
                          OS_PRn_ACCESS_RO    read only permission
                          (n=0-7)

                ex.
                    In case to set region3 readonly and region6 no permission, call
                    OS_SetDPermissionsForProtectionRegion(
                             OS_PR3_ACCESS_MASK | OS_PR6_ACCESS_MASK,
                             OS_PR3_ACCESS_RO | OS_PR6_ACCESS_NA );

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_SetDPermissionsForProtectionRegion( register u32 setMask, register u32 flags )
{
    mrc     p15, 0, r2, c5, c0, 2
    bic     r2, r2, r0
    orr     r2, r2, r1
    mcr     p15, 0, r2, c5, c0, 2
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDPermissionsForProtectionRegion

  Description:  get data access limitation for each protection region

  Arguments:    None

  Returns:      Return value & OS_PRn_ACCESS_MASK (n=0-7) shows access limitation.

                   (access limitation)
                          OS_PRn_ACCESS_NA    no permission to access
                          OS_PRn_ACCESS_RW    read and write permission
                          OS_PRn_ACCESS_RO    read only permission
                          (n=0-7)

 *---------------------------------------------------------------------------*/
asm u32 OS_GetDPermissionsForProtectionRegion( void )
{
    mrc     p15, 0, r0, c5, c0, 2
    bx      lr
}

//================================================================================
//               Write buffer for each protection region
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableWriteBufferForProtectionRegion
                OS_DisableWriteBufferForProtectionRegion

  Description:  enable/disable write buffer for each protection region

  Arguments:    flags :  Each bit shows the region to be enable/disable.
                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_EnableWriteBufferForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c3, c0, 0
    orr     r1, r1, r0
    mcr     p15, 0, r1, c3, c0, 0
    bx      lr
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
asm void OS_DisableWriteBufferForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r1, c3, c0, 0
    bic     r1, r1, r0
    mcr     p15, 0, r1, c3, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetWriteBufferEnableFlagsForProtectionRegion

  Description:  get write buffer status for each protection region

  Arguments:    None

  Returns:      write buffer status.
                bit for each protection region, means 1... enable, 0... disable

                           bit0(LSB) ... protection region 0
                           bit1      ... protection region 1
                             :                  :
                           bit7      ... protection region 7

 *---------------------------------------------------------------------------*/
asm u32 OS_GetWriteBufferEnableFlagsForProtectionRegion( register u32 flags )
{
    mrc     p15, 0, r0, c3, c0, 0
    bx      lr
}

//================================================================================
//          protection region
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegionParam

  Description:  set up parameter of protection region

  Arguments:    regionNo :   region number
                param    :   parameter ( created from base address and size )

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_SetProtectionRegionParam( int regionNo, u32 param )
{
	static void (*f[])(u32) =
	{
		OS_SetProtectionRegion0,
		OS_SetProtectionRegion1,
		OS_SetProtectionRegion2,
		OS_SetProtectionRegion3,
		OS_SetProtectionRegion4,
		OS_SetProtectionRegion5,
		OS_SetProtectionRegion6,
		OS_SetProtectionRegion7
	};
    SDK_ASSERT( regionNo>=0 && regionNo<OS_PROTECTION_REGION_NUM );

	f[regionNo](param);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegion*

  Description:  set up parameter of each protection region

  Arguments:    param    :   parameter ( created from base address and size )

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_SetProtectionRegion0( u32 param )
{
    mcr     p15, 0, r0, c6, c0, 0
    bx      lr
}

asm void OS_SetProtectionRegion1( u32 param )
{
    mcr     p15, 0, r0, c6, c1, 0
    bx      lr
}

asm void OS_SetProtectionRegion2( u32 param )
{
    mcr     p15, 0, r0, c6, c2, 0
    bx      lr
}

asm void OS_SetProtectionRegion3( u32 param )
{
    mcr     p15, 0, r0, c6, c3, 0
    bx      lr
}

asm void OS_SetProtectionRegion4( u32 param )
{
    mcr     p15, 0, r0, c6, c4, 0
    bx      lr
}

asm void OS_SetProtectionRegion5( u32 param )
{
    mcr     p15, 0, r0, c6, c5, 0
    bx      lr
}

asm void OS_SetProtectionRegion6( u32 param )
{
    mcr     p15, 0, r0, c6, c6, 0
    bx      lr
}

asm void OS_SetProtectionRegion7( u32 param )
{
    mcr     p15, 0, r0, c6, c7, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegionParam

  Description:  get parameter of protection region

  Arguments:    regionNo :   region number

  Returns:      parameter of protection region ( created from base address and size )
 *---------------------------------------------------------------------------*/
u32  OS_GetProtectionRegionParam( int regionNo )
{
	static u32 (*f[])(void) =
	{
		OS_GetProtectionRegion0,
		OS_GetProtectionRegion1,
		OS_GetProtectionRegion2,
		OS_GetProtectionRegion3,
		OS_GetProtectionRegion4,
		OS_GetProtectionRegion5,
		OS_GetProtectionRegion6,
		OS_GetProtectionRegion7
	};
    SDK_ASSERT( regionNo>=0 && regionNo<OS_PROTECTION_REGION_NUM );

	return f[regionNo]();
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegion*

  Description:  get parameter of each protection region

  Arguments:    None

  Returns:      parameter of protection region ( created from base address and size )
 *---------------------------------------------------------------------------*/
asm u32 OS_GetProtectionRegion0( void )
{
    mrc     p15, 0, r0, c6, c0, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion1( void )
{
    mrc     p15, 0, r0, c6, c1, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion2( void )
{
    mrc     p15, 0, r0, c6, c2, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion3( void )
{
    mrc     p15, 0, r0, c6, c3, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion4( void )
{
    mrc     p15, 0, r0, c6, c4, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion5( void )
{
    mrc     p15, 0, r0, c6, c5, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion6( void )
{
    mrc     p15, 0, r0, c6, c6, 0
    bx      lr
}

asm u32 OS_GetProtectionRegion7( void )
{
    mrc     p15, 0, r0, c6, c7, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegionEx

  Description:  set up parameter of protection region

  Arguments:    regionNo :   region number
                address  :   base address
                sizeDef  :   size value.

                regionNo must be immidiate number, from 0 to 7.
                sizeStr is OS_PR_SIZE_xxx  (xxx=4KB, 8KB, 16KB, ..., 4GB )

                ex. OSi_SetProtectionRegion( 3, 0x2000000, OS_PR_SIZE_4KB );

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_SetProtectionRegionEx( int regionNo, u32 address, u32 sizeDef )
{
	u32 mask = 0xfffff000 << OSi_PRDEF_TO_VAL(sizeDef);
	u32 param = OSi_CalcPRParam( (address), sizeDef, mask ) | HW_C6_PR_ENABLE;

	OS_SetProtectionRegionParam( regionNo, param );
}

/*---------------------------------------------------------------------------*
  Name:         OS_DumpProtectionRegion

  Description:  dump protection region list (for DEBUG)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
void OS_DumpProtectionRegion( void )
{
	int n;
    OSIntrMode enable = OS_DisableInterrupts();
	u32 i_cache  = OS_GetICacheEnableFlagsForProtectionRegion();
	u32 d_cache  = OS_GetDCacheEnableFlagsForProtectionRegion();
	u32 i_permit = OS_GetIPermissionsForProtectionRegion();
	u32 d_permit = OS_GetDPermissionsForProtectionRegion();

	OS_TPrintf("----Protection Region\nPR ICache DCache IPermit DPermit  Address    Size\n");
	for( n=0; n<OS_PROTECTION_REGION_NUM; n++ )
	{
		char* dpermit;
		char* ipermit;

		switch( (i_permit >> (n<<2)) & HW_C5_PERMIT_MASK )
		{
			case HW_C5_PERMIT_NA: ipermit = "--"; break;
			case HW_C5_PERMIT_RW: ipermit = "RW"; break;
			case HW_C5_PERMIT_RO: ipermit = "R-"; break;
			default: ipermit = "??"; break;
		}
		switch( (d_permit >> (n<<2)) & HW_C5_PERMIT_MASK )
		{
			case HW_C5_PERMIT_NA: dpermit = "--"; break;
			case HW_C5_PERMIT_RW: dpermit = "RW"; break;
			case HW_C5_PERMIT_RO: dpermit = "R-"; break;
			default: dpermit = "??"; break;
		}

		OS_TPrintf(" %d   %c      %c      %s      %s    0x%08x 0x%x\n",
				  n,
				  (i_cache & (1<<n))? 'O': '-',
				  (d_cache & (1<<n))? 'O': '-',
				  ipermit,
				  dpermit,
				  OS_GetProtectionRegionAddress(n),
				  0x1000 << OSi_PRDEF_TO_VAL(OS_GetProtectionRegionSize(n)), i_permit, d_permit );
	}

	(void)OS_RestoreInterrupts(enable);
}
#endif // ifndef SDK_FINALROM

#include <nitro/codereset.h>
