/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     elf_loader.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-01-08#$
  $Rev: 11276 $
  $Author: shirait $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"

//OSHeapHandle        EL_Heap;
ELAdrEntry*           ELAdrEntStart = NULL;
ELUnresolvedEntry*    ELUnrEntStart = NULL;

extern u16        dbg_print_flag;
extern u16        unresolved_table_block_flag;

#define MAKELST_DS_API        "    (void)EL_Export"    //API function name to add to the address table on DS

extern char     c_source_line_str[256];
extern FILE*    CSourceFilep;

extern void file_write( char* c_str, FILE* Fp);


/*------------------------------------------------------
  Local Function Declarations
 -----------------------------------------------------*/
// Relocate ELF object or that archive in a buffer
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf);
// Core function that relocates ELF object to buffer
u16 ELi_LoadObject( ELHandle* ElfHandle, void* obj_offset, void* buf);
// Stub functions that read data from the ELF object
void ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
void ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);




/*---------------------------------------------------------
 Find size of the ELF object
    
    buf: Address of ELF image
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf)
{
    Elf32_Ehdr    Ehdr;
    u32           size;
    
    if( ELF_LoadELFHeader( buf, &Ehdr) == NULL) {
        return 0;
    }
    size = (u32)(Ehdr.e_shoff + (Ehdr.e_shentsize * Ehdr.e_shnum));
    return size;
}


/*------------------------------------------------------
  Initializes the dynamic link system.
 -----------------------------------------------------*/
void EL_Init( void)
{
//    void* heap_start;
    printf( "\n");
    /*--- Memory allocation related settings ---*/
/*    OS_InitArena();
    heap_start = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo( heap_start );
    EL_Heap = OS_CreateHeap( OS_ARENA_MAIN, heap_start, (void*)((u32)(OS_GetMainArenaHi())+1));
    OS_SetCurrentHeap( OS_ARENA_MAIN, EL_Heap);*/
    /*--------------------------------------*/
}

/*------------------------------------------------------
  Initialize the ELHandle structure
 -----------------------------------------------------*/
BOOL EL_InitHandle( ELHandle* ElfHandle)
{
    if( ElfHandle == NULL) {    /*NULL check*/
        return FALSE;
    }

    /*Set default values*/
    ElfHandle->ShdrEx = NULL;
    ElfHandle->SymEx = NULL;

    ElfHandle->process = EL_INITIALIZED;    /*Set the flag*/

    return TRUE;
}

/*------------------------------------------------------
  Relocate ELF object or that archive in a buffer
    
    ElfHandle: Header structure
    ObjFile: Object file or archive file structure
    buf: Buffer for loading
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf)
{
    u16 result;
    u32 len;

    /*Set read function*/
    ElfHandle->ELi_ReadStub = ELi_ReadFile;
    ElfHandle->FileStruct = ObjFile;

    fseek( ObjFile, 0, SEEK_END);
    len = ftell( ObjFile);
    fseek( ObjFile, 0, SEEK_SET);
    result = ELi_LoadLibrary( ElfHandle, NULL, len, buf);

    return result;
}

/*------------------------------------------------------
  Relocate ELF object or that archive in a buffer
    
    ElfHandle: Header structure
    obj_image: RAM image address for OBJ file or archive file
    buf: Buffer for loading
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromMem( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf)
{
    u16 result;
    
    /*Set read function*/
    ElfHandle->ELi_ReadStub = ELi_ReadMem;
    ElfHandle->FileStruct = NULL;
    
    result = ELi_LoadLibrary( ElfHandle, obj_image, obj_len, buf);

    return result;
}

