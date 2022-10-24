/*---------------------------------------------------------------------------*
  Project:  TwlSDK - STD - include
  File:     stdlib.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_STD_STDLIB_H_
#define NITRO_STD_STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

//---- aliases
#define STD_AToI			STD_ConvertAsciiToInt
#define STD_AToL 	        STD_ConvertAsciiToLong


//================================================================================
//		Convert numeral String
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         STD_ConvertAsciiToInt

  Description:  convert numeral string to integer
  				same to atoi()

  Arguments:    s : string

  Returns:      value
 *---------------------------------------------------------------------------*/
extern int STD_ConvertAsciiToInt( const char* s );

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertAsciiToLong

  Description:  convert numeral string to long integer
  				same to atol()

  Arguments:    s : string

  Returns:      value
 *---------------------------------------------------------------------------*/
extern long int STD_ConvertAsciiToLong( const char* s );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_STD_STDLIB_H_ */
#endif
