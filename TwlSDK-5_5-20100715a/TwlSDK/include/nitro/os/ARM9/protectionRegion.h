/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     protectionRegion.h

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

#ifndef NITRO_OS_PROTECTIONREGION_H_
#define NITRO_OS_PROTECTIONREGION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

#define OS_PROTECTION_REGION_NUM	8

#define OS_PR0_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR0_SFT)
#define OS_PR0_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR0_SFT)
#define OS_PR0_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR0_SFT)
#define OS_PR0_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR0_SFT)

#define OS_PR1_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR1_SFT)
#define OS_PR1_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR1_SFT)
#define OS_PR1_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR1_SFT)
#define OS_PR1_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR1_SFT)

#define OS_PR2_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR2_SFT)
#define OS_PR2_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR2_SFT)
#define OS_PR2_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR2_SFT)
#define OS_PR2_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR2_SFT)

#define OS_PR3_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR3_SFT)
#define OS_PR3_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR3_SFT)
#define OS_PR3_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR3_SFT)
#define OS_PR3_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR3_SFT)

#define OS_PR4_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR4_SFT)
#define OS_PR4_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR4_SFT)
#define OS_PR4_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR4_SFT)
#define OS_PR4_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR4_SFT)

#define OS_PR5_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR5_SFT)
#define OS_PR5_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR5_SFT)
#define OS_PR5_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR5_SFT)
#define OS_PR5_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR5_SFT)

#define OS_PR6_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR6_SFT)
#define OS_PR6_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR6_SFT)
#define OS_PR6_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR6_SFT)
#define OS_PR6_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR6_SFT)

#define OS_PR7_ACCESS_MASK   (HW_C5_PERMIT_MASK << HW_C5_PR7_SFT)
#define OS_PR7_ACCESS_NA     (HW_C5_PERMIT_NA   << HW_C5_PR7_SFT)
#define OS_PR7_ACCESS_RW     (HW_C5_PERMIT_RW   << HW_C5_PR7_SFT)
#define OS_PR7_ACCESS_RO     (HW_C5_PERMIT_RO   << HW_C5_PR7_SFT)

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
void    OS_EnableICacheForProtectionRegion(register u32 flags);
void    OS_DisableICacheForProtectionRegion(register u32 flags);


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
u32     OS_GetICacheEnableFlagsForProtectionRegion(void);

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
void    OS_SetIPermissionsForProtectionRegion(u32 setMask, u32 flags);

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
u32     OS_GetIPermissionsForProtectionRegion(void);


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
void    OS_EnableDCacheForProtectionRegion(u32 flags);
void    OS_DisableDCacheForProtectionRegion(u32 flags);

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
u32     OS_GetDCacheEnableFlagsForProtectionRegion(void);

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
void    OS_SetDPermissionsForProtectionRegion(u32 setMask, u32 flags);

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
u32     OS_GetDPermissionsForProtectionRegion(void);


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
void    OS_EnableWriteBufferForProtectionRegion(u32 flags);
void    OS_DisableWriteBufferForProtectionRegion(u32 flags);

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
u32     OS_GetWriteBufferEnableFlagsForProtectionRegion(u32 flags);



//================================================================================
//          protection region
//================================================================================

typedef enum
{
    OSi_PR_BASE_MASK_4KB = 0xfffff000,
    OSi_PR_BASE_MASK_8KB = 0xffffe000,
    OSi_PR_BASE_MASK_16KB = 0xffffc000,
    OSi_PR_BASE_MASK_32KB = 0xffff8000,
    OSi_PR_BASE_MASK_64KB = 0xffff0000,
    OSi_PR_BASE_MASK_128KB = 0xfffe0000,
    OSi_PR_BASE_MASK_256KB = 0xfffc0000,
    OSi_PR_BASE_MASK_512KB = 0xfff80000,
    OSi_PR_BASE_MASK_1MB = 0xfff00000,
    OSi_PR_BASE_MASK_2MB = 0xffe00000,
    OSi_PR_BASE_MASK_4MB = 0xffc00000,
    OSi_PR_BASE_MASK_8MB = 0xff800000,
    OSi_PR_BASE_MASK_16MB = 0xff000000,
    OSi_PR_BASE_MASK_32MB = 0xfe000000,
    OSi_PR_BASE_MASK_64MB = 0xfc000000,
    OSi_PR_BASE_MASK_128MB = 0xf8000000,
    OSi_PR_BASE_MASK_256MB = 0xf0000000,
    OSi_PR_BASE_MASK_512MB = 0xe0000000,
    OSi_PR_BASE_MASK_1GB = 0xc0000000,
    OSi_PR_BASE_MASK_2GB = 0x80000000,
    OSi_PR_BASE_MASK_4GB = 0x00000000
}
OSiProtectionRegionBaseMask;

#define OS_PR_SIZE_4KB        HW_C6_PR_4KB
#define OS_PR_SIZE_8KB        HW_C6_PR_8KB
#define OS_PR_SIZE_16KB       HW_C6_PR_16KB
#define OS_PR_SIZE_32KB       HW_C6_PR_32KB
#define OS_PR_SIZE_64KB       HW_C6_PR_64KB
#define OS_PR_SIZE_128KB      HW_C6_PR_128KB
#define OS_PR_SIZE_256KB      HW_C6_PR_256KB
#define OS_PR_SIZE_512KB      HW_C6_PR_512KB
#define OS_PR_SIZE_1MB        HW_C6_PR_1MB
#define OS_PR_SIZE_2MB        HW_C6_PR_2MB
#define OS_PR_SIZE_4MB        HW_C6_PR_4MB
#define OS_PR_SIZE_8MB        HW_C6_PR_8MB
#define OS_PR_SIZE_16MB       HW_C6_PR_16MB
#define OS_PR_SIZE_32MB       HW_C6_PR_32MB
#define OS_PR_SIZE_64MB       HW_C6_PR_64MB
#define OS_PR_SIZE_128MB      HW_C6_PR_128MB
#define OS_PR_SIZE_256MB      HW_C6_PR_256MB
#define OS_PR_SIZE_512MB      HW_C6_PR_512MB
#define OS_PR_SIZE_1GB        HW_C6_PR_1GB
#define OS_PR_SIZE_2GB        HW_C6_PR_2GB
#define OS_PR_SIZE_4GB        HW_C6_PR_4GB