/*------------------------------------------------------
  Relocate ELF object or that archive in a buffer
    
    ElfHandle: Header structure
    obj_image: RAM image address for OBJ file or archive file
    buf: Buffer for loading
 -----------------------------------------------------*/
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf)
{
    u16     result, all_result;
    u32     image_pointer;
    u32     arch_size;
    u32     elf_num = 0;                /*Number of ELF objects*/
    u32     obj_size;
    ArchHdr ArHdr;
    char    OBJMAG[8];
    char    ELFMAG[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    all_result = EL_RELOCATED;
    ElfHandle->ar_head = obj_image;
    image_pointer = 0;

    ElfHandle->ELi_ReadStub( OBJMAG, ElfHandle->FileStruct, (u32)obj_image, 0, 8);    /*Get the OBJ string*/
    /*--------------- For archive files  ---------------*/
    if( strncmp( OBJMAG, ARMAG, 8) == 0) {
        arch_size = sizeof( ArchHdr);
        image_pointer += 8;                /*To first entry*/
        
        ElfHandle->buf_current = buf;
        while( image_pointer < obj_len) {
            ElfHandle->ELi_ReadStub( OBJMAG, ElfHandle->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4);    /*Get the OBJ string*/
            if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
                elf_num++;
                result = ELi_LoadObject( ElfHandle, (void*)(image_pointer+arch_size), ElfHandle->buf_current);
                if( result < all_result) {        /*Apply to all_result only when there are bad results*/
                    all_result = result;
                }
                /*Set default values*/
                ElfHandle->ShdrEx = NULL;
                ElfHandle->SymEx = NULL;
                ElfHandle->process = EL_INITIALIZED;    /*Set the flag*/
            }else{
            }
            /*To next entry*/
            ElfHandle->ELi_ReadStub( &ArHdr, ElfHandle->FileStruct, (u32)(obj_image), image_pointer, arch_size);
            obj_size = AR_GetEntrySize( &ArHdr);
            if( obj_size % 2)  //Padded by '\n' when the object size is an odd number.
            {
                obj_size++;
            }
            image_pointer += arch_size + obj_size;
        }
    }else{/*--------------- For ELF files  ---------------*/
        if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
            elf_num++;
            all_result = ELi_LoadObject( ElfHandle, 0, buf);
        }
    }
    /*-------------------------------------------------------*/

    if( elf_num) {
        return all_result;
    }else{
        return EL_FAILED;
    }
}

/*------------------------------------------------------
  Relocate ELF object to buffer
    
    ElfHandle: Header structure
    obj_offset: Offset from RAM image address of object file
    buf: Buffer for loading
 -----------------------------------------------------*/
