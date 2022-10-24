/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/util.h

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

#ifndef NITRO_CRYPTO_UTIL_H_
#define NITRO_CRYPTO_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_SetAllocator

  Description:  This function registers the memory management function to be used with the CRYPTO library.
                This function has been left for backwards compatibility.

  Arguments:    alloc - pointer to the memory allocation function
                free  - pointer to the memory release function

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define CRYPTO_SetAllocator(alloc, free)    CRYPTO_SetMemAllocator(alloc, free, NULL)


/*---------------------------------------------------------------------------*
  Name:         CRYPTO_SetMemAllocator

  Description:  This function registers the memory management function to be used with the CRYPTO library.

  Arguments:    alloc - pointer to the memory allocation function
                free  - pointer to the memory release function
                realloc - Pointer to a function for changing the memory size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CRYPTO_SetMemAllocator(
    void* (*alloc) (u32),
    void  (*free) (void*),
    void* (*realloc) (void*,u32,u32)
);



#define BER_INTEGER              2
#define BER_BIT_STRING           3
#define BER_OCTET_STRING         4
#define BER_NULL                 5
#define BER_OBJECT               6
#define BER_SEQUENCE            16
#define BER_CONSTRUCTED       0x20

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_DerSkip

  Description:  Skips the tag and length fields of ASN.1 (DER) objects.
                

  Arguments:    datap - Pointer to a buffer
                dlenp - Buffer length
                type - Tag number
                lenp - Value of the length field

  Returns:      1 : Skipped successfully
                0 : Failed to skip
 *---------------------------------------------------------------------------*/
int
CRYPTO_DerSkip(
    unsigned char **datap,
    unsigned int   *dlenp,
    unsigned char   type,
    unsigned int   *lenp
);

#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_UTIL_H_
