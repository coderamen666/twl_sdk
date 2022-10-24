/*---------------------------------------------------------------------------*
  Project:  TwlSDK - rtc - include
  File:     fifo_ex.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_RTC_COMMON_FIFO_EX_H_
#define TWL_RTC_COMMON_FIFO_EX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
/* Additional PXI Command Definitions */
#define RTC_PXI_COMMAND_READ_COUNTER        0x50        /* Read the up counter */
#define RTC_PXI_COMMAND_READ_FOUT           0x51        /* Read the FOUT setting value */
#define RTC_PXI_COMMAND_READ_ALARM_EX1      0x52        /* Read the Alarm 1 setting value (extended version) */
#define RTC_PXI_COMMAND_READ_ALARM_EX2      0x53        /* Read the Alarm 2 setting value (extended version) */
#define RTC_PXI_COMMAND_WRITE_FOUT          0x61        /* Write the FOUT setting value */
#define RTC_PXI_COMMAND_WRITE_ALARM_EX1     0x62        /* Write the Alarm 1 setting value (extended version) */
#define RTC_PXI_COMMAND_WRITE_ALARM_EX2     0x63        /* Write the Alarm 2 setting value (extended version) */


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_RTC_COMMON_FIFO_EX_H_ */
