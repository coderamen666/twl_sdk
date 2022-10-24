/*---------------------------------------------------------------------------*
  Project:  TwlSDK - STD - include
  File:     string.h

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-26#$
  $Rev: 8696 $
  $Author: yasuda_kei $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_STD_STRING_H_
#define NITRO_STD_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/mi/memory.h>

//---- aliases
#define STD_StrCpy          STD_CopyString
#define STD_StrLCpy         STD_CopyLString
#define STD_StrChr          STD_SearchChar
#define STD_StrRChr         STD_SearchCharReverse
#define STD_StrStr          STD_SearchString
#define STD_StrLen          STD_GetStringLength
#define STD_StrNLen         STD_GetStringNLength
#define STD_StrCat          STD_ConcatenateString
#define STD_StrLCat         STD_ConcatenateLString
#define STD_StrCmp          STD_CompareString
#define STD_StrNCmp         STD_CompareNString
#define STD_StrLCmp         STD_CompareLString

#define STD_MemCpy          STD_CopyMemory
#define STD_MemMove         STD_MoveMemory
#define STD_MemSet          STD_FillMemory

/*---------------------------------------------------------------------------*
  Name:         STD_CopyString

  Description:  same to strcpy

  Arguments:    destp: destination pointer
                srcp: src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_CopyString(char *destp, const char *srcp);

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLStringZeroFill

  Description:  do not correspond with strlcpy

  Arguments:    destp: destination pointer
                srcp: src pointer
                n: copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
extern int STD_CopyLStringZeroFill(char *destp, const char *srcp, int n);

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLString

  Description:  same to strlcpy

  Arguments:    destp: destination pointer
                srcp: src pointer
                siz: copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
extern int STD_CopyLString(char *destp, const char *srcp, int siz);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchChar

  Description:  This is the same as strchr.

  Arguments:    srcp: src string
                c: character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchChar(const char *srcp, int c);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchCharReverse

  Description:  This is the same as strrchr.

  Arguments:    srcp: src string
                c: character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchCharReverse(const char *srcp, int c);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchString

  Description:  same to strstr

  Arguments:    srcp: src string
                str: string to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchString(const char *srcp, const char *str);

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringLength

  Description:  Get string length. Same as strlen

  Arguments:    str: string

  Returns:      string length
 *---------------------------------------------------------------------------*/
extern int STD_GetStringLength(const char *str);

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringNLength

  Description:  Gets the length of a string, up to 'len'. This is the same as strnlen.

  Arguments:    str: string
                len: max length

  Returns:      string length
 *---------------------------------------------------------------------------*/
extern int STD_GetStringNLength(const char *str, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateString

  Description:  Concatenate strings. Same as strcat

  Arguments:    str1: original string
                str2: string to concatenate

  Returns:      concatenated string
 *---------------------------------------------------------------------------*/
extern char *STD_ConcatenateString(char *str1, const char *str2);

/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateLString

  Description:  Concatenates strings. This is the same as strlcat.

  Arguments:    str1: original string
                str2: string to concatenate
                size: buffer size of str1

  Returns:      length of str1 + length of str2
 *---------------------------------------------------------------------------*/
extern int STD_ConcatenateLString(char *str1, const char *str2, int size);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareString

  Description:  Compare strings. Same as strcmp

  Arguments:    str1, str2: strings

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareString(const char *str1, const char *str2);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  same to strncmp

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareNString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareLString

  Description:  same to strlcmp

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareLString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  This is the same as strncasecmp.

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareNIString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_TSScanf

  Description:  sscanf with size reduced.
                Supports basic format specifications "%(*?)([lh]{,2})([diouxXpn])".

  Arguments:    src           Input string
                fmt           Format control string

  Returns:      Total number of substituted values.
                Returns -1 if the function ends without a substitution or an error is detected.
 *---------------------------------------------------------------------------*/
extern int STD_TSScanf(const char *src, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSScanf

  Description:  This version supports va_list used with STD_TSScanf.
                Supports basic format specification "%(*?)([lh]{,2})[diouxX]".

  Arguments:    src           Input string
                fmt           Format control string
                vlist         Parameters

  Returns:      Total number of substituted values.
                Returns -1 if the function ends without a substitution or an error is detected.
 *---------------------------------------------------------------------------*/
extern int STD_TVSScanf(const char *src, const char *fmt, va_list vlist);

/*---------------------------------------------------------------------------*
  Name:         STD_TSPrintf

  Description:  With the exception of the format of the arguments, identical to STD_TVSNPrintf.

  Arguments:    dst           The buffer that will store the result
                fmt           Format control string

  Returns:      Identical to STD_VSNPrintf.
 *---------------------------------------------------------------------------*/
extern int STD_TSPrintf(char *dst, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSPrintf

  Description:  With the exception of the format of the arguments, identical to STD_TVSNPrintf.

  Arguments:    dst           The buffer that will store the result
                fmt           Format control string
                vlist         Parameters

  Returns:      Identical to STD_VSNPrintf.
 *---------------------------------------------------------------------------*/
extern int STD_TVSPrintf(char *dst, const char *fmt, va_list vlist);

/*---------------------------------------------------------------------------*
  Name:         STD_TSNPrintf

  Description:  With the exception of the format of the arguments, identical to STD_TVSNPrintf.

  Arguments:    dst           The buffer that will store the result
                len           Buffer length
                fmt           Format control string
 
  Returns:      Identical to STD_VSNPrintf.
 *---------------------------------------------------------------------------*/
extern int STD_TSNPrintf(char *dst, size_t len, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSNPrintf

  Description:  sprintf redone to save on size.
                Supports basic format specifications.
                %([-+# ]?)([0-9]*)(\.?)([0-9]*)([l|ll|h||hh]?)([diouxXpncs%])

  Note:         The '+' and '#' characters are disabled to match the behavior of sprintf() in CodeWarrior's MSL.
                
                { // example
                  char buf[5];
                  sprintf(buf, "%-i\n", 45);  // "45"  (OK)
                  sprintf(buf, "%0i\n", 45);  // "45"  (OK)
                  sprintf(buf, "% i\n", 45);  // " 45" (OK)
                  sprintf(buf, "%+i\n", 45);  // "%+i" ("+45" expected)
                  sprintf(buf, "%#x\n", 45);  // "%#x" ("0x2d" expected)
                  // but, this works correctly!
                  sprintf(buf, "% +i\n", 45); // "+45" (OK)
                }

  Arguments:    dst           The buffer that will store the result
                len           Buffer length
                fmt           Format control string
                vlist         Parameters

  Returns:      Returns the number of characters (exclusive of '\0') when the format string is output correctly.
                When the buffer size is sufficient, all text is output and a terminator is provided.
                When the buffer size is not enough, termination occurs at dst[len-1].
                Nothing happens when len is 0.

 *---------------------------------------------------------------------------*/
extern int STD_TVSNPrintf(char *dst, size_t len, const char *fmt, va_list vlist);


static inline void* STD_CopyMemory(void *destp, const void *srcp, u32 size)
{
    MI_CpuCopy(srcp, destp, size);
    return destp;
}

static inline void* STD_MoveMemory(void *destp, const void *srcp, u32 size)
{
    MI_CpuMove(srcp, destp, size);
    return destp;
}

static inline void* STD_FillMemory(void *destp, u8 data, u32 size)
{
	MI_CpuFill(destp, data, size);
	return destp;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_STD_STRING_H_ */
#endif
