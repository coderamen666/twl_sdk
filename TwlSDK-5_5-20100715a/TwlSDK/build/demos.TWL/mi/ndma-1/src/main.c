/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - MI - ndma-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-27#$
  $Rev: 1202 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <twl.h>

void	memDump(void* addr, int size);
void    VBlankIntr(void);
void	MyNDMACallback(void* arg);

u8 src[0x1000] ATTRIBUTE_ALIGN(32) = {0};
u8 dest[0x1000] ATTRIBUTE_ALIGN(32) = {0};

volatile int flag = 0;

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();

	OS_Printf("*** start ndma-1 demo\n");

	//---- DMA global config
	MI_InitNDmaConfig();

	//---- interrupt setting
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)GX_VBlankIntr(TRUE);

	//----------------------------------------------------------------
	// demo1
	OS_Printf( "memory fill using NDMA\n" );

	memDump( &dest[0], 0x20 );

	OS_Printf("fill with 0x12345678\n");

	//---- DMA fill
	DC_InvalidateRange( &dest[0], 0x20 );
	MI_NDmaFill( 0, &dest[0], 0x12345678, 0x20 );

	memDump( &dest[0], 0x20 );

	//----------------------------------------------------------------
	// demo2
	OS_Printf( "\nmemory copy using NDMA\n" );

	{
		int n;
		for( n=0; n<0x20; n++ )
		{
			src[n] = (u8)(n*5);
		}
	}

	OS_Printf("src\n");
	memDump( &src[0], 0x20 );
	OS_Printf("dest\n");
	memDump( &dest[0], 0x20 );
	OS_Printf("copy\n");

	//---- DMA copy
	DC_FlushRange( &src[0], 0x20 );
	DC_InvalidateRange( &dest[0], 0x20 );
	MI_NDmaCopy( 0, &src[0], &dest[0], 0x20 );

	memDump( &dest[0], 0x20 );

	//----------------------------------------------------------------
	// demo3
	OS_Printf( "\nmemory copy using async NDMA\n" );

	MI_CpuFill8( &dest[0], 0, 0x100 );
	{
		int n;
		for( n=0; n<0x20; n++ )
		{
			src[n] = (u8)(n*2);
		}
	}

	OS_Printf("src\n");
	memDump( &src[0], 0x20 );
	OS_Printf("dest\n");
	memDump( &dest[0], 0x20 );
	OS_Printf("copy async\n");

	DC_FlushRange( &src[0], 0x20 );
	DC_InvalidateRange( &dest[0], 0x20 );
    MI_NDmaCopyAsync(0,&src[0], &dest[0], 0x20, MyNDMACallback, (void*)&flag );

	memDump( &dest[0], 0x20 );

	while(flag==0)
	{
		OS_Printf("waiting...\n");
	}

	//----------------------------------------------------------------
	OS_Printf("*** End of demo\n");
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         memDump

  Description:  dump memory

  Arguments:    addr : address
                size : size ( byte )

  Returns:      None
 *---------------------------------------------------------------------------*/
void memDump(void* addr, int size)
{
	int n;
	u8* p = (u8*)addr;

	while(1)
	{
		OS_Printf( "%08X : ", p );
		for( n=0; n<16; n++ )
		{
			OS_Printf(" %02X", *(u8*)p );
			p++;
			size--;
			if ( size == 0 )
			{
				break;
			}
		}
		OS_Printf("\n");
		if ( size == 0 )
		{
			break;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    //---- check interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  Name:         MyNDmaCallback

  Description:  NDMA callback

  Arguments:    arg :

  Returns:      None
 *---------------------------------------------------------------------------*/
void MyNDMACallback(void* arg)
{
	int* p = (int*)arg;
	*p = 1;
}


/*====== End of main.c ======*/

