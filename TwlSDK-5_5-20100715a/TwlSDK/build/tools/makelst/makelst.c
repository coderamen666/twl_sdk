/*---------------------------------------------------------------------------*
  Project:  TwlSDK - - makelst
  File:     makelst.c

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
#include    <string.h>
#include    <ctype.h>
#include    <getopt.h>              // getopt()
#include    "types.h"
#include    "elf.h"
#include    "elf_loader.h"
#include    "searcharg.h"



#define  DS_ROM_HEADER_SIZE    0x4000




extern int    modc;
extern char** modv;
extern char** dll_argv;
extern char** static_argv;
extern char** header_argv;
extern int    dll_index;
extern int    static_index;
extern int    header_index;
extern char*  output_fname;




char     c_source_line_str[256];

#define  C_SOURCE_FILENAME    "staticsymlist.c"
FILE*    CSourceFilep;

/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u32  adr_ALIGN( u32 addr, u32 align_size);
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
    int             i;
    FILE            *FHp;
    u32             binbuf[4];
    ELHandle        ElfH;
    char*           c_filename;
    u16             result;

//    printf( "binbuf : %x\n", binbuf);

    /*-----------------------------------------------------*/
    dbg_print_flag = 0;
    SA_searchopt( argc, argv);

    EL_Init();
    unresolved_table_block_flag = 0;    //Release prohibition against adding to unresolved table
    /*----------- Parse the dll file ----------------*/
    {
        for( i=0; i<dll_index; i++) {
            FHp = fopen( dll_argv[i], "rb");
            if( FHp == NULL) {
                printf( "cannot open file \"%s\".\n", dll_argv[i]);
                    exit( 1);
            }
            EL_InitHandle( &ElfH);
            result = EL_LoadLibraryfromFile( &ElfH,  FHp, binbuf);
            fclose( FHp);
        }
    }
    /*--------------------------------------------*/
    
    EL_ResolveAllLibrary();


    /*----------- Parse the static file ----------------*/
    unresolved_table_block_flag = 1;    //Blck adding to unresolved table
    {
        for( i=0; i<static_index; i++) {
            FHp = fopen( static_argv[i], "rb");
            if( FHp == NULL) {
                printf( "cannot open file \"%s\".\n", static_argv[i]);
                exit( 1);
            }
            EL_InitHandle( &ElfH);
            result = EL_LoadLibraryfromFile( &ElfH,  FHp, binbuf);
            fclose( FHp);
        }
    }
    /*-----------------------------------------------*/

    
    /*If there is an -o option, change to specified filename*/
    if( output_fname) {
        c_filename = output_fname;
    }else{
        /*Generate C source filename*/
        c_filename = malloc( strlen( C_SOURCE_FILENAME));
        strcpy( c_filename, C_SOURCE_FILENAME);
    }
    
    CSourceFilep = fopen( c_filename, "w");
    if( !CSourceFilep) {
        printf( "error : cannot create file \"%s\".\n\n", c_filename);
        exit( 1);
    }
    file_write( "/*This file generated automatically by the \"makelst\".*/\n", CSourceFilep);
    file_write( "\n", CSourceFilep);
    file_write( "#ifndef __STATIC_SYM_LIST__\n", CSourceFilep);
    file_write( "#define __STATIC_SYM_LIST__\n", CSourceFilep);
    file_write( "\n", CSourceFilep);
#ifndef SDK_TWL
    file_write( "#include <nitro.h>\n", CSourceFilep);
#else
    file_write( "#include <twl.h>\n", CSourceFilep);
#endif  
    file_write( "#include <twl/el.h>\n", CSourceFilep);
    {
        for( i=0; i<header_index; i++) {
            const char* header_name = header_argv[i];
            if (header_name[0] != '<' && header_name[0] != '"')
            {
                fprintf( CSourceFilep, "#include <%s>\n", header_name);
            }
            else
            {
                fprintf( CSourceFilep, "#include %s\n", header_name);
            }
        }
    }
    
    file_write( "\n", CSourceFilep);
    
    EL_ResolveAllLibrary();                //Marking of the extracted symbol
    EL_ExtractStaticSym1();                //Structure configuration portion
    file_write( "\n\n", CSourceFilep);
    
    file_write( "/*--------------------------------\n", CSourceFilep);
    file_write( "  API\n", CSourceFilep);
    file_write( " --------------------------------*/\n", CSourceFilep);
    file_write( "void EL_AddStaticSym( void)\n", CSourceFilep);
    file_write( "{\n", CSourceFilep);
    EL_ExtractStaticSym2();                //API calling portion
    file_write( "}\n", CSourceFilep);
    
    unresolved_table_block_flag = 0;    //Release prohibition against adding to unresolved table
    
    file_write( "\n", CSourceFilep);
    file_write( "#endif /*__STATIC_SYM_LIST__*/\n", CSourceFilep);
    fclose( CSourceFilep);
    /*---------------------------------------------*/

    printf( "\"C\" source file \"%s\" is generated.\n\n", c_filename);
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
