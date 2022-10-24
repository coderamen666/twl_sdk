/*---------------------------------------------------------------------------*
  Project:  TwlSDK - stripdebug
  File:     stripdebug.c

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
#include    <stdio.h>
#include    <stdlib.h>              // atoi()
#include    <ctype.h>
#include    <getopt.h>              // getopt()
#include    <string.h>
#include    "types.h"
#include    "elf.h"
#include    "elf_loader.h"
#include    "searcharg.h"


#define    DS_ROM_HEADER_SIZE    0x4000

char*      input_elf_fname    = NULL;
char*      stripped_elf_fname = NULL;
char*      added_elf_fname    = NULL;
char       c_source_line_str[256];

#define    STRIPPED_ELF_FILENAME    "stripped-"
#define    ADDED_ELF_FILENAME       "added-"
FILE*      NewElfFilep;

/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u32 adr_ALIGN( u32 addr, u32 align_size);
void file_write( char* c_str, FILE* Fp);


/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u16        dbg_print_flag;
u16        unresolved_table_block_flag = 0;

/*---------------------------------------------------------------------------*
 *  MAIN
 *---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    FILE        *FHp;
    u32*        newelfbuf;
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    u32*        dbgelfbuf;
#endif
//    u32*        aligned_newelfbuf;
    u32         elfsize;
    ELHandle    ElfH, ElfH2;
    char*       elf_filename;
    u32         elf_namesize;
    char*       slash_pointer;
    u16         result;
    

/*    
    for( i=0; i<argc; i++) {
        fprintf(stdout, "%s\n", argv[i]);
    }
*/

    /*-----------------------------------------------------*/
    dbg_print_flag = 0;
    SA_searchopt( argc, argv);

    EL_Init();
    unresolved_table_block_flag = 0;    //Release prohibition against adding to unresolved table

    /* Check the DLL */
    FHp = fopen( input_elf_fname, "rb");
    if( FHp == NULL) {
        printf( "cannot open file \"%s\".\n", input_elf_fname);
        exit( 1);
    }
    fseek( FHp, 0, SEEK_END);
    elfsize = ftell( FHp);
    fseek( FHp, 0, SEEK_SET);
    
    printf( "input elf size    = 0x%x\n", (int)elfsize);
    
    EL_InitHandle( &ElfH);
    EL_InitHandle( &ElfH2);
  
//    aligned_newelfbuf = (((u32)newelfbuf) & (~0x20)) + 0x20; //0x20 bytes align
    newelfbuf = (u32*)malloc( elfsize);
    result = EL_LoadLibraryfromFile( &ElfH,  FHp, newelfbuf, 0);
  
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    dbgelfbuf = (u32*)malloc( elfsize * 2);
    result = EL_LoadLibraryfromFile( &ElfH2,  FHp, dbgelfbuf, 1);
#endif
  
    fclose( FHp);
//    EL_ResolveAllLibrary();

  
    /*---------------------------------------------*/
    /*If there is an -o option, change to specified filename*/
    if( stripped_elf_fname != NULL) {
        elf_filename = stripped_elf_fname;
    }else{
        /*Generate elf filename*/
        elf_namesize = strlen( STRIPPED_ELF_FILENAME) + strlen( input_elf_fname) + 1;//+1 is the last null character
        elf_filename = malloc( elf_namesize);
        memset( elf_filename, 0, elf_namesize);
    
        slash_pointer = strrchr( input_elf_fname, '/');
        if( slash_pointer == NULL) {                      //When there is no slash
            strcpy( elf_filename, STRIPPED_ELF_FILENAME); //Prefix
            strcat( elf_filename, input_elf_fname);       //Filename
        }else{                                            //When there is a slash
            memcpy( elf_filename, input_elf_fname, (slash_pointer - input_elf_fname)+1);
            strcat( elf_filename, STRIPPED_ELF_FILENAME);
            strcat( elf_filename, slash_pointer+1);
        }
    }
    
    NewElfFilep = fopen( elf_filename, "wb");
    if( !NewElfFilep) {
        printf( "error : cannot create file \"%s\".\n\n", elf_filename);
        exit( 1);
    }

    printf( "stripped elf size = 0x%x\n", (int)(ElfH.newelf_size));
    fwrite( newelfbuf, 1, ElfH.newelf_size, NewElfFilep);
  
    fclose( NewElfFilep);
    free( newelfbuf);
    printf( "stripped elf file \"%s\" is generated.\n\n", elf_filename);
    if( stripped_elf_fname == NULL) {    //If there is no -o option, this is deallocated because of malloc
        free( elf_filename);
    }
    /*---------------------------------------------*/

  

    /*---------------------------------------------*/
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    /*If there is an -O option, change to specified filename*/
    if( added_elf_fname != NULL) {
        elf_filename = added_elf_fname;
    }else{
        /*Generate elf filename*/
        elf_namesize = strlen( ADDED_ELF_FILENAME) + strlen( input_elf_fname) + 1;//+1 is the last null character
        elf_filename = malloc( elf_namesize);
        memset( elf_filename, 0, elf_namesize);
    
        slash_pointer = strrchr( input_elf_fname, '/');
        if( slash_pointer == NULL) {                      //When there is no slash
            strcpy( elf_filename, ADDED_ELF_FILENAME);    //Prefix
            strcat( elf_filename, input_elf_fname);       //Filename
        }else{                                            //When there is a slash
            memcpy( elf_filename, input_elf_fname, (slash_pointer - input_elf_fname)+1);
            strcat( elf_filename, ADDED_ELF_FILENAME);
            strcat( elf_filename, slash_pointer+1);
        }
    }
    
    NewElfFilep = fopen( elf_filename, "wb");
    if( !NewElfFilep) {
        printf( "error : cannot create file \"%s\".\n\n", elf_filename);
        exit( 1);
    }

    printf( "added elf size = 0x%x\n", (int)(ElfH2.newelf_size));
    fwrite( dbgelfbuf, 1, ElfH2.newelf_size, NewElfFilep);
    
    fclose( NewElfFilep);
    free( dbgelfbuf);
    printf( "added elf file \"%s\" is generated.\n\n", elf_filename);
    if( added_elf_fname == NULL) {    //If there is no -o option, this is deallocated because of malloc
        free( elf_filename);
    }
#endif
    /*---------------------------------------------*/
  
    exit( 0);

    /*-----------------------------------------------------*/
    
    return 0;
}

/*---------------------------------------------------------------------------*
 *  Align addresses
 *---------------------------------------------------------------------------*/
u32 adr_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}

/*---------------------------------------------------------------------------*
 *  Write string to file
 *---------------------------------------------------------------------------*/
void file_write( char* c_str, FILE* Fp)
{
    fwrite( c_str, 1, strlen( c_str), Fp);
}
