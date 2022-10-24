/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2di_SplitChar.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2DI_SPLITCHAR_H_
#define G2DI_SPLITCHAR_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif


// Character extraction callback
typedef u16 (*NNSiG2dSplitCharCallback)(const void** ppChar);


/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dGetChar*

  Description:  This is a function for extracting an NNSiG2dGetCharCallback type character code.
                Gets the character code for the first character from the byte stream and moves the stream pointer to the next character.
                

  Arguments:    ppChar: The pointer to the buffer where the pointer to the byte array is stored.
                        Upon return from the function, this pointer's target buffer stores a pointer to the start of the next character.
                        

  Returns:      The character code of the first character of *ppChar.
 *---------------------------------------------------------------------------*/
u16 NNSi_G2dSplitCharUTF16(const void** ppChar);
u16 NNSi_G2dSplitCharUTF8(const void** ppChar);
u16 NNSi_G2dSplitCharShiftJIS(const void** ppChar);
u16 NNSi_G2dSplitChar1Byte(const void** ppChar);






#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2DI_SPLITCHAR_H_