// convert OS_PR_SIZE_xxx to number.
// OS_PR_SIZE_4KB = 0, OS_PR_SIZE_8KB = 1, ...
#define OSi_PRDEF_TO_VAL(prDef) (((prDef)-0x16) >> 1)

// convert number to OS_PR_SIZE_xxx
// 0 = OS_PR_SIZE_4KB, 1 = OS_PR_SIZE_8KB, ...
#define OSi_VAL_TO_PRDEF(val)   (((val)<<1) + 0x16)

/*---------------------------------------------------------------------------*
  Name:         OSi_CalcPRParam

  Description:  set up parameter of protection region

  Arguments:    address  :   base address
                size     :   HW_C6_PR_xxx (xxx=4KB, ,... 4GB)
                mask     ;   OSi_PR_BASE_MASK_xxx (xxx=4KB, ... 4GB)

  Returns:      parameter to set protection region
 *---------------------------------------------------------------------------*/
static inline u32 OSi_CalcPRParam(u32 address, u32 size, u32 mask)
{
#ifdef SDK_DEBUG
    SDK_ASSERT((address & mask) == address);
#endif
    return ((address & mask) | size);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegionParam

  Description:  set up parameter of protection region

  Arguments:    regionNo :   region number
                param    :   parameter ( created from base address and size )

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetProtectionRegionParam( int regionNo, u32 param );

/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegion*

  Description:  set up parameter of each protection region

  Arguments:    param    :   parameter ( created from base address and size )

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetProtectionRegion0(u32 param);
void    OS_SetProtectionRegion1(u32 param);
void    OS_SetProtectionRegion2(u32 param);
void    OS_SetProtectionRegion3(u32 param);
void    OS_SetProtectionRegion4(u32 param);
void    OS_SetProtectionRegion5(u32 param);
void    OS_SetProtectionRegion6(u32 param);
void    OS_SetProtectionRegion7(u32 param);

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegionParam

  Description:  get parameter of protection region

  Arguments:    regionNo :   region number

  Returns:      parameter of protection region ( created from base address and size )
 *---------------------------------------------------------------------------*/
u32     OS_GetProtectionRegionParam( int regionNo );

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegion*

  Description:  get parameter of each protection region

  Arguments:    None

  Returns:      parameter of protection region ( created from base address and size )
 *---------------------------------------------------------------------------*/
u32     OS_GetProtectionRegion0(void);
u32     OS_GetProtectionRegion1(void);
u32     OS_GetProtectionRegion2(void);
u32     OS_GetProtectionRegion3(void);
u32     OS_GetProtectionRegion4(void);
u32     OS_GetProtectionRegion5(void);
u32     OS_GetProtectionRegion6(void);
u32     OS_GetProtectionRegion7(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetProtectionRegion

  Description:  set up parameter of protection region

  Arguments:    regionNo :   region number
                address  :   base address
                sizeStr  :   string that stands for size

               regionNo must be immidiate number, from 0 to 7.
               address can be valiable
               sizeStr is 4KB or 8KB or 16KB or ... 4GB

               ex. OSi_SetProtectionRegion( 3, 0x2000000, 4KB );

  Returns:      None
 *---------------------------------------------------------------------------*/
#define OS_SetProtectionRegion( regionNo, address, sizeStr )  \
  OS_SetProtectionRegionParam( (regionNo), \
							   OSi_CalcPRParam( (address), HW_C6_PR_##sizeStr, OSi_PR_BASE_MASK_##sizeStr ) \
                              | HW_C6_PR_ENABLE )

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
void OS_SetProtectionRegionEx( int regionNo, u32 address, u32 sizeDef );

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegionAddress

  Description:  get parameter of protection region

  Arguments:    regionNo :   region number

  Returns:      base address of protection region.
 *---------------------------------------------------------------------------*/
#define OS_GetProtectionRegionAddress( regionNo )  ( OS_GetProtectionRegionParam((regionNo)) & HW_C6_PR_BASE_MASK )

/*---------------------------------------------------------------------------*
  Name:         OS_GetProtectionRegionSize

  Description:  get parameter of protection region

  Arguments:    regionNo :   region number

  Returns:      size parameter of protection region.  
                parameter is HW_C6_PR_SIZE_xxx, xxx is 4KB or 8KB or ... 4GB
 *---------------------------------------------------------------------------*/
#define OS_GetProtectionRegionSize( regionNo )  ( OS_GetProtectionRegionParam((regionNo)) & HW_C6_PR_SIZE_MASK )

/*---------------------------------------------------------------------------*
  Name:         OS_DumpProtectionRegion

  Description:  dump protection region list (for DEBUG)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
void OS_DumpProtectionRegion( void );
#else
#define OS_DumpProtectionRegion()  ((void)0)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_PROTECTIONREGION_H_ */
#endif