u16 ELi_LoadObject( ELHandle* ElfHandle, void* obj_offset, void* buf)
{
    u16         i, j;
    u32         num_of_entry;
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx    DmyShdrEx;
    char        sym_str[128];    //For debug print
    u32         offset;            //For debug print
    
    /* Check initialization of ELHandle */
    if( ElfHandle->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* Buffer NULL check */
    if( buf == NULL) {
        return EL_FAILED;
    }
    /* Get ELF header */
    ElfHandle->ELi_ReadStub( &(ElfHandle->CurrentEhdr), ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* Section handle build */
    ElfHandle->elf_offset = obj_offset;
    ElfHandle->buf_current = buf;
    ElfHandle->shentsize = ElfHandle->CurrentEhdr.e_shentsize;

    /*---------- Create ELShdrEx List  ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        memset( CurrentShdrEx, 0, sizeof(ELShdrEx));    //Clear zero
        
        /*Set flag to distinguish whether this is debugging information*/
        if( ELi_ShdrIsDebug( ElfHandle, i) == TRUE) {    /*When it is debugging information*/
            CurrentShdrEx->debug_flag = 1;
        }else{                                            /*When not debugging information*/
            /*Copy section header*/
            ELi_GetShdr( ElfHandle, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
        }
    }
    CurrentShdrEx->next = NULL;
    ElfHandle->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------*/

    /*---------- Check and copy all sections  ----------*/
/*    printf( "\nLoad to RAM:\n");
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {                //When it is debugging information
            printf( "skip debug-section %02x\n", i);
        }else{                                                //When not debugging information
            // .text section
            if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            // .data, .data1 section (Initialized data)
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            // .bss section
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //Do not copy
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_AllocSectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            // .rodata, .rodata1 section
            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }

            printf( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);
        }
    }
    // After copy ends
    ElfHandle->process = EL_COPIED;*/
    /*----------------------------------------------------*/

    /*---------- Relocate  ----------*/
    if( unresolved_table_block_flag == 0) {
        if( dbg_print_flag == 1) {
            printf( "\nRelocate Symbols:\n");
        }
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                /*When it is debugging information*/
        }else{                                                /*When not debugging information*/

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of REL = %x\n", (int)(num_of_entry));
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", (int)(CurrentShdrEx->Shdr.sh_link));
                    printf( "info   : %x\n", (int)(CurrentShdrEx->Shdr.sh_info));
                    printf( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                }
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( ElfHandle, i, &(ElfHandle->Rel), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->SymShdr));
                    ELi_GetSent( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->Sym),
                                 (u32)(ElfHandle->SymShdr.sh_entsize * ELF32_R_SYM( ElfHandle->Rel.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( ElfHandle, ElfHandle->SymShdr.sh_link, ElfHandle->Sym.st_name, sym_str, 128);

                    if( dbg_print_flag == 1) {
                        printf( "%08x  ", (int)(ElfHandle->Rel.r_offset));
                        printf( "%08x ", (int)(ElfHandle->Rel.r_info));
                        printf( "                  ");
                        printf( "%08x ", (int)(ElfHandle->Sym.st_value));
                        printf( sym_str);
                        printf( "\n");
                    }
                    /*To next entry*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf( "\n");
                }
                /*Relocate*/
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf( "\n");
                }
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of RELA = %x\n", (int)(num_of_entry));
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", (int)(CurrentShdrEx->Shdr.sh_link));
                    printf( "info   : %x\n", (int)(CurrentShdrEx->Shdr.sh_info));
                    printf( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                }
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( ElfHandle, i, &(ElfHandle->Rela), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->SymShdr));
                    ELi_GetSent( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->Sym),
                                 (u32)(ElfHandle->SymShdr.sh_entsize * ELF32_R_SYM( ElfHandle->Rela.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( ElfHandle, ElfHandle->SymShdr.sh_link, ElfHandle->Sym.st_name, sym_str, 128);

                    if( dbg_print_flag == 1) {
                        printf( "%08x  ", (int)(ElfHandle->Rela.r_offset));
                        printf( "%08x ", (int)(ElfHandle->Rela.r_info));
                        printf( "                  ");
                        printf( "%08x ", (int)(ElfHandle->Sym.st_value));
                        printf( sym_str);
                        printf( "\n");
                    }
                    /*To next entry*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf( "\n");
                }
                /*Relocate*/
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf( "\n");
                }
            }
        }
    }
    }else{    /*When not a DLL but a static module*/
        for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
            if( CurrentShdrEx->debug_flag == 1) {                /*When it is debugging information*/
            }else{
                if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                    ELi_DiscriminateGlobalSym( ElfHandle, i);
                }
            }
        }
    }

    /*---------- Deallocate the ELShdrEx List  ----------*/
    CurrentShdrEx = ElfHandle->ShdrEx;
    if( CurrentShdrEx) {
        while( CurrentShdrEx->next != NULL) {
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            free( FwdShdrEx);
        }
        ElfHandle->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*Flush cache before the DLL in RAM is called*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return (ElfHandle->process);
}

/*------------------------------------------------------
  Resolve unresolved symbols using the address table
 -----------------------------------------------------*/
