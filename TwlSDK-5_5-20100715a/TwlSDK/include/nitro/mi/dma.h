/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     dma.h

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

#ifndef NITRO_MI_DMA_H_
#define NITRO_MI_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#ifndef SDK_TWL
#include <nitro/memorymap.h>
#endif

//================================================================================
//                    DMA control definition
//================================================================================
//---- maximum DMA channel No.
#define MI_DMA_MAX_NUM             3

// utility constants for high-level libraries (CARD, FS, ...)
#define	MI_DMA_CHANNEL_MASK        0x00000003UL
#define	MI_DMA_NOT_USE	           0xFFFFFFFFUL
#define MI_DMA_USING_NEW           0x00000010UL
#define MI_DMA_USING_FORMER        0x00000000UL

//---- enable
#define MI_DMA_ENABLE              (1UL << REG_MI_DMA0CNT_E_SHIFT)      // DMA enable
#define MI_DMA_DISABLE             (0UL << REG_MI_DMA0CNT_E_SHIFT)      // DMA enable
#define MI_DMA_IF_ENABLE           (1UL << REG_MI_DMA0CNT_I_SHIFT)      // interrupt enable
#define MI_DMA_IF_DISABLE          (0UL << REG_MI_DMA0CNT_I_SHIFT)      // interrupt enable

//---- DMA timing
#ifdef SDK_ARM9
#  define MI_DMA_TIMING_MASK       (REG_MI_DMA0CNT_MODE_MASK)   // mask  of start field
#  define MI_DMA_TIMING_SHIFT      (REG_MI_DMA0CNT_MODE_SHIFT)  // shift of start field
#  define MI_DMA_TIMING_IMM        (0UL << REG_MI_DMA0CNT_MODE_SHIFT)   // start immediately
#  define MI_DMA_TIMING_V_BLANK    (1UL << REG_MI_DMA0CNT_MODE_SHIFT)   // start by VBlank
#  define MI_DMA_TIMING_H_BLANK    (2UL << REG_MI_DMA0CNT_MODE_SHIFT)   // start by HBlank
#  define MI_DMA_TIMING_DISP       (3UL << REG_MI_DMA0CNT_MODE_SHIFT)   // display sync
#  define MI_DMA_TIMING_DISP_MMEM  (4UL << REG_MI_DMA0CNT_MODE_SHIFT)   // main memory display sync
#  define MI_DMA_TIMING_CARD       (5UL << REG_MI_DMA0CNT_MODE_SHIFT)   // card
#  define MI_DMA_TIMING_CARTRIDGE  (6UL << REG_MI_DMA0CNT_MODE_SHIFT)   // cartridge
#  define MI_DMA_TIMING_GXFIFO     (7UL << REG_MI_DMA0CNT_MODE_SHIFT)   // geometry FIFO
#  define MIi_DMA_TIMING_ANY       (u32)(~0)    // for internal use (MIi_CardDmaCopy32)
#else  // SDK_ARM9
#  define MI_DMA_TIMING_MASK       (REG_MI_DMA0CNT_TIMING_MASK) // mask  of start field
#  define MI_DMA_TIMING_SHIFT      (REG_MI_DMA0CNT_TIMING_SHIFT)        // shift of start field
#  define MI_DMA_TIMING_IMM        (0UL << REG_MI_DMA0CNT_TIMING_SHIFT) // start immediately
#  define MI_DMA_TIMING_V_BLANK    (1UL << REG_MI_DMA0CNT_TIMING_SHIFT) // start by VBlank
#  define MI_DMA_TIMING_CARD       (2UL << REG_MI_DMA0CNT_TIMING_SHIFT) // card
#  define MI_DMA_TIMING_WIRELESS   (3UL << REG_MI_DMA0CNT_TIMING_SHIFT) // DMA0,2:wireless interrupt
#  define MI_DMA_TIMING_CARTRIDGE  MI_DMA_TIMING_WIRELESS       // DMA1,3:cartridge warning
#endif //SDK_ARM9

//---- DMA bus width
#define MI_DMA_16BIT_BUS           (0UL << REG_MI_DMA0CNT_SB_SHIFT)     // 16bit width
#define MI_DMA_32BIT_BUS           (1UL << REG_MI_DMA0CNT_SB_SHIFT)     // 32bit width

