/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IO Register List - 
  File:     twl/hw/ARM9/ioreg_DSP.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/
//
//  I was generated automatically, don't edit me directly!!!
//
#ifndef TWL_HW_ARM9_IOREG_DSP_H_
#define TWL_HW_ARM9_IOREG_DSP_H_

#ifndef SDK_ASM
#include <nitro/types.h>
#include <twl/hw/ARM9/mmap_global.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definition of Register offsets, addresses and variables.
 */


/* PDATA */

#define REG_PDATA_OFFSET                                   0x4300
#define REG_PDATA_ADDR                                     (HW_REG_BASE + REG_PDATA_OFFSET)
#define reg_DSP_PDATA                                      (*( REGType16v *) REG_PDATA_ADDR)

/* PADR */

#define REG_PADR_OFFSET                                    0x4304
#define REG_PADR_ADDR                                      (HW_REG_BASE + REG_PADR_OFFSET)
#define reg_DSP_PADR                                       (*( REGType16v *) REG_PADR_ADDR)

/* PCFG */

#define REG_PCFG_OFFSET                                    0x4308
#define REG_PCFG_ADDR                                      (HW_REG_BASE + REG_PCFG_OFFSET)
#define reg_DSP_PCFG                                       (*( REGType16v *) REG_PCFG_ADDR)

/* PSTS */

#define REG_PSTS_OFFSET                                    0x430c
#define REG_PSTS_ADDR                                      (HW_REG_BASE + REG_PSTS_OFFSET)
#define reg_DSP_PSTS                                       (*(const REGType16v *) REG_PSTS_ADDR)

/* PSEM */

#define REG_PSEM_OFFSET                                    0x4310
#define REG_PSEM_ADDR                                      (HW_REG_BASE + REG_PSEM_OFFSET)
#define reg_DSP_PSEM                                       (*( REGType16v *) REG_PSEM_ADDR)

/* PMASK */

#define REG_PMASK_OFFSET                                   0x4314
#define REG_PMASK_ADDR                                     (HW_REG_BASE + REG_PMASK_OFFSET)
#define reg_DSP_PMASK                                      (*( REGType16v *) REG_PMASK_ADDR)

/* PCLEAR */

#define REG_PCLEAR_OFFSET                                  0x4318
#define REG_PCLEAR_ADDR                                    (HW_REG_BASE + REG_PCLEAR_OFFSET)
#define reg_DSP_PCLEAR                                     (*( REGType16v *) REG_PCLEAR_ADDR)

/* SEM */

#define REG_SEM_OFFSET                                     0x431c
#define REG_SEM_ADDR                                       (HW_REG_BASE + REG_SEM_OFFSET)
#define reg_DSP_SEM                                        (*(const REGType16v *) REG_SEM_ADDR)

/* COM0 */

#define REG_COM0_OFFSET                                    0x4320
#define REG_COM0_ADDR                                      (HW_REG_BASE + REG_COM0_OFFSET)
#define reg_DSP_COM0                                       (*( REGType16v *) REG_COM0_ADDR)

/* REP0 */

#define REG_REP0_OFFSET                                    0x4324
#define REG_REP0_ADDR                                      (HW_REG_BASE + REG_REP0_OFFSET)
#define reg_DSP_REP0                                       (*(const REGType16v *) REG_REP0_ADDR)

/* COM1 */

#define REG_COM1_OFFSET                                    0x4328
#define REG_COM1_ADDR                                      (HW_REG_BASE + REG_COM1_OFFSET)
#define reg_DSP_COM1                                       (*( REGType16v *) REG_COM1_ADDR)

/* REP1 */

#define REG_REP1_OFFSET                                    0x432c
#define REG_REP1_ADDR                                      (HW_REG_BASE + REG_REP1_OFFSET)
#define reg_DSP_REP1                                       (*(const REGType16v *) REG_REP1_ADDR)

/* COM2 */

#define REG_COM2_OFFSET                                    0x4330
#define REG_COM2_ADDR                                      (HW_REG_BASE + REG_COM2_OFFSET)
#define reg_DSP_COM2                                       (*( REGType16v *) REG_COM2_ADDR)

/* REP2 */

#define REG_REP2_OFFSET                                    0x4334
#define REG_REP2_ADDR                                      (HW_REG_BASE + REG_REP2_OFFSET)
#define reg_DSP_REP2                                       (*(const REGType16v *) REG_REP2_ADDR)


/*
 * Definitions of Register fields
 */


/* PDATA */

/* PADR */

