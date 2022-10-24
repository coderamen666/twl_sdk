/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.c

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

#include <stdlib.h>
#include <string.h>
#include "loader_subset.h"


extern ELUnresolvedEntry*    ELUnrEntStart;

extern u16        dbg_print_flag;
extern u32        unresolved_table_block_flag;


//Use the following statements to determine whether the processor is ARM7.
//#ifdef  SDK_ARM7
//#endif    /*SDK_ARM7*/


/*------------------------------------------------------
  Align the addresses
 -----------------------------------------------------*/
#define ALIGN( addr, align_size ) (((addr)  & ~((align_size) - 1)) + (align_size))

u32 ELi_ALIGN( u32 addr, u32 align_size);
u32 ELi_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}


/*------------------------------------------------------
  Copy the content of the symbol string to the buffer.
 -----------------------------------------------------*/
void* ELi_CopySymStrToBuffer( ELHandle* ElfHandle, ELShdrEx* SymStrShdrEx)
{
    u32 load_start;

    /*Align*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);

    memcpy( (void*)load_start, SymStrShdrEx->str_table, SymStrShdrEx->str_table_size);

    /*Correct the size of the section header*/
    SymStrShdrEx->Shdr.sh_size = SymStrShdrEx->str_table_size;

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + SymStrShdrEx->str_table_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  Copy the content of the section string to the buffer.
 -----------------------------------------------------*/
void* ELi_CopyShStrToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32       load_start, i;
    u32       total_size = 0;
    ELShdrEx* CurrentShdrEx;

    /*Align*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
//    load_adr = load_start;

    /*Update locations where the section header references character entries*/
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {
        }else{
            CurrentShdrEx->Shdr.sh_name = total_size;
            strcpy( (void*)(load_start+total_size), CurrentShdrEx->str);
            total_size += (strlen( CurrentShdrEx->str) + 1);
        }
    }

    /*Correct the size of the section header*/
    Shdr->sh_size = total_size;

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + total_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  Copy the symbol entries together to the buffer
 -----------------------------------------------------*/
void* ELi_CopySymToBuffer( ELHandle* ElfHandle)
{
    u32      load_start, load_adr;
    u32      total_size = 0;
    ELSymEx* CurrentSymEx;

    /*Align*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    load_adr = load_start;

    CurrentSymEx = ElfHandle->SymEx;
    while( CurrentSymEx != NULL) {
        /*copy*/
        memcpy( (u8*)load_adr, &(CurrentSymEx->Sym),
                sizeof( Elf32_Sym));
        CurrentSymEx = CurrentSymEx->next;
        load_adr += sizeof( Elf32_Sym);
        total_size += sizeof( Elf32_Sym);
//                        printf( "symbol found\n");
    }
    
    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + total_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  Copy the section to the buffer
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr  sh_size;

    /*Align*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);

    /*Set size*/
    sh_size = Shdr->sh_size;

    /*copy*/
    ElfHandle->ELi_ReadStub( (void*)load_start,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset)+(u32)(Shdr->sh_offset),
                             sh_size);

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*Return the loaded header address*/
    return (void*)load_start;
}


/*------------------------------------------------------
  Allocate a section in the buffer (do not copy) and embed a 0 for that region.
  
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32           load_start;
    Elf32_Addr    sh_size;

    /*Align*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*Set size*/
    sh_size = Shdr->sh_size;

    /*Move the buffer pointer*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*Embed with 0 (because .bss section is expected)*/
    memset( (void*)load_start, 0, sh_size);
    
    /*Return the allocated header address  */
    return (void*)load_start;
}


/*------------------------------------------------------
  Get section header for the specified index to the buffer
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr)
{
    u32 offset;
    
    offset = (ElfHandle->CurrentEhdr.e_shoff) + ((u32)(ElfHandle->shentsize) * index);
    
    ElfHandle->ELi_ReadStub( Shdr,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  Get section entry for the specified index to the buffer
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size)
{
    Elf32_Shdr    Shdr;
    //u32            entry_adr;

    ELi_GetShdr( ElfHandle, index, &Shdr);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                             size);
}


/*------------------------------------------------------
  Get the index entry for the specified section header to the buffer.
    (Only section with a fixed entry size, such as Rel, Rela, Sym, and the like.)
    Shdr: Specified section header
    index: Entry number (from 0)
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf)
{
    u32 offset;

    offset = (u32)(Shdr->sh_offset) + ((Shdr->sh_entsize) * index);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             Shdr->sh_entsize);
}

/*------------------------------------------------------
  Get specified index character string of the STR section header

    Shdr: Specified section header
    index: Entry Index (from 0)
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*Header address of the string entry*/
    ELi_GetSent( ElfHandle, strsh_index, str, ent_index, len);
}

/*------------------------------------------------------
  Get the length of specified index string for the STR section header

    Shdr: Specified section header
    index: Entry index (from 0)
 -----------------------------------------------------*/
