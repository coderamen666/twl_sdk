/*---------------------------------------------------------------------------*
  Project:  TwlSDK - stripdebug
  File:     searcharg.c

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "types.h"

#include "elf_loader.h"
#include "searcharg.h"

extern const unsigned long SDK_DATE_OF_LATEST_FILE;


/* getopt */
extern int optind, opterr, optopt;

/**/
extern u16        dbg_print_flag;
extern char*      input_elf_fname;
extern char*      stripped_elf_fname;
extern char*      added_elf_fname;



void SA_Usage( void)
{
    fprintf( stderr,
             "TWL-SDK Development Tool - stripdebug - strip debug-section from armelf.\n");
    fprintf( stderr,
             "Build %lu\n\n", SDK_DATE_OF_LATEST_FILE);
#if (SPECIAL_SECTION_ENABLE == 1) /*Add special section*/
    fprintf( stderr,
             "Usage: stripdebug [-o output-file] [-O output-debugger-file] dll-file\n\n");
#else
    fprintf( stderr,
             "Usage: stripdebug [-o output-file] dll-file\n\n");
#endif
    exit( 1);
}


void SA_searchopt( int argc, char* argv[])
{
    int n;

    if( argc <= 1) {
        SA_Usage();
    }
  
    while( (n = getopt( argc, argv, "hdo:O:")) != -1)
    {
//      printf( "%c = 0x%x\n", n, n);
        switch( n) {
        case 'd':
            dbg_print_flag = 1;
            break;
        case 'o':
            if( stripped_elf_fname != NULL) { //Prevent specifying duplications
                fprintf( stderr, "ERROR! redefined output filename.\n");
                SA_Usage();
            }
            //stripped_elf_fname = optarg;
            stripped_elf_fname = malloc(strlen(optarg)+1);
            strcpy(stripped_elf_fname, optarg);
            break;
        case 'O':
            if( added_elf_fname != NULL) { //Prevent specifying duplications
                fprintf( stderr, "ERROR! redefined output filename.\n");
                SA_Usage();
            }
            //added_elf_fname = optarg;
            added_elf_fname = malloc(strlen(optarg)+1);
            strcpy(added_elf_fname, optarg);
            break;
        case 'h':
        default: // '?'
            SA_Usage();
            break;
        }
    }

    /* getopt sorts argv */
    if( optind == (argc-1)) {
        //input_elf_fname = argv[optind];
        input_elf_fname = malloc(strlen(argv[optind])+1);
        strcpy(input_elf_fname, argv[optind]);
    }else{
        SA_Usage(); //Up to one input file
    }
}

void portoption( void)
{
//    if( k != 2) {
        printf( "error : too few input files.\n\n");
//    }
}
