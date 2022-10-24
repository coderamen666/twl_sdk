/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl - HW - common
  File:     mmap_wramEnv.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_HW_COMMON_WRAMENV_H_
#define TWL_HW_COMMON_WRAMENV_H_

#ifdef __cplusplus
extern "C" {
#endif

//---- 
//HW_PRV_WRAM_SYSRV+4:  SCFG_EXT 4Byte -> 4Byte

#define HWi_WSYS04_SIZE                                   4
#define HWi_WSYS04_OFFSET                                 0
#define HWi_WSYS04_WRAMOFFSET                             (HWi_WSYS04_OFFSET+4)
#define HWi_WSYS04_ADDR                                   (HW_PRV_WRAM_SYSRV + HWi_WSYS04_WRAMOFFSET)

//bit:31
#define HWi_WSYS04_EXT_CFG_SHIFT                          REG_SCFG_EXT_CFG_SHIFT
#define HWi_WSYS04_EXT_CFG_SIZE                           REG_SCFG_EXT_CFG_SIZE
#define HWi_WSYS04_EXT_CFG_MASK                           REG_SCFG_EXT_CFG_MASK
//bit:28
#define HWi_WSYS04_EXT_PUENABLE_SHIFT                     REG_SCFG_EXT_PUENABLE_SHIFT
#define HWi_WSYS04_EXT_PUENABLE_SIZE                      REG_SCFG_EXT_PUENABLE_SIZE
#define HWi_WSYS04_EXT_PUENABLE_MASK                      REG_SCFG_EXT_PUENABLE_MASK
//bit:28
#define HWi_WSYS04_EXT_SD20_SHIFT                         REG_SCFG_EXT_SD20_SHIFT
#define HWi_WSYS04_EXT_SD20_SIZE                          REG_SCFG_EXT_SD20_SIZE
#define HWi_WSYS04_EXT_SD20_MASK                          REG_SCFG_EXT_SD20_MASK
//bit:25-16
#define HWi_WSYS04_EXT_NEWB_ACCESS_E_SHIFT                REG_SCFG_EXT_NEWB_ACCESS_E_SHIFT
#define HWi_WSYS04_EXT_NEWB_ACCESS_E_SIZE                 REG_SCFG_EXT_NEWB_ACCESS_E_SIZE
#define HWi_WSYS04_EXT_NEWB_ACCESS_E_MASK                 REG_SCFG_EXT_NEWB_ACCESS_E_MASK
//bit:25
#define HWi_WSYS04_EXT_WRAM_SHIFT                         REG_SCFG_EXT_WRAM_SHIFT
#define HWi_WSYS04_EXT_WRAM_SIZE                          REG_SCFG_EXT_WRAM_SIZE
#define HWi_WSYS04_EXT_WRAM_MASK                          REG_SCFG_EXT_WRAM_MASK
//bit:24
#define HWi_WSYS04_EXT_MC_B_SHIFT                         REG_SCFG_EXT_MC_B_SHIFT
#define HWi_WSYS04_EXT_MC_B_SIZE                          REG_SCFG_EXT_MC_B_SIZE
#define HWi_WSYS04_EXT_MC_B_MASK                          REG_SCFG_EXT_MC_B_MASK
//bit:23
#define HWi_WSYS04_EXT_GPIO_SHIFT                         REG_SCFG_EXT_GPIO_SHIFT
#define HWi_WSYS04_EXT_GPIO_SIZE                          REG_SCFG_EXT_GPIO_SIZE
#define HWi_WSYS04_EXT_GPIO_MASK                          REG_SCFG_EXT_GPIO_MASK
//bit:22
#define HWi_WSYS04_EXT_I2C_SHIFT                          REG_SCFG_EXT_I2C_SHIFT
#define HWi_WSYS04_EXT_I2C_SIZE                           REG_SCFG_EXT_I2C_SIZE
#define HWi_WSYS04_EXT_I2C_MASK                           REG_SCFG_EXT_I2C_MASK
//bit:21
#define HWi_WSYS04_EXT_I2S_SHIFT                          REG_SCFG_EXT_I2S_SHIFT
#define HWi_WSYS04_EXT_I2S_SIZE                           REG_SCFG_EXT_I2S_SIZE
#define HWi_WSYS04_EXT_I2S_MASK                           REG_SCFG_EXT_I2S_MASK
//bit:20
#define HWi_WSYS04_EXT_MIC_SHIFT                          REG_SCFG_EXT_MIC_SHIFT
#define HWi_WSYS04_EXT_MIC_SIZE                           REG_SCFG_EXT_MIC_SIZE
#define HWi_WSYS04_EXT_MIC_MASK                           REG_SCFG_EXT_MIC_MASK
//bit:19
#define HWi_WSYS04_EXT_SD2_SHIFT                          REG_SCFG_EXT_SD2_SHIFT
#define HWi_WSYS04_EXT_SD2_SIZE                           REG_SCFG_EXT_SD2_SIZE
#define HWi_WSYS04_EXT_SD2_MASK                           REG_SCFG_EXT_SD2_MASK
//bit:18
#define HWi_WSYS04_EXT_SD1_SHIFT                          REG_SCFG_EXT_SD1_SHIFT
#define HWi_WSYS04_EXT_SD1_SIZE                           REG_SCFG_EXT_SD1_SIZE
#define HWi_WSYS04_EXT_SD1_MASK                           REG_SCFG_EXT_SD1_MASK
//bit:17
#define HWi_WSYS04_EXT_AES_SHIFT                          REG_SCFG_EXT_AES_SHIFT
#define HWi_WSYS04_EXT_AES_SIZE                           REG_SCFG_EXT_AES_SIZE
#define HWi_WSYS04_EXT_AES_MASK                           REG_SCFG_EXT_AES_MASK
//bit:16
#define HWi_WSYS04_EXT_DMAC_SHIFT                         REG_SCFG_EXT_DMAC_SHIFT
#define HWi_WSYS04_EXT_DMAC_SIZE                          REG_SCFG_EXT_DMAC_SIZE
#define HWi_WSYS04_EXT_DMAC_MASK                          REG_SCFG_EXT_DMAC_MASK
//bit:15-8
#define HWi_WSYS04_EXT_NITROB_EX_E_SHIFT                  REG_SCFG_EXT_NITROB_EX_E_SHIFT
#define HWi_WSYS04_EXT_NITROB_EX_E_SIZE                   REG_SCFG_EXT_NITROB_EX_E_SIZE
#define HWi_WSYS04_EXT_NITROB_EX_E_MASK                   REG_SCFG_EXT_NITROB_EX_E_MASK
//bit:15-14
#define HWi_WSYS04_EXT_PSRAM_SHIFT                        REG_SCFG_EXT_PSRAM_SHIFT
#define HWi_WSYS04_EXT_PSRAM_SIZE                         REG_SCFG_EXT_PSRAM_SIZE
#define HWi_WSYS04_EXT_PSRAM_MASK                         REG_SCFG_EXT_PSRAM_MASK
//bit:13
#define HWi_WSYS04_EXT_VRAM_SHIFT                         REG_SCFG_EXT_VRAM_SHIFT
#define HWi_WSYS04_EXT_VRAM_SIZE                          REG_SCFG_EXT_VRAM_SIZE
#define HWi_WSYS04_EXT_VRAM_MASK                          REG_SCFG_EXT_VRAM_MASK
//bit:12
#define HWi_WSYS04_EXT_LCDC_SHIFT                         REG_SCFG_EXT_LCDC_SHIFT
#define HWi_WSYS04_EXT_LCDC_SIZE                          REG_SCFG_EXT_LCDC_SIZE
#define HWi_WSYS04_EXT_LCDC_MASK                          REG_SCFG_EXT_LCDC_MASK
//bit:11
#define HWi_WSYS04_EXT_SIO_SHIFT                          REG_SCFG_EXT_SIO_SHIFT
#define HWi_WSYS04_EXT_SIO_SIZE                           REG_SCFG_EXT_SIO_SIZE
#define HWi_WSYS04_EXT_SIO_MASK                           REG_SCFG_EXT_SIO_MASK
//bit:10
#define HWi_WSYS04_EXT_DSEL_SHIFT                         REG_SCFG_EXT_DSEL_SHIFT
#define HWi_WSYS04_EXT_DSEL_SIZE                          REG_SCFG_EXT_DSEL_SIZE
#define HWi_WSYS04_EXT_DSEL_MASK                          REG_SCFG_EXT_DSEL_MASK
//bit:9
#define HWi_WSYS04_EXT_SPI_SHIFT                          REG_SCFG_EXT_SPI_SHIFT
#define HWi_WSYS04_EXT_SPI_SIZE                           REG_SCFG_EXT_SPI_SIZE
#define HWi_WSYS04_EXT_SPI_MASK                           REG_SCFG_EXT_SPI_MASK
//bit:8
#define HWi_WSYS04_EXT_INTC_SHIFT                         REG_SCFG_EXT_INTC_SHIFT
#define HWi_WSYS04_EXT_INTC_SIZE                          REG_SCFG_EXT_INTC_SIZE
#define HWi_WSYS04_EXT_INTC_MASK                          REG_SCFG_EXT_INTC_MASK
//bit:7-0
#define HWi_WSYS04_EXT_NITROB_MOD_E_SHIFT                 REG_SCFG_EXT_NITROB_MOD_E_SHIFT
#define HWi_WSYS04_EXT_NITROB_MOD_E_SIZE                  REG_SCFG_EXT_NITROB_MOD_E_SIZE
#define HWi_WSYS04_EXT_NITROB_MOD_E_MASK                  REG_SCFG_EXT_NITROB_MOD_E_MASK
//bit:7
#define HWi_WSYS04_EXT_MC_SHIFT                           REG_SCFG_EXT_MC_SHIFT
#define HWi_WSYS04_EXT_MC_SIZE                            REG_SCFG_EXT_MC_SIZE
#define HWi_WSYS04_EXT_MC_MASK                            REG_SCFG_EXT_MC_MASK
//bit:2
#define HWi_WSYS04_EXT_SND_SHIFT                          REG_SCFG_EXT_SND_SHIFT
#define HWi_WSYS04_EXT_SND_SIZE                           REG_SCFG_EXT_SND_SIZE
#define HWi_WSYS04_EXT_SND_MASK                           REG_SCFG_EXT_SND_MASK
//bit:1
#define HWi_WSYS04_EXT_SDMA_SHIFT                         REG_SCFG_EXT_SDMA_SHIFT
#define HWi_WSYS04_EXT_SDMA_SIZE                          REG_SCFG_EXT_SDMA_SIZE
#define HWi_WSYS04_EXT_SDMA_MASK                          REG_SCFG_EXT_SDMA_MASK
//bit:0
#define HWi_WSYS04_EXT_DMA_SHIFT                          REG_SCFG_EXT_DMA_SHIFT
#define HWi_WSYS04_EXT_DMA_SIZE                           REG_SCFG_EXT_DMA_SIZE
#define HWi_WSYS04_EXT_DMA_MASK                           REG_SCFG_EXT_DMA_MASK

//----------------------------------------------------------------
//HW_PRV_WRAM_SYSRV+8:  SCFG_OP & SCFG_ROM(L) & SCFG_WL 4Byte -> 1Byte

#define HWi_WSYS08_SIZE                                   1
#define HWi_WSYS08_OFFSET                                 4
#define HWi_WSYS08_WRAMOFFSET                             (HWi_WSYS08_OFFSET+4)
#define HWi_WSYS08_ADDR                                   (HW_PRV_WRAM_SYSRV + HWi_WSYS08_WRAMOFFSET)

//bit:7
#define HWi_WSYS08_WL_OFFB_SHIFT                          7
#define HWi_WSYS08_WL_OFFB_SIZE                           1
#define HWi_WSYS08_WL_OFFB_MASK                           0x80
//bit:6
#define HWi_WSYS08_ROM_ARM7FUSE_SHIFT                     6
#define HWi_WSYS08_ROM_ARM7FUSE_SIZE                      1
#define HWi_WSYS08_ROM_ARM7FUSE_MASK                      0x40
//bit:5
#define HWi_WSYS08_ROM_ARM7RSEL_SHIFT                     5
#define HWi_WSYS08_ROM_ARM7RSEL_SIZE                      1
#define HWi_WSYS08_ROM_ARM7RSEL_MASK                      0x20
//bit:3
#define HWi_WSYS08_ROM_ARM9RSEL_SHIFT                     3
#define HWi_WSYS08_ROM_ARM9RSEL_SIZE                      1
#define HWi_WSYS08_ROM_ARM9RSEL_MASK                      0x08
//bit:2
#define HWi_WSYS08_ROM_ARM9SEC_SHIFT                      2
#define HWi_WSYS08_ROM_ARM9SEC_SIZE                       1
#define HWi_WSYS08_ROM_ARM9SEC_MASK                       0x04
//bit:1
#define HWi_WSYS08_OP_OP1_SHIFT                           1
#define HWi_WSYS08_OP_OP1_SIZE                            1
#define HWi_WSYS08_OP_OP1_MASK                            0x02
//bit:0
#define HWi_WSYS08_OP_OP0_SHIFT                           0
#define HWi_WSYS08_OP_OP0_SIZE                            1
#define HWi_WSYS08_OP_OP0_MASK                            0x01
//bit:1-0
#define HWi_WSYS08_OP_OPT_SHIFT                           0
#define HWi_WSYS08_OP_OPT_SIZE                            2
#define HWi_WSYS08_OP_OPT_MASK                            0x03

//----------------------------------------------------------------
//HW_PRV_WRAM_SYSRV+9:  SCFG_JTAG & SCFG_CLK 2Byte -> 1Byte

#define HWi_WSYS09_SIZE                                   1
#define HWi_WSYS09_OFFSET                                 5
#define HWi_WSYS09_WRAMOFFSET                             (HWi_WSYS09_OFFSET+4)
#define HWi_WSYS09_ADDR                                   (HW_PRV_WRAM_SYSRV + HWi_WSYS09_WRAMOFFSET)

//bit:7
#define HWi_WSYS09_CLK_SNDMCLK_SHIFT                      7
#define HWi_WSYS09_CLK_SNDMCLK_SIZE                       1
#define HWi_WSYS09_CLK_SNDMCLK_MASK                       0x80
//bit:6
#define HWi_WSYS09_CLK_WRAMHCLK_SHIFT                     6
#define HWi_WSYS09_CLK_WRAMHCLK_SIZE                      1
#define HWi_WSYS09_CLK_WRAMHCLK_MASK                      0x40
//bit:5
#define HWi_WSYS09_CLK_AESHCLK_SHIFT                      5
#define HWi_WSYS09_CLK_AESHCLK_SIZE                       1
#define HWi_WSYS09_CLK_AESHCLK_MASK                       0x20
//bit:4
#define HWi_WSYS09_CLK_SD2HCLK_SHIFT                      4
#define HWi_WSYS09_CLK_SD2HCLK_SIZE                       1
#define HWi_WSYS09_CLK_SD2HCLK_MASK                       0x10
//bit:3
#define HWi_WSYS09_CLK_SD1HCLK_SHIFT                      3
#define HWi_WSYS09_CLK_SD1HCLK_SIZE                       1
#define HWi_WSYS09_CLK_SD1HCLK_MASK                       0x08
//bit:2
#define HWi_WSYS09_JTAG_DSPJE_SHIFT                       2
#define HWi_WSYS09_JTAG_DSPJE_SIZE                        1
#define HWi_WSYS09_JTAG_DSPJE_MASK                        0x04
//bit:1
#define HWi_WSYS09_JTAG_CPUJE_SHIFT                       1
#define HWi_WSYS09_JTAG_CPUJE_SIZE                        1
#define HWi_WSYS09_JTAG_CPUJE_MASK                        0x02
//bit:0
#define HWi_WSYS09_JTAG_ARM7SEL_SHIFT                     0
#define HWi_WSYS09_JTAG_ARM7SEL_SIZE                      1
#define HWi_WSYS09_JTAG_ARM7SEL_MASK                      0x01

//----------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_HW_COMMON_WRAMENV_H_ */
#endif
