/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-10#$
  $Rev: 9620 $
  $Author: shirait $
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  Copies a veneer into a buffer
    start: Address to jump from
    data: Address to jump to
 -----------------------------------------------------*/
void* ELi_CopyVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  Copies a veneer into a buffer
    start: Caller
    data: Jump target
    threshold : If there is already a veneer within this range, reuse it
 -----------------------------------------------------*/
void* ELi_CopyV4tVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  Copies a segment into a buffer
 -----------------------------------------------------*/
void* ELi_CopySegmentToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  Copies a section into a buffer
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  Allocates a section in a buffer (without copying)
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  Gets the program header at the specified index and puts it in a buffer
 -----------------------------------------------------*/
void ELi_GetPhdr( ELDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  Gets the section header at the specified index and puts it in a buffer
 -----------------------------------------------------*/
void ELi_GetShdr( ELDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  Gets the section entry at the specified index and puts it in a buffer
 -----------------------------------------------------*/
void ELi_GetSent( ELDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  Gets the entry at the specified index of a given section header and puts it in a buffer
  (only for sections with a fixed entry size)
 -----------------------------------------------------*/
void ELi_GetEntry( ELDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  Gets the string at the specified index of the STR section header
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  Redefines a symbol
 -----------------------------------------------------*/
BOOL ELi_RelocateSym( ELDesc* elElfDesc, ELObject* MYObject, u32 relsh_index);

/*------------------------------------------------------
  Puts a global symbol into the address table
 -----------------------------------------------------*/
BOOL ELi_GoPublicGlobalSym( ELDesc* elElfDesc, ELObject* MYObject, u32 symtblsh_index);

/*------------------------------------------------------
  Releases the symbol lists created in ELi_RelocateSym and ELi_GoPublicGlobalSym
  (After calling either one, invoke this function)
 -----------------------------------------------------*/
void ELi_FreeSymList( ELDesc* elElfDesc);

/*------------------------------------------------------
  Resolves a symbol based on unresolved information
 -----------------------------------------------------*/
BOOL ELi_DoRelocate( ELDesc* elElfDesc, ELObject* MYObject, ELImportEntry* UnresolvedInfo);


/*------------------------------------------------------
  Extracts an ELSymEx object from the specified index in the list
 -----------------------------------------------------*/
//ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  Extracts an ELShdrEx object from the specified index in the list
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  Determines whether the section at the specified index is debugging information
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELDesc* elElfDesc, u32 index);


/*------------------------------------------------------
  Inspects the SymEx table in elElfDesc and determines if the code at the specified offset from the specified index is ARM or THUMB
  
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELDesc* elElfDesc, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 Initializes an import information entry
 --------------------------------------------------------*/
static void ELi_InitImport( ELImportEntry* ImportInfo);


/*------------------------------------------------------
  Extracts an entry from the import information table (Do not delete ImpEnt!)
 -----------------------------------------------------*/
BOOL ELi_ExtractImportEntry( ELImportEntry** StartEnt, ELImportEntry* ImpEnt);


/*---------------------------------------------------------
 Adds an entry to the import information table
 --------------------------------------------------------*/
void ELi_AddImportEntry( ELImportEntry** ELUnrEntStart, ELImportEntry* UnrEnt);


/*------------------------------------------------------
  Releases the entire import information table
 -----------------------------------------------------*/
void ELi_FreeImportTbl( ELImportEntry** ELImpEntStart);


/*------------------------------------------------------
  Releases the veneer table
 -----------------------------------------------------*/
void* ELi_FreeVenTbl( ELDesc* elElfDesc, ELObject* MYObject);



#endif /*_LOADER_SUBSET_H_*/
