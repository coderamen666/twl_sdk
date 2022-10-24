/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_memory.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro/types.h>
#include <nitro/platform.h>
#include <nitro/mi/memory.h>
#include <nitro/math/math.h>


//****Bug fix****
//  Because halfword access instructions such as ldrh and strh are not passed through by the inline assembler due to a CW bug, instruction values are written directly using dcd in order to avoid this bug.
//   
//  When the bug is fixed, the 'define' below will be removed.

//  Fixed with CodeWarrior 0.4 or later
//#define CW_BUG_FOR_LDRH_AND_STRH


#define HALFW_CONDAL  0xe0000000       // Condition(ALL)
#define HALFW_CONDNE  0x10000000       // Condition(NE)
#define HALFW_CONDEQ  0x00000000       // Condition(EQ)

#define HALFW_OFF_PL  0x00800000       // Offset plus
#define HALFW_OFF_MI  0x00000000       // Offset minus
#define HALFW_LOAD    0x00100000       // Load
#define HALFW_STORE   0x00000000       // Store
#define HALFW_RN(n)   ((n)<<16)        // Register Rn
#define HALFW_RD(n)   ((n)<<12)        // Register Rd

#define HALFW_DEF1    0x004000B0       // Fixed
#define HALFW_DEF2    0x014000B0       // Fixed

#define HALFW_IMM(n)   ( ((n)&0xf)  |  (((n)&0xf0)<<4) )        // Immediate


#define HALFW_DCD( cond, d, n, offset, sign, ldst, def ) \
   dcd (def)|(cond)|(sign)|(ldst)|HALFW_RN(n)|HALFW_RD(d)|HALFW_IMM(offset)

//---- ldrh Rn, [Rd], +#offset
#define LDRH_AD1( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_PL, HALFW_LOAD, HALFW_DEF1 )

//---- ldrh Rn, [Rd, +#offset]
#define LDRH_AD2( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_PL, HALFW_LOAD, HALFW_DEF2 )

//---- ldrh Rn, [Rd], -#offset
#define LDRH_AD3( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_MI, HALFW_LOAD, HALFW_DEF1 )

//---- ldrh Rn, [Rd, -#offset]
#define LDRH_AD4( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_MI, HALFW_LOAD, HALFW_DEF2 )

//---- strh Rn, [Rd], +#offset
#define STRH_AD1( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_PL, HALFW_STORE, HALFW_DEF1 )

//---- strh Rn, [Rd, +#offset]
#define STRH_AD2( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_PL, HALFW_STORE, HALFW_DEF2 )

//---- strh Rn, [Rd], -#offset
#define STRH_AD3( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_MI, HALFW_STORE, HALFW_DEF1 )

//---- strh Rn, [Rd, -#offset]
#define STRH_AD4( cond, d, n, offset ) \
   HALFW_DCD( cond, d, n, offset, HALFW_OFF_MI, HALFW_STORE, HALFW_DEF2 )



