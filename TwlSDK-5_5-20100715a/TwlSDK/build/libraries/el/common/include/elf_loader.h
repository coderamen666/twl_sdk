/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     elf_loader.h

  Copyright 2006-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_




#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
//#include <stdio.h>
#include "elf.h"
#include <twl/el.h>



#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------
  Extended section header (support for load addresses and so on)
 -----------------------------------------------------*/
typedef struct {
  void*         next;
  u16           index;
  u16           debug_flag;    /*0: Not debugging information; 1: Debugging information*/
  u32           loaded_adr;
  u32           alloc_adr;
  u32           loaded_size;
  Elf32_Shdr    Shdr;
}ELShdrEx;


/*------------------------------------------------------
  Extended symbol entry (support for load addresses and so on)
 -----------------------------------------------------*/
typedef struct {
  void*      next;
  u16        debug_flag;       /*0: Not debugging information; 1: Debugging information*/
  u16        thumb_flag;
  u32        relocation_val;
  Elf32_Sym  Sym;
}ELSymEx;


/*------------------------------------------------------
  Linked list of veneers (used by ELi_DoRelocate)
 -----------------------------------------------------*/
typedef struct {
  void* next;
  u32   adr;     /* Starting address of veneer code */
  u32   data;    /* Jump destination address stored in a veneer's literal pool */
}ELVeneer;


/*------------------------------------------------------
  Table with relocation data to use for importing

  If the address table is accessed later, addresses will be resolved as follows.
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void*       next;             /*Next entry*/
  char*       sym_str;          /*Name of an unresolved external symbol reference*/
  u32         r_type;           /*Relocation type ( ELF32_R_TYPE(Rela.r_info) )*/
  u32         S_;               /*Address of an unresolved external symbol reference*/
  s32         A_;               /*Resolved*/
  u32         P_;               /*Resolved*/
  u32         T_;               /*ARM or Thumb flag for an unresolved external symbol reference*/
  u32         sh_type;          /*SHT_REL or SHT_RELA*/
  struct ELObject*   Dlld;      /*Object with the imported entry when it is resolved. NULL when it is unresolved*/
}ELImportEntry;



/* Process value for ELDesc */
typedef enum ELProcess {
  EL_PROC_NOTHING      = 0,
  EL_PROC_INITIALIZED = 0x5A,
  EL_PROC_COPIED      = 0xF0,
  EL_PROC_RELOCATED
} ELProcess;


/*------------------------------------------------------
  Individual ELF object management
 -----------------------------------------------------*/
typedef struct {
  void*          next;                   /* Next object entry */
  void*          lib_start;              /* Address relocated as a library */
  u32            lib_size;               /* Size after being made into a library */
  u32            buf_limit_addr;         /* The last address in the buffer given for relocation, incremented by 1 */
  void*          buf_current;            /* Buffer pointer */
  ELAdrEntry*    ExportAdrEnt;           /* Export information */
  ELAdrEntry*    HiddenAdrEnt;           /* Location to store export information when unlinking */
  ELImportEntry* ResolvedImportAdrEnt;   /* Resolved import information */
  ELImportEntry* UnresolvedImportAdrEnt; /* Unresolved import information */
  ELVeneer*      ELVenEntStart;          /* Linked list of veneers */
  ELVeneer*      ELV4tVenEntStart;       /* Linked list of v4t veneers */
  u32            stat;                   /* Status */
  u32            file_id;                /* File ID when a DLL was placed through a ROM */
  u32            process;                /* Cast the ELProcess type and save */
  u32            result;                 /* Cast the ELResult type and save */
}ELObject;


/*------------------------------------------------------
  Overall ELF object management
 -----------------------------------------------------*/
