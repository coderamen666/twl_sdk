/*---------------------------------------------------------------------------*
  Project:  TwlSDK - - makelst
  File:     searcharg.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "searcharg.h"


extern const unsigned long SDK_DATE_OF_LATEST_FILE;

int    modc;
char** modv;
char** dll_argv;
char** static_argv;
char** header_argv;
int    dll_index    = 0;
int    static_index = 0;
int    header_index = 0;
char*  output_fname = NULL;



void SA_Usage( void)
{
    fprintf( stderr, "Development Tool - makelst - Make \"C\" source file\n");
    fprintf( stderr, "Build %lu\n\n", SDK_DATE_OF_LATEST_FILE);
    fprintf( stderr, "Usage: makelst [-o output-file] [--static static-files ...] [--dll dll-files ...] [--header header-files ...]\n\n");
    exit( 1);
}


/* Convert -dll A B C ...  to -dll A -dll B -dll C ...*/
void SA_modify( int argc, char* argv[])
{
    int i=0;
    int j=0;
    char* opt_name;
    modv = malloc( 4 * (argc * 3));

    for( ; i<argc; ) {
        if( (strcmp( argv[i], "--dll") == 0) ||
            (strcmp( argv[i], "--static") == 0) ||
            (strcmp( argv[i], "--header") == 0)) { //Insert option
          
            opt_name = argv[i++];
            if( argc <= i) { break;}
            while( strncmp( argv[i], "-", 1) != 0) {
                modv[j++] = opt_name;
                modv[j++] = argv[i++];
                if( argc <= i) { break;}
            }
        }else{ //Ordinary support
            modv[j++] = argv[i++];
        }
    }
    modc = j;
//    printf( "modc = %d\n", modc);
}


/*Parse the argument*/
void SA_searchopt( int argc, char* argv[])
{
    int n;
    struct option optionInfo[] = {
        { "static" , required_argument, NULL, 's'},
        { "dll"    , required_argument, NULL, 'l'},
        { "header" , required_argument, NULL, 'H'},
        { NULL, 0, NULL, 0}
    };

    if( argc <= 1) {
        SA_Usage();
    }
  
    /*Convert argument*/
    SA_modify( argc, argv);

    dll_argv    = malloc( 4 * argc);
    static_argv = malloc( 4 * argc);
    header_argv = malloc( 4 * argc);

    while( (n = getopt_long( modc, modv, "do:h", &optionInfo[0], NULL))
           != -1)
    {
        switch( n) {
        case 'd':
//          dbg_print_flag = 1;
          break;
        case 'o':
          if( output_fname != NULL) {
              fprintf( stderr, "ERROR! redefined output filename.\n");
              SA_Usage();
          }
          output_fname = optarg;
          break;
        case 's': // "--static"
          static_argv[static_index++] = optarg;
          break;
        case 'l': // "--dll"
          dll_argv[dll_index++] = optarg;
          break;
        case 'H': // "--header"
          header_argv[header_index++] = optarg;
          break;
        case 'h':
          SA_Usage();
          break;
        default: // '?'
          SA_Usage();
          break;
        }
    }

    /* When the static file is not specified */
    if( static_index == 0) {
        fprintf( stderr, "ERROR! no input static file(s).\n");
        SA_Usage();
    }
    /* When the DLL is not specified */
    if( dll_index == 0) {
        fprintf( stderr, "ERROR! no input dll file(s).\n");
        SA_Usage();
    }
    /* When there are elements that are not options */
    if( optind != modc) {
        SA_Usage();
    }
}

