/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     elf_loader.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

#include "types.h"
#include "elf.h"
#include "stdio.h"


/*------------------------------------------------------
  Extended section header (support for load addresses and so on)
 -----------------------------------------------------*/
typedef struct {
  void*			next;
  u16			index;
  u16			debug_flag;		/*0: Not debugging information. 1: Debugging information*/
  u32			loaded_adr;
  u32			alloc_adr;
  u32			loaded_size;
  Elf32_Shdr	Shdr;
}ELShdrEx;


/*------------------------------------------------------
  Extended symbol entry (support for load addresses and so on)
 -----------------------------------------------------*/
typedef struct {
  void*		next;
  u16		debug_flag;			/*0: Not debugging information. 1: Debugging information*/
  u16		thumb_flag;
  u32		relocation_val;
  Elf32_Sym	Sym;
}ELSymEx;



/*------------------------------------------------------
  ELF object management
 -----------------------------------------------------*/
typedef void (*ELi_ReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*			ar_head;		/* Starting address for the AR or ELF file */
  void*			elf_offset;		/* Offset to the start of the ELF object */
  void*			buf_current;	/* For Loader operations */
  u16			shentsize;		/* Size of a section header */
  u16			process;		/* Relocatable state */

  ELShdrEx*		ShdrEx;			/* Start of the ShdrEx list */

  Elf32_Ehdr	CurrentEhdr;	/* ELF header */

  Elf32_Rel		Rel;			/* Relocation entry */
  Elf32_Rela	Rela;
  Elf32_Sym		Sym;			/* Symbol entry */
  Elf32_Shdr	SymShdr;

  ELSymEx*		SymEx;			/* Beginning of the SymEx list */

  ELi_ReadFunc	ELi_ReadStub;	/* Read stub function */
  void*			FileStruct;		/* File structure */

  u32			mem_adr;		/*This will contain the sh_addr of the section that was loaded first (parameters for DS ROM headers)*/
}ELHandle;



/*------------------------------------------------------
  Address table
 -----------------------------------------------------*/
typedef struct {
  void*		next;				/*Next address entry*/
  char*		name;				/*String*/
  void*		adr;				/*Address*/
  u16		func_flag;			/*0: Data. 1: Function.*/
  u16		thumb_flag;			/*0: ARM code. 1: Thumb code.*/
}ELAdrEntry;


/*------------------------------------------------------
  Unresolved relocation data table

  If the address table is accessed later, addresses will be resolved as follows.
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void* next;					/*Next entry*/
  char*	sym_str;				/*Name of an unresolved external symbol reference*/
  u32	r_type;					/*Relocation type ( ELF32_R_TYPE(Rela.r_info) )*/
  u32	S_;						/*Address of an unresolved external symbol reference*/
  s32	A_;						/*Resolved*/
  u32	P_;						/*Resolved*/
  u32	T_;						/*ARM or Thumb flag for an unresolved external symbol reference*/
  u32	sh_type;				/*SHT_REL or SHT_RELA*/
  u32	remove_flag;			/*Flag to set on resolution (identifies what may be deleted)*/
  ELAdrEntry*	AdrEnt;			/*Location of the entry found in the address table*/
}ELUnresolvedEntry;



/* Process value for ELHandle */
#define EL_FAILED			0x00
#define EL_INITIALIZED		0x5A
#define EL_COPIED			0xF0
#define EL_RELOCATED		0xF1



/*---------------------------------------------------------
 Finds the size of an ELF object
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf);

/*------------------------------------------------------
  Initializes the dynamic link system
 -----------------------------------------------------*/
void EL_Init( void);

/*------------------------------------------------------
  Initializes an ELHandle structure.
 -----------------------------------------------------*/
BOOL EL_InitHandle( ELHandle* ElfHandle);

/*------------------------------------------------------
  Relocates an ELF object or its archive from a file into a buffer
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf);

/*------------------------------------------------------
  Relocates an ELF object or its archive from memory into a buffer
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromMem( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf);

/*------------------------------------------------------
  Uses the address table to resolve unresolved symbols
 -----------------------------------------------------*/
u16 EL_ResolveAllLibrary( void);


/*------------------------------------------------------
  Writes out marked symbols as structures to a public file
 -----------------------------------------------------*/
void EL_ExtractStaticSym1( void);
/*------------------------------------------------------
  Writes out marked symbols as functions to a public file
 -----------------------------------------------------*/
void EL_ExtractStaticSym2( void);


/*------------------------------------------------------
  Removes an entry from the address table
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  Adds an entry to the address table
 -----------------------------------------------------*/
void EL_AddAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  Finds the entry corresponding to the specified string from the address table
 -----------------------------------------------------*/
ELAdrEntry* EL_GetAdrEntry( char* ent_name);

/*------------------------------------------------------
  Returns the address corresponding to the specified string from the address table
 -----------------------------------------------------*/
void* EL_GetGlobalAdr( char* ent_name);





/*Other functions that are likely to be required*/
//Function that calculates the number of bytes in memory required for loading
//EL_FreeLibrary

#endif	/*_ELF_LOADER_H_*/
