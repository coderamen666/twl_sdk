/*---------------------------------------------------------------------------*
  Project:  TwlSDK - inlucde - SCFG
  File:     scfg.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-08#$
  $Rev: 8902 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifndef TWL_COMMON_SCFG_H_
#define TWL_COMMON_SCFG_H_

#include <twl/misc.h>
#include <twl/types.h>

#include <nitro/os/common/system.h>
#include <twl/hw/common/mmap_wramEnv.h>
#include <twl/hw/common/mmap_shared.h>
#ifdef SDK_ARM9
#include <twl/hw/ARM9/ioreg.h>
#else  // SDK_ARM7
#include <twl/hw/ARM7/ioreg.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
//--------------------------------------------------------------------------------
//---------------- register SCFG_ROM
//---- SystemROM select
//     (written as ARM9, but ARM7 is compatible ARM9)
typedef enum
{
	SCFG_SYSTEM_ROM_FOR_TWL = 				(0 << REG_SCFG_A9ROM_RSEL_SHIFT),
	SCFG_SYSTEM_ROM_FOR_NITRO = 			(1 << REG_SCFG_A9ROM_RSEL_SHIFT)
} SCFGSystemRomType;

#define SCFG_SYSTEM_ROM_MASK				(REG_SCFG_A9ROM_RSEL_MASK)

//---- SecureROM cut-off flag
//     (written as ARM9, but ARM7 is compatible ARM9)
typedef enum
{
	SCFG_SECURE_ROM_ACCESSIBLE = 			(0 << REG_SCFG_A9ROM_SEC_SHIFT),
	SCFG_SECURE_ROM_INACCESSIBLE =			(1 << REG_SCFG_A9ROM_SEC_SHIFT)
} SCFGSecureRomAccessible;

#define SCFG_SECURE_ROM_ACCESSIBLE_MASK		(REG_SCFG_A9ROM_SEC_MASK)

//---------------- register SCFG_CLK
#define SCFG_CLOCK_SUPPLY_WRAM				(REG_SCFG_CLK_WRAMHCLK_MASK)
#ifdef SDK_ARM9
#define SCFG_CAMERA_CKI_FLAG				(REG_SCFG_CLK_CAMCKI_MASK)
#define SCFG_CLOCK_SUPPLY_CAMERA			(REG_SCFG_CLK_CAMHCLK_MASK)
#define SCFG_CLOCK_SUPPLY_DSP				(REG_SCFG_CLK_DSPHCLK_MASK)

typedef enum
{
	SCFG_CPU_SPEED_1X =						(0 << REG_SCFG_CLK_CPUSPD_SHIFT),
	SCFG_CPU_SPEED_2X =						(1 << REG_SCFG_CLK_CPUSPD_SHIFT)
} SCFGCpuSpeed;

#define SCFG_CPU_SPEED_MASK					(REG_SCFG_CLK_CPUSPD_MASK)
#endif

//---------------- register SCFG_RST
#ifdef SDK_ARM9
#define SCFG_DSP_RESET_SET					(0 << REG_SCFG_RST_DSPRSTB_SHIFT)
#define SCFG_DSP_RESET_RELEASE				(1 << REG_SCFG_RST_DSPRSTB_SHIFT)
#define SCFG_DSP_RESET_MASK					(REG_SCFG_RST_DSPRSTB_MASK)
#endif

//---------------- register SCFG_EXT
#define SCFG_CONFIG_ENABLE					(1 << REG_SCFG_EXT_CFG_SHIFT)
#define SCFG_CONFIG_DISABLE					(0 << REG_SCFG_EXT_CFG_SHIFT)
#define SCFG_CONFIG_ENABLE_MASK				(REG_SCFG_EXT_CFG_MASK)

#define SCFG_WRAM_ACCESS					(1 << REG_SCFG_EXT_WRAM_SHIFT)		// read only in ARM9

#ifdef SDK_ARM9
#define SCFG_DSP_ACCESS						(1 << REG_SCFG_EXT_DSP_SHIFT)
#define SCFG_CAMERA_ACCESS					(1 << REG_SCFG_EXT_CAM_SHIFT)
#define SCFG_NDMA_ACCESS					(1 << REG_SCFG_EXT_DMAC_SHIFT)
#endif

#define SCFG_EXPANDED_VRAM					(1 << REG_SCFG_EXT_VRAM_SHIFT)
#define SCFG_EXPANDED_LCDC					(1 << REG_SCFG_EXT_LCDC_SHIFT)
#define SCFG_EXPANDED_INTC					(1 << REG_SCFG_EXT_INTC_SHIFT)

typedef enum
{
	SCFG_PSRAM_BOUNDARY_4MB 		=		(0 << REG_SCFG_EXT_PSRAM_SHIFT),
	SCFG_PSRAM_BOUNDARY_4MB_2    	=		(1 << REG_SCFG_EXT_PSRAM_SHIFT),  // treated as 4MB
	SCFG_PSRAM_BOUNDARY_16MB		=		(2 << REG_SCFG_EXT_PSRAM_SHIFT),
	SCFG_PSRAM_BOUNDARY_32MB		=		(3 << REG_SCFG_EXT_PSRAM_SHIFT)
} SCFGPsramBoundary;

#define SCFG_PSRAM_BOUNDARY_MASK			(REG_SCFG_EXT_PSRAM_MASK)


#define SCFG_FIXED_CARD						(1 << REG_SCFG_EXT_MC_SHIFT)
#ifdef SDK_ARM9
#define SCFG_FIXED_DIVIDER					(1 << REG_SCFG_EXT_DIV_SHIFT)
#define SCFG_FIXED_2DENGINE					(1 << REG_SCFG_EXT_G2DE_SHIFT)
#define SCFG_FIXED_RENDERER					(1 << REG_SCFG_EXT_REN_SHIFT)
#define SCFG_FIXED_GEOMETRY					(1 << REG_SCFG_EXT_GEO_SHIFT)
#endif
#define SCFG_FIXED_DMAC						(1 << REG_SCFG_EXT_DMA_SHIFT)


typedef enum
{
	SCFG_CARD_DETECT_MODE_0 =				0,
	SCFG_CARD_DETECT_MODE_1 =				1,
	SCFG_CARD_DETECT_MODE_2 =				2,
	SCFG_CARD_DETECT_MODE_3 =				3
} SCFGCardDetectMode;

#define SCFG_CARD_DETECT_MODE_MASK			3


typedef enum
{
	SCFG_CARD_DETECT_CHATTERING_LOW	=		0,
	SCFG_CARD_DETECT_CHATTERING_HIGH =		1
} SCFGCardDetectChattering;

#define SCFG_CARD_DETECT_CHATTERING_MASK	1

//---------------- register SCFG_OP
#define SCFG_OP_PRODUCT						0
#define SCFG_OP_A9_A7_DEV					1
#define SCFG_OP_A9_DEV						2
#define SCFG_OP_PROM						3


#define SCFGi_CHANGEBIT_8( addr, flag, eval, orval ) \
          do { *(u8*)(addr) = (u8)( ((*(u8*)(addr)) & (0xff ^ (flag))) | ((eval)? (flag): 0) | orval ); } while(0)

#define SCFGi_CHANGEBIT_16( addr, flag, eval, orval ) \
          do { *(u16*)(addr) = (u16)( ((*(u16*)(addr)) & (0xffff ^ (flag))) | ((eval)? (flag): 0) | orval ); } while(0)

#define SCFGi_CHANGEBIT_32( addr, flag, eval, orval )  \
          do { *(u32*)(addr) = (u32)( ((*(u32*)(addr)) & (0xffffffff ^ (flag))) | ((eval)? (flag): 0) | orval ); } while(0)

#define SCFGi_CLEARBIT_8(  addr, flag )    SCFGi_CHANGEBIT_8(  (addr), (flag), 0, 0 )
#define SCFGi_CLEARBIT_16( addr, flag )    SCFGi_CHANGEBIT_16( (addr), (flag), 0, 0 )
#define SCFGi_CLEARBIT_32( addr, flag )    SCFGi_CHANGEBIT_32( (addr), (flag), 0, 0 )


//================================================================================
//  for PXI system
//================================================================================
//---- PXI command field
#define SCFG_PXI_COMMAND_MASK       0x03f00000
#define SCFG_PXI_COMMAND_SHIFT      20
#define SCFG_PXI_ORDINAL_MASK       0x000f0000
#define SCFG_PXI_ORDINAL_SHIFT      16
#define SCFG_PXI_DATA_MASK          0x0000ffff
#define SCFG_PXI_DATA_SHIFT         0

typedef enum {
	SCFGi_PXI_COMMAND_READ = 1,
	SCFGi_PXI_COMMAND_READ_OP = 2
}
SCFGPxiCommand;

typedef void (*SCFGCallback)( u64, void* );


#ifdef SDK_ARM9
//================================================================================
//	for register SCFG_ROM
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_GetSystemRomType

  Description:  get ARM9 System ROM type

  Arguments:    None

  Returns:      SCFG_SYSTEM_ROM_FOR_TWL    : use TWL-CPU 64KB ROM
                SCFG_SYSTEM_ROM_FOR_NITRO  : use NITRO-CPU 8KB ROM
 *---------------------------------------------------------------------------*/