#include <nitro/code32.h>
//=======================================================================
//           MEMORY OPERATIONS
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_CpuClear16

  Description:  Fills memory with specified data.
                16-bit version.

  Arguments:    data: Fill data
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuClear16( register u16 data, register void* destp, register u32 size )
{
        mov     r3, #0                  // n = 0

@00:
        cmp     r3, r2                  // n < size ?
        strlth  r0, [r1, r3]            // *((vu16 *)(destp + n)) = data
        addlt   r3, r3, #2              // n += 2
        blt     @00

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuCopy16

  Description:  Copies memory by CPU.
                16-bit version

  Arguments:    srcp: Source address
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuCopy16( register const void *srcp, register void *destp, register u32 size )
{
        mov     r12, #0                 // n = 0

@10:
        cmp     r12, r2                 // n < size ?

#ifndef CW_BUG_FOR_LDRH_AND_STRH
        ldrlth  r3, [r0, r12]           // *((vu16 *)(destp + n)) = *((vu16 *)(srcp + n))
#else
        dcd     0xb19030bc
#endif
#ifndef CW_BUG_FOR_LDRH_AND_STRH
        strlth  r3, [r1, r12]
#else
        dcd     0xb18130bc
#endif
        addlt   r12, r12, #2            // n += 2
        blt     @10

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuSend16

  Description:  Sends u16 data to fixed address.
                16-bit version.

  Arguments:    src: Data stream to send
                dest: Destination address. Not incremented.
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuSend16( register const void *srcp, register volatile void* destp, register u32 size )
{
        mov     r12, #0                 // n = 0

@11:
        cmp     r12, r2                 // n < size ?
#ifndef CW_BUG_FOR_LDRH_AND_STRH
        ldrlth  r3, [r0, r12]           // *((vu16 *)(destp + n)) = *((vu16 *)(srcp + n))
#else
        dcd     0xb19030bc
#endif
        strlth  r3, [r1, #0]
        addlt   r12, r12, #2            // n += 2
        blt     @11

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuRecv16

  Description:  Receives u16 data from fixed address.
                16-bit version.

  Arguments:    src: Source address. Not incremented.
                dest: Data buffer to receive
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuRecv16( register volatile const void *srcp, register void* destp, register u32 size )
{
        mov     r12, #0                 // n = 0

@12:
        cmp     r12, r2                 // n < size ?
        ldrlth  r3, [r0]                // *((vu16 *)(destp + n)) = *((vu16 *)(srcp + n))
        strlth  r3, [r1, r12]
        addlt   r12, r12, #2            // n += 2
        blt     @12

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuPipe16

  Description:  Pipes data from fixed address to fixed address.
                16-bit version.

  Arguments:    src: Source address. Not incremented.
                dest: Destination address. Not incremented.
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuPipe16( register volatile const void *srcp, register volatile void* destp, register u32 size )
{
        mov     r12, #0                 // n = 0

@13:
        cmp     r12, r2                 // n < size ?
        ldrlth  r3, [r0]                // *((vu32 *)(destp)) = *((vu32 *)(srcp))
        strlth  r3, [r1]
        addlt   r12, r12, #2            // n += 2
        blt     @13

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuMove16

  Description:  Moves memory data (16-bit version).

  Arguments:    src:  Source address, must be in 2-byte alignment
                dest:  Destination address, must be in 2-byte alignment
                size:  Size (bytes), must be in 2-byte alignment

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void CpuCopy16Reverse( register const void *srcp, register void *destp, register u32 size )
{
        mov     r12, r1                 // r12: destEndp = destp
        add     r0, r0, r2              // r0:  srcp  += size
        add     r1, r1, r2              // r1:  destp += size

@14:
        cmp     r12, r1                 // while (destEndp < destp)
        ldrlth  r2, [r0, #-2]!          // *(--(vu32 *)(destp)) = *(--(vu32 *)(srcp))
        strlth  r2, [r1, #-2]!
        blt     @14

        bx      lr
}

void MIi_CpuMove16(const void *src, void *dest, u32 size)
{
    if( ( (u32)dest <= (u32)src )
     || ( (u32)src + size <= (u32)dest ) )
    {
        MIi_CpuCopy16(src, dest, size);
    }
    else
    {
        CpuCopy16Reverse(src, dest, size);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuFind16

  Description:  Finds memory data (16-bit version).

  Arguments:    src:  Source address, must be in 2 byte alignment
                data:  Target data
                size:  Size (bytes), must be in 2 byte alignment

  Returns:      Pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
void* MIi_CpuFind16(const void *src, u16 data, u32 size)
{
    const u16* p = src;
    u32 i;

    for( i = 0; i < size; i += 2, ++p )
    {
        if( *p == data )
        {
            return (void*)p;
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuComp16

  Description:  Compares memory data (16-bit version).

  Arguments:    mem1:  Target address 1, must be in 2-byte alignment
                mem2:  Target address 2, must be in 2-byte alignment
                size:  Size (bytes), must be in 2-byte alignment

  Returns:      < 0: mem1 smaller than mem2.
                = 0: mem1 equals mem2.
                > 0: mem1 larger than mem2.
 *---------------------------------------------------------------------------*/
int MIi_CpuComp16(const void *mem1, const void *mem2, u32 size)
{
    const u16* p1 = mem1;
    const u16* p2 = mem2;
    const u16* p1end = (const u16*)( (const u8*)p1 + size );

    while( p1 < p1end )
    {
        int d = (int)*p1++ - (int)*p2++;

        if( d != 0 )
        {
            return d;
        }
    }

    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         MIi_CpuClear32

  Description:  Fills memory with specified data.
                32-bit version.

  Arguments:    data: Fill data
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuClear32( register u32 data, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size

@20:
        cmp     r1, r12                 // while (destp < destEndp)
        stmltia r1!, {r0}               // *((vu32 *)(destp++)) = data
        blt     @20
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuCopy32

  Description:  Copies memory by CPU.
                32-bit version.

  Arguments:    srcp: Source address
                destp: Destination address
                size: size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuCopy32( register const void *srcp, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size

@30:
        cmp     r1, r12                 // while (destp < destEndp)
        ldmltia r0!, {r2}               // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        stmltia r1!, {r2}
        blt     @30

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuSend32

  Description:  Sends u32 data to fixed address.
                32-bit version.

  Arguments:    src: Data stream to send
                dest: Destination address. Not incremented
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuSend32( register const void *srcp, register volatile void *destp, register u32 size )
{
        add     r12, r0, r2             // r12: srcEndp = srcp + size

@31:
        cmp     r0, r12                 // while (srcp < srcEndp)
        ldmltia r0!, {r2}               // *((vu32 *)(destp)) = *((vu32 *)(srcp)++)
        strlt   r2, [r1]
        blt     @31

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuRecv32

  Description:  Receives u32 data from fixed address.
                32-bit version.

  Arguments:    src: Source address. Not incremented
                dest: Data buffer to receive
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuRecv32( volatile const void *srcp, register void *destp, register u32 size )
{
        add     r12, r1, r2             // r12: destEndp = destp + size

@32:
        cmp     r1, r12                 // while (dest < destEndp)
        ldrlt   r2, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        stmltia r1!, {r2}
        blt     @32

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuPipe32

  Description:  Pipes data from fixed address to fixed address.
                32-bit version.

  Arguments:    src: Source address. Not incremented
                dest: Destination address. Not incremented
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuPipe32( volatile const void *srcp, register volatile void *destp, register u32 size )
{
        mov     r12, #0                 // n = 0

@33:
        cmp     r12, r2                 // n < size ?
        ldrlt   r3, [r0]                // *((vu32 *)(destp)) = *((vu32 *)(srcp))
        strlt   r3, [r1]
        addlt   r12, r12, #4            // n += 4
        blt     @33

        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuMove32

  Description:  Moves memory data (32-bit version).

  Arguments:    src:  Source address, must be in 4-byte alignment
                dest:  Destination address, must be in 4-byte alignment
                size:  Size (bytes), must be in 4-byte alignment

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void CpuCopy32Reverse( register const void *srcp, register void *destp, register u32 size )
{
        mov     r12, r1                 // r12: destEndp = destp
        add     r0, r0, r2              // r0:  srcp  += size
        add     r1, r1, r2              // r1:  destp += size

@34:
        cmp     r12, r1                 // while (destEndp < destp)
        ldrlt   r2, [r0, #-4]!          // *(--(vu32 *)(destp)) = *(--(vu32 *)(srcp))
        strlt   r2, [r1, #-4]!
        blt     @34

        bx      lr
}

void MIi_CpuMove32(const void *src, void *dest, u32 size)
{
    if( ( (u32)dest <= (u32)src )
     || ( (u32)src + size <= (u32)dest ) )
    {
        MIi_CpuCopy32(src, dest, size);
    }
    else
    {
        CpuCopy32Reverse(src, dest, size);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuFind32

  Description:  Finds memory data (32-bit version).

  Arguments:    src:  Source address, must be in 4-byte alignment
                data:  Target data
                size:  Size (bytes), must be in 4-byte alignment

  Returns:      Pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
void* MIi_CpuFind32(const void *src, u32 data, u32 size)
{
    const u32* p = src;
    u32 i;

    for( i = 0; i < size; i += 4, ++p )
    {
        if( *p == data )
        {
            return (void*)p;
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuComp32

  Description:  Compares memory data (32-bit version).

  Arguments:    mem1:  Target address 1, must be in 4-byte alignment
                mem2:  Target address 2, must be in 4-byte alignment
                size:  Size (bytes), must be in 4-byte alignment

  Returns:      < 0: mem1 smaller than mem2.
                = 0: mem1 equals mem2.
                > 0: mem1 larger than mem2.
 *---------------------------------------------------------------------------*/
int MIi_CpuComp32(const void *mem1, const void *mem2, u32 size)
{
    const u32* p1 = mem1;
    const u32* p2 = mem2;
    const u32* p1end = (const u32*)( (const u8*)p1 + size );

    for( ; p1 < p1end; ++p1, ++p2 )
    {
        const u32 v1 = *p1;
        const u32 v2 = *p2;

        if( v1 != v2 )
        {
            return (v1 < v2) ? -1: 1;
        }
    }

    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         MIi_CpuClearFast

  Description:  Fills memory with specified data.
                High speed by writing 32 bytes at a time using stm.

  Arguments:    data: Fill data
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuClearFast( register u32 data, register void *destp, register u32 size )
{
        stmfd   sp!, {r4-r9}

        add     r9, r1, r2              // r9:  destEndp = destp + size
        mov     r12, r2, lsr #5         // r12: destBlockEndp = destp + size/32*32
        add     r12, r1, r12, lsl #5

        mov     r2, r0
        mov     r3, r2
        mov     r4, r2
        mov     r5, r2
        mov     r6, r2
        mov     r7, r2
        mov     r8, r2

@40:
        cmp     r1, r12                 // while (destp < destBlockEndp)
        stmltia r1!, {r0, r2-r8}        // *((vu32 *)(destp++)) = data
        blt     @40
@41:
        cmp     r1, r9                  // while (destp < destEndp)
        stmltia r1!, {r0}               // *((vu32 *)(destp++)) = data
        blt     @41

        ldmfd   sp!, {r4-r9}
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuCopyFast

  Description:  Copies memory by CPU.
                High speed by loading/writing 32byte at a time using stm/ldm.

  Arguments:    srcp: Source address
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuCopyFast( register const void *srcp, register void *destp, register u32 size )
{
        stmfd   sp!, {r4-r10}

        add     r10, r1, r2             // r10:  destEndp = destp + size
        mov     r12, r2, lsr #5         // r12: destBlockEndp = destp + size/32*32
        add     r12, r1, r12, lsl #5

@50:
        cmp     r1, r12                 // while (destp < destBlockEndp)
        ldmltia r0!, {r2-r9}            // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        stmltia r1!, {r2-r9}
        blt     @50
@51:
        cmp     r1, r10                 // while (destp < destEndp)
        ldmltia r0!, {r2}               // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        stmltia r1!, {r2}
        blt     @51

        ldmfd   sp!, {r4-r10}
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuSendFast

  Description:  Moves memory data (32 byte version).
                High speed by loading 32 bytes at a time using ldm.

  Arguments:    src:  Data stream to send
                dest:  Destination address, not incremented
                size:  Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuSendFast( register const void *srcp, register volatile void *destp, register u32 size )
{
        stmfd   sp!, {r4-r10}

        add     r10, r0, r2             // r10:  destEndp = destp + size
        mov     r12, r2, lsr #5         // r12: destBlockEndp = destp + size/32*32
        add     r12, r0, r12, lsl #5

@50:
        cmp     r0, r12                 // while (destp < destBlockEndp)
        ldmltia r0!, {r2-r9}            // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        strlt   r2, [r1]
        strlt   r3, [r1]
        strlt   r4, [r1]
        strlt   r5, [r1]
        strlt   r6, [r1]
        strlt   r7, [r1]
        strlt   r8, [r1]
        strlt   r9, [r1]
        blt     @50
@51:
        cmp     r0, r10                 // while (destp < destEndp)
        ldmltia r0!, {r2}               // *((vu32 *)(destp)++) = *((vu32 *)(srcp)++)
        strlt   r2, [r1]
        blt     @51

        ldmfd   sp!, {r4-r10}
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuRecvFast

  Description:  Moves memory data (32-byte version).
                High speed by writing 32 bytes at a time using stm.

  Arguments:    src:  Source address. not incremented
                dest:  Data buffer to receive
                size:  Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MIi_CpuRecvFast(volatile const void *srcp, register void *destp, register u32 size)
{
        stmfd   sp!, {r4-r10}

        add     r10, r1, r2             // r10:  destEndp = destp + size
        mov     r12, r2, lsr #5         // r12: destBlockEndp = destp + size/32*32
        add     r12, r1, r12, lsl #5

@50:
        cmp     r1, r12                 // while (destp < destBlockEndp)
        ldrlt   r2, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r3, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r4, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r5, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r6, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r7, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r8, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        ldrlt   r9, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        stmltia r1!, {r2-r9}
        blt     @50
@51:
        cmp     r1, r10                 // while (destp < destEndp)
        ldrlt   r2, [r0]                // *((vu32 *)(destp)++) = *((vu32 *)(srcp))
        stmltia r1!, {r2}
        blt     @51

        ldmfd   sp!, {r4-r10}
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MIi_CpuMoveFast

  Description:  Moves memory data (32-byte version).

  Arguments:    src:  Source address, must be in 4-byte alignment
                dest:  Destination address, must be in 4-byte alignment
                size:  Size (byte), must be in 4-byte alignment

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void CpuCopyFastReverse( register const void *srcp, register void *destp, register u32 size )
{
        stmfd   sp!, {r4-r10}

        mov     r10, r1                 // r10: destEndp = destp
        mov     r12, r2, lsr #5         // r12: destBlockBeginp = destp + (size/32)*32
        add     r12, r1, r12, lsl #5
        add     r0, r0, r2              // r0:  srcp  += size
        add     r1, r1, r2              // r1:  destp += size

@52:
        cmp     r12, r1                 // while( destBlockBeginP < destp )
        ldrlt   r2, [r0, #-4]!          // *(--(vu32 *)(destp)) = *(--(vu32 *)(srcp))
        strlt   r2, [r1, #-4]!
        blt     @52
@53:
        cmp     r10, r1                 // while (destEndp < destp)
        ldmltdb r0!, {r2-r9}
        stmltdb r1!, {r2-r9}
        blt     @53

        ldmfd   sp!, {r4-r10}
        bx      lr
}

void MIi_CpuMoveFast(const void *src, void *dest, u32 size)
{
    if( ( (u32)dest <= (u32)src )
     || ( (u32)src + size <= (u32)dest ) )
    {
        MIi_CpuCopyFast(src, dest, size);
    }
    else
    {
        CpuCopyFastReverse(src, dest, size);
    }
}

//=======================================================================
//           FOR CONVENIENCE (memory copy)
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_Copy16B

  Description:  Copies 16-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy16B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2, r3, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2}
    stmia   r1!, {r2}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Copy32B

  Description:  Copies 32-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy32B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2, r3, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3}
    stmia   r1!, {r2, r3}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Copy36B

  Description:  Copies 36-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy36B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2, r3, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Copy48B

  Description:  Copies 48-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy48B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2, r3, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Copy64B

  Description:  Copies 64-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy64B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2, r3, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0!, {r2, r3, r12}
    stmia   r1!, {r2, r3, r12}
    ldmia   r0,  {r0, r2, r3, r12}
    stmia   r1!, {r0, r2, r3, r12}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Copy128B

  Description:  Copies 128-byte data by CPU.

  Arguments:    srcp: Source address
                destp: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Copy128B(register const void* pSrc, register void* pDest)
{
    stmfd   sp!, {r4}

    ldmia   r0!, {r2, r3, r4, r12}         // r0-r3, r12 need not saved
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}
    ldmia   r0!, {r2, r3, r4, r12}
    stmia   r1!, {r2, r3, r4, r12}

    ldmfd   sp!, {r4}
    bx      lr
}

//=======================================================================
//           FOR SDK USE (needless set alignment)
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_CpuFill8

  Description:  Fills memory with specified data.
                Consider for alignment automatically.

  Arguments:    dstp: destination address
                data: fill data
                size: size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_SMALL_BUILD
asm void MI_CpuFill8( register void *dstp, register u8 data, register u32 size )
{
        mov     r12, #0                 // n = 0
@1:
        cmp     r12, r2                 // n < size ?
        strltb  r1, [r0, r12]           // *((u8*)( dstp + n ) ) = data

        addlt   r12, r12, #1            // n ++
        blt     @1

        bx      lr
}
#else  //ifdef SDK_SMALL_BUILD
asm void MI_CpuFill8( register void *dstp, register u8 data, register u32 size )
{
    cmp     r2, #0
    bxeq    lr

    // 16-bit alignment of dstp
    tst     r0, #1
    beq     @_1
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r12, [r0, #-1]
#else
    LDRH_AD4( HALFW_CONDAL, 12, 0, 1 ) // *** For CW BUG
#endif
    and     r12, r12, #0x00FF
    orr     r3, r12, r1, lsl #8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r3, [r0, #-1]
#else
    STRH_AD4( HALFW_CONDAL, 3, 0, 1 ) // *** For CW BUG
#endif
    add     r0, r0, #1
    subs    r2, r2, #1
    bxeq    lr
@_1:

    // 32-bit alignment
    cmp     r2, #2
    bcc     @_6
    orr     r1, r1, r1, lsl #8
    tst     r0, #2
    beq     @_8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r1, [r0], #2
#else
    STRH_AD1( HALFW_CONDAL, 1, 0, 2 ) // *** For CW BUG
#endif
    subs    r2, r2, #2
    bxeq    lr
@_8:
    // 32-bit transfer
    orr     r1, r1, r1, lsl #16
    bics    r3, r2, #3
    beq     @_10
    sub     r2, r2, r3
    add     r12, r3, r0
@_9:
    str     r1, [r0], #4
    cmp     r0, r12
    bcc     @_9

@_10:
    //  Last 16-bit transfer
    tst     r2, #2
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strneh  r1, [r0], #2
#else
    STRH_AD1( HALFW_CONDNE, 1, 0, 2 ) // *** For CW BUG
#endif

@_6:
    //  Last 8-bit transfer
    tst     r2, #1
    bxeq    lr
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r3, [r0]
#else
    LDRH_AD2( HALFW_CONDAL, 3, 0, 0 ) // *** For CW BUG
#endif
    and     r3, r3, #0xFF00
    and     r1, r1, #0x00FF
    orr     r1, r1, r3
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r1, [r0]
#else
    STRH_AD2( HALFW_CONDAL, 1, 0, 0 ) // *** For CW BUG
#endif
    bx      lr
}

asm void MI_CpuFill( register void *dstp, register u8 data, register u32 size )
{
    cmp     r2, #0
    bxeq    lr

    /* If less than 8 bytes, process directly here */
    cmp     r2, #8
    bgt	    _fill_and_align
    
_fill1_less_than_equal_8:
    rsb     r3, r2, #0x8
    add     pc, pc, r3, lsl #2
    nop
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    strb    r1, [r0], #1
    bx      lr

_fill_and_align:
    /* If more than 8 bytes, process awareness of alignment */

    /* Fill the register with 4 bytes */
    orr     r1, r1, r1, lsl #8
    orr     r1, r1, r1, lsl #16

    /* Process the fractions at the leading end of dst first */
    tst     r0, #1
    subne   r2, r2, #1
    strneb  r1, [r0], #1
    
    tst     r0, #2
    subne   r2, r2, #2
    strneh  r1, [r0], #2

    tst     r0, #4
    subne   r2, r2, #4
    strne   r1, [r0], #4

_fill32:
    cmp     r2, #32
    blt	    _fill4

_fill32_pre:
    stmfd   sp!, {r4-r10}
    mov     r4, r1
    mov     r5, r1
    mov     r6, r1
    mov     r7, r1
    mov     r8, r1
    mov     r9, r1
    mov     r10, r1
    subs    r2, r2, #32

_fill32_loop:
    stmgeia r0!, {r1,r4-r10}
    subges  r2, r2, #32
    bge     _fill32_loop
    add     r2, r2, #32
    
_fill32_post:
    ldmfd   sp!, {r4-r10}

_fill4:
    cmp     r2, #4
    blt     _fill1_less_than_4
    subs    r2, r2, #4

_fill4_loop:
    strge   r1, [r0], #4
    subs    r2, r2, #4
    bge     _fill4_loop
    add     r2, r2, #4
    
_fill1_less_than_4:
    subs    r2, r2, #1
    strgeb  r1, [r0], #1
    subges  r2, r2, #1
    strgeb  r1, [r0], #1
    subges  r2, r2, #1
    strgeb  r1, [r0], #1

    bx      lr
}

#endif // ifdef SDK_SMALL_BUILD

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy8

  Description:  Copies memory by CPU.
                Consider for alignment automatically.

  Arguments:    srcp: Source address
                dstp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_SMALL_BUILD
asm void MI_CpuCopy8( register const void *srcp, register void *dstp, register u32 size )
{
        mov     r12, #0                 // n = 0
@1:
        cmp     r12, r2                 // n < size ?
        ldrltb  r3, [r0, r12]           // *((vu8 *)(destp + p)) = *((vu8 *)(srcp + n))
        strltb  r3, [r1, r12]

        addlt   r12, r12, #1            // n ++
        blt     @1

        bx      lr
}

#else  //ifdef SDK_SMALL_BUILD
asm void MI_CpuCopy8( register const void *srcp, register void *dstp, register u32 size )
{
    cmp     r2, #0
    bxeq    lr

    // 16-bit alignment of dstp
    tst     r1, #1
    beq     @_1
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r12, [r1, #-1]
#else
    LDRH_AD4( HALFW_CONDAL, 12, 1, 1 ) // *** For CW BUG
#endif
    and     r12, r12, #0x00FF
    tst     r0, #1
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrneh  r3, [r0, #-1]
#else
    LDRH_AD4( HALFW_CONDNE, 3, 0, 1 ) // *** For CW BUG
#endif
    movne   r3, r3, lsr #8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldreqh  r3, [r0]
#else
    LDRH_AD2( HALFW_CONDEQ, 3, 0, 0 ) // *** For CW BUG
#endif
    orr     r3, r12, r3, lsl #8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r3, [r1, #-1]
#else
    STRH_AD4( HALFW_CONDAL, 3, 1, 1 ) // *** For CW BUG
#endif
    add     r0, r0, #1
    add     r1, r1, #1
    subs    r2, r2, #1
    bxeq    lr
@_1:

    // Check the 16- or 32-bit synchronization of the address fraction
    eor     r12, r1, r0
    tst     r12, #1
    beq     @_2

    // Doesn't synchronize at all, so use irregular 16-bit transfer
    //  tmp = *(u16*)src++ >> 8;
    //  while((size -= 2) >= 0) {
    //      tmp |= (*(u16*)src++ << 8);
    //      *(u16*)dst++ = (u16)tmp;
    //      tmp >>= 16;
    //  }
    bic     r0, r0, #1
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r12, [r0], #2
#else
        LDRH_AD1( HALFW_CONDAL, 12, 0, 2 ) // *** For CW BUG
#endif
    mov     r3, r12, lsr #8
    subs    r2, r2, #2
    bcc     @_3
@_4:
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r12, [r0], #2
#else
    LDRH_AD1( HALFW_CONDAL, 12, 0, 2 ) // *** For CW BUG
#endif
    orr     r12, r3, r12, lsl #8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r12, [r1], #2
#else
    STRH_AD1( HALFW_CONDAL, 12, 1, 2 ) // *** For CW BUG
#endif
    mov     r3, r12, lsr #16
    subs    r2, r2, #2
    bcs     @_4
    
@_3:
    //  if(size & 1)
    //      *dst = (u16)((*dst & 0xFF00) | tmp);
    //  return;
    tst     r2, #1
    bxeq    lr
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r12, [r1]
#else
    LDRH_AD2( HALFW_CONDAL, 12, 1, 0 ) // *** For CW BUG
#endif
    and     r12, r12, #0xFF00
    orr     r12, r12, r3
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r12, [r1]
#else
    STRH_AD2( HALFW_CONDAL, 12, 1, 0 ) // *** For CW BUG
#endif
    bx      lr

@_2:
    tst     r12, #2
    beq     @_5
    // 16-bit transfer
    bics    r3, r2, #1
    beq     @_6
    sub     r2, r2, r3
    add     r12, r3, r1
@_7:
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r3, [r0], #2
#else
    LDRH_AD1( HALFW_CONDAL, 3, 0, 2 ) // *** For CW BUG
#endif
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r3, [r1], #2
#else
    STRH_AD1( HALFW_CONDAL, 3, 1, 2 ) // *** For CW BUG
#endif
    cmp     r1, r12
    bcc     @_7
    b       @_6

@_5:
    // 32-bit alignment
    cmp     r2, #2
    bcc     @_6
    tst     r1, #2
    beq     @_8
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r3, [r0], #2
#else
    LDRH_AD1( HALFW_CONDAL, 3, 0, 2 ) // *** For CW BUG
#endif
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r3, [r1], #2
#else
        STRH_AD1( HALFW_CONDAL, 3, 1, 2 ) // *** For CW BUG
#endif
    subs    r2, r2, #2
    bxeq    lr
@_8:
    // 32-bit transfer
    bics    r3, r2, #3
    beq     @_10
    sub     r2, r2, r3
    add     r12, r3, r1
@_9:
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    cmp     r1, r12
    bcc     @_9

@_10:
    //  Last 16-bit transfer
    tst     r2, #2
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrneh  r3, [r0], #2
    strneh  r3, [r1], #2
#else
    LDRH_AD1( HALFW_CONDNE, 3, 0, 2 ) // *** For CW BUG
    STRH_AD1( HALFW_CONDNE, 3, 1, 2 ) // *** For CW BUG
#endif

@_6:
    //  Last 8-bit transfer
    tst     r2, #1
    bxeq    lr
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    ldrh    r2, [r1]
    ldrh    r0, [r0]
#else
    LDRH_AD2( HALFW_CONDAL, 2, 1, 0 ) // *** For CW BUG
    LDRH_AD2( HALFW_CONDAL, 0, 0, 0 ) // *** For CW BUG
#endif
    and     r2, r2, #0xFF00
    and     r0, r0, #0x00FF
    orr     r0, r2, r0
#ifndef CW_BUG_FOR_LDRH_AND_STRH
    strh    r0, [r1]
#else
    STRH_AD2( HALFW_CONDAL, 0, 1, 0 ) // *** For CW BUG
#endif
    bx      lr
}
#endif //ifdef SDK_SMALL_BUILD



/*---------------------------------------------------------------------------*
  Name:         MI_CpuFind8

  Description:  Finds memory data (8-bit version).

  Arguments:    src:  Source address, no limitation for alignment
                data:  Target data
                size:  Size (byte), no limitation for alignment

  Returns:      Pointer to found data or NULL.
 *---------------------------------------------------------------------------*/
void*   MI_CpuFind8(const void *src, u8 data, u32 size)
{
    const u8* p8 = (const u8*)src;

    if( size == 0 )
    {
        return NULL;
    }

    // If the address is not 2-byte aligned
    // Check only 1 byte and align in 2 bytes
    if( ((u32)p8 & 0x1) != 0 )
    {
        const u16 v = *(u16*)(p8 - 1);

        if( (v >> 8) == data )
        {
            return (void*)p8;
        }

        size--;
        p8++;
    }

    // Check in 2-byte units
    {
        const u16* p16 = (const u16*)p8;
        const u16* p16end = p16 + MATH_ROUNDDOWN(size, 2);

        for( ;  p16 < p16end; ++p16 )
        {
            const u16 v = *p16;

            if( (v & 0xFF) == data )
            {
                return (void*)( (u8*)p16 + 0 );
            }
            if( (v >> 8) == data )
            {
                return (void*)( (u8*)p16 + 1 );
            }
        }
    }

    // At this point the size is an odd number
    // Check remaining 1 byte
    if( (size & 0x1) != 0 )
    {
        const u16 v = *(u16*)(p8 + size - 1);

        if( (v & 0xFF) == data )
        {
            return (void*)(p8 + size - 1);
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuComp8

  Description:  Compares memory data (8-bit version).

  Arguments:    mem1:  Target address 1, no limitation for alignment
                mem2:  Target address 2, no limitation for alignment
                size:  Size (byte), no limitation for alignment

  Returns:      < 0: mem1 smaller than mem2.
                = 0: mem1 equals mem2.
                > 0: mem1 larger than mem2.
 *---------------------------------------------------------------------------*/
int     MI_CpuComp8(const void *mem1, const void *mem2, u32 size)
{
    const u8* p1 = mem1;
    const u8* p2 = mem2;
    const u8* p1end = (const u8*)( (const u8*)p1 + size );

    while( p1 < p1end )
    {
        const int d = (int)*p1++ - (int)*p2++;

        if( d != 0 )
        {
            return d;
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuCopy

  Description:  Copies memory by CPU.
                Byte access/ldm-stm version.

  Arguments:    srcp: Source address
                destp: Destination address.
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if PLATFORM_BYTES_ENDIAN == PLATFORM_ENDIAN_LITTLE
/* Little-endian */
#define FORWARD_(n)         lsl #((n) * 8)
#define BACKWARD_(n)        lsr #(32 - (n) * 8)
#define FORWARD_MASK(n)     #((1 << ((n) * 8)) - 1)
#elif PLATFORM_BYTES_ENDIAN == PLATFORM_ENDIAN_BIG
/* Big-endian */
#define FORWARD_(n)         lsr #((n) * 8)
#define BACKWARD_(n)        lsl #(32 - (n) * 8)
#define FORWARD_MASK(n)     #((1 << ((n) * 8)) - 1)
#else
#error
#endif

asm void MI_CpuCopy( register const void *srcp, register void *destp, register u32 size )
{
    /* Sequentially transfer from the front */

    /* If less than 8 bytes, process directly here */
    cmp     r2, #8
    bgt	    _forward_blt
    rsb     r3, r2, #0x8
    add     pc, pc, r3, lsl #3
    nop
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    bx      lr

_forward_blt:
    /* If more than 8 bytes, process awareness of alignment */

    /* Process the fractions at the leading end of src first */
    tst     r0, #1
    subne   r2, r2, #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    tst     r0, #2
    subne   r2, r2, #2
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1

    /* Branch processing according to the phases of src and dst */
    and     r3, r1, #3
    bic     r1, r1, #3
    cmp     r3, #0
    beq     _forward_blt_0
    cmp     r3, #1
    beq     _forward_blt_1
    cmp     r3, #2
    beq     _forward_blt_2
    b       _forward_blt_3
    
_forward_blt_0:
    /* Processing when (dst & 3 == 0) */
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_forward_blt_0_32:
    ldmgeia r0!, {r4-r10,lr}
    stmgeia r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _forward_blt_0_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}

    subs    r2, r2, #4
_forward_blt_0_4:
    ldrge   r3, [r0], #4
    strge   r3, [r1], #4
    subges  r2, r2, #4
    bge     _forward_blt_0_4
    add     r2, r2, #4
    b       _forward_blt_end

_forward_blt_1:
    /* Processing when (dst & 3 == 1) */
#define SHIFT  1
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, FORWARD_(4 - SHIFT)
    mov     r12, r12, BACKWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_forward_blt_1_32:
    ldmgeia r0!, {r4-r10,lr}
    movge   r3, r4, BACKWARD_(SHIFT)
    orrge   r4, r12, r4, FORWARD_(SHIFT)
    movge   r12, r5, BACKWARD_(SHIFT)
    orrge   r5, r3, r5, FORWARD_(SHIFT)
    movge   r3, r6, BACKWARD_(SHIFT)
    orrge   r6, r12, r6, FORWARD_(SHIFT)
    movge   r12, r7, BACKWARD_(SHIFT)
    orrge   r7, r3, r7, FORWARD_(SHIFT)
    movge   r3, r8, BACKWARD_(SHIFT)
    orrge   r8, r12, r8, FORWARD_(SHIFT)
    movge   r12, r9, BACKWARD_(SHIFT)
    orrge   r9, r3, r9, FORWARD_(SHIFT)
    movge   r3, r10, BACKWARD_(SHIFT)
    orrge   r10, r12, r10, FORWARD_(SHIFT)
    movge   r12, lr, BACKWARD_(SHIFT)
    orrge   lr, r3, lr, FORWARD_(SHIFT)
    stmgeia r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _forward_blt_1_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_forward_blt_1_4:
    ldrge   r3, [r0], #4
    orrge   r12, r12, r3, FORWARD_(SHIFT)
    strge   r12, [r1], #4
    movge   r12, r3, BACKWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _forward_blt_1_4
    add     r2, r2, #4
    /* Shared end process */
    sub     r0, r0, #SHIFT
    add     r2, r2, #SHIFT
    b       _forward_blt_end
#undef SHIFT

_forward_blt_2:
    /* Processing when (dst & 3 == 2) */
#define SHIFT  2
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, FORWARD_(4 - SHIFT)
    mov     r12, r12, BACKWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_forward_blt_2_32:
    ldmgeia r0!, {r4-r10,lr}
    movge   r3, r4, BACKWARD_(SHIFT)
    orrge   r4, r12, r4, FORWARD_(SHIFT)
    movge   r12, r5, BACKWARD_(SHIFT)
    orrge   r5, r3, r5, FORWARD_(SHIFT)
    movge   r3, r6, BACKWARD_(SHIFT)
    orrge   r6, r12, r6, FORWARD_(SHIFT)
    movge   r12, r7, BACKWARD_(SHIFT)
    orrge   r7, r3, r7, FORWARD_(SHIFT)
    movge   r3, r8, BACKWARD_(SHIFT)
    orrge   r8, r12, r8, FORWARD_(SHIFT)
    movge   r12, r9, BACKWARD_(SHIFT)
    orrge   r9, r3, r9, FORWARD_(SHIFT)
    movge   r3, r10, BACKWARD_(SHIFT)
    orrge   r10, r12, r10, FORWARD_(SHIFT)
    movge   r12, lr, BACKWARD_(SHIFT)
    orrge   lr, r3, lr, FORWARD_(SHIFT)
    stmgeia r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _forward_blt_2_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_forward_blt_2_4:
    ldrge   r3, [r0], #4
    orrge   r12, r12, r3, FORWARD_(SHIFT)
    strge   r12, [r1], #4
    movge   r12, r3, BACKWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _forward_blt_2_4
    add     r2, r2, #4
    /* Shared end process */
    sub     r0, r0, #SHIFT
    add     r2, r2, #SHIFT
    b       _forward_blt_end
#undef SHIFT

_forward_blt_3:
    /* Processing when (dst & 3 == 3) */
#define SHIFT  3
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, FORWARD_(4 - SHIFT)
    mov     r12, r12, BACKWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_forward_blt_3_32:
    ldmgeia r0!, {r4-r10,lr}
    movge   r3, r4, BACKWARD_(SHIFT)
    orrge   r4, r12, r4, FORWARD_(SHIFT)
    movge   r12, r5, BACKWARD_(SHIFT)
    orrge   r5, r3, r5, FORWARD_(SHIFT)
    movge   r3, r6, BACKWARD_(SHIFT)
    orrge   r6, r12, r6, FORWARD_(SHIFT)
    movge   r12, r7, BACKWARD_(SHIFT)
    orrge   r7, r3, r7, FORWARD_(SHIFT)
    movge   r3, r8, BACKWARD_(SHIFT)
    orrge   r8, r12, r8, FORWARD_(SHIFT)
    movge   r12, r9, BACKWARD_(SHIFT)
    orrge   r9, r3, r9, FORWARD_(SHIFT)
    movge   r3, r10, BACKWARD_(SHIFT)
    orrge   r10, r12, r10, FORWARD_(SHIFT)
    movge   r12, lr, BACKWARD_(SHIFT)
    orrge   lr, r3, lr, FORWARD_(SHIFT)
    stmgeia r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _forward_blt_3_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_forward_blt_3_4:
    ldrge   r3, [r0], #4
    orrge   r12, r12, r3, FORWARD_(SHIFT)
    strge   r12, [r1], #4
    movge   r12, r3, BACKWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _forward_blt_3_4
    add     r2, r2, #4
    /* Shared end process */
    sub     r0, r0, #SHIFT
    add     r2, r2, #SHIFT
    b       _forward_blt_end
#undef SHIFT

_forward_blt_end:
    /* Transfer the end fraction */
    tst     r2, #4
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    tst     r2, #2
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    tst     r2, #1
    ldrneb  r3, [r0], #1
    strneb  r3, [r1], #1
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_CpuMove

  Description:  Moves memory data (mixed version).

  Arguments:    srcp: Source address
                destp: Destination address
                size: Size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_CpuMove( register const void *srcp, register void *destp, register u32 size )
{
    /* Simply determines whether transfer is really necessary and determines the transfer direction */
    cmp     r2, #0
    subnes  r3, r0, r1
    bxeq    lr
    bgt     MI_CpuCopy

    /*
     * NOTE:
     * _forward_blt_1, _forward_blt_2, _forward_blt_3 and _backward_blt_1, _backward_blt_2, _backward_blt_3 switch constant SHIFT to 1, 2, 3 and have the same processes.
     * 
     * 
     * If you know a method to describe several lines of asm code with arm-elf-gcc using a macro, please do it all together.
     * 
     * It is also acceptable to consume one empty register for sharing.
     * (However, when the shifting amount is specified with a register, one cycle will be increased, so in exchange for size conservation, processing costs will increase.
     *  
     *  If that is true, there is no need to go to the trouble to describe with asm.)
     */

_backward:
    /* Sequentially transfer from the back end */
    add     r1, r1, r2
    add     r0, r0, r2

    /* If less than 8 bytes, process directly here */
    cmp     r2, #8
    bgt	    _backward_blt
    rsb     r3, r2, #0x8
    add     pc, pc, r3, lsl #3
    nop
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    ldrb    r3, [r0, #-1]!
    strb    r3, [r1, #-1]!
    bx      lr

_backward_blt:
    /* If more than 8 bytes, process awareness of alignment */

    /* Process the fractions at the trailing end of src first */
    tst     r0, #2
    subne   r2, r2, #2
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    tst     r0, #1
    subne   r2, r2, #1
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!

    /* Branch processing according to the phases of src and dst */
    and     r3, r1, #3
    bic     r1, r1, #3
    cmp     r3, #0
    beq     _backward_blt_0
    cmp     r3, #1
    beq     _backward_blt_1
    cmp     r3, #2
    beq     _backward_blt_2
    b       _backward_blt_3

_backward_blt_0:
    /* Processing when (dst & 3 == 0) */
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_backward_blt_0_32:
    ldmgedb r0!, {r4-r10,lr}
    stmgedb r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _backward_blt_0_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}

    subs    r2, r2, #4
_backward_blt_0_4:
    ldrge   r3, [r0, #-4]!
    strge   r3, [r1, #-4]!
    subges  r2, r2, #4
    bge     _backward_blt_0_4
    add     r2, r2, #4
    b       _backward_blt_end

_backward_blt_1:
    /* Processing when (dst & 3 == 1) */
#define SHIFT  1
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, BACKWARD_(4 - SHIFT)
    mov     r12, r12, FORWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_backward_blt_1_32:
    ldmgedb r0!, {r4-r10,lr}
    movge   r3, lr, FORWARD_(SHIFT)
    orrge   lr, r12, lr, BACKWARD_(SHIFT)
    movge   r12, r10, FORWARD_(SHIFT)
    orrge   r10, r3, r10, BACKWARD_(SHIFT)
    movge   r3, r9, FORWARD_(SHIFT)
    orrge   r9, r12, r9, BACKWARD_(SHIFT)
    movge   r12, r8, FORWARD_(SHIFT)
    orrge   r8, r3, r8, BACKWARD_(SHIFT)
    movge   r3, r7, FORWARD_(SHIFT)
    orrge   r7, r12, r7, BACKWARD_(SHIFT)
    movge   r12, r6, FORWARD_(SHIFT)
    orrge   r6, r3, r6, BACKWARD_(SHIFT)
    movge   r3, r5, FORWARD_(SHIFT)
    orrge   r5, r12, r5, BACKWARD_(SHIFT)
    movge   r12, r4, FORWARD_(SHIFT)
    orrge   r4, r3, r4, BACKWARD_(SHIFT)
    stmgeda r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _backward_blt_1_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_backward_blt_1_4:
    ldrge   r3, [r0, #-4]!
    orrge   r12, r12, r3, BACKWARD_(SHIFT)
    strge   r12, [r1], #-4
    movge   r12, r3, FORWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _backward_blt_1_4
    add     r2, r2, #4
    /* Shared end process */
    add     r1, r1, #4
    add     r0, r0, #(4 - SHIFT)
    add     r2, r2, #(4 - SHIFT)
    b       _backward_blt_end
#undef SHIFT

_backward_blt_2:
    /* Processing when (dst & 3 == 2) */
#define SHIFT  2
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, BACKWARD_(4 - SHIFT)
    mov     r12, r12, FORWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_backward_blt_2_32:
    ldmgedb r0!, {r4-r10,lr}
    movge   r3, lr, FORWARD_(SHIFT)
    orrge   lr, r12, lr, BACKWARD_(SHIFT)
    movge   r12, r10, FORWARD_(SHIFT)
    orrge   r10, r3, r10, BACKWARD_(SHIFT)
    movge   r3, r9, FORWARD_(SHIFT)
    orrge   r9, r12, r9, BACKWARD_(SHIFT)
    movge   r12, r8, FORWARD_(SHIFT)
    orrge   r8, r3, r8, BACKWARD_(SHIFT)
    movge   r3, r7, FORWARD_(SHIFT)
    orrge   r7, r12, r7, BACKWARD_(SHIFT)
    movge   r12, r6, FORWARD_(SHIFT)
    orrge   r6, r3, r6, BACKWARD_(SHIFT)
    movge   r3, r5, FORWARD_(SHIFT)
    orrge   r5, r12, r5, BACKWARD_(SHIFT)
    movge   r12, r4, FORWARD_(SHIFT)
    orrge   r4, r3, r4, BACKWARD_(SHIFT)
    stmgeda r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _backward_blt_2_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_backward_blt_2_4:
    ldrge   r3, [r0, #-4]!
    orrge   r12, r12, r3, BACKWARD_(SHIFT)
    strge   r12, [r1], #-4
    movge   r12, r3, FORWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _backward_blt_2_4
    add     r2, r2, #4
    /* Shared end process */
    add     r1, r1, #4
    add     r0, r0, #(4 - SHIFT)
    add     r2, r2, #(4 - SHIFT)
    b       _backward_blt_end
#undef SHIFT

_backward_blt_3:
    /* Processing when (dst & 3 == 3) */
#define SHIFT  3
    /* ldm-shift-stm in 32-byte units */
    ldr     r12, [r1]
    mov     r12, r12, BACKWARD_(4 - SHIFT)
    mov     r12, r12, FORWARD_(SHIFT)
    stmfd   sp!, {r4-r10,lr}
    subs    r2, r2, #32
_backward_blt_3_32:
    ldmgedb r0!, {r4-r10,lr}
    movge   r3, lr, FORWARD_(SHIFT)
    orrge   lr, r12, lr, BACKWARD_(SHIFT)
    movge   r12, r10, FORWARD_(SHIFT)
    orrge   r10, r3, r10, BACKWARD_(SHIFT)
    movge   r3, r9, FORWARD_(SHIFT)
    orrge   r9, r12, r9, BACKWARD_(SHIFT)
    movge   r12, r8, FORWARD_(SHIFT)
    orrge   r8, r3, r8, BACKWARD_(SHIFT)
    movge   r3, r7, FORWARD_(SHIFT)
    orrge   r7, r12, r7, BACKWARD_(SHIFT)
    movge   r12, r6, FORWARD_(SHIFT)
    orrge   r6, r3, r6, BACKWARD_(SHIFT)
    movge   r3, r5, FORWARD_(SHIFT)
    orrge   r5, r12, r5, BACKWARD_(SHIFT)
    movge   r12, r4, FORWARD_(SHIFT)
    orrge   r4, r3, r4, BACKWARD_(SHIFT)
    stmgeda r1!, {r4-r10,lr}
    subges  r2, r2, #32
    bge     _backward_blt_3_32
    add     r2, r2, #32
    ldmfd   sp!, {r4-r10,lr}
    /* ldr-shift-str in 4-byte units */
    subs    r2, r2, #4
_backward_blt_3_4:
    ldrge   r3, [r0, #-4]!
    orrge   r12, r12, r3, BACKWARD_(SHIFT)
    strge   r12, [r1], #-4
    movge   r12, r3, FORWARD_(SHIFT)
    subges  r2, r2, #4
    bge     _backward_blt_3_4
    add     r2, r2, #4
    /* Shared end process */
    add     r1, r1, #4
    add     r0, r0, #(4 - SHIFT)
    add     r2, r2, #(4 - SHIFT)
    b       _backward_blt_end
#undef SHIFT

_backward_blt_end:
    /* Transfer the leading end fraction */
    tst     r2, #4
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    tst     r2, #2
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    tst     r2, #1
    ldrneb  r3, [r0, #-1]!
    strneb  r3, [r1, #-1]!
    bx      lr
}

#undef FORWARD_
#undef BACKWARD_
#undef FORWARD_MASK

#include <nitro/codereset.h>


#include <nitro/code16.h>
//=======================================================================
//           FOR CONVENIENCE (filling zero)
//=======================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_Zero32B

  Description:  Fills 32-byte area with 0 by CPU.

  Arguments:    pDest: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Zero32B(register void* pDest)
{
    mov     r1,  #0
    mov     r2,  #0
    stmia   r0!, {r1, r2}
    mov     r3,  #0
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Zero36B

  Description:  Fills 36-byte area with 0 by CPU.

  Arguments:    pDest: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Zero36B(register void* pDest)
{
    mov     r1,  #0
    mov     r2,  #0
    mov     r3,  #0
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Zero48B

  Description:  Fills 48-byte area with 0 by CPU.

  Arguments:    pDest: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Zero48B(register void* pDest)
{
    mov     r1,  #0
    mov     r2,  #0
    mov     r3,  #0
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}

    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_Zero64B

  Description:  Fills 64-byte area with 0 by CPU.

  Arguments:    pDest: Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void MI_Zero64B(register void* pDest)
{
    mov     r1,  #0
    mov     r2,  #0
    stmia   r0!, {r1, r2}
    mov     r3,  #0
    stmia   r0!, {r1, r2}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}
    stmia   r0!, {r1, r2, r3}

    bx      lr
}

//---- End limitation of THUMB-Mode
#include <nitro/codereset.h>
