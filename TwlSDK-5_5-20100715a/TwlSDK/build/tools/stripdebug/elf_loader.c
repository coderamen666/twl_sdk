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

#include "types.h"
#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"
#include <string.h>
#include <stdlib.h>


//OSHeapHandle        EL_Heap;
ELAdrEntry*           ELAdrEntStart = NULL;
ELUnresolvedEntry*    ELUnrEntStart = NULL;

extern u16 dbg_print_flag;
extern u32 unresolved_table_block_flag;
extern u32 ELi_ALIGN( u32 addr, u32 align_size);

extern u16 ELi_LoadObject2( ELHandle* ElfHandle, void* obj_offset, void* buf);

     
#define MAKELST_DS_API        "    elAddAdrEntry"    //API function name to add to the address table on DS

extern char    c_source_line_str[256];
extern FILE*   NewElfFilep;


/*------------------------------------------------------
  Local Function Declarations
 -----------------------------------------------------*/
// Relocate ELF object or that archive in a buffer
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf, u16 type);
// Core function that relocates ELF object to buffer
u16 ELi_LoadObject( ELHandle* ElfHandle, void* obj_offset, void* buf);
// Stub functions that read data from the ELF object
void ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
void ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);



/*Express hex value with decimal string*/
void ELi_SetDecSize( char* dec, u32 hex)
{
    u16 i;
    u32 tmp = 1000000000;
    u16 ptr = 0;
  
    memset( dec, 0x20, 10);
    for( i=0; i<10; i++) {
        if( ((hex / tmp) != 0) || (ptr != 0)) {
            dec[ptr++] = 0x30 + (hex/tmp);
            hex -= (hex/tmp) * tmp;
        }
        tmp /= 10;
    }
}



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
    ElfHandle->ShdrEx   = NULL;
    ElfHandle->SymEx    = NULL;
    ElfHandle->SymExTbl = NULL;
    ElfHandle->SymExTarget = 0xFFFFFFFF;

    ElfHandle->process = EL_INITIALIZED;    /*Set the flag*/

    return TRUE;
}

/*------------------------------------------------------
  Relocate ELF object or that archive in a buffer
    
    ElfHandle: Header structure
    ObjFile: Object file or archive file structure
    buf: Buffer for loading
    type: 0: Create elf with debug information removed, 1: Create elf for debugging
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf, u16 type)
{
    u16 result;
    u32 len;

    /*Set read function*/
    ElfHandle->ELi_ReadStub = ELi_ReadFile;
    ElfHandle->FileStruct = ObjFile;

    fseek( ObjFile, 0, SEEK_END);
    len = ftell( ObjFile);
    fseek( ObjFile, 0, SEEK_SET);
    result = ELi_LoadLibrary( ElfHandle, NULL, len, buf, type);

    return result;
}