/* PCFG */

#define REG_DSP_PCFG_MEMSEL_SHIFT                          12
#define REG_DSP_PCFG_MEMSEL_SIZE                           4
#define REG_DSP_PCFG_MEMSEL_MASK                           0xf000

#define REG_DSP_PCFG_PRIE2_SHIFT                           11
#define REG_DSP_PCFG_PRIE2_SIZE                            1
#define REG_DSP_PCFG_PRIE2_MASK                            0x0800

#define REG_DSP_PCFG_PRIE1_SHIFT                           10
#define REG_DSP_PCFG_PRIE1_SIZE                            1
#define REG_DSP_PCFG_PRIE1_MASK                            0x0400

#define REG_DSP_PCFG_PRIE0_SHIFT                           9
#define REG_DSP_PCFG_PRIE0_SIZE                            1
#define REG_DSP_PCFG_PRIE0_MASK                            0x0200

#define REG_DSP_PCFG_WFEIE_SHIFT                           8
#define REG_DSP_PCFG_WFEIE_SIZE                            1
#define REG_DSP_PCFG_WFEIE_MASK                            0x0100

#define REG_DSP_PCFG_WFFIE_SHIFT                           7
#define REG_DSP_PCFG_WFFIE_SIZE                            1
#define REG_DSP_PCFG_WFFIE_MASK                            0x0080

#define REG_DSP_PCFG_RFNEIE_SHIFT                          6
#define REG_DSP_PCFG_RFNEIE_SIZE                           1
#define REG_DSP_PCFG_RFNEIE_MASK                           0x0040

#define REG_DSP_PCFG_RFFIE_SHIFT                           5
#define REG_DSP_PCFG_RFFIE_SIZE                            1
#define REG_DSP_PCFG_RFFIE_MASK                            0x0020

#define REG_DSP_PCFG_RS_SHIFT                              4
#define REG_DSP_PCFG_RS_SIZE                               1
#define REG_DSP_PCFG_RS_MASK                               0x0010

#define REG_DSP_PCFG_DRS_SHIFT                             2
#define REG_DSP_PCFG_DRS_SIZE                              2
#define REG_DSP_PCFG_DRS_MASK                              0x000c

#define REG_DSP_PCFG_AIM_SHIFT                             1
#define REG_DSP_PCFG_AIM_SIZE                              1
#define REG_DSP_PCFG_AIM_MASK                              0x0002

#define REG_DSP_PCFG_DSPR_SHIFT                            0
#define REG_DSP_PCFG_DSPR_SIZE                             1
#define REG_DSP_PCFG_DSPR_MASK                             0x0001

#ifndef SDK_ASM
#define REG_DSP_PCFG_FIELD( memsel, prie2, prie1, prie0, wfeie, wffie, rfneie, rffie, rs, drs, aim, dspr ) \
    (u16)( \
    ((u32)(memsel) << REG_DSP_PCFG_MEMSEL_SHIFT) | \
    ((u32)(prie2) << REG_DSP_PCFG_PRIE2_SHIFT) | \
    ((u32)(prie1) << REG_DSP_PCFG_PRIE1_SHIFT) | \
    ((u32)(prie0) << REG_DSP_PCFG_PRIE0_SHIFT) | \
    ((u32)(wfeie) << REG_DSP_PCFG_WFEIE_SHIFT) | \
    ((u32)(wffie) << REG_DSP_PCFG_WFFIE_SHIFT) | \
    ((u32)(rfneie) << REG_DSP_PCFG_RFNEIE_SHIFT) | \
    ((u32)(rffie) << REG_DSP_PCFG_RFFIE_SHIFT) | \
    ((u32)(rs) << REG_DSP_PCFG_RS_SHIFT) | \
    ((u32)(drs) << REG_DSP_PCFG_DRS_SHIFT) | \
    ((u32)(aim) << REG_DSP_PCFG_AIM_SHIFT) | \
    ((u32)(dspr) << REG_DSP_PCFG_DSPR_SHIFT))
#endif


/* PSTS */

#define REG_DSP_PSTS_RCOMIM2_SHIFT                         15
#define REG_DSP_PSTS_RCOMIM2_SIZE                          1
#define REG_DSP_PSTS_RCOMIM2_MASK                          0x8000

#define REG_DSP_PSTS_RCOMIM1_SHIFT                         14
#define REG_DSP_PSTS_RCOMIM1_SIZE                          1
#define REG_DSP_PSTS_RCOMIM1_MASK                          0x4000

