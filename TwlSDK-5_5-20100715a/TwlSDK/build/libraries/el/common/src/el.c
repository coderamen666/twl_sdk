/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     el.c

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

#include "el_config.h"

#include <twl/el.h>


#include "elf.h"
#include "arch.h"
#include "elf_loader.h"
#include "loader_subset.h"


/*------------------------------------------------------
  External Variables
 -----------------------------------------------------*/
extern ELAlloc     i_elAlloc;
extern ELFree      i_elFree;


/*------------------------------------------------------
  External Functions
 -----------------------------------------------------*/
extern BOOL elRemoveObjEntry( ELObject** StartEnt, ELObject* ObjEnt);


/*------------------------------------------------------
  Global Variables
 -----------------------------------------------------*/
ELDesc* i_eldesc     = NULL;    // For link
ELDesc* i_eldesc_sim = NULL;    // For simulation link


/*---------------------------------------------------------------------------*
  Name:         EL_Init

  Description:  Initializes the dynamic link system.

  Arguments:    alloc: Heap-allocation function
                free: Heap-deallocation function

  Returns:      0 if successful; -1 if failed.
 *---------------------------------------------------------------------------*/
s32 EL_Init( ELAlloc alloc, ELFree free)
{
    if( i_eldesc != NULL) {
        return( -1);    //Initialized
    }
  
    ELi_Init( alloc, free);

    i_eldesc = ELi_Malloc( NULL, NULL, ((sizeof( ELDesc))*2)); //For link and simulation-link
    if( i_eldesc == NULL) {
        return( -1);
    }

    (void)ELi_InitDesc( i_eldesc);
    i_eldesc_sim = &(i_eldesc[1]);
    (void)ELi_InitDesc( i_eldesc_sim);

    return( 0);
}

#if 1
/*---------------------------------------------------------------------------*
  Name:         EL_CalcEnoughBufferSizeforLink*

  Description:  Returns a buffer size sufficient to test and relocate the dynamic link.
                Returns a slightly larger size when the dynamic object is referencing externally.
                

  Arguments:    FilePath: Path name of the dynamic object
                buf: Buffer address for the relocating destination (writing is not actually performed)
                readfunc: Read function prepared by the application
                len: Size of the dynamic object
                obj_image: Address on the memory where the dynamic object exists
                obj_len: Size of the dynamic object
                link_mode: Mode for testing dynamic links

  Returns:      Buffer size when successful; -1 if failed.
 *---------------------------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkFile( const char* FilePath, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;

    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibraryfromFile( i_eldesc_sim, FilePath, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //Veneer-included size because after ELi_ResolveAllLibrary

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}

/*------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLink( ELReadImage readfunc, u32 len, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;
  
    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibrary( i_eldesc_sim, readfunc, len, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //Veneer-included size because after ELi_ResolveAllLibrary

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}

/*------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkImage( void* obj_image, u32 obj_len, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;
  
    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibraryfromMem( i_eldesc_sim, obj_image, obj_len, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //Veneer-included size because after ELi_ResolveAllLibrary

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}
#endif


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
ELDlld EL_LinkFileEx( const char* FilePath, void* buf, u32 buf_size)
{
    return( EL_LoadLibraryfromFile( i_eldesc, FilePath, buf, buf_size));
}

/*------------------------------------------------------*/
ELDlld EL_LinkEx( ELReadImage readfunc, u32 len, void* buf, u32 buf_size)
{
    return( EL_LoadLibrary( i_eldesc, readfunc, len, buf, buf_size));
}

/*------------------------------------------------------*/
ELDlld EL_LinkImageEx( void* obj_image, u32 obj_len, void* buf, u32 buf_size)
{
    return( EL_LoadLibraryfromMem( i_eldesc, obj_image, obj_len, buf, buf_size));
}

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
ELDlld EL_LinkFile( const char* FilePath, void* buf)
{
    return( EL_LoadLibraryfromFile( i_eldesc, FilePath, buf, 0xFFFFFFFF));
}

/*------------------------------------------------------*/
ELDlld EL_Link( ELReadImage readfunc, u32 len, void* buf)
{
    return( EL_LoadLibrary( i_eldesc, readfunc, len, buf, 0xFFFFFFFF));
}

/*------------------------------------------------------*/
ELDlld EL_LinkImage( void* obj_image, u32 obj_len, void* buf)
{
    return( EL_LoadLibraryfromMem( i_eldesc, obj_image, obj_len, buf, 0xFFFFFFFF));
}


/*---------------------------------------------------------------------------*
  Name:         EL_ResolveAll

  Description:  Resolves unresolved symbols.

  Arguments:    None.

  Returns:      EL_RELOCATED when successful, EL_FAILED if failed.
 *---------------------------------------------------------------------------*/
u16 EL_ResolveAll( void)
{
    if( ELi_ResolveAllLibrary( i_eldesc) == EL_PROC_RELOCATED) {
        return( EL_RELOCATED);
    }
    return( EL_FAILED);
}


/*---------------------------------------------------------------------------*
  Name:         EL_Export

  Description:  Registers symbol information to the DLL system.

  Arguments:    None.

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL EL_Export( ELAdrEntry* AdrEnt)
{
    /* (TODO) Do not apply to simulation because not added to i_eldesc_sim */
    return ELi_Export( i_eldesc, AdrEnt);
}


/*---------------------------------------------------------------------------*
  Name:         EL_GetGlobalAdr

  Description:  Searches a symbol from the dynamic module and returns the address.

  Arguments:    my_dlld: Handle of dynamic module targeted for the search
                ent_name: Symbol name

  Returns:      Address of symbol when successful, 0 if failed.
 *---------------------------------------------------------------------------*/
void* EL_GetGlobalAdr( ELDlld my_dlld, const char* ent_name)
{
    return( ELi_GetGlobalAdr( i_eldesc, my_dlld, ent_name));
}


/*---------------------------------------------------------------------------*
  Name:         EL_Unlink

  Description:  Unlinks the dynamic module.

  Arguments:    my_dlld: Handle of dynamic module to unlink

  Returns:      EL_SUCCESS when successful, EL_FAILED if failed.
 *---------------------------------------------------------------------------*/
u16 EL_Unlink( ELDlld my_dlld)
{
    if( ELi_Unlink( i_eldesc, my_dlld) == TRUE) {
        /* ReLink not possible because corresponding object structure is destroyed */
        (void)elRemoveObjEntry( &(i_eldesc->ELObjectStart), (ELObject*)my_dlld); // No problem if it fails
        return( EL_SUCCESS);
    }
    return( EL_FAILED);
}


/*---------------------------------------------------------------------------*
  Name:         EL_GetResultCode

  Description:  Returns detailed results of final processes.

  Arguments:    None.

  Returns:      Error code of type ELResult.
 *---------------------------------------------------------------------------*/
ELResult EL_GetResultCode( void)
{
    if( i_eldesc == NULL) {
        return( EL_RESULT_FAILURE);
    }
    return( (ELResult)(i_eldesc->result));
}

#if 0
ELResult EL_GetResultCode( ELDlld my_dlld)
{
    ELObject*      MYObject;
  
    if( my_dlld == 0) {
        return( EL_RESULT_INVALID_PARAMETER);
    }
  
    // Object that references results
    MYObject = (ELObject*)my_dlld;

    return( MYObject->result);
}
#endif