u16 EL_ResolveAllLibrary( void)
{
    ELAdrEntry*           AdrEnt;
    ELUnresolvedEntry*    RemoveUnrEnt;
    ELUnresolvedEntry*    UnrEnt;
//    ELUnresolvedEntry*    CurrentUnrEnt;
//    ELUnresolvedEntry*    FwdUnrEnt;
//    u32                    relocation_val;
//    ELAdrEntry            AddAdrEnt;
    char                  sym_str[128];

    UnrEnt = ELUnrEntStart;
    if( dbg_print_flag == 1) {
        printf( "\nResolve all symbols:\n");
    }
    while( UnrEnt != NULL) {
        if( UnrEnt->remove_flag == 0) {
            AdrEnt = EL_GetAdrEntry( UnrEnt->sym_str);        /*Search from address table*/
            if( AdrEnt) {                                    /*When found in address table*/
                UnrEnt->S_ = (u32)(AdrEnt->adr);
                UnrEnt->T_ = (u32)(AdrEnt->thumb_flag);
                if( unresolved_table_block_flag == 0) {
                    if( dbg_print_flag == 1) {
                        printf( "\n symbol found %s : %8x\n", UnrEnt->sym_str, (int)(UnrEnt->S_));
                    }
                    UnrEnt->remove_flag = 1;
    //                ELi_RemoveUnresolvedEntry( UnrEnt);    //Delete from unresolved list because this was resolved
                }else{
                    if( dbg_print_flag == 1) {
                        printf( "\n static symbol found %s : %8x\n", UnrEnt->sym_str, (int)(UnrEnt->S_));
                    }
                    UnrEnt->AdrEnt = AdrEnt;            //Set the found address entry
                    UnrEnt->remove_flag = 2;            //Marking (special value used only with makelst)
                    strcpy( sym_str, UnrEnt->sym_str);
                    while( 1) {
                        RemoveUnrEnt = ELi_GetUnresolvedEntry( sym_str);
                        if( RemoveUnrEnt == NULL) {
                            break;
                        }else{
                            RemoveUnrEnt->remove_flag = 1;
                        }
                    }
                }
    /*            relocation_val = ELi_DoRelocate( UnrEnt);    //Resolve symbol
                if( !relocation_val) {
                    return EL_FAILED;
                }*/
            
            }else{                                            /*When not found in address table*/
                if( unresolved_table_block_flag == 0) {
                    if( dbg_print_flag == 1) {
                        printf( "ERROR! cannot find symbol : %s\n", UnrEnt->sym_str);
                    }
                }else{
                    if( dbg_print_flag == 1) {
                        printf( "ERROR! cannot find static symbol : %s\n", UnrEnt->sym_str);
                    }
                }
    /*            AddAdrEnt.next = NULL;
                AddAdrEnt.name = UnrEnt->sym_str;
                AddAdrEnt.
                EL_AddAdrEntry( */
    //            return EL_FAILED;
            }
        }
        UnrEnt = (ELUnresolvedEntry*)(UnrEnt->next);                            /*To next unresolved entry*/
    }
    
    /*---------- Deallocate the ELUnresolvedEntry List  ----------*/
/*    CurrentUnrEnt = ELUnrEntStart;
    if( CurrentUnrEnt) {
        while( CurrentUnrEnt->next != NULL) {
            FwdUnrEnt = CurrentUnrEnt;
            CurrentUnrEnt = CurrentUnrEnt->next;
            free( FwdUnrEnt->sym_str);    // symbol name string
            free( FwdUnrEnt);            //Structure itself
        }
        ELUnrEntStart = NULL;
    }*/
    /*-------------------------------------------*/

    /*Flush cache before the DLL in RAM is called*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return EL_RELOCATED;
}



/*------------------------------------------------------
  Write marked symbol to a public file as a structure
 -----------------------------------------------------*/
void EL_ExtractStaticSym1( void)
{
//    ELAdrEntry*            AdrEnt;
//    ELUnresolvedEntry*    RemoveUnrEnt;
    ELUnresolvedEntry*    UnrEnt;
//    ELUnresolvedEntry*    CurrentUnrEnt;
//    ELUnresolvedEntry*    FwdUnrEnt;
//    u32                    relocation_val;
//    ELAdrEntry            AddAdrEnt;
    char                  sym_str[256];


    UnrEnt = ELUnrEntStart;


    file_write( "/*--------------------------------\n", CSourceFilep);
    file_write( "  extern symbol\n", CSourceFilep);
    file_write( " --------------------------------*/
\n", CSourceFilep);
    
    while( UnrEnt != NULL) {
        if( UnrEnt->remove_flag == 2) {//Marking (special value used only with makelst)
            if( (UnrEnt->AdrEnt->func_flag) != STT_FUNC
                && (UnrEnt->AdrEnt->name[0] == '_' && UnrEnt->AdrEnt->name[1] != '_')) {
                memset( sym_str, 0, 128);
                strcpy( sym_str, "extern u8 ");
                strcat( sym_str, UnrEnt->sym_str);
                strcat( sym_str, ";\n");
                file_write( sym_str, CSourceFilep);
            }
        }
        UnrEnt = (ELUnresolvedEntry*)(UnrEnt->next);                            /*To next unresolved entry*/
    }

    file_write( "\n\n", CSourceFilep);


    file_write( "/*--------------------------------\n", CSourceFilep);
    file_write( "  symbol structure\n", CSourceFilep);
    file_write( " --------------------------------*/
\n", CSourceFilep);
    
    UnrEnt = ELUnrEntStart;

    while( UnrEnt != NULL) {
        if( UnrEnt->remove_flag == 2) {//Marking (special value used only with makelst)
            memset( sym_str, 0, 128);
            strcpy( sym_str, "ELAdrEntry AdrEnt_");
            strcat( sym_str, UnrEnt->sym_str);
            strcat( sym_str, " = {\n");
            file_write( sym_str, CSourceFilep);
            
            file_write( "    (void*)NULL,\n", CSourceFilep);
            
            strcpy( sym_str, "    (char*)\"");
            strcat( sym_str, UnrEnt->sym_str);
            strcat( sym_str, "\\0\", \n");
            file_write( sym_str, CSourceFilep);

            if( (UnrEnt->AdrEnt->func_flag) == STT_FUNC) {
                strcpy( sym_str, "    (void*)");
            }else{
                strcpy( sym_str, "    (void*)&");
            }
            strcat( sym_str, UnrEnt->sym_str);
            strcat( sym_str, ",\n");
            file_write( sym_str, CSourceFilep);
            
            if( (UnrEnt->AdrEnt->func_flag) == 1) {
                file_write( "    0,\n", CSourceFilep);
            }else{
                file_write( "    1,\n", CSourceFilep);
            }
            if( (UnrEnt->AdrEnt->thumb_flag) == 0) {
                file_write( "    0,\n", CSourceFilep);
            }else{
                file_write( "    1,\n", CSourceFilep);
            }
            file_write( "};\n", CSourceFilep);

//            printf( "\n static symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
        }
        UnrEnt = (ELUnresolvedEntry*)(UnrEnt->next);                            /*To next unresolved entry*/
    }
}

/*------------------------------------------------------
  Write marked symbol to a public file as an API
 -----------------------------------------------------*/
void EL_ExtractStaticSym2( void)
{
//    ELAdrEntry*            AdrEnt;
//    ELUnresolvedEntry*    RemoveUnrEnt;
    ELUnresolvedEntry*    UnrEnt;
//    ELUnresolvedEntry*    CurrentUnrEnt;
//    ELUnresolvedEntry*    FwdUnrEnt;
//    u32                    relocation_val;
//    ELAdrEntry            AddAdrEnt;
    char                  sym_str[256];

    UnrEnt = ELUnrEntStart;

    while( UnrEnt != NULL) {
        if( UnrEnt->remove_flag == 2) {//Marking (special value used only with makelst)
            memset( sym_str, 0, 128);
            strcpy( sym_str, MAKELST_DS_API);
            strcat( sym_str, "( &AdrEnt_");
            strcat( sym_str, UnrEnt->sym_str);
            strcat( sym_str, ");\n");
            file_write( sym_str, CSourceFilep);
            
//            printf( "\n static symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
        }
        UnrEnt = (ELUnresolvedEntry*)(UnrEnt->next);                            /*To next unresolved entry*/
    }
}


/*------------------------------------------------------
  Delete entry from address table
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    DmyAdrEnt.next = ELAdrEntStart;
    CurrentAdrEnt = &DmyAdrEnt;

    while( CurrentAdrEnt->next != AdrEnt) {
        if( CurrentAdrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
    }

    CurrentAdrEnt->next = AdrEnt->next;
    ELAdrEntStart = DmyAdrEnt.next;
    
    return TRUE;
}

/*------------------------------------------------------
  Add entry to address table
 -----------------------------------------------------*/
void EL_AddAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    if( !ELAdrEntStart) {
        ELAdrEntStart = AdrEnt;
    }else{
        DmyAdrEnt.next = ELAdrEntStart;
        CurrentAdrEnt = &DmyAdrEnt;

        while( CurrentAdrEnt->next != NULL) {
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
        CurrentAdrEnt->next = (void*)AdrEnt;
    }
    AdrEnt->next = NULL;
}

/*------------------------------------------------------
  Search for entry corresponding to specified string in address table
 -----------------------------------------------------*/
ELAdrEntry* EL_GetAdrEntry( char* ent_name)
{
    ELAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt == NULL) {
        return NULL;
    }
    while( strcmp( CurrentAdrEnt->name, ent_name) != 0) {
        CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        if( CurrentAdrEnt == NULL) {
            break;
        }
    }
    return CurrentAdrEnt;
}

/*------------------------------------------------------
  Return address corresponding to specified string in address table
 -----------------------------------------------------*/
void* EL_GetGlobalAdr( char* ent_name)
{
    u32         adr;
    ELAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = EL_GetAdrEntry( ent_name);

    if( CurrentAdrEnt) {
        if( CurrentAdrEnt->thumb_flag) {
            adr = (u32)(CurrentAdrEnt->adr) + 1;
        }else{
            adr = (u32)(CurrentAdrEnt->adr);
        }
    }else{
        adr = 0;
    }

    return (void*)(adr);
}


/*------------------------------------------------------
  Deallocate the address table (failed because it tried to delete up to the entry registered by the application)
 -----------------------------------------------------*/
#if 0
void* EL_FreeAdrTbl( void)
{
    ELAdrEntry*    FwdAdrEnt;
    ELAdrEntry*    CurrentAdrEnt;
    
    /*---------- Deallocate the ELAdrEntry List  ----------*/
    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt) {
        while( CurrentAdrEnt->next != NULL) {
            FwdAdrEnt = CurrentAdrEnt;
            CurrentAdrEnt = CurrentAdrEnt->next;
            free( FwdAdrEnt->name);        //Symbol name string
            free( FwdAdrEnt);            //Structure itself
        }
        ELAdrEntStart = NULL;
    }
    /*------------------------------------*/
}
#endif

/*------------------------------------------------------
  Stub that reads data from the ELF object
 -----------------------------------------------------*/
void ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
    fseek( file_struct, file_offset, SEEK_SET);
    fread( buf, 1, size, file_struct);
    
/*    FS_SeekFile( file_struct, (s32)(file_offset), FS_SEEK_SET);
    FS_ReadFile( file_struct, buf, (s32)(size));*/
}

void ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
/*    MI_CpuCopy8( (void*)(file_base + file_offset),
                buf,
                size);*/
    memcpy( buf,
            (void*)(file_base + file_offset),
            size);
}
