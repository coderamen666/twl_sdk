/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "types.h"
#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  Copies the contents of a symbol string to a buffer
 -----------------------------------------------------*/
void* ELi_CopySymStrToBuffer( ELHandle* ElfHandle, ELShdrEx* SymStrShdrEx);


/*------------------------------------------------------
  Copies the contents of a section string to a buffer
 -----------------------------------------------------*/
void* ELi_CopyShStrToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  Copies symbol entries together into a buffer
 -----------------------------------------------------*/
void* ELi_CopySymToBuffer( ELHandle* ElfHandle);


/*------------------------------------------------------
  Copies a section into a buffer
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  Allocates a section in a buffer (without copying)
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  Gets the section header at the specified index and puts it in a buffer
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  Gets the section entry at the specified index and puts it in a buffer
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  Gets the entry at the specified index of a given section header and puts it in a buffer
	(only for sections with a fixed entry size)
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  Gets the string at the specified index of the STR section header
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len);

/*------------------------------------------------------
  Gets the length of the string at the specified index of the STR section header
 -----------------------------------------------------*/
u32 ELi_GetStrLen( ELHandle* ElfHandle, u32 index, u32 offset);


/*------------------------------------------------------
  Creates a symbol list

 -----------------------------------------------------*/
void ELi_BuildSymList( ELHandle* elElfDesc, u32 symsh_index, u32** sym_table);


/*------------------------------------------------------
  Releases a symbol list

 -----------------------------------------------------*/
void ELi_FreeSymList( ELHandle* elElfDesc, u32** sym_table);


/*------------------------------------------------------
  Redefines a symbol
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index);


/*------------------------------------------------------
    Function unique to makelst
    Registers global items from the symbol section to the address table
    
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index);


/*------------------------------------------------------
  Resolves a symbol based on unresolved information
 -----------------------------------------------------*/
u32	ELi_DoRelocate( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  Extracts an ELSymEx object from the specified index in the list
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  Extracts an ELShdrEx object from the specified index in the list
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  Determines whether the section at the specified index is debugging information
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index);


/*------------------------------------------------------
  Inspects the SymEx table in ElfHandle and determines if the code at the specified offset from the specified index is ARM or Thumb
  
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 Initializes an unresolved data entry
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  Removes an entry from the unresolved data table
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*---------------------------------------------------------
 Adds an entry to the unresolved data table
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*------------------------------------------------------
  Finds the entry corresponding to the specified string from the unresolved data table
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name);


#endif	/*_LOADER_SUBSET_H_*/