/*------------------------------------------------------
  Relocate ELF object or the archive in a buffer
    
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
    
    result = ELi_LoadLibrary( ElfHandle, obj_image, obj_len, buf, 0);

    return result;
}

/*------------------------------------------------------
  Relocate ELF object or the archive in a buffer
    
    ElfHandle: Header structure
    obj_image: RAM image address for OBJ file or archive file
    buf: Buffer for loading
 -----------------------------------------------------*/
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf, u16 type)
{
    u16     result, all_result;
    u32     image_pointer;
    u32     new_image_pointer;
    u32     arch_size;
    u32     elf_num = 0;                /*Number of ELF objects*/
    u32     obj_size;
    u32     newarch_size = 0;
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


        new_image_pointer = (u32)buf;
        memcpy( buf, OBJMAG, 8); //Copy to stripped ELF
        new_image_pointer += 8;
        newarch_size += 8;
        

        ElfHandle->buf_current = (void*)new_image_pointer;
        while( image_pointer < obj_len) {
            ElfHandle->ELi_ReadStub( &ArHdr, ElfHandle->FileStruct, (u32)(obj_image), image_pointer, arch_size);
            ElfHandle->ELi_ReadStub( OBJMAG, ElfHandle->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4);    /*Get the OBJ string*/
            if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
                elf_num++;

                memcpy( (void*)new_image_pointer, (const void*)&ArHdr, arch_size); //Copy ar header to stripped ELF

                if( type == 0) {
                result = ELi_LoadObject( ElfHandle, (void*)(image_pointer+arch_size),
                                         (void*)(new_image_pointer + arch_size));
//                                         (ElfHandle->buf_current + arch_size));
                }else{
                result = ELi_LoadObject2( ElfHandle, (void*)(image_pointer+arch_size),
                                          (void*)(new_image_pointer + arch_size));
                }

                ELi_SetDecSize( (char*)(((ArchHdr*)new_image_pointer)->ar_size), ElfHandle->newelf_size); //Update arch header size
                new_image_pointer += (ElfHandle->newelf_size + arch_size);
                newarch_size += (ElfHandle->newelf_size + arch_size);
                
                if( result < all_result) {        /*Apply to all_result only when there are bad results*/
                    all_result = result;
                }
                /*Set default values*/
                ElfHandle->ShdrEx = NULL;
                ElfHandle->SymEx = NULL;
                ElfHandle->process = EL_INITIALIZED;    /*Set the flag*/
            }else{
                memcpy( (void*)new_image_pointer, (const void*)&ArHdr, arch_size); //Copy ar header to stripped ELF

                /*Copy next (entry) of ar header to stripped ELF*/
                ElfHandle->ELi_ReadStub( (void*)(new_image_pointer+arch_size), ElfHandle->FileStruct,
                                         (u32)(obj_image),
                                         (image_pointer+arch_size), AR_GetEntrySize( &ArHdr));

                new_image_pointer += (AR_GetEntrySize( &ArHdr) + arch_size);
                newarch_size += (AR_GetEntrySize( &ArHdr) + arch_size);
            }
            /*To next entry*/
            obj_size = AR_GetEntrySize( &ArHdr);
            if( obj_size %2)  //Padded by '\n' when the object size is an odd number.
            {
                obj_size++;
            }
            image_pointer += arch_size + obj_size;
        }
        ElfHandle->newelf_size = newarch_size;
    }else{/*--------------- For ELF files  ---------------*/
        if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
            elf_num++;
            if( type == 0) {
                all_result = ELi_LoadObject( ElfHandle, 0, buf);
            }else{
                all_result = ELi_LoadObject2( ElfHandle, 0, buf);
            }
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
//    u32         num_of_entry;
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx    DmyShdrEx;
//    char        sym_str[128];     //For debugging print
//    u32         offset;           //For debugging print
    u32         newelf_shoff = 0; //Write pointer to stripped ELF image
    u32         buf_shdr;
    u32         section_num = 0;
//    u32         newelf_size;
    u32         tmp_buf;
    u32         *shdr_table;      //Table for new/old number correspondence for section headers
//    u32         *sym_table;       //Table corresponding to the new/old numbers of symbol entries
    u32         last_local_symbol_index = 0xFFFFFFFF;


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
    ElfHandle->buf_current = (void*)((u32)buf + sizeof( Elf32_Ehdr));
    ElfHandle->shentsize = ElfHandle->CurrentEhdr.e_shentsize;

    /*Section header table build*/
    shdr_table = (u32*)malloc( 4 * ElfHandle->CurrentEhdr.e_shnum);
  
    /*---------- Create ELShdrEx and shdr_table Lists  ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        memset( CurrentShdrEx, 0, sizeof(ELShdrEx));     //Clear zero
        
        /*Set flag to distinguish whether this is debugging information*/
        if( ELi_ShdrIsDebug( ElfHandle, i) == TRUE) {    /*When it is debugging information*/
            CurrentShdrEx->debug_flag = 1;
            shdr_table[i] = 0xFFFFFFFF;
        }else{                                           /*When not debugging information*/
            /*Copy section header*/
            ELi_GetShdr( ElfHandle, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
            shdr_table[i] = section_num;                 /*Create section new/old table*/
//            printf( "shdr_table[0x%x] = 0x%x\n", i, section_num);
            section_num++;
            /*Get section string*/
#if 0 // Do not dynamically allocate section string buffer
            CurrentShdrEx->str = (char*)malloc( 128);    //Get 128 character buffer
            ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                           CurrentShdrEx->Shdr.sh_name,
                           CurrentShdrEx->str, 128);
#else
            {
                u32 length;
                
                length = ELi_GetStrLen( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx, CurrentShdrEx->Shdr.sh_name) + 1;
                
                CurrentShdrEx->str = (char*)malloc(length);
                if(CurrentShdrEx->str == 0)
                {
                    return EL_FAILED;
                }
                ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                               CurrentShdrEx->Shdr.sh_name,
                               CurrentShdrEx->str, length);
            }
#endif
        }
    }