#define REG_DSP_PSTS_RCOMIM0_SHIFT                         13
#define REG_DSP_PSTS_RCOMIM0_SIZE                          1
#define REG_DSP_PSTS_RCOMIM0_MASK                          0x2000

#define REG_DSP_PSTS_RRI2_SHIFT                            12
#define REG_DSP_PSTS_RRI2_SIZE                             1
#define REG_DSP_PSTS_RRI2_MASK                             0x1000

#define REG_DSP_PSTS_RRI1_SHIFT                            11
#define REG_DSP_PSTS_RRI1_SIZE                             1
#define REG_DSP_PSTS_RRI1_MASK                             0x0800

#define REG_DSP_PSTS_RRI0_SHIFT                            10
#define REG_DSP_PSTS_RRI0_SIZE                             1
#define REG_DSP_PSTS_RRI0_MASK                             0x0400

#define REG_DSP_PSTS_PSEMI_SHIFT                           9
#define REG_DSP_PSTS_PSEMI_SIZE                            1
#define REG_DSP_PSTS_PSEMI_MASK                            0x0200

#define REG_DSP_PSTS_WFEI_SHIFT                            8
#define REG_DSP_PSTS_WFEI_SIZE                             1
#define REG_DSP_PSTS_WFEI_MASK                             0x0100

#define REG_DSP_PSTS_WFFI_SHIFT                            7
#define REG_DSP_PSTS_WFFI_SIZE                             1
#define REG_DSP_PSTS_WFFI_MASK                             0x0080

#define REG_DSP_PSTS_RFNEI_SHIFT                           6
#define REG_DSP_PSTS_RFNEI_SIZE                            1
#define REG_DSP_PSTS_RFNEI_MASK                            0x0040

#define REG_DSP_PSTS_RFFI_SHIFT                            5
#define REG_DSP_PSTS_RFFI_SIZE                             1
#define REG_DSP_PSTS_RFFI_MASK                             0x0020

#define REG_DSP_PSTS_PRST_SHIFT                            2
#define REG_DSP_PSTS_PRST_SIZE                             1
#define REG_DSP_PSTS_PRST_MASK                             0x0004

#define REG_DSP_PSTS_WTIP_SHIFT                            1
#define REG_DSP_PSTS_WTIP_SIZE                             1
#define REG_DSP_PSTS_WTIP_MASK                             0x0002

#define REG_DSP_PSTS_RTIP_SHIFT                            0
#define REG_DSP_PSTS_RTIP_SIZE                             1
#define REG_DSP_PSTS_RTIP_MASK                             0x0001

#ifndef SDK_ASM
#define REG_DSP_PSTS_FIELD( rcomim2, rcomim1, rcomim0, rri2, rri1, rri0, psemi, wfei, wffi, rfnei, rffi, prst, wtip, rtip ) \
    (u16)( \
    ((u32)(rcomim2) << REG_DSP_PSTS_RCOMIM2_SHIFT) | \
    ((u32)(rcomim1) << REG_DSP_PSTS_RCOMIM1_SHIFT) | \
    ((u32)(rcomim0) << REG_DSP_PSTS_RCOMIM0_SHIFT) | \
    ((u32)(rri2) << REG_DSP_PSTS_RRI2_SHIFT) | \
    ((u32)(rri1) << REG_DSP_PSTS_RRI1_SHIFT) | \
    ((u32)(rri0) << REG_DSP_PSTS_RRI0_SHIFT) | \
    ((u32)(psemi) << REG_DSP_PSTS_PSEMI_SHIFT) | \
    ((u32)(wfei) << REG_DSP_PSTS_WFEI_SHIFT) | \
    ((u32)(wffi) << REG_DSP_PSTS_WFFI_SHIFT) | \
    ((u32)(rfnei) << REG_DSP_PSTS_RFNEI_SHIFT) | \
    ((u32)(rffi) << REG_DSP_PSTS_RFFI_SHIFT) | \
    ((u32)(prst) << REG_DSP_PSTS_PRST_SHIFT) | \
    ((u32)(wtip) << REG_DSP_PSTS_WTIP_SHIFT) | \
    ((u32)(rtip) << REG_DSP_PSTS_RTIP_SHIFT))
#endif


/* PSEM */

/* PMASK */

/* PCLEAR */

/* SEM */

/* COM0 */

/* REP0 */

/* COM1 */

/* REP1 */

/* COM2 */

/* REP2 */

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_HW_ARM9_IOREG_DSP_H_ */
#endif
