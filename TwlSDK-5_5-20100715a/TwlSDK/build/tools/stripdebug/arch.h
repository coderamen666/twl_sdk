/*---------------------------------------------------------------------------*
  Project:  TwlSDK - stripdebug
  File:     arch.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef ARCH_H_
#define ARCH_H_

#include "types.h"

/*---------------------------------------------------------
 Archive Header
 --------------------------------------------------------*/
#define  ARMAG   "!<arch>\n"    /* Magic string */
#define  SARMAG  8              /* Length of magic string */

#define  ARFMAG       "\`\n"    /* Header trailer string */
#define  AR_NAME_LEN  16		/* ar_name size, includes `/' */


typedef struct    /* Archive file member header: printable ASCII */
{
    char    ar_name[16];    /* File member name: `/' terminated */
    char    ar_date[12];    /* File member date: decimal */
    char    ar_uid[6];      /* File member user id: decimal */
    char    ar_gid[6];      /* File member group id: decimal */
    char    ar_mode[8];     /* File member mode: octal */
    char    ar_size[10];    /* File member size: decimal */
    char    ar_fmag[2];     /* ARFMAG: string to end header */
}ArchHdr;                   /* Total 60 (0x3C) bytes */






/*---------------------------------------------------------
 Get the entry size from an entry header
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr);


#endif /*ARCH_H_*/