//---- continuous mode
#define MI_DMA_CONTINUOUS_OFF      (0UL << REG_MI_DMA0CNT_CM_SHIFT)     // continuous mode off
#define MI_DMA_CONTINUOUS_ON       (1UL << REG_MI_DMA0CNT_CM_SHIFT)     // continuous mode on

//---- direction of src/destination address
#define MI_DMA_SRC_INC             (0UL << REG_MI_DMA0CNT_SAR_SHIFT)    // increment source address
#define MI_DMA_SRC_DEC             (1UL << REG_MI_DMA0CNT_SAR_SHIFT)    // decrement source address
#define MI_DMA_SRC_FIX             (2UL << REG_MI_DMA0CNT_SAR_SHIFT)    // fix source address
#define MI_DMA_DEST_INC            (0UL << REG_MI_DMA0CNT_DAR_SHIFT)    // imcrement destination address
#define MI_DMA_DEST_DEC            (1UL << REG_MI_DMA0CNT_DAR_SHIFT)    // decrement destination address
#define MI_DMA_DEST_FIX            (2UL << REG_MI_DMA0CNT_DAR_SHIFT)    // fix destination address
#define MI_DMA_DEST_RELOAD         (3UL << REG_MI_DMA0CNT_DAR_SHIFT)    // increment destination address and reload

//---- forwarding size
#define MI_DMA_COUNT_MASK          (REG_MI_DMA0CNT_WORDCNT_MASK)        // mask  of forwarding size
#define MI_DMA_COUNT_SHIFT         (REG_MI_DMA0CNT_WORDCNT_SHIFT)       // shift of forwarding size

//================================================================
// for convinience to access NDMA register
//================================================================
#define MI_DMA_NUM_WOFFSET(ndmaNo)     (ndmaNo * 3)
#define MI_DMA_REG_SAD_WOFFSET          0
#define MI_DMA_REG_DAD_WOFFSET     		1
#define MI_DMA_REG_CNT_WOFFSET 			2

#define MI_DMA_REGADDR(dmaNo, reg)    (((vu32*)REG_DMA0SAD_ADDR) + MI_DMA_NUM_WOFFSET(dmaNo) + (reg) )
#define MI_DMA_REG(dmaNo, reg)        ( *( MI_DMA_REGADDR(dmaNo, reg) ) )

//--------------------------------------------------------------------------------
//     definitions for convenience.
//
//---- directions and width
#define MI_DMA_SINC_DINC_16        ( MI_DMA_SRC_INC | MI_DMA_DEST_INC | MI_DMA_16BIT_BUS )
#define MI_DMA_SFIX_DINC_16        ( MI_DMA_SRC_FIX | MI_DMA_DEST_INC | MI_DMA_16BIT_BUS )
#define MI_DMA_SINC_DFIX_16        ( MI_DMA_SRC_INC | MI_DMA_DEST_FIX | MI_DMA_16BIT_BUS )
#define MI_DMA_SFIX_DFIX_16        ( MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | MI_DMA_16BIT_BUS )
#define MI_DMA_SINC_DINC_32        ( MI_DMA_SRC_INC | MI_DMA_DEST_INC | MI_DMA_32BIT_BUS )
#define MI_DMA_SFIX_DINC_32        ( MI_DMA_SRC_FIX | MI_DMA_DEST_INC | MI_DMA_32BIT_BUS )
#define MI_DMA_SINC_DFIX_32        ( MI_DMA_SRC_INC | MI_DMA_DEST_FIX | MI_DMA_32BIT_BUS )
#define MI_DMA_SFIX_DFIX_32        ( MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | MI_DMA_32BIT_BUS )

#define MI_DMA_IMM16ENABLE         ( MI_DMA_ENABLE | MI_DMA_TIMING_IMM | MI_DMA_16BIT_BUS )
#define MI_DMA_IMM32ENABLE         ( MI_DMA_ENABLE | MI_DMA_TIMING_IMM | MI_DMA_32BIT_BUS )
#define MI_DMA_IMM16DISABLE        ( MI_DMA_DISABLE | MI_DMA_TIMING_IMM | MI_DMA_16BIT_BUS )
#define MI_DMA_IMM32DISABLE        ( MI_DMA_DISABLE | MI_DMA_TIMING_IMM | MI_DMA_32BIT_BUS )

