/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl - HW
  File:     mmap_shared.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11065 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_HW_COMMON_MMAP_PARAMETER_H_
#define TWL_HW_COMMON_MMAP_PARAMETER_H_
#ifdef  __cplusplus
extern  "C" {
#endif

//--------------------------------------------------------------------------------
//---- deliver argument area			0x02000000 - 0x02000300
#define HW_PARAM_DELIVER_ARG_OFS           0x000
#define HW_PARAM_DELIVER_ARG_SIZE          0x300
#define HW_PARAM_DELIVER_ARG               (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_OFS)
#define HW_PARAM_DELIVER_ARG_END           (HW_PARAM_DELIVER_ARG + HW_PARAM_DELIVER_ARG_SIZE)

//---- deliver argument
#define HW_PARAM_DELIVER_ARG_TITLEID_OFS   0x0
#define HW_PARAM_DELIVER_ARG_GAMECODE_OFS  0x8
#define HW_PARAM_DELIVER_ARG_MAKERCODE_OFS 0xc
#define HW_PARAM_DELIVER_ARG_DATASIZE_OFS  0xe
#define HW_PARAM_DELIVER_ARG_FLAG_OFS      0x10
#define HW_PARAM_DELIVER_ARG_DATA_OFS      0x11

#define HW_PARAM_DELIVER_ARG_TITLEID       (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_TITLEID_OFS)
#define HW_PARAM_DELIVER_ARG_GAMECODE      (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_GAMECODE_OFS)
#define HW_PARAM_DELIVER_ARG_MAKERCODE     (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_MAKERCODE_OFS)
#define HW_PARAM_DELIVER_ARG_DATASIZE      (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_DATASIZE_OFS)
#define HW_PARAM_DELIVER_ARG_FLAG          (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_FLAG_OFS)
#define HW_PARAM_DELIVER_ARG_DATA          (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_DELIVER_ARG_DATA_OFS)

//----------------------------------------------------------------
//---- launcher parameter area			0x02000300 - 0x02000400
#define HW_PARAM_LAUNCH_PARAM_OFS          0x300
#define HW_PARAM_LAUNCH_PARAM_SIZE         0x100
#define HW_PARAM_LAUNCH_PARAM              (HW_MAIN_MEM_PARAMETER_BUF + HW_PARAM_LAUNCH_PARAM_OFS)
#define HW_PARAM_LAUNCH_PARAM_END          (HW_PARAM_LAUNCH_PARAM + HW_PARAM_LAUNCH_PARAM_SIZE)

//----------------------------------------------------------------
//---- TWLSettingsData					0x02000400 - 0x02005e0
#define HW_PARAM_TWL_SETTINGS_DATA_DEFAULT		HW_PARAM_LAUNCH_PARAM_END
#define HW_PARAM_TWL_SETTINGS_DATA				( OS_GetSystemWork()->preloadParameterAddr )
#define HW_PARAM_TWL_SETTINGS_DATA_SIZE			0x1e0
#define HW_PARAM_TWL_SETTINGS_DATA_END			( OS_GetSystemWork()->preloadParameterAddr + HW_PARAM_TWL_SETTINGS_DATA_SIZE )

//---- wireless firmware initial data	0x020005e0 - 0x02000600
#define HW_PARAM_WIRELESS_FIRMWARE_DATA			HW_PARAM_TWL_SETTINGS_DATA_END
#define HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE	0x20
#define HW_PARAM_WIRELESS_FIRMWARE_DATA_END		( HW_PARAM_WIRELESS_FIRMWARE_DATA + HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE )

//---- TWL-HW-NormalInfo				0x02000600 - 0x0200620
#define HW_PARAM_TWL_HW_NORMAL_INFO				HW_PARAM_WIRELESS_FIRMWARE_DATA_END
#define HW_PARAM_TWL_HW_NORMAL_INFO_SIZE		0x20
#define HW_PARAM_TWL_HW_NORMAL_INFO_END			( HW_PARAM_TWL_HW_NORMAL_INFO + HW_PARAM_TWL_HW_NORMAL_INFO_SIZE )

//---- reserved area					0x02000620 - 0x02004000
#define HW_PARAM_TWL_RESERVED_AREA				HW_PARAM_TWL_HW_NORMAL_INFO_END
#define HW_PARAM_TWL_RESERVED_AREA_SIZE			0x39e0
#define HW_PARAM_TWL_RESERVED_AREA_END			( HW_PARAM_TWL_RESERVED_AREA + HW_PARAM_TWL_RESERVED_AREA_SIZE )

//--------------------------------------------------------------------------------
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_HW_COMMON_MMAP_PARAMETER_H_ */
