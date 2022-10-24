/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - argument-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-09#$
  $Rev: 7146 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    int     argc;
    int     n, c;
    BOOL    opt_b = FALSE;
    const char *opt_s = NULL;
    const char *opt_t = NULL;

    OS_Init();

    //---- display argc
    OS_Printf("---------------- display value of OS_GetArgc()\n" );
    argc = OS_GetArgc();
    OS_Printf("argc = %d\n", argc);

    //---- display argv
    OS_Printf("---------------- display value of OS_GetArgv()\n" );
    for (n = 0; n < argc+1; n++)
    {
        const char *argv = OS_GetArgv(n);
		OS_Printf("[%d] address=0x%08x string=[%s]\n",
				  n,
				  argv? argv: 0,
				  argv? argv: "(null)" );
    }
    OS_Printf("\n");

    //---- get option
    OS_Printf("---------------- try OS_GetOpt(\"bs:t::\")\n");
    while ((c = OS_GetOpt("bs:t::")) > 0)
    {
        switch (c)
        {
        case 'b':
            opt_b = TRUE;
            break;

        case 's':
            opt_s = OS_GetOptArg();
            break;

        case 't':
            opt_t = OS_GetOptArg();
            break;

        case '?':
        default:
            OS_Printf("Error --- option '%c'\n", OS_GetOptOpt());
            break;
        }
    }

    OS_Printf("OPTION-b: %s\n", opt_b ? "TRUE" : "FALSE");
    OS_Printf("OPTION-s: %s\n", opt_s ? opt_s : "(null)");
    OS_Printf("OPTION-t: %s\n", opt_t ? opt_t : "(null)");
	{
		const char* arg = OS_GetOptArg();
		OS_Printf("OS_GetOptArg(): %s\n", arg? arg: "(null)" );
	}

    for (n = OS_GetOptInd(); n < argc; n++)
    {
        OS_Printf("ARGV[%d] : %s\n", n, OS_GetArgv(n));
    }
    OS_Printf("\n");

	//---- get option
    OS_Printf("---------------- try OS_GetOpt(\"-bs:t::\")\n");

    opt_b = FALSE;
    opt_s = NULL;
    opt_t = NULL;
    (void)OS_GetOpt(NULL);             // Rewind

    while ((c = OS_GetOpt("-bs:t::")) > 0)
    {
        switch (c)
        {
        case 1:
            OS_Printf("----found string. OS_GetOptArg(): %s\n", OS_GetOptArg());
            OS_Printf("OPTION-b: %s\n", opt_b ? "TRUE" : "FALSE");
            OS_Printf("OPTION-s: %s\n", opt_s ? opt_s : "(null)");
            OS_Printf("OPTION-t: %s\n", opt_t ? opt_t : "(null)");
            break;

        case 'b':
            opt_b = TRUE;
            break;

        case 's':
            opt_s = OS_GetOptArg();
            break;

        case 't':
            opt_t = OS_GetOptArg();
            break;

        case '?':
        default:
            OS_Printf("Error --- option '%c'\n", OS_GetOptOpt());
            break;
        }
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
