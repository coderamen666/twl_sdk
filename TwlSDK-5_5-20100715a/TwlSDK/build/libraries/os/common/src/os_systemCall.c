/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     os_systemCall.c

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
#ifdef SDK_ARM9
#include <nitro.h>

//----------------------------------------------------------------
asm void SVC_WaitByLoop( s32 count )
{
  swi         3
  bx          lr
}

#if 0
//----------------------------------------------------------------
asm void SVC_WaitIrq( BOOL clear, OSIrqMask irqFlags )
{
  mov         r2,#0
  swi         4
  bx          lr
}
#endif

//----------------------------------------------------------------
asm void SVC_WaitVBlankIntr( void )
{
  mov         r2,#0
  swi         5
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_Halt( void )
{
  swi         6
  bx          lr
}

//----------------------------------------------------------------
asm s32 SVC_Div( s32 number, s32 denom )
{
  swi         9
  bx          lr
}

//----------------------------------------------------------------
asm s32 SVC_DivRem( s32 number, s32 denom )
{
  swi         9
  mov         r0,r1
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_CpuSet( const void *srcp, void *destp, u32 dmaCntData )
{
  swi         11
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_CpuSetFast( const void *srcp, void *destp, u32 dmaCntData )
{
  swi         12
  bx          lr
}

//----------------------------------------------------------------
asm u16 SVC_Sqrt( u32 src )
{
  swi         13
  bx          lr
}

//----------------------------------------------------------------
asm u16 SVC_GetCRC16( u32 start, const void *datap, u32 size )
{
  swi         14
  bx          lr
}

//----------------------------------------------------------------
asm BOOL SVC_IsMmemExpanded( void )
{
  swi         15
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_UnpackBits( const void *srcp, void *destp, const MIUnpackBitsParam *paramp )
{
  swi         16
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_UncompressLZ8( const void *srcp, void *destp )
{
  swi         17
  bx          lr
}

//----------------------------------------------------------------
asm s32 SVC_UncompressLZ16FromDevice( const void *srcp, void *destp, const void *paramp, const MIReadStreamCallbacks *callbacks )
{
  swi         18
  bx          lr
}

//----------------------------------------------------------------
asm s32 SVC_UncompressHuffmanFromDevice( const void *srcp, void *destp, u8 *tableBufp, const MIReadStreamCallbacks *callbacks )
{
  swi         19
  bx          lr
}

//----------------------------------------------------------------
asm void SVC_UncompressRL8( const void *srcp, void *destp )
{
  swi         20
  bx          lr
}

//----------------------------------------------------------------
asm s32 SVC_UncompressRL16FromDevice( const void *srcp, void *destp, const void *paramp, const MIReadStreamCallbacks *callbacks )
{
  swi         21
  bx          lr
}
#endif // ifdef SDK_ARM9