#if 0 /*Comment out because the addition of a special section was performed on the added-elf side*/
    section_num++;                    //Add special section
    ElfHandle->CurrentEhdr.e_shnum++; //Add special section
    CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
    CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
    memset( CurrentShdrEx, 0, sizeof(ELShdrEx));    //Clear zero
//    CurrentShdrEx->Shdr.sh_name
    CurrentShdrEx->Shdr.sh_type = SPECIAL_SECTION_TYPE;
    CurrentShdrEx->Shdr.sh_flags = 0;
    CurrentShdrEx->Shdr.sh_addr = 0;
    CurrentShdrEx->Shdr.sh_offset = 0;
    CurrentShdrEx->Shdr.sh_size = (section_num - 1)*4; //(ElfHandle->CurrentEhdr.e_shnum)*4; (When old section number is in the index)
    CurrentShdrEx->Shdr.sh_link = 0;
    CurrentShdrEx->Shdr.sh_info = 0;
    CurrentShdrEx->Shdr.sh_addralign = 4;
    CurrentShdrEx->Shdr.sh_entsize = 4;
#if 0 // Do not dynamically allocate section string buffer
    CurrentShdrEx->str = (char*)malloc( 128);
#else
    CurrentShdrEx->str = (char*)malloc( strlen(SPECIAL_SECTION_NAME) + 1 );
    if(CurrentShdrEx->str == 0)
    {
        return EL_FAILED;
    }
#endif
    strcpy( CurrentShdrEx->str, SPECIAL_SECTION_NAME);
#endif
    CurrentShdrEx->next = NULL;
    ElfHandle->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------------------*/

   
    /*---------- Check and copy all sections  ----------*/
