/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - STD
  File:     std_stdlib.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-07#$
  $Rev: 9262 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <limits.h>
//================================================================================
//      Convert numeral String
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         STD_ConvertAsciiToInt

  Description:  Convert numeral string to integer.
                Same to atoi()

  Arguments:    s : pointer to string

  Returns:      value
 *---------------------------------------------------------------------------*/
int STD_ConvertAsciiToInt( const char* s )
{
    BOOL isPlus = TRUE;
    unsigned int val = 0;

    //---- skip space
    while( *s == ' ' )
    {
        s ++;
    }

    //---- check sign
    if ( *s == '-' )
    {
        isPlus = FALSE;
        s ++;
    }
    else if ( *s == '+' )
    {
        s ++;
    }

    //---- loop while number
    while( '0' <= *s && *s <= '9' )
    {
        val = (val * 10) + (*s - '0');
        //---- check overflow
        if ( val > INT_MAX )
        {
            return (isPlus)? INT_MAX: INT_MIN;
        }
        s++;
    }
    return (isPlus)? (int) val: (int) -val;
}

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertAsciiToLong

  Description:  Convert numeral string to long integer.
                Same to atol()

  Arguments:    s : pointer to string

  Returns:      value
 *---------------------------------------------------------------------------*/
long int STD_ConvertAsciiToLong( const char* s )
{
    BOOL isPlus   = TRUE;
    unsigned long int val = 0;

    //---- skip space
    while( *s == ' ' )
    {
        s ++;
    }

    //---- check sign
    if ( *s == '-' )
    {
        isPlus = FALSE;
        s ++;
    }
    else if ( *s == '+' )
    {
        s ++;
    }

    //---- loop while number
    while( '0' <= *s && *s <= '9' )
    {
        val = (val * 10) + (*s - '0');
        //---- check overflow
        if ( val > LONG_MAX )
        {
            return (isPlus)? LONG_MAX: LONG_MIN;
        }
        s++;
    }
    return (isPlus)? (long int) val: (long int) -val;
}