u32 ELi_GetStrLen( ELHandle* ElfHandle, u32 index, u32 offset)
{
    Elf32_Shdr Shdr;
    char buf[1];
    u32 count = 0;
    
    ELi_GetShdr( ElfHandle, index, &Shdr);
    
    ElfHandle->ELi_ReadStub( buf,
                         ElfHandle->FileStruct,
                         (u32)(ElfHandle->ar_head),
                         (u32)(ElfHandle->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                         1);
    while( buf[0] != 0 )
    {
        count++;
        ElfHandle->ELi_ReadStub( buf,
                         ElfHandle->FileStruct,
                         (u32)(ElfHandle->ar_head),
                         (u32)(ElfHandle->elf_offset) + (u32)(Shdr.sh_offset) + offset + count,
                         1);
    }
    return count;
}

/*------------------------------------------------------
  Create the symbol list

 -----------------------------------------------------*/
void ELi_BuildSymList( ELHandle* elElfDesc, u32 symsh_index, u32** sym_table)
{
    u32         i;
    u32         num_of_sym;        //Overall symbol count
    u16         debug_flag;
    Elf32_Sym   TestSym;
    ELSymEx*    CurrentSymEx;
    ELShdrEx*   CurrentShdrEx;
    Elf32_Shdr  SymShdr;
    ELSymEx     DmySymEx;
    u32         sym_num = 0;

    if( elElfDesc->SymExTarget == symsh_index) {
//        printf( "%s skip.\n", __FUNCTION__);
        return;                              //List already created
    }else{
        ELi_FreeSymList( elElfDesc, sym_table); /*Deallocate the symbol list*/
    }
    
//    printf( "%s build\n", __FUNCTION__);

    /*Get SYMTAB section header*/
    ELi_GetShdr( elElfDesc, symsh_index, &SymShdr);
    
    /*Build new/old symbol correspondence table*/
    *sym_table = (u32*)malloc( 4 * (SymShdr.sh_size / SymShdr.sh_entsize));

    num_of_sym = (SymShdr.sh_size) / (SymShdr.sh_entsize);    //Overall symbol count
    elElfDesc->SymExTbl = malloc( num_of_sym * 4);
    
    /*---------- Create ELSymEx List  ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        
        /*Copy symbol entry*/
        ELi_GetEntry( elElfDesc, &SymShdr, i, &TestSym);

        /*-- Set debug information flag --*/
        CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, TestSym.st_shndx);
        if( CurrentShdrEx) {
            debug_flag = CurrentShdrEx->debug_flag;
        }else{
            debug_flag = 0;
        }/*-------------------------------*/

        if( debug_flag == 1) {
            elElfDesc->SymExTbl[i] = NULL;
            (*sym_table)[i] = 0xFFFFFFFF;
        }else{
            CurrentSymEx->next = malloc( sizeof(ELSymEx));
            CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
            
            memcpy( &(CurrentSymEx->Sym), &TestSym, sizeof(TestSym));
            
            elElfDesc->SymExTbl[i] = CurrentSymEx;

            (*sym_table)[i] = sym_num;

            sym_num++;
            
//            printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
            
            /*-- Set the ELSymEx Thumb flag (Only the function symbol is required) -----*/
            if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
                CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( elElfDesc, CurrentSymEx->Sym.st_shndx,
                                                               CurrentSymEx->Sym.st_value));
            }else{
                CurrentSymEx->thumb_flag = 0;
            }/*--------------------------------------------------------*/
        }
    }
    
    CurrentSymEx->next = NULL;
    elElfDesc->SymEx = DmySymEx.next;
    /*-------------------------------------------*/


    elElfDesc->SymExTarget = symsh_index;
}


/*------------------------------------------------------
  Deallocate the symbol list

 -----------------------------------------------------*/
void ELi_FreeSymList( ELHandle* elElfDesc, u32** sym_table)
{
    ELSymEx*  CurrentSymEx;
    ELSymEx*  FwdSymEx;

    if( elElfDesc->SymExTbl != NULL) {
        free( elElfDesc->SymExTbl);
        elElfDesc->SymExTbl = NULL;
    }
    
    /*---------- Free the ELSymEx List  ----------*/
    CurrentSymEx = elElfDesc->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        elElfDesc->SymEx = NULL;
    }
    /*-----------------------------------------*/

/*    if( *sym_table) { // Deallocated by the application here and so commented out
        free( *sym_table);
    }*/


    elElfDesc->SymExTarget = 0xFFFFFFFF;
}