//---- clear
#define MI_CNT_CLEAR16(size)       ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_CLEAR32(size)       ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_CLEAR16_IF(size)    ( MI_CNT_CLEAR16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_CLEAR32_IF(size)    ( MI_CNT_CLEAR32((size)) | MI_DMA_IF_ENABLE )

#define MI_CNT_SET_CLEAR16(size)   ( MI_DMA_IMM16DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_SET_CLEAR32(size)   ( MI_DMA_IMM32DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_SET_CLEAR16_IF(size)( MI_CNT_SET_CLEAR16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SET_CLEAR32_IF(size)( MI_CNT_SET_CLEAR32((size)) | MI_DMA_IF_ENABLE )

//---- copy
#define MI_CNT_COPY16(size)        ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_INC | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_COPY32(size)        ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_INC | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_COPY16_IF(size)     ( MI_CNT_COPY16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_COPY32_IF(size)     ( MI_CNT_COPY32((size)) | MI_DMA_IF_ENABLE )

#define MI_CNT_SET_COPY16(size)    ( MI_DMA_IMM16DISABLE | MI_DMA_SRC_INC | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_SET_COPY32(size)    ( MI_DMA_IMM32DISABLE | MI_DMA_SRC_INC | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_SET_COPY16_IF(size) ( MI_CNT_SET_COPY16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SET_COPY32_IF(size) ( MI_CNT_SET_COPY32((size)) | MI_DMA_IF_ENABLE )

//---- send
#define MI_CNT_SEND16(size)        ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | ((size)/2) )
#define MI_CNT_SEND32(size)        ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | ((size)/4) )
#define MI_CNT_SEND16_IF(size)     ( MI_CNT_SEND16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SEND32_IF(size)     ( MI_CNT_SEND32((size)) | MI_DMA_IF_ENABLE )

#define MI_CNT_SET_SEND16(size)    ( MI_DMA_IMM16DISABLE | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | ((size)/2) )
#define MI_CNT_SET_SEND32(size)    ( MI_DMA_IMM32DISABLE | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | ((size)/4) )
#define MI_CNT_SET_SEND16_IF(size) ( MI_CNT_SET_SEND16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SET_SEND32_IF(size) ( MI_CNT_SET_SEND32((size)) | MI_DMA_IF_ENABLE )

//---- recv
#define MI_CNT_RECV16(size)        ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_RECV32(size)        ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_RECV16_IF(size)     ( MI_CNT_RECV16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_RECV32_IF(size)     ( MI_CNT_RECV32((size)) | MI_DMA_IF_ENABLE )

#define MI_CNT_SET_RECV16(size)    ( MI_DMA_IMM16DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_SET_RECV32(size)    ( MI_DMA_IMM32DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_SET_RECV16_IF(size) ( MI_CNT_SET_RECV16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SET_RECV32_IF(size) ( MI_CNT_SET_RECV32((size)) | MI_DMA_IF_ENABLE )

//---- pipe
#define MI_CNT_PIPE16(size)        ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | ((size)/2) )
#define MI_CNT_PIPE32(size)        ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | ((size)/4) )
#define MI_CNT_PIPE16_IF(size)     ( MI_CNT_PIPE16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_PIPE32_IF(size)     ( MI_CNT_PIPE32((size)) | MI_DMA_IF_ENABLE )

#define MI_CNT_SET_PIPE16(size)    ( MI_DMA_IMM16DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | ((size)/2) )
#define MI_CNT_SET_PIPE32(size)    ( MI_DMA_IMM32DISABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | ((size)/4) )
#define MI_CNT_SET_PIPE16_IF(size) ( MI_CNT_SET_PIPE16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_SET_PIPE32_IF(size) ( MI_CNT_SET_PIPE32((size)) | MI_DMA_IF_ENABLE )

//---- HBlank copy (only for ARM9)
#ifdef SDK_ARM9
#  define MI_CNT_HBCOPY16(size)    ( MI_DMA_ENABLE | MI_DMA_TIMING_H_BLANK | MI_DMA_SRC_INC | MI_DMA_DEST_RELOAD | MI_DMA_CONTINUOUS_ON | MI_DMA_16BIT_BUS | ((size)/2) )
#  define MI_CNT_HBCOPY32(size)    ( MI_DMA_ENABLE | MI_DMA_TIMING_H_BLANK | MI_DMA_SRC_INC | MI_DMA_DEST_RELOAD | MI_DMA_CONTINUOUS_ON | MI_DMA_32BIT_BUS | ((size)/4) )
#  define MI_CNT_HBCOPY16_IF(size) ( MI_CNT_HBCOPY16((size)) | MI_DMA_IF_ENABLE )
#  define MI_CNT_HBCOPY32_IF(size) ( MI_CNT_HBCOPY32((size)) | MI_DMA_IF_ENABLE )
#endif // SDK_ARM9

