/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     el.h

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
#ifndef TWL_COMMON_EL_H_
#define TWL_COMMON_EL_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------
  Typedef
 -----------------------------------------------------*/
typedef u32   ELDlld;
typedef void *(*ELAlloc)(size_t size);
typedef void  (*ELFree)(void* ptr);
typedef u32   (*ELReadImage)( u32 offset, void* buf, u32 size);


/*------------------------------------------------------
  Constants specified with the EL_CalcEnoughBufferSizeforLink* functions
 -----------------------------------------------------*/
typedef enum ELLinkMode {
  EL_LINKMODE_ROUGH = 0,
  EL_LINKMODE_ONESHOT,
  EL_LINKMODE_RELATION
} ELLinkMode;

/*------------------------------------------------------
  Error code
 -----------------------------------------------------*/
#define EL_SUCCESS   0
#define EL_FAILED    1
#define EL_RELOCATED 0xF1    //For EL_ResolveAll only

/*------------------------------------------------------
  Detailed error codes obtained with EL_GetResultCode
 -----------------------------------------------------*/
typedef enum ELResult {
  EL_RESULT_SUCCESS = 0,
  EL_RESULT_FAILURE = 1,
  EL_RESULT_INVALID_PARAMETER,
  EL_RESULT_INVALID_ELF,
  EL_RESULT_UNSUPPORTED_ELF,
  EL_RESULT_CANNOT_ACCESS_ELF, //Error when opening or reading an ELF file
  EL_RESULT_NO_MORE_RESOURCE   //malloc failed
} ELResult;


/*------------------------------------------------------
  Address table for exporting
 -----------------------------------------------------*/
typedef struct {
  void*      next;              /*Next address entry*/
  char*      name;              /*String*/
  void*      adr;               /*Address*/
  u16        func_flag;         /*0: Data. 1: Function.*/
  u16        thumb_flag;        /*0: ARM code. 1: Thumb code.*/
}ELAdrEntry;


/*---------------------------------------------------------------------------*
  Name:         EL_Init

  Description:  Initializes the dynamic link system.

  Arguments:    alloc: Heap-allocation function
                free: Heap deallocation function

  Returns:      0 if successful; -1 if failed.
 *---------------------------------------------------------------------------*/
s32 EL_Init( ELAlloc alloc, ELFree free);


/*---------------------------------------------------------------------------*
  Name:         EL_CalcEnoughBufferSizeforLink*

  Description:  Returns a buffer size sufficient to test and relocate the dynamic link.
                Return a slightly larger size when the dynamic object is referencing externally.
                

  Arguments:    FilePath: Path name of the dynamic object
                buf: Buffer address for the relocating destination (writing is not actually performed)
                readfunc: Read function prepared by the application
                len: Size of the dynamic object
                obj_image: Address on the memory where the dynamic object exists
                obj_len: Size of the dynamic object
                link_mode: Mode for testing dynamic links
                           (This constant is used to make a trade-off between processing time and accuracy)

  Returns:      Buffer size when successful; -1 if failed.
 *---------------------------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkFile( const char* FilePath, const void* buf, ELLinkMode link_mode);
s32 EL_CalcEnoughBufferSizeforLink( ELReadImage readfunc, u32 len, const void* buf, ELLinkMode link_mode);
s32 EL_CalcEnoughBufferSizeforLinkImage( void* obj_image, u32 obj_len, const void* buf, ELLinkMode link_mode);


/*---------------------------------------------------------------------------*
  Name:         EL_Link*Ex

  Description:  Relocates a dynamic object (from a file, user function, or memory) while checking whether it fits in the given buffer.
                

  Arguments:    FilePath: Path name of the dynamic object
                readfunc: Read function prepared by the application
                len: Size of the dynamic object
                obj_image: Address on the memory where the dynamic object exists
                obj_len: Size of the dynamic object
                buf: Buffer address for the relocation destination
                buf_size: Size of the buffer to relocate to

  Returns:      Dynamic module handle when successful, 0 if failed.
 *---------------------------------------------------------------------------*/
