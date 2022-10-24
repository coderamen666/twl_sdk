/*---------------------------------------------------------------------------*
  Project:  TwlSDK - makelst
  File:     arch.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "arch.h"


/*---------------------------------------------------------
 Find size of entry from entry header 
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr)
{
    u16 i;
    u32 digit = 1;
    u32 size = 0;

    /*----- Check how many digits there are -----*/
    for( i=0; i<10; i++) {
        if( ArHdr->ar_size[i] == 0x20) {
            break;
        }else{
            digit *= 10;
        }
    }
    digit /= 10;
    /*----------------------------*/

    /*----- Calculate size  -----*/
    for( i=0; i<10; i++) {
        size += (*(((u8*)(ArHdr->ar_size))+i) - 0x30) * digit;	//Convert char to u8 
        if( digit == 1) {
            break;
        }else{
	        digit /= 10;
        }
    }
    /*----------------------------*/
    
    return size;
}