//---- VBlank copy
#define MI_CNT_VBCOPY16(size)      ( MI_DMA_ENABLE | MI_DMA_TIMING_V_BLANK | MI_DMA_SRC_INC | MI_DMA_DEST_INC | MI_DMA_16BIT_BUS | ((size)/2) )
#define MI_CNT_VBCOPY32(size)      ( MI_DMA_ENABLE | MI_DMA_TIMING_V_BLANK | MI_DMA_SRC_INC | MI_DMA_DEST_INC | MI_DMA_32BIT_BUS | ((size)/4) )
#define MI_CNT_VBCOPY16_IF(size)   ( MI_CNT_VBCOPY16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_VBCOPY32_IF(size)   ( MI_CNT_VBCOPY32((size)) | MI_DMA_IF_ENABLE )

//---- CARD read
// (other settings are not available)
#define MI_CNT_CARDRECV32(size)    ( MI_DMA_ENABLE | MI_DMA_TIMING_CARD | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | MI_DMA_32BIT_BUS | ((size)/4) )


//---- memory copy (only for ARM9)
#ifdef SDK_ARM9
#  define MI_CNT_MMCOPY(size)      ( MI_DMA_ENABLE | MI_DMA_TIMING_DISP_MMEM | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | MI_DMA_CONTINUOUS_ON | MI_DMA_32BIT_BUS | (4) )
#endif // SDK_ARM9

//---- geometry command copy (only for ARM9)
#ifdef SDK_ARM9
#  define MI_CNT_GXCOPY(size)      ( MI_DMA_ENABLE | MI_DMA_TIMING_GXFIFO | MI_DMA_SRC_INC | MI_DMA_DEST_FIX | MI_DMA_32BIT_BUS | ((size)/4) )
#  define MI_CNT_GXCOPY_IF(size)   ( MI_CNT_GXCOPY(size) | MI_DMA_IF_ENABLE )
#endif // SDK_ARM9


//---- DMA callback
typedef void (*MIDmaCallback) (void *);