//    printf( "\nLoad to RAM:\n");
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {                 //When it is debugging information
//            printf( "skip debug-section %02x\n", i);
        }else{                                                //When not debugging information
            // .text section
            if( /*(CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&*/
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            // .data, .data1 section (Initialized data)
/*            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }*/
            // bss section
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //Do not copy
                CurrentShdrEx->loaded_adr = ELi_ALIGN( (u32)(ElfHandle->buf_current), 4);
/*                CurrentShdrEx->loaded_adr = (u32)
                                ELi_AllocSectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));*/
            }
            // .rodata, .rodata1 section
/*            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }*/
#if 0 /*Comment out because the addition of a special section was performed on the added-elf side*/
            else if( CurrentShdrEx->Shdr.sh_type == SPECIAL_SECTION_TYPE) {
                //Series of processes equivalent to ELi_CopySectionToBuffer
                u32 load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
                u32 sh_size    = CurrentShdrEx->Shdr.sh_size;
                u32* reverse_shdr_table = (u32*)malloc( 4 * (section_num-1)); //-1 is to exclude a special section
                /*Index converts to new section number table*/
                for( j=0; j<(ElfHandle->CurrentEhdr.e_shnum - 1); j++) {
                    if( shdr_table[j] != 0xFFFFFFFF) {
                        reverse_shdr_table[shdr_table[j]] = j;
                    }
                }
                memcpy( (u32*)load_start, reverse_shdr_table, sh_size); //Copy new/old section correspondence table
                free( reverse_shdr_table);
                CurrentShdrEx->loaded_adr = load_start;
                ElfHandle->buf_current = (void*)(load_start + sh_size);
            }
#endif
            //Copy .arm_vfe_header, .ARM.exidx, .ARM.attributes and the like
            else if( CurrentShdrEx->Shdr.sh_type >= SHT_LOPROC) {
                //Copy to memory
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            //Symbol table section
            else if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                ELi_BuildSymList( ElfHandle, i, &(CurrentShdrEx->sym_table)); //Create symbol list
                {
                    ELSymEx* CurrentSymEx;
                    ELShdrEx* StrShEx;    //Character string section
#if 0 // Do not dynamically allocate section string buffer
                    char     symstr[128];
#else
                    char     *symstr;
#endif
                    u32      symbol_num = 0;

                    /*--- Correct symbol entry data ---*/
                    CurrentSymEx = ElfHandle->SymEx;
                    while( CurrentSymEx != NULL) {  //If you track SymEx, debug symbols are not included
                        /*Get character string section*/
                        StrShEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                         CurrentShdrEx->Shdr.sh_link);
                        /*--- Build character string section content ---*/
#if 0 // Do not dynamically allocate section string buffer
                        ELi_GetStrAdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, //Get symbol string
                                       CurrentSymEx->Sym.st_name,
                                       symstr, 128);
#else
                        {
                            u32 length;
                            
                            length = ELi_GetStrLen( ElfHandle, CurrentShdrEx->Shdr.sh_link, CurrentSymEx->Sym.st_name ) + 1;
                            
                            symstr = (char*)malloc(length);
                            if(symstr == 0)
                            {
                                return EL_FAILED;
                            }
                            ELi_GetStrAdr( ElfHandle, CurrentShdrEx->Shdr.sh_link,
                                           CurrentSymEx->Sym.st_name,
                                           symstr, length);
                        }
#endif
                        StrShEx->str_table = realloc( StrShEx->str_table,     //Add symbol string
                                                      (StrShEx->str_table_size) +
                                                      strlen( symstr) + 1);
                        strcpy( (u8*)((u32)StrShEx->str_table + StrShEx->str_table_size),
                                symstr);

                        CurrentSymEx->Sym.st_name = StrShEx->str_table_size; //Correct symbol entry data

                        StrShEx->str_table_size += ( strlen( symstr) + 1);
#if 0 // Do not dynamically allocate section string buffer
#else
                        free(symstr);
#endif
                        /*---------------------------------*/
                        //Find final STB_LOCAL symbol (becomes the value of sh_info for the SYMTAB section header)
                        if( ((ELF32_ST_BIND( (CurrentSymEx->Sym.st_info))) != STB_LOCAL)&&
                            (last_local_symbol_index == 0xFFFFFFFF)) {
                            last_local_symbol_index = symbol_num;
                        }
                        /*---------------------------------*/

                        symbol_num++;

                        /*---------------------------------*/
                        if( (CurrentSymEx->Sym.st_shndx != SHN_UNDEF) &&
                            (CurrentSymEx->Sym.st_shndx < SHN_LORESERVE)) {
                            CurrentSymEx->Sym.st_shndx = shdr_table[CurrentSymEx->Sym.st_shndx]; //Update section number for the symbol entry
                        }
                        /*---------------------------------*/
                        CurrentSymEx = CurrentSymEx->next;
                    }/*-------------------------------------*/
                    
                    /*--- Update the symbol table section header ---*/
                    CurrentShdrEx->loaded_adr = (u32)(ELi_CopySymToBuffer( ElfHandle));
                    if( (CurrentShdrEx->Shdr.sh_link != SHN_UNDEF) &&
                        (CurrentShdrEx->Shdr.sh_link < SHN_LORESERVE)) {
                            CurrentShdrEx->Shdr.sh_link = shdr_table[CurrentShdrEx->Shdr.sh_link]; //Update string section number
                    }
                    CurrentShdrEx->Shdr.sh_size = symbol_num * sizeof( Elf32_Sym);
                    CurrentShdrEx->Shdr.sh_info = last_local_symbol_index; //Reference the ARM ELF materials
                    /*----------------------------------------------*/
                }
                ELi_FreeSymList( ElfHandle, (u32**)(CurrentShdrEx->sym_table));    //Deallocate the symbol list
            }