typedef BOOL (*ELi_ReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*         ar_head;        /* Starting address for the AR or ELF file */
  void*         elf_offset;     /* Offset to the start of the ELF object */

  u16           reserve;
  u16           shentsize;      /* Size of a section header */
  u32           process;        /* Cast the ELProcess type and save */
  u32           result;         /* Cast the ELResult type and save */

  ELShdrEx*     ShdrEx;         /* Start of the ShdrEx list */

  Elf32_Ehdr    CurrentEhdr;    /* ELF header */

  Elf32_Rel     Rel;            /* Relocation entry */
  Elf32_Rela    Rela;
  Elf32_Sym     Sym;            /* Symbol entry */
  Elf32_Shdr    SymShdr;

  ELSymEx*      SymEx;          /* Start of the SymEx list (connect only non-debugging symbols) */
  ELSymEx**     SymExTbl;       /* SymEx address table (for all symbols)*/
  u32           SymExTarget;    /* Number of the symbol section that created the SymEx list */

  ELi_ReadFunc  i_elReadStub;   /* Read stub function */
  void*         FileStruct;     /* File structure */
    
  u32           entry_adr;      /* Entry address */
  
  ELObject*     ELObjectStart;  /* Linked list of objects */
  ELObject*     ELStaticObj;    /* Pointer to the static structure connected to the linked list */
}ELDesc;


/*---------------------------------------------------------
 Find size of the ELF object
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf);

/*---------------------------------------------------------
 Find the size of the linked library
 --------------------------------------------------------*/
u32 EL_GetLibSize( ELDlld my_dlld);


/*------------------------------------------------------
  Initialize the dynamic link system
 -----------------------------------------------------*/
#if 0
//#ifndef SDK_TWL
void ELi_Init( void);
#else
void ELi_Init( ELAlloc alloc, ELFree free);
void* ELi_Malloc( ELDesc* elElfDesc, ELObject* MYObject, size_t size);
#endif

/*------------------------------------------------------
  Initialize the ELDesc structure
 -----------------------------------------------------*/
BOOL ELi_InitDesc( ELDesc* elElfDesc);

/*------------------------------------------------------
  Relocates an ELF object or its archive from a file into a buffer
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromFile( ELDesc* elElfDesc, const char* FilePath, void* buf, u32 buf_size);

/*------------------------------------------------------
  Relocates an ELF object or its archive through the user read API
 -----------------------------------------------------*/
ELDlld EL_LoadLibrary( ELDesc* elElfDesc, ELReadImage readfunc, u32 len, void* buf, u32 buf_size);

/*------------------------------------------------------
  Relocates an ELF object or its archive from memory into a buffer
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromMem( ELDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf, u32 buf_size);

/*------------------------------------------------------
  Uses the address table to resolve unresolved symbols
 -----------------------------------------------------*/
ELProcess ELi_ResolveAllLibrary( ELDesc* elElfDesc);


/*------------------------------------------------------
  Add entry from application to address table
 -----------------------------------------------------*/
BOOL ELi_Export( ELDesc* elElfDesc, ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  Add entry of static side to address table
  (This is defined as a weak symbol in the ELF library, and will be overwritten by the definition in files created by makelst)
    
 -----------------------------------------------------*/
void EL_AddStaticSym( void);


/*------------------------------------------------------
  Return address corresponding to specified string in address table
 -----------------------------------------------------*/
void* ELi_GetGlobalAdr( ELDesc* elElfDesc, ELDlld my_dlld, const char* ent_name);


/*------------------------------------------------------
  Unlink the object
 -----------------------------------------------------*/
BOOL ELi_Unlink( ELDesc* elElfDesc, ELDlld my_dlld);


/*------------------------------------------------------
  Check whether there are unresolved external references remaining in the object
 -----------------------------------------------------*/
BOOL EL_IsResolved( ELDlld my_dlld);


/*------------------------------------------------------
  Set of error codes/process codes
 -----------------------------------------------------*/
void ELi_SetResultCode( ELDesc* elElfDesc, ELObject* MYObject, ELResult result);
void ELi_SetProcCode( ELDesc* elElfDesc, ELObject* MYObject, ELProcess process);


/*------------------------------------------------------
  Allocates the heap
 -----------------------------------------------------*/
void* ELi_Malloc( ELDesc* elElfDesc, ELObject* MYObject, size_t size);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif    /*_ELF_LOADER_H_*/