//================================================================================
//            memory operation using DMA (sync)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill32

  Description:  fill memory with specified data.
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaFill32(u32 dmaNo, void *dest, u32 data, u32 size, BOOL dmaEnable);
static inline void    MI_DmaFill32(u32 dmaNo, void *dest, u32 data, u32 size)
{
	MIi_DmaFill32(dmaNo, dest, data, size, TRUE);
}
static inline void    MI_DmaFill32_SetUp(u32 dmaNo, void *dest, u32 data, u32 size)
{
	MIi_DmaFill32(dmaNo, dest, data, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy32

  Description:  copy memory with DMA
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size)
{
	MIi_DmaCopy32(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaCopy32_SetUp(u32 dmaNo, const void *src, void *dest, u32 size)
{
	MIi_DmaCopy32(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaClear32

  Description:  fill memory with 0
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_DmaClear32(u32 dmaNo, void *dest, u32 size)
{
    MIi_DmaFill32(dmaNo, dest, 0, size, TRUE);
}
static inline void MI_DmaClear32_SetUp(u32 dmaNo, void *dest, u32 size)
{
    MIi_DmaFill32(dmaNo, dest, 0, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend32

  Description:  send u32 data to fixed address
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaSend32(u32 dmaNo, const void *src, volatile void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaSend32(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
	MIi_DmaSend32(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaSend32_SetUp(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
	MIi_DmaSend32(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32

  Description:  receive u32 data from fixed address
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaRecv32(u32 dmaNo, volatile const void *src, void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaRecv32(u32 dmaNo, volatile const void *src, void *dest, u32 size)
{
	MIi_DmaRecv32(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaRecv32_SetUp(u32 dmaNo, volatile const void *src, void *dest, u32 size)
{
	MIi_DmaRecv32(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe32

  Description:  pipe data from fixed address to fixed address.
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaPipe32(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaPipe32(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size)
{
	MIi_DmaPipe32(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaPipe32_SetUp(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size)
{
	MIi_DmaPipe32(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill16

  Description:  fill memory with specified data.
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaFill16(u32 dmaNo, void *dest, u16 data, u32 size, BOOL dmaEnable);
static inline void    MI_DmaFill16(u32 dmaNo, void *dest, u16 data, u32 size)
{
	MIi_DmaFill16(dmaNo, dest, data, size, TRUE);
}
static inline void    MI_DmaFill16_SetUp(u32 dmaNo, void *dest, u16 data, u32 size)
{
	MIi_DmaFill16(dmaNo, dest, data, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy16

  Description:  copy memory with DMA
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size)
{
	MIi_DmaCopy16(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaCopy16_SetUp(u32 dmaNo, const void *src, void *dest, u32 size)
{
	MIi_DmaCopy16(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaClear16

  Description:  fill memory with 0
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_DmaClear16(u32 dmaNo, void *dest, u32 size)
{
    MIi_DmaFill16(dmaNo, dest, 0, size, TRUE);
}
static inline void MI_DmaClear16_SetUp(u32 dmaNo, void *dest, u32 size)
{
    MIi_DmaFill16(dmaNo, dest, 0, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend16

  Description:  send u16 data to fixed address
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaSend16(u32 dmaNo, const void *src, volatile void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaSend16(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
	MIi_DmaSend16(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaSend16_SetUp(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
	MIi_DmaSend16(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16

  Description:  receive u16 data from fixed address
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaRecv16(u32 dmaNo, volatile const void *src, void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaRecv16(u32 dmaNo, volatile const void *src, void *dest, u32 size)
{
	MIi_DmaRecv16(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaRecv16_SetUp(u32 dmaNo, volatile const void *src, void *dest, u32 size)
{
	MIi_DmaRecv16(dmaNo, src, dest, size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe16

  Description:  pipe data from fixed address to fixed address.
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaPipe16(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, BOOL dmaEnable);
static inline void    MI_DmaPipe16(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size)
{
	MIi_DmaPipe16(dmaNo, src, dest, size, TRUE);
}
static inline void    MI_DmaPipe16_SetUp(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size)
{
	MIi_DmaPipe16(dmaNo, src, dest, size, FALSE);
}

//================================================================================
//            memory operation using DMA (async)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill32Async

  Description:  fill memory with specified data.
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaFill32Async(u32 dmaNo, void *dest, u32 data, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaFill32Async(u32 dmaNo, void *dest, u32 data, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaFill32Async(dmaNo, dest, data, size, callback, arg, TRUE);
}
static inline void    MI_DmaFill32Async_SetUp(u32 dmaNo, void *dest, u32 data, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaFill32Async(dmaNo, dest, data, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy32Async

  Description:  copy memory with DMA
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaCopy32Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaCopy32Async_SetUp(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaCopy32Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaClear32Async

  Description:  fill memory with 0
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_DmaClear32Async(u32 dmaNo, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
    MIi_DmaFill32Async(dmaNo, dest, 0, size, callback, arg, TRUE);
}
static inline void MI_DmaClear32Async_SetUp(u32 dmaNo, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
    MIi_DmaFill32Async(dmaNo, dest, 0, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend32Async

  Description:  send u32 data to fixed address
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaSend32Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaSend32Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaSend32Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaSend32Async_SetUp(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaSend32Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32Async

  Description:  receive u32 data from fixed address
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaRecv32Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaRecv32Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaRecv32Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaRecv32Async_SetUp(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaRecv32Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe32Async

  Description:  pipe data from fixed address to fixed address.
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaPipe32Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaPipe32Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaPipe32Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaPipe32Async_SetUp(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaPipe32Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaFill16Async

  Description:  fill memory with specified data.
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaFill16Async(u32 dmaNo, void *dest, u16 data, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaFill16Async(u32 dmaNo, void *dest, u16 data, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaFill16Async(dmaNo, dest, data, size, callback, arg, TRUE);
}
static inline void    MI_DmaFill16Async_SetUp(u32 dmaNo, void *dest, u16 data, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaFill16Async(dmaNo, dest, data, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaCopy16Async

  Description:  copy memory with DMA
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaCopy16Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaCopy16Async_SetUp(u32 dmaNo, const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaCopy16Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaClear16Async

  Description:  fill memory with 0
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_DmaClear16Async(u32 dmaNo, void *dest, u32 size, MIDmaCallback callback,
                                      void *arg)
{
    MIi_DmaFill16Async(dmaNo, dest, 0, size, callback, arg, TRUE);
}
static inline void MI_DmaClear16Async_SetUp(u32 dmaNo, void *dest, u32 size, MIDmaCallback callback,
                                      void *arg)
{
    MIi_DmaFill16Async(dmaNo, dest, 0, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaSend16Async

  Description:  send u16 data to fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaSend16Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaSend16Async(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaSend16Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaSend16Async_SetUp(u32 dmaNo, const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaSend16Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16Async

  Description:  receive u16 data from fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaRecv16Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaRecv16Async(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaRecv16Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaRecv16Async_SetUp(u32 dmaNo, volatile const void *src, void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaRecv16Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         MI_DmaPipe16Async

  Description:  receive u16 data from fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_DmaPipe16Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg, BOOL dmaEnable);
static inline void    MI_DmaPipe16Async(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaPipe16Async(dmaNo, src, dest, size, callback, arg, TRUE);
}
static inline void    MI_DmaPipe16Async_SetUp(u32 dmaNo, volatile const void *src, volatile void *dest, u32 size, MIDmaCallback callback, void *arg)
{
	MIi_DmaPipe16Async(dmaNo, src, dest, size, callback, arg, FALSE);
}

//================================================================================
//            HBlank DMA
//================================================================================
//  32bit
void    MI_HBlankDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size);
//  16bit
void    MI_HBlankDmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size);
//  32bit, with interrupt when finish DMA
void    MI_HBlankDmaCopy32If(u32 dmaNo, const void *src, void *dest, u32 size);
//  16bit, with interrupt when finish DMA
void    MI_HBlankDmaCopy16If(u32 dmaNo, const void *src, void *dest, u32 size);

//================================================================================
//        VBlank DMA
//================================================================================
//  32bit sync
void    MI_VBlankDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size);
//  16bit sync
void    MI_VBlankDmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size);
//  32bit async
void    MI_VBlankDmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size,
                                MIDmaCallback callback, void *arg);
//  16bit async
void    MI_VBlankDmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size,
                                MIDmaCallback callback, void *arg);

//================================================================================
//        CARD DMA
//================================================================================
//  32bit sync (CARD-DMA can not use DMA-callback because of continuous-mode)
void    MIi_CardDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size);

//================================================================================
//        main memory display DMA
//================================================================================
//  (Sync)
void    MI_DispMemDmaCopy(u32 dmaNo, const void *src);

//================================================================================
//         geometry FIFO DMA
//================================================================================
//       32bit version only.
//       size unit is byte, equal to 'the amount of commands * 4'
//  (Sync)  
void    MI_SendGXCommand(u32 dmaNo, const void *src, u32 commandLength);
//  (Async, in flagment)
void    MI_SendGXCommandAsync(u32 dmaNo, const void *src, u32 commandLength, MIDmaCallback callback,
                              void *arg);
//  (Sync, at once)
void    MI_SendGXCommandFast(u32 dmaNo, const void *src, u32 commandLength);
//  (Async, at once)
void    MI_SendGXCommandAsyncFast(u32 dmaNo, const void *src, u32 commandLength,
                                  MIDmaCallback callback, void *arg);

//================================================================================
//       DMA WAIT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_IsDmaBusy

  Description:  check whether DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    MI_IsDmaBusy(u32 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         MI_WaitDma

  Description:  wait while DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_WaitDma(u32 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         MI_StopDma

  Description:  stop DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_StopDma(u32 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         MI_StopAllDma

  Description:  stop all DMA

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_StopAllDma(void);

//================================================================================
//        restart DMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_DmaRestart

  Description:  restart DMA
                Just set the enable bit.

  Arguments:    ndmaNo : DMA channel	MI_NDMA_REG( ndmaNo, MI_NDMA_REG_CNT_WOFFSET ) |= MI_DMA_ENABLE;
 No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRestart(u32 dmaNo);


//================================================================================
//		 setting each register directly (internal)
//================================================================================
void MIi_SetDmaSrc16( u32 dmaNo, void *src );
void MIi_SetDmaSrc32( u32 dmaNo, void *src );
void MIi_SetDmaDest16( u32 dmaNo, void *dest );
void MIi_SetDmaDest32( u32 dmaNo, void *dest );
void MIi_SetDmaSize16( u32 dmaNo, u32 size );
void MIi_SetDmaSize32( u32 dmaNo, u32 size );

//--------------------------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_DMA_H_ */
#endif
