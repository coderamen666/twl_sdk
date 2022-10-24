/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     memory.h

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

#ifndef NITRO_MI_MEMORY_H_
#define NITRO_MI_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>


//======================================================================
void    MIi_CpuClear16(u16 data, void *destp, u32 size);
void    MIi_CpuCopy16(const void *srcp, void *destp, u32 size);
void    MIi_CpuSend16(const void *srcp, volatile void *destp, u32 size);
void    MIi_CpuRecv16(volatile const void *srcp, void *destp, u32 size);
void    MIi_CpuPipe16(volatile const void *srcp, volatile void *destp, u32 size);
void    MIi_CpuMove16(const void *src, void *dest, u32 size);
void*   MIi_CpuFind16(const void *src, u16 data, u32 size);
int     MIi_CpuComp16(const void *mem1, const void *mem2, u32 size);

void    MIi_CpuClear32(u32 data, void *destp, u32 size);
void    MIi_CpuCopy32(const void *srcp, void *destp, u32 size);
void    MIi_CpuSend32(const void *srcp, volatile void *destp, u32 size);
void    MIi_CpuRecv32(volatile const void *srcp, void *destp, u32 size);
void    MIi_CpuPipe32(volatile const void *srcp, volatile void *destp, u32 size);
void    MIi_CpuMove32(const void *src, void *dest, u32 size);
void*   MIi_CpuFind32(const void *src, u32 data, u32 size);
int     MIi_CpuComp32(const void *mem1, const void *mem2, u32 size);

void    MIi_CpuClearFast(u32 data, void *destp, u32 size);
void    MIi_CpuCopyFast(const void *srcp, void *destp, u32 size);
void    MIi_CpuSendFast(const void *srcp, volatile void *destp, u32 size);
void    MIi_CpuRecvFast(volatile const void *srcp, void *destp, u32 size);
void    MIi_CpuMoveFast(const void *src, void *dest, u32 size);


