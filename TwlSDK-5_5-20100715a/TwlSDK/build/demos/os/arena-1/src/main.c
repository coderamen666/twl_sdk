/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - simple-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-01#$
  $Rev: 6953 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif


void writeTest( void* address );

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         TwlMain / NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    void   *addrLo;
    void   *addrHi;

	OS_EnableMainExArena();
    OS_Init();

    OS_Printf("==== sample start\n");

	//---- Display each arena info
	OS_DumpAllArenaInfo();

    //---- Try allocating from the arena
    OS_Printf("---- allocate 0x100 byte from arena hi and lo.\n");
    //---- Main memory arena
    addrLo = OS_GetArenaLo(OS_ARENA_MAIN);
    addrHi = OS_GetArenaHi(OS_ARENA_MAIN);
    OS_Printf("---- MainRAM: Lo:%x Hi:%x\n", addrLo, addrHi);
    addrLo = OS_AllocFromArenaLo(OS_ARENA_MAIN, 0x100, 8);
    addrHi = OS_AllocFromArenaHi(OS_ARENA_MAIN, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_MAIN, TRUE );
	writeTest( addrLo );

    //---- Extended main memory arena
    addrLo = OS_GetArenaLo(OS_ARENA_MAINEX);
    addrHi = OS_GetArenaHi(OS_ARENA_MAINEX);
	OS_Printf("---- Extended MainRAM: Lo:%x Hi:%x\n", addrLo, addrHi);
	addrLo = OS_AllocFromArenaLo(OS_ARENA_MAINEX, 0x100, 8);
	addrHi = OS_AllocFromArenaHi(OS_ARENA_MAINEX, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_MAINEX, TRUE );
	writeTest( addrLo );

    //---- ITCM arena
    addrLo = OS_GetArenaLo(OS_ARENA_ITCM);
    addrHi = OS_GetArenaHi(OS_ARENA_ITCM);
    OS_Printf("---- ITCM: Lo:%x Hi:%x\n", addrLo, addrHi);
    addrLo = OS_AllocFromArenaLo(OS_ARENA_ITCM, 0x100, 8);
    addrHi = OS_AllocFromArenaHi(OS_ARENA_ITCM, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_ITCM, TRUE );
	writeTest( addrLo );

    //---- DTCM arena
    addrLo = OS_GetArenaLo(OS_ARENA_DTCM);
    addrHi = OS_GetArenaHi(OS_ARENA_DTCM);
	OS_Printf("---- DTCM: Lo:%x Hi:%x\n", addrLo, addrHi);
	addrLo = OS_AllocFromArenaLo(OS_ARENA_DTCM, 0x100, 8);
	addrHi = OS_AllocFromArenaHi(OS_ARENA_DTCM, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_DTCM, TRUE );
	writeTest( addrLo );

    //---- Shared region arena
    addrLo = OS_GetArenaLo(OS_ARENA_SHARED);
    addrHi = OS_GetArenaHi(OS_ARENA_SHARED);
    OS_Printf("---- Shared Memory: Lo:%x Hi:%x\n", addrLo, addrHi);
    addrLo = OS_AllocFromArenaLo(OS_ARENA_SHARED, 0x100, 8);
    addrHi = OS_AllocFromArenaHi(OS_ARENA_SHARED, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_SHARED, TRUE );
	writeTest( addrLo );

    //---- WRAM arena
    addrLo = OS_GetArenaLo(OS_ARENA_WRAM_MAIN);
    addrHi = OS_GetArenaHi(OS_ARENA_WRAM_MAIN);
    OS_Printf("---- WorkRAM: Lo:%x Hi:%x\n", addrLo, addrHi);
    addrLo = OS_AllocFromArenaLo(OS_ARENA_WRAM_MAIN, 0x100, 8);
    addrHi = OS_AllocFromArenaHi(OS_ARENA_WRAM_MAIN, 0x100, 8);
	OS_DumpArenaInfo( OS_ARENA_WRAM_MAIN, TRUE );
	writeTest( addrLo );

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         writeTest

  Description:  Try to write.

  Arguments:    address: Address to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void writeTest( void* address )
{
	if ( address )
	{
		*(u32*)address = 1;
	}
}

/*====== End of main.c ======*/