/*            printf( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);*/
        }
    }

    /*---------- Processing for REL, RELA, STRTAB sections ----------*/
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {                //When it is debugging information
        }else{                                               //When not debugging information
            //REL section
            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                {
                    Elf32_Rel*  CurrentRel;
                    ELShdrEx*   SymShdrEx;
                    u32         new_sym_num;
                    /*Get the symbol section*/
                    SymShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                       CurrentShdrEx->Shdr.sh_link);
                    /*Correct the Rel section of the copy source*/
                    for( j=0; j<(CurrentShdrEx->Shdr.sh_size/CurrentShdrEx->Shdr.sh_entsize); j++) {
                        CurrentRel = (Elf32_Rel*)(CurrentShdrEx->loaded_adr + (j * sizeof( Elf32_Rel)));
                        new_sym_num = SymShdrEx->sym_table[ELF32_R_SYM(CurrentRel->r_info)];
                        CurrentRel->r_info = ELF32_R_INFO( new_sym_num,
                                                            ELF32_R_TYPE(CurrentRel->r_info));
                        }
                }
            }
            //RELA section
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                {
                    Elf32_Rela* CurrentRela;
                    ELShdrEx*   SymShdrEx;
                    u32         new_sym_num;
                    /*Get the symbol section*/
                    SymShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                       CurrentShdrEx->Shdr.sh_link);
                    /*Correct the Rela section of the copy source*/
                    for( j=0; j<(CurrentShdrEx->Shdr.sh_size/CurrentShdrEx->Shdr.sh_entsize); j++) {
                        CurrentRela = (Elf32_Rela*)(CurrentShdrEx->loaded_adr + (j * sizeof( Elf32_Rela)));
//                        printf( "symnum: 0x%x\n", ELF32_R_SYM(CurrentRela->r_info));
//                        printf( "sym_table:0x%x", (u32)(SymShdrEx->sym_table));
                        new_sym_num = SymShdrEx->sym_table[ELF32_R_SYM(CurrentRela->r_info)];
                        CurrentRela->r_info = ELF32_R_INFO( new_sym_num,
                                                            ELF32_R_TYPE(CurrentRela->r_info));
                        }
                }
            }
            //String table section
            else if( CurrentShdrEx->Shdr.sh_type == SHT_STRTAB) {
                if( i == ElfHandle->CurrentEhdr.e_shstrndx) { //Section name string table section
                    CurrentShdrEx->loaded_adr = (u32)
                        ELi_CopyShStrToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                }else{
                    CurrentShdrEx->loaded_adr = (u32)
                        ELi_CopySymStrToBuffer( ElfHandle, CurrentShdrEx);
//                    printf( "sym str section:0x%x, size:0x%x\n", i, CurrentShdrEx->str_table_size);
                }
            }
        }
    }
    /*-------------------------------------------------------*/

    
    /*Only the section content up to this point was copied to stripped ELF*/

  
    /*---------- Copy section header to stripped elf ----------*/
    buf_shdr = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    ElfHandle->buf_current = (void*)buf_shdr;
//    printf( "buf_shdr = 0x%x\n", buf_shdr);
//    printf( "buf = 0x%x\n", (u32)buf);
    
    newelf_shoff = buf_shdr - ((u32)(buf));
//    printf( "newelf_shoff = 0x%x\n", newelf_shoff);

    section_num = 0;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        tmp_buf = buf_shdr + ( section_num * sizeof( Elf32_Shdr));
        
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        if( CurrentShdrEx->debug_flag == 1) {                //Convert to the NULL section for debugging information
//            memset( (u8*)tmp_buf, '\0', sizeof( Elf32_Shdr));
        }else{
            /*Update offset*/
            if( CurrentShdrEx->loaded_adr != 0) {
                CurrentShdrEx->Shdr.sh_offset = (CurrentShdrEx->loaded_adr - (u32)buf);
            }
            /*Copy section header to stripped ELF image*/
            if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                /*The symbol table sh_type indicates the index of the symbol entry*/
              //For SYMTAB, because the sh_link is already converted at line 406, there is no need here.
              //For SYMTAB, because the sh_info is already converted at line 428, there is no need here.
            }else{
                CurrentShdrEx->Shdr.sh_link = shdr_table[CurrentShdrEx->Shdr.sh_link]; //Update section number
                CurrentShdrEx->Shdr.sh_info = shdr_table[CurrentShdrEx->Shdr.sh_info]; //Update section number
            }
            memcpy( (u8*)tmp_buf, &(CurrentShdrEx->Shdr), 
                    sizeof( Elf32_Shdr));
            section_num++;                        /*Update section count*/
        }
    }

    // After copy ends
    ElfHandle->process = EL_COPIED;
    /*------------------------------------------------------------*/

    ElfHandle->newelf_size = (buf_shdr - (u32)buf) + (section_num*sizeof( Elf32_Shdr));
