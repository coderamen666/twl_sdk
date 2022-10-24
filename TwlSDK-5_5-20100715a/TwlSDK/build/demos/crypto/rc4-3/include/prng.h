/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     prng.h

  Copyright 2006 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: prng.h,v $
  Revision 1.1  2006/03/08 08:53:41  seiki_masashi
  Added rc4-3 demo

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef PRNG_H_
#define PRNG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/types.h>

/*---------------------------------------------------------------------------*
    function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         AddRandomSeed

  Description:  This function adds a random seed to the PRNG.

  Arguments:    random_seed - Pointer to the random seed to add
                len - Length of the random seed to add

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AddRandomSeed(u8* random_seed, u32 len);

/*---------------------------------------------------------------------------*
  Name:         ChurnRandomPool

  Description:  This function adds the current system status as a random seed to the PRNG.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChurnRandomPool(void);

/*---------------------------------------------------------------------------*
  Name:         GetRandomBytes

  Description:  This function gets a random number from the PRNG.

  Arguments:    Pointer to the buffer that stores the random number
                len - Length of the random number to get

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GetRandomBytes(u8* buffer, u32 len);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif  /* PRNG_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