static inline SCFGSystemRomType SCFG_GetSystemRomType(void)
{
	return (*(u32*)(HW_SYS_CONF_BUF + HWi_WSYS08_OFFSET) & HWi_WSYS08_ROM_ARM9RSEL_MASK)? SCFG_SYSTEM_ROM_FOR_NITRO: SCFG_SYSTEM_ROM_FOR_TWL;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
#define SCFG_GetSystemRomType_ARM9()   SCFG_GetSystemRomType()

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsSecureRomAccessible

  Description:  check if the ARM9 secure ROM is accessible from AHB

  Arguments:    None

  Returns:      TRUE  : accessible
                FALSE : inaccessible
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsSecureRomAccessible(void)
{
	return (*(u32*)(HW_SYS_CONF_BUF + HWi_WSYS08_OFFSET) & HWi_WSYS08_ROM_ARM9SEC_MASK)? FALSE: TRUE;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define SCFG_IsSecureRomAccessible_ARM9()   SCFG_IsSecureRomAccessible()

//================================================================================
//	for register SCFG_CLK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_SetCameraCKIClock

  Description:  set CAM_CKI clock for camera

  Arguments:    sw : TRUE  : CAM_CKI enable (output 16.76MHz)
                     FALSE : CAM_CKI disable (output low level)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetCameraCKIClock( BOOL sw )
{
	SCFGi_CHANGEBIT_16( &reg_SCFG_CLK, SCFG_CAMERA_CKI_FLAG, sw, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsCameraCKIClockEnable

  Description:  check if CAM_CKI clock for camera is enable status

  Arguments:    None

  Returns:      TRUE  : enable
                FALSE : disable
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsCameraCKIClockEnable(void)
{
	return (BOOL)((reg_SCFG_CLK & SCFG_CAMERA_CKI_FLAG)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsClockSuppliedToWram

  Description:  check if clock to the new shared WRAM block is supplied

  Arguments:    None

  Returns:      TRUE  : clock is supplied
                FALSE : clock is stopped
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsClockSuppliedToWram(void)
{
	return (BOOL)((reg_SCFG_CLK & SCFG_CLOCK_SUPPLY_WRAM)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SupplyClockToCamera

  Description:  control to supply clock to the CAMERA I/F block

  Arguments:    TRUE  : supply clock
                FALSE : stop clock

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SupplyClockToCamera( BOOL sw )
{
	SCFGi_CHANGEBIT_16( &reg_SCFG_CLK, SCFG_CLOCK_SUPPLY_CAMERA, sw, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsClockSuppliedToCamera

  Description:  check if clock to the CAMERA I/F block is supplied

  Arguments:    None

  Returns:      TRUE  : clock is supplied
                FALSE : clock is stopped
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsClockSuppliedToCamera(void)
{
	return (BOOL)((reg_SCFG_CLK & SCFG_CLOCK_SUPPLY_CAMERA)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SupplyClockToDSP

  Description:  control to supply clock to the DSP block

  Arguments:    TRUE  : supply clockenable
                FALSE : stop clock

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SupplyClockToDSP( BOOL sw )
{
	SCFGi_CHANGEBIT_16( &reg_SCFG_CLK, SCFG_CLOCK_SUPPLY_DSP, sw, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsClockSuppliedToDSP

  Description:  check if clock to DSP block is supplied

  Arguments:    None

  Returns:      TRUE  : clock is supplied
                FALSE : clock is stopped
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsClockSuppliedToDSP(void)
{
	return (BOOL)((reg_SCFG_CLK & SCFG_CLOCK_SUPPLY_DSP)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetCpuSpeed

  Description:  set ARM9 CPU speed

  Arguments:    cpuSpeed : SCFG_CPU_SPEED_1X : same as NITRO (67.03MHz)
                           SCFG_CPU_SPEED_2X : doubled against NITRO (134.06MHz)
  Returns:      None
 *---------------------------------------------------------------------------*/
void SCFG_SetCpuSpeed( SCFGCpuSpeed cpuSpeed );

/*---------------------------------------------------------------------------*
  Name:         SCFG_GetCpuSpeed

  Description:  get ARM9 CPU speed

  Arguments:    None

  Returns:      SCFG_CPU_SPEED_1X : same as NITRO (67.03MHz)
                SCFG_CPU_SPEED_2X : doubled against NITRO (134.06MHz)
 *---------------------------------------------------------------------------*/
static inline SCFGCpuSpeed SCFG_GetCpuSpeed(void)
{
	return (SCFGCpuSpeed)(reg_SCFG_CLK & SCFG_CPU_SPEED_MASK);
}

//================================================================================
// for register SCFG_RST
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFGi_SendResetToDSP

  Description:  do reset to DSP block

  Arguments:    sw: TRUE  : do reset
                    FALSE : release reset

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFGi_SendResetToDSP( BOOL sw )
{
	SCFGi_CHANGEBIT_16( &reg_SCFG_RST, SCFG_DSP_RESET_MASK, sw, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ResetDSP

  Description:  do reset to DSP block

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_ResetDSP( void )
{
	SCFGi_SendResetToDSP( FALSE );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReleaseResetDSP

  Description:  release to reset to DSP block

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_ReleaseResetDSP( void )
{
	SCFGi_SendResetToDSP( TRUE );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsDSPReset

  Description:  check if reset status to DSP block

  Arguments:    None

  Returns:      TRUE  : reset
                FALSE : released
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsDSPReset(void)
{
	return (BOOL)((reg_SCFG_RST & SCFG_DSP_RESET_MASK)? FALSE: TRUE);
}

//================================================================================
// for register SCFG_EXT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_SetConfigBlockInaccessible

  Description:  (for ARM9)
                cut off ARM9 configure block to be inaccessible from CPU
                (for ARM7)
                cut off ARM7 configure block to be inaccessible from CPU

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetConfigBlockInaccessible(void)
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_CONFIG_ENABLE_MASK, 0, SCFG_CONFIG_DISABLE );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsConfigBlockAccessible

  Description:  (for ARM9)
                check if ARM9 configure block is accessible from CPU
                (for ARM7)
                check if ARM7 configure block is accessible from CPU

  Arguments:    None

  Returns:      TRUE  : accessible
                FALSE : inaccessible
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsConfigBlockAccessible(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_CONFIG_ENABLE_MASK)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsWramAccessible

  Description:  check if shared WRAM_A,B,C is accessible

  Arguments:    None

  Returns:      TRUE  : accesssable from the bus which is specified 
                        in another register
                FALSE : inaccessible from any bus
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsWramAccessible(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_WRAM_ACCESS)? TRUE: FALSE);
}

#define SCFG_IsWRAMAccessible   SCFG_IsWramAccessible

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetDSPAccessible

  Description:  set DSP block to be accessible

  Arguments:    sw : TRUE  : set to be accessible
                     FALSE : set to be inaccessible

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetDSPAccessible( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_DSP_ACCESS, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsDSPccessible

  Description:  check if DSP block is accessible

  Arguments:    None

  Returns:      TRUE  : accesssable from AHB
                FALSE : inaccessible
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsDSPAccessible(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_DSP_ACCESS)? TRUE: FALSE);
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_SetCameraAccessible

  Description:  set Camera block to be accessible

  Arguments:    sw : TRUE  : set to be accessible
                     FALSE : set to be inaccessible

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetCameraAccessible( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_CAMERA_ACCESS, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsCameraAccessible

  Description:  check if Camera I/F block is accesible

  Arguments:    None

  Returns:      TRUE  : accesssable from AHB
                FALSE : inaccessible
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsCameraAccessible(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_CAMERA_ACCESS)? TRUE: FALSE);
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_SetNDmaAccessible

  Description:  set new DMA block to be accessible

  Arguments:    sw : TRUE  : set to be accessible
                     FALSE : set to be inaccessible

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetNDmaAccessible( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_NDMA_ACCESS, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsNDmaAccessible

  Description:  check if new DMA block is accessible

  Arguments:    None

  Returns:      TRUE  : accesssable from AHB
                FALSE : inaccessible
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsNDmaAccessible(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_NDMA_ACCESS)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetIntcExpanded

  Description:  set ARM9 INTC circuit to be expanded

  Arguments:    sw : TRUE  : expand circuit
                     FALSE : not expand (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetIntcExpanded( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_EXPANDED_INTC, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsIntcExpanded

  Description:  check if ARM9 INTC circuit is expanded

  Arguments:    None

  Returns:      TRUE  : expanded
                FALSE : not expanded (same as Nitro CPU)
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsIntcExpanded(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_EXPANDED_INTC)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetLCDCExpanded

  Description:  set LCDC circuit to be expanded

  Arguments:    sw : TRUE  : expand circuit
                     FALSE : not expand (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetLCDCExpanded( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_EXPANDED_LCDC, sw, 0 );

	//---- changed SCFG data in shared area
	SCFGi_CHANGEBIT_32( HW_SYS_CONF_BUF + HWi_WSYS04_OFFSET, HWi_WSYS04_EXT_LCDC_MASK, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsLCDCExpanded

  Description:  check if LCDC circuit is expanded

  Arguments:    None

  Returns:      TRUE  : expanded
                FALSE : not expanded (same as Nitro CPU)
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsLCDCExpanded(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_EXPANDED_LCDC)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetVramExpanded

  Description:  set VRAM access to be expanded

  Arguments:    sw : TRUE  : expand circuit
                     FALSE : not expand (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetVramExpanded( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_EXPANDED_VRAM, sw, 0 );

	//---- changed SCFG data in shared area
	SCFGi_CHANGEBIT_32( HW_SYS_CONF_BUF + HWi_WSYS04_OFFSET, HWi_WSYS04_EXT_VRAM_MASK, sw, 0 );
}

#define SCFG_SetVRAMExpanded SCFG_SetVramExpanded 

/*---------------------------------------------------------------------------*
  Name:         SCFG_IsVRamExpanded

  Description:  check if VRAM access is expanded

  Arguments:    None

  Returns:      TRUE  : expanded
                FALSE : not expanded (same as Nitro CPU)
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsVramExpanded(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_EXPANDED_VRAM)? TRUE: FALSE);
}

#define SCFG_IsVRAMExpanded SCFG_IsVramExpanded 

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetPsramBoundary

  Description:  set the boundary for PSRAM access

  Arguments:    boundary: SCFG_PSRAM_BOUNDARY_4MB   : 4MB
                          SCFG_PSRAM_BOUNDARY_16MB  : 16MB
                          SCFG_PSRAM_BOUNDARY_32MB  : 32MB

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetPsramBoundary( SCFGPsramBoundary boundary )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_PSRAM_BOUNDARY_MASK, 0, boundary );

	//---- changed SCFG data in shared area
	SCFGi_CHANGEBIT_32( HW_SYS_CONF_BUF + HWi_WSYS04_OFFSET, HWi_WSYS04_EXT_PSRAM_MASK, 0, boundary );
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_GetPsramBoundary

  Description:  get the setting of the boundary for PSRAM access

  Arguments:    None

  Returns:      SCFG_PSRAM_BOUNDARY_4MB   : 4MB
                SCFG_PSRAM_BOUNDARY_16MB  : 16MB
                SCFG_PSRAM_BOUNDARY_32MB  : 32MB
 *---------------------------------------------------------------------------*/
static inline SCFGPsramBoundary SCFG_GetPsramBoundary(void)
{
	return (SCFGPsramBoundary)(reg_SCFG_EXT & SCFG_PSRAM_BOUNDARY_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetCardFixed

  Description:  set the Card I/F circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetCardFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_CARD, sw, 0 );

	//---- changed SCFG data in shared area
	SCFGi_CHANGEBIT_32( HW_SYS_CONF_BUF + HWi_WSYS04_OFFSET, HWi_WSYS04_EXT_MC_MASK, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsCardFixed

  Description:  Check if the Card I/F circuit is fixed.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsCardFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_CARD)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetDividerFixed

  Description:  set the divider circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetDividerFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_DIVIDER, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsDividerFixed

  Description:  Check if the divider circuit is fixed.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsDividerFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_DIVIDER)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_Set2DEngineFixed

  Description:  set the 2D graphic engine circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_Set2DEngineFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_2DENGINE, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_Is2DEngineFixed

  Description:  Check if the 2D graphic engine circuit is fixed.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_Is2DEngineFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_2DENGINE)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetRendererFixed

  Description:  set the renderer circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetRendererFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_RENDERER, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsRendererFixed

  Description:  Check if the renderer circuit is fixed.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsRendererFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_RENDERER)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetGeometryFixed

  Description:  set the geometory circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetGeometryFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_GEOMETRY, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsGeometryFixed

  Description:  Check if the geometory circuit is fixed.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsGeometryFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_GEOMETRY)? TRUE: FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_SetDmacFixed

  Description:  set the former DMAC circuit to be fixed

  Arguments:    sw : TRUE  : fixed circuit
                     FALSE : not fixed (same as Nitro CPU)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void SCFG_SetDmacFixed( BOOL sw )
{
	SCFGi_CHANGEBIT_32( &reg_SCFG_EXT, SCFG_FIXED_DMAC, sw, 0 );
}
/*---------------------------------------------------------------------------*
  Name:         SCFG_IsDmacFixed

  Description:  Check if the former DMAC circuit is fix.

  Arguments:    None

  Returns:      TRUE  : fixed
                FALSE : same as NITRO CPU
 *---------------------------------------------------------------------------*/
static inline BOOL SCFG_IsDmacFixed(void)
{
	return (BOOL)((reg_SCFG_EXT & SCFG_FIXED_DMAC)? TRUE: FALSE);
}

//================================================================================
// for register MI_MC
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_GetCardDetectMode

  Description:  Get card detect mode. (see manual)

  Arguments:    slot : SCFG_CARD_SLOT_1 : slot 1
                       SCFG_CARD_SLOT_2 : slot 2

  Returns:      SCFG_CARD_DETECT_MODE_0 : detect mode 0
                SCFG_CARD_DETECT_MODE_1 : detect mode 1
                SCFG_CARD_DETECT_MODE_2 : detect mode 2
                SCFG_CARD_DETECT_MODE_3 : detect mode 3
 *---------------------------------------------------------------------------*/
static inline SCFGCardDetectMode SCFG_GetCardDetectMode( int slot )
{
#pragma unused(slot)
	int shift = 2;
	return (SCFGCardDetectMode)((reg_MI_MC & (SCFG_CARD_DETECT_MODE_MASK << shift) ) >> shift);
}

/*---------------------------------------------------------------------------*
  Name:         SCFG_GetCardDetectForChattering

  Description:  Get CDET after chattering removal

  Arguments:    slot : SCFG_CARD_SLOT_1 : slot 1
                       SCFG_CARD_SLOT_2 : slot 2

  Returns:      SCFG_CARD_DETECT_CHATTERING_LOW  : low
                SCFG_CARD_DETECT_CHATTERING_HIGH : high
 *---------------------------------------------------------------------------*/
static inline SCFGCardDetectChattering SCFG_GetCardDetectForChattering( int slot )
{
#pragma unused(slot)
	int shift = 0;
	return (SCFGCardDetectChattering)((reg_MI_MC & (SCFG_CARD_DETECT_CHATTERING_MASK << shift) ) >> shift);
}
#endif


//================================================================================
// Init
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_Init

  Description:  initialize scfg

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SCFG_Init(void);

//================================================================================
//  FUSE
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadFuseDataAsync

  Description:  (ARM9)
                send a read command to ARM7
                (ARM7)
                read fuse data

                The result will be returned to 1st argument of callback.

  Arguments:    callback : callback
                arg      : callback argument 

  Returns:      (ARM9)
                TRUE  : success to send a command
                FALSE : failed (locked.)
                (ARM7)
                always TRUE
 *---------------------------------------------------------------------------*/
BOOL SCFG_ReadFuseDataAsync( SCFGCallback callback, void* arg );

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadFuseData

  Description:  (ARM9)
                read fuse data from ARM7.
                wait till finish to read
                (ARM7)
                read fuse data

  Arguments:    None

  Returns:      (ARM9)
                0  : failed
                >0 : fuse data
                (ARM7)
                (u64)-1  : failed
                other    : fuse data
 *---------------------------------------------------------------------------*/
u64 SCFG_ReadFuseData( void );

#ifdef  SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadBondingOptionAsync

  Description:  send a bonding option read command to ARM7
                The result will be returned to 1st argument of callback.

  Arguments:    callback : callback
                arg      : callback argument 

  Returns:      TRUE  : success to send a command
                FALSE : failed (locked.)
 *---------------------------------------------------------------------------*/
BOOL SCFG_ReadBondingOptionAsync( SCFGCallback callback, void* arg );

/*---------------------------------------------------------------------------*
  Name:         SCFG_ReadBondingOption

  Description:  read bonding option information from ARM7.
                wait till finish to read

  Arguments:    None

  Returns:      0xffff  : failed
                0 ~ 3   : bonding option information
 *---------------------------------------------------------------------------*/
u16 SCFG_ReadBondingOption( void );
#endif

//--------------------------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_COMMON_SCFG_H_ */
#endif