//    printf( "newelf_size = 0x%x\n", ElfHandle->newelf_size);

    /*---------- Update ELF Header ----------*/
    ElfHandle->CurrentEhdr.e_shnum = section_num; /*Apply the reduced section count to the ELF header*/
    ElfHandle->CurrentEhdr.e_shstrndx = shdr_table[ElfHandle->CurrentEhdr.e_shstrndx];
    //Update the section header offset
    ElfHandle->CurrentEhdr.e_shoff = newelf_shoff;
    memcpy( (u8*)buf, &(ElfHandle->CurrentEhdr), sizeof( Elf32_Ehdr)); /*Copy ELF header to the stripped ELF image*/
    /*-----------------------------------*/


    /*---------- Relocate  ----------*/
/*    if( unresolved_table_block_flag == 0) {
        if( dbg_print_flag == 1) {
            printf( "\nRelocate Symbols:\n");
        }
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                //When it is debugging information
        }else{                                                //When not debugging information

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of REL = %x\n", num_of_entry);
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                    printf( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
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
                        printf( "%08x  ", ElfHandle->Rel.r_offset);
                        printf( "%08x ", ElfHandle->Rel.r_info);
                        printf( "                  ");
                        printf( "%08x ", ElfHandle->Sym.st_value);
                        printf( sym_str);
                        printf("\n");
                    }
                    // To next entry
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf("\n");
                }
                //Relocate
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf("\n");
                }
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of RELA = %x\n", num_of_entry);
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                    printf( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
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
                        printf( "%08x  ", ElfHandle->Rela.r_offset);
                        printf( "%08x ", ElfHandle->Rela.r_info);
                        printf( "                  ");
                        printf( "%08x ", ElfHandle->Sym.st_value);
                        printf( sym_str);
                        printf("\n");
                    }
                    // To next entry
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf("\n");
                }
                //Relocate
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf("\n");
                }
            }
        }
    }
    }else{    //When not dll but a static module
        for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
            if( CurrentShdrEx->debug_flag == 1) {                //When it is debugging information
            } else {
                if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                    ELi_DiscriminateGlobalSym( ElfHandle, i);
                }
            }
        }
    }*/

    /*---------- Deallocate the ELShdrEx List  ----------*/
    CurrentShdrEx = ElfHandle->ShdrEx;
    if( CurrentShdrEx) {
        do{
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            if( FwdShdrEx->debug_flag == 0) {
                free( FwdShdrEx->str);    //When not debugging section
            }
            free( FwdShdrEx);
        }while( CurrentShdrEx != NULL);
        ElfHandle->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*Deallocate the section header table*/
    free( shdr_table);


  
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
#if 0 // Do not dynamically allocate section string buffer
    char                  sym_str[128];
#else
    char                  *sym_str;
#endif
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
//                        printf( "\n symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
                    }
                    UnrEnt->remove_flag = 1;
    //                ELi_RemoveUnresolvedEntry( UnrEnt);    //Delete from unresolved list because this was resolved
                }else{
                    if( dbg_print_flag == 1) {
                        printf( "\n static symbol found %s : %8x\n", UnrEnt->sym_str, (int)(UnrEnt->S_));
                    }
                    UnrEnt->AdrEnt = AdrEnt;            //Set the found address entry
                    UnrEnt->remove_flag = 2;            //Marking (special value used only with makelst)
#if 0 // Do not dynamically allocate section string buffer
#else
                    sym_str = (char*)malloc( strlen( UnrEnt->sym_str ) + 1 );
                    if(sym_str == 0)
                    {
                        return EL_FAILED;
                    }
#endif
                    strcpy( sym_str, UnrEnt->sym_str);
                    while( 1) {
                        RemoveUnrEnt = ELi_GetUnresolvedEntry( sym_str);
                        if( RemoveUnrEnt == NULL) {
                            break;
                        }else{
                            RemoveUnrEnt->remove_flag = 1;
                        }
                    }
#if 0 // Do not dynamically allocate section string buffer
#else
                    free(sym_str);
#endif
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
  Delete entry from address table
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    DmyAdrEnt.next = ELAdrEntStart;
    CurrentAdrEnt  = &DmyAdrEnt;

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

#if 0
/*------------------------------------------------------
  Deallocate the address table (not allowed because it tried to delete up to the entry registered by the application)
 -----------------------------------------------------*/
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
//  printf( "0x%x, 0x%x\n", file_offset, size);
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