/*------------------------------------------------------
  Read ELSymEx of the index specified from the list.
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index)
{
    u32         i;
    ELSymEx*    SymEx;

    SymEx = SymExStart;
    for( i=0; i<index; i++) {
        SymEx = (ELSymEx*)(SymEx->next);
        if( SymEx == NULL) {
            break;
        }
    }
    return SymEx;
}

/*------------------------------------------------------
  Read ELShdrx of the index specified from the list.
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index)
{
    u32         i;
    ELShdrEx*   ShdrEx;

    ShdrEx = ShdrExStart;
    for( i=0; i<index; i++) {
        ShdrEx = (ELShdrEx*)(ShdrEx->next);
        if( ShdrEx == NULL) {
            break;
        }
    }
    return ShdrEx;
}



/*------------------------------------------------------
  Determine whether the section for the specified index is debug information

<Definition of debugging information>
- Sections whose section name begins with .debug.

- Section, such as .rel.debug - section, where the sh_info indicates the debug information section number
  
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index)
{
    Elf32_Shdr  TmpShdr;
    char        shstr[6];

    /*-- Get six characters of character string for section name --*/
    ELi_GetShdr( ElfHandle, index, &TmpShdr);
    ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( strncmp( shstr, ".debug", 6) == 0) {    /*When debugging section*/
        return TRUE;
    }else{                        /*When relocated section relates to the debugging section*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( ELi_ShdrIsDebug( ElfHandle, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  Checks the ElfHandle SymEx table, and determines whether the code in the specified offset of the specified index is ARM or THUMB.
  
  (Preset ElfHandle->SymShdr and ElfHandle->SymEx.)
    
    sh_index: Section index to be checked
    offset: Offset in section to check
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset)
{
    u32         i;
    u32         thumb_flag;
    Elf32_Shdr* SymShdr;
    char        str_adr[3];
    ELSymEx*    CurrentSymEx;

    /*Get symbol section header and SymEx list*/
    SymShdr = &(ElfHandle->SymShdr);
    CurrentSymEx = ElfHandle->SymEx;

    i = 0;
    thumb_flag = 0;
    while( CurrentSymEx != NULL) {
        
        if( CurrentSymEx->Sym.st_shndx == sh_index) {
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, str_adr, 3);
            if( strncmp( str_adr, "$a\0", strlen("$a\0")) == 0) {
                thumb_flag = 0;
            }else if( strncmp( str_adr, "$t\0", strlen("$t\0")) == 0) {
                thumb_flag = 1;
            }
            if( CurrentSymEx->Sym.st_value > offset) {
                break;
            }
        }
        
        CurrentSymEx = CurrentSymEx->next;
        i++;
    }

    return thumb_flag;
}


/*---------------------------------------------------------
 Initialize unresolved information entries
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo)
{
    UnresolvedInfo->sym_str = NULL;
    UnresolvedInfo->r_type = 0;
    UnresolvedInfo->S_ = 0;
    UnresolvedInfo->A_ = 0;
    UnresolvedInfo->P_ = 0;
    UnresolvedInfo->T_ = 0;
    UnresolvedInfo->remove_flag = 0;
}

/*------------------------------------------------------
  Delete entry from the unresolved information table
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*   CurrentUnrEnt;

    if( UnrEnt == NULL) {
        return FALSE;
    }
    
    DmyUnrEnt.next = ELUnrEntStart;
    CurrentUnrEnt = &DmyUnrEnt;

    while( CurrentUnrEnt->next != UnrEnt) {
        if( CurrentUnrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentUnrEnt = (ELUnresolvedEntry*)(CurrentUnrEnt->next);
        }
    }

    CurrentUnrEnt->next = UnrEnt->next;
    free( UnrEnt->sym_str);
    free( UnrEnt);
    ELUnrEntStart = DmyUnrEnt.next;
    
    return TRUE;
}

/*---------------------------------------------------------
 Add entry to the unresolved information table
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*   CurrentUnrEnt;

    if( !ELUnrEntStart) {
        ELUnrEntStart = UnrEnt;
    }else{
        DmyUnrEnt.next = ELUnrEntStart;
        CurrentUnrEnt = &DmyUnrEnt;

        while( CurrentUnrEnt->next != NULL) {
            CurrentUnrEnt = CurrentUnrEnt->next;
        }
        CurrentUnrEnt->next = (void*)UnrEnt;
    }
    UnrEnt->next = NULL;
}

/*------------------------------------------------------
  Search for entry corresponding to specified string in the unresolved information table
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name)
{
    ELUnresolvedEntry* CurrentUnrEnt;

    CurrentUnrEnt = ELUnrEntStart;
    if( CurrentUnrEnt == NULL) {
        return NULL;
    }
    while( 1) {
        if( (strcmp( CurrentUnrEnt->sym_str, ent_name) == 0)&&
            (CurrentUnrEnt->remove_flag == 0)) {
            break;
        }
        CurrentUnrEnt = (ELUnresolvedEntry*)CurrentUnrEnt->next;
        if( CurrentUnrEnt == NULL) {
            break;
        }
    }
    return CurrentUnrEnt;
}