//================================================================================
//               32 bit version
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_CpuFill32

  Description:  fill memory with specified data.  (32 bit version)

  Arguments:    dest :  destination address to fill data, must be in 4 byte alignment
                data :  data to fill
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuFill32(void *dest, u32 data, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuClear32(data, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy32

  Description:  copy memory data (32 bit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuCopy32(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuCopy32(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuClear32

  Description:  fill memory with 0 (32 bit version)

  Arguments:    dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuClear32(void *dest, u32 size)
{
    MI_CpuFill32(dest, 0, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuSend32

  Description:  write some data to fixed address (32 bit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuSend32(const void *src, volatile void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuSend32(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuRecv32

  Description:  receive u32 data from fixed address
                32bit version

  Arguments:    src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuRecv32(volatile const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuRecv32(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuPipe32

  Description:  pipe data from fixed address to fixed address.
                32bit version

  Arguments:    src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuPipe32(volatile const void *src, volatile void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuPipe32(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuMove32

  Description:  move memory data (32 bit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuMove32(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuMove32(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuFind32

  Description:  find memory data (32 bit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                data :  target data
                size :  size (byte), must be in 4 byte alignment

  Returns:      pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
static inline void* MI_CpuFind32(const void *src, u32 data, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");

    return MIi_CpuFind32(src, data, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuComp32

  Description:  compare memory data (32 bit version)

  Arguments:    mem1 :  target address 1, must be in 4 byte alignment
                mem2 :  target address 2, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      < 0 : mem1 smaller than mem2
                = 0 : mem1 equals mem2
                > 0 : mem1 larger than mem2
 *---------------------------------------------------------------------------*/
static inline int MI_CpuComp32(const void *mem1, const void *mem2, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)mem1 & 3) == 0, "target address 1 must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)mem2 & 3) == 0, "target address 2 must be in 4-byte alignment");

    return MIi_CpuComp32(mem1, mem2, size);
}

//================================================================================
//               16 bit version
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_CpuFill16

  Description:  fill memory with specified data.  (16 bit version)

  Arguments:    dest :  destination address to fill data, must be in 2 byte alignment
                data :  data to fill
                size :  size (byte), must be in 2 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuFill16(void *dest, u16 data, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "source address must be in 2-byte alignment");

    MIi_CpuClear16(data, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy16

  Description:  copy memory data (16 bit version)

  Arguments:    src  :  source address, must be in 2 byte alignment
                dest :  destination address, must be in 2 byte alignment
                size :  size (byte), must be in 2 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuCopy16(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "destination address must be in 2-byte alignment");

    MIi_CpuCopy16(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuClear16

  Description:  fill memory with 0 (16 bit version)

  Arguments:    dest :  destination address, must be in 2 byte alignment
                size :  size (byte), must be in 2 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuClear16(void *dest, u32 size)
{
    MI_CpuFill16(dest, 0, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuSend16

  Description:  write some data to fixed address (16 bit version)

  Arguments:    src  :  source address, must be in 2 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 2 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuSend16(const void *src, volatile void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "destination address must be in 2-byte alignment");

    MIi_CpuSend16(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuRecv16

  Description:  receive u16 data from fixed address
                16bit version

  Arguments:    src   : source address. not incremented
                dest  : data buffer to receive
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuRecv16(volatile const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "destination address must be in 2-byte alignment");

    MIi_CpuRecv16(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuRecv16

  Description:  pipe data from fixed address to fixed address.
                16bit version

  Arguments:    src   : source address. not incremented
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuPipe16(volatile const void *src, volatile void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "destination address must be in 2-byte alignment");

    MIi_CpuPipe16(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuMove16

  Description:  move memory data (16 bit version)

  Arguments:    src  :  source address, must be in 2 byte alignment
                dest :  destination address, must be in 2 byte alignment
                size :  size (byte), must be in 2 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuMove16(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 1) == 0, "destination address must be in 2-byte alignment");

    MIi_CpuMove16(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuFind16

  Description:  find memory data (16 bit version)

  Arguments:    src  :  source address, must be in 2 byte alignment
                data :  target data
                size :  size (byte), must be in 2 byte alignment

  Returns:      pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
static inline void* MI_CpuFind16(const void *src, u16 data, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)src & 1) == 0, "source address must be in 2-byte alignment");

    return MIi_CpuFind16(src, data, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuComp16

  Description:  compare memory data (16 bit version)

  Arguments:    mem1 :  target address 1, must be in 2 byte alignment
                mem2 :  target address 2, must be in 2 byte alignment
                size :  size (byte), must be in 2 byte alignment

  Returns:      < 0 : mem1 smaller than mem2
                = 0 : mem1 equals mem2
                > 0 : mem1 larger than mem2
 *---------------------------------------------------------------------------*/
static inline int MI_CpuComp16(const void *mem1, const void *mem2, u32 size)
{
    SDK_TASSERTMSG((size & 1) == 0, "size & 1 must be 0");
    SDK_TASSERTMSG(((u32)mem1 & 1) == 0, "target address 1 must be in 2-byte alignment");
    SDK_TASSERTMSG(((u32)mem2 & 1) == 0, "target address 2 must be in 2-byte alignment");

    return MIi_CpuComp16(mem1, mem2, size);
}

//================================================================================
//               32 byte unit version
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_CpuFillFast

  Description:  fill memory with specified data quickly.  (32 byte unit version)

  Arguments:    dest :  destination address to fill data
                data :  data to fill
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuFillFast(void *dest, u32 data, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "source address must be in 4-byte alignment");

    MIi_CpuClearFast(data, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopyFast

  Description:  copy memory data quickly (32 byte unit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuCopyFast(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuCopyFast(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuClearFast

  Description:  fill memory with 0 quickly (32 byte unit version)

  Arguments:    dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4-byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuClearFast(void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MI_CpuFillFast(dest, 0, size);
}
/*---------------------------------------------------------------------------*
  Name:         MI_CpuSendFast

  Description:  move memory data (32 byte unit version)

  Arguments:    src  :  data stream to send
                dest :  destination address, not incremented
                size :  size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuSendFast( register const void *src, register volatile void *dest, register u32 size )
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

	MIi_CpuSendFast(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuRecvFast

  Description:  move memory data (32 byte unit version)

  Arguments:    src  :  source address. not incremented
                dest :  data buffer to receive
                size :  size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuRecvFast(volatile const void *src, register void *dest, register u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

	MIi_CpuRecvFast(src, dest, size);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuMoveFast

  Description:  move memory data (32 byte unit version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuMoveFast(const void *src, void *dest, u32 size)
{
    SDK_TASSERTMSG((size & 3) == 0, "size & 3 must be 0");
    SDK_TASSERTMSG(((u32)src & 3) == 0, "source address must be in 4-byte alignment");
    SDK_TASSERTMSG(((u32)dest & 3) == 0, "destination address must be in 4-byte alignment");

    MIi_CpuMoveFast(src, dest, size);
}


//================================================================================
//               8 bit version
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_CpuFill8

  Description:  fill memory with specified data.  (8 bit version)

  Arguments:    dest :  destination address to fill data, no limitation for alignment
                data :  data to fill
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_CpuFill8(void *dest, u8 data, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy8

  Description:  copy memory data (8 bit version)

  Arguments:    src  :  source address, no limitation for alignment
                dest :  destination address, no limitation for alignment
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_CpuCopy8(const void *src, void *dest, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuFind8

  Description:  find memory data (8 bit version)

  Arguments:    src  :  source address, no limitation for alignment
                data :  target data
                size :  size (byte), no limitation for alignment

  Returns:      pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
void*   MI_CpuFind8(const void *src, u8 data, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuComp8

  Description:  compare memory data (8 bit version)

  Arguments:    mem1 :  target address 1, no limitation for alignment
                mem2 :  target address 2, no limitation for alignment
                size :  size (byte), no limitation for alignment

  Returns:      < 0 : mem1 smaller than mem2
                = 0 : mem1 equals mem2
                > 0 : mem1 larger than mem2
 *---------------------------------------------------------------------------*/
int     MI_CpuComp8(const void *mem1, const void *mem2, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuClear8

  Description:  fill memory with 0 (8 bit version)

  Arguments:    dest :  destination address, no limitation for alignment
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuClear8(void *dest, u32 size)
{
    MI_CpuFill8(dest, 0, size);
}

//================================================================================
//               32 bit version
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_ReadWord

  Description:  read 32 bit data from specified address

  Arguments:    adrs :  address to read

  Returns:      data which is read
 *---------------------------------------------------------------------------*/
#ifndef SDK_ASM
#define MI_ReadWord( adrs )   (*(vu32 *)(adrs))
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_WriteWord

  Description:  write 32 bit data to specified adress

  Arguments:    adrs :  address to write
                val  :  data to write

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_ASM
#define MI_WriteWord( adrs, val )  do { (*(vu32 *)(adrs)) = (u32)(val); } while(0)
#endif


//================================================================================
//               mixed version
//================================================================================

/*---------------------------------------------------------------------------*
  Name:         MI_CpuFill

  Description:  fill memory with specified data.  (mixed version)

  Arguments:    dest :  destination address to fill data, no limitation for alignment
                data :  data to fill
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_CpuFill(void *dest, u8 data, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy

  Description:  copy memory data (mixed version)

  Arguments:    src  :  source address, no limitation for alignment
                dest :  destination address, no limitation for alignment
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_CpuCopy(const void *srcp, void *destp, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuMove

  Description:  move memory data (mixed version)

  Arguments:    src  :  source address, must be in 4 byte alignment
                dest :  destination address, must be in 4 byte alignment
                size :  size (byte), must be in 4 byte alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_CpuMove(const void *srcp, void *destp, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MI_CpuClear

  Description:  fill memory with 0 (mixed version)

  Arguments:    dest :  destination address, no limitation for alignment
                size :  size (byte), no limitation for alignment

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_CpuClear(void *dest, u32 size)
{
    MI_CpuFill(dest, 0, size);
}

//================================================================================
//      the following functions are prepared for SDK private use.
//      don't use in application thoughtlessly
//================================================================================
void    MI_Copy16B(register const void *pSrc, register void *pDest);
void    MI_Copy32B(register const void *pSrc, register void *pDest);
void    MI_Copy36B(register const void *pSrc, register void *pDest);
void    MI_Copy48B(register const void *pSrc, register void *pDest);
void    MI_Copy64B(register const void *pSrc, register void *pDest);
void    MI_Copy128B(register const void *pSrc, register void *pDest);

void    MI_Zero32B(register void *pDest);
void    MI_Zero36B(register void *pDest);
void    MI_Zero48B(register void *pDest);
void    MI_Zero64B(register void *pDest);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_MEMORY_H_ */
#endif
