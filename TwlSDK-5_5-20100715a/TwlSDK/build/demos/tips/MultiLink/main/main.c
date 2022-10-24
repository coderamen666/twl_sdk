/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tips - demos - MultiLink
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

void    Ext_Module(void);
void    Ext_Obj_Module(void);

void NitroMain(void)
{
    OS_Init();
    
    OS_Printf("NitroMain Start\n");
    Ext_Module();
    Ext_Obj_Module();
    OS_Printf("NitroMain End\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
