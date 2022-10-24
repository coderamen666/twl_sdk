/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - fnd - archive2
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "sdk_init.h"
#include "nns_util.h"

// Index definition file for archive
#include "picture.naix"


static void	LoadPicture(void);

/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample program for reading background data from a compressed archive and displaying a screen.
				

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    InitSystem();
	InitMemory();
    InitVRAM();

    InitDisplay();
	LoadPicture();
	G2_SetBG0Offset(0,20);

    for(;;)
    {
		SVC_WaitVBlankIntr();
		ReadGamePad();
	}
}

/*---------------------------------------------------------------------------*
  Name:         LoadPicture

  Description:  Loads one compressed archive and transfers the character, color palette, and screen stored therein to the VRAM.
  				
  				

  				Removes the archive after transferring the data to VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
LoadPicture(void)
{
	NNSFndArchive* archive;

	// Expands the compressed archive to main memory and mounts it in the file system.
	// The call to DC_FlushRange() is made from within LoadCompressedArchive().
	// LoadCompressedArchive() is in nns_util.c.
	if ((archive = LoadCompressedArchive("PCT", "/data/picture_LZ.bin")) != NULL)
	{
		// Specifies the file name and gets data.
		void* chrData = NNS_FndGetArchiveFileByName("PCT:/picture/flower/flower256.chr");
		void* scnData = NNS_FndGetArchiveFileByName("PCT:/picture/flower/flower256.scn");

		// Specifies the index and gets data.
		void* palData = NNS_FndGetArchiveFileByIndex(archive, NARC_picture_flower256_pal);

		// Load data into VRAM
		GX_LoadBG0Char(chrData, 0, 0x10000);
    	GX_LoadBG0Scr (scnData, 0, 0x00800);
		GX_LoadBGPltt (palData, 0, 0x00200);

		// The data in main memory is not needed after being loaded into VRAM, so the archive is deleted.
		// 
		// RemoveArchive() is in nns_util.c
		RemoveArchive(archive);
	}
}
