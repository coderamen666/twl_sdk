/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM - include - common
  File:     pm_common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_PM_COMMON_H_
#define TWL_PM_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif

//================================================================
//    PMIC extention register parameter
//================================================================
//---------------- address
#define REG_PMIC_CTL2_ADDR       0x10     // R/W

//---------------- each register spec
//---- PMIC_CTL2
#define PMIC_CTL2_RESET         (1<< 0)
#define PMIC_CTL2_FREE          (1<< 1)
#define PMIC_CTL2_BACK_LIGHT_1  (1<< 2)
#define PMIC_CTL2_BACK_LIGHT_2  (1<< 3)


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_PM_COMMON_H_ */
#endif