ELDlld EL_LinkFileEx( const char* FilePath, void* buf, u32 buf_size);
ELDlld EL_LinkEx( ELReadImage readfunc, u32 len, void* buf, u32 buf_size);
ELDlld EL_LinkImageEx( void* obj_image, u32 obj_len, void* buf, u32 buf_size);

/*---------------------------------------------------------------------------*
  Name:         EL_Link*

  Description:  Relocates dynamic objects (files/user functions/from memory).

  Arguments:    FilePath: Path name of the dynamic object
                readfunc: Read function prepared by the application
                len: Size of the dynamic object
                obj_image: Address on the memory where the dynamic object exists
                obj_len: Size of the dynamic object
                buf: Buffer address for the relocation destination

  Returns:      Dynamic module handle when successful, 0 if failed.
 *---------------------------------------------------------------------------*/
ELDlld EL_LinkFile( const char* FilePath, void* buf);
ELDlld EL_Link( ELReadImage readfunc, u32 len, void* buf);
ELDlld EL_LinkImage( void* obj_image, u32 obj_len, void* buf);


/*---------------------------------------------------------------------------*
  Name:         EL_ResolveAll

  Description:  Resolves unresolved symbols.

  Arguments:    None.

  Returns:      EL_RELOCATED when successful, EL_FAILED if failed.
 *---------------------------------------------------------------------------*/
u16 EL_ResolveAll( void);


/*---------------------------------------------------------------------------*
  Name:         EL_Export

  Description:  Registers symbol information to the DLL system.

  Arguments:    None.

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL EL_Export( ELAdrEntry* AdrEnt);


/*---------------------------------------------------------------------------*
  Name:         EL_AddStaticSym

  Description:  Registers dynamic module symbol information with the DLL system.
                (This is defined as a weak symbol in the ELF library, and will be overwritten by the definition in files created by makelst.)
                  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void EL_AddStaticSym( void);


/*---------------------------------------------------------------------------*
  Name:         EL_GetGlobalAdr

  Description:  Searches a symbol from the dynamic module and returns the address.

  Arguments:    my_dlld: Handle of dynamic module targeted for the search
                ent_name: Symbol name

  Returns:      Address of symbol when successful, 0 if failed.
 *---------------------------------------------------------------------------*/
void* EL_GetGlobalAdr( ELDlld my_dlld, const char* ent_name);


/*---------------------------------------------------------------------------*
  Name:         EL_Unlink

  Description:  Unlinks the dynamic module.

  Arguments:    my_dlld: Handle of dynamic module to unlink

  Returns:      EL_SUCCESS when successful, EL_FAILED if failed.
 *---------------------------------------------------------------------------*/
u16 EL_Unlink( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_IsResolved

  Description:  Determines whether all of a dynamic module's external references have been resolved.

  Arguments:    my_dlld: Dynamic module handle to check

  Returns:      TRUE if there are still unresolved external references.
 *---------------------------------------------------------------------------*/
BOOL EL_IsResolved( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_GetLibSize

  Description:  Returns the size of a linked dynamic module.
                The current size of the dynamic module will be returned, even if it still has unresolved external references.
                You can get the final size after relocation is complete by confirming in advance that EL_ResolveAll returns EL_RELOCATED.
                
                

  Arguments:    my_dlld: Dynamic module handle that will have its size checked

  Returns:      Size if successful; 0 if failed.
 *---------------------------------------------------------------------------*/
u32 EL_GetLibSize( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_GetResultCode

  Description:  Returns detailed results of final processes.

  Arguments:    None.

  Returns:      Error code of type ELResult.
 *---------------------------------------------------------------------------*/
ELResult EL_GetResultCode( void);


#ifdef __cplusplus
}    /* extern "C" */
#endif

#endif    /*TWL_COMMON_EL_H_*/
