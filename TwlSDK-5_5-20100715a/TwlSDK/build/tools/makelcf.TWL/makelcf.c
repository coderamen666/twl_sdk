/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makelcf
  File:     makelcf.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>             // atoi()
#include	<ctype.h>
#include	<getopt.h>             // getopt()
#include	"makelcf.h"
#include	"defval.h"
#include	"misc.h"

extern const unsigned long SDK_DATE_OF_LATEST_FILE;

/*---------------------------------------------------------------------------*
 *  MAIN
 *---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int     n;
    int     narg;
    int     t;

    while ((n = getopt(argc, argv, "hdD:M:V:")) != -1)
    {
        switch (n)
        {
        case 'h':
            goto usage;

        case 'd':
            DebugMode = TRUE;
            break;

        case 'D':
            AddDefVal(optarg);
            break;

        case 'M':
            if (!AddDefValFromFile(optarg))
            {
                fprintf(stderr, "Stop.\n");
                return 1;
            }
            break;

        case 'V':
            t = atoi(optarg);
            if (t >= 1)
            {
                char    str[128];
                sprintf(str, "NITRO_LCFSPEC=%d", t);    // NITRO_LCFSPEC is defined
                AddDefVal(str);
            }
            else
            {
                fprintf(stderr, "Unknown version number [%s]. Stop.\n", optarg);
                return 1;
            }
            break;

        default:
            break;
        }
    }

    narg = argc - optind;

    if (narg == 2 || narg == 3)
    {
        if (ParseSpecFile(argv[optind]))
        {
            fprintf(stderr, "Stop.\n");
            return 1;
        }

        if (ParseTlcfFile(argv[optind + 1]))
        {
            fprintf(stderr, "Stop.\n");
            return 1;
        }

        if (CreateLcfFile(narg == 3 ? argv[optind + 2] : NULL))
        {
            fprintf(stderr, "Stop.\n");
            return 1;
        }
        return 0;
    }

  usage:
    {
        fprintf(stderr,
                "NITRO-SDK Development Tool - makelcf - Make linker command file\n"
                "Build %lu\n\n"
                "Usage:  makelcf [-DNAME=VALUE...] [-MDEFINES_FILE] [-V1|-V2] SPECFILE LCF-TEMPALTE [LCFILE]\n\n",
                SDK_DATE_OF_LATEST_FILE);
    }
    return 2;
}
