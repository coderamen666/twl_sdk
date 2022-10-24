/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     util.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-03-15#$
  $Rev: 11308 $
  $Author: yada $

 *---------------------------------------------------------------------------*/


#if	!defined(NITRO_FS_UTIL_H_)
#define NITRO_FS_UTIL_H_

#include <nitro/misc.h>
#include <nitro/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// Enable this definition when you want to use the file system from the ARM7
// #define SDK_ARM7FS

// This is only defined for environments that should include the full set of file system functionality
#if	!defined(SDK_ARM7) || defined(SDK_ARM7FS)
#define	FS_IMPLEMENT
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_IsSlash

  Description:  Internal function.
                Determine if a character is a directory delimiter.

  Arguments:    c: Character to determine

  Returns:      TRUE if a directory delimiting character.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FSi_IsSlash(u32 c)
{
    return (c == '/') || (c == '\\');
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPosition

  Description:  Advances the access position of a Shift JIS string by one character.

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      The access position that results from advancing pos by one character.
 *---------------------------------------------------------------------------*/
SDK_INLINE int FSi_IncrementSjisPosition(const char *str, int pos)
{
    return pos + 1 + STD_IsSjisLeadByte(str[pos]);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPosition

  Description:  Returns one character from the Shift_JIS string reference position.

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      pos is either one returned character from the referenced position or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPosition(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPositionToSlash

  Description:  Advances reference position of Shift_JIS string to either directory delimiter or end.
                

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears after pos or the end position.
 *---------------------------------------------------------------------------*/
int FSi_IncrementSjisPositionToSlash(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPositionToSlash

  Description:  Return reference position of Shift_JIS string to either directory separator character or beginning.
                

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears before pos or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPositionToSlash(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_TrimSjisTrailingSlash

  Description:  Deletes if end of Shift_JIS string is a directory delimiter.

  Arguments:    str: Shift_JIS string

  Returns:      Length of string.
 *---------------------------------------------------------------------------*/
int FSi_TrimSjisTrailingSlash(char *str);

/*---------------------------------------------------------------------------*
  Name:         FSi_StrNICmp

  Description:  Performs a case-insensitive string comparison for only the specified number of bytes.
                Note that this does not consider the terminating null character.

  Arguments:    str1: Source string to compare
                str2: Target string to compare
                len: Number of bytes to compare

  Returns:      The result of comparing (str1 - str2).
 *---------------------------------------------------------------------------*/
SDK_INLINE int FSi_StrNICmp(const char *str1, const char *str2, u32 len)
{
    int     retval = 0;
    int     i;
    for (i = 0; i < len; ++i)
    {
        u32     c = (u8)(str1[i] - 'A');
        u32     d = (u8)(str2[i] - 'A');
        if (c <= 'Z' - 'A')
        {
            c += 'a' - 'A';
        }
        if (d <= 'Z' - 'A')
        {
            d += 'a' - 'A';
        }
        retval = (int)(c - d);
        if (retval != 0)
        {
            break;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsUnicodeSlash

  Description:  Determines whether a Unicode character is a path delimiter.

  Arguments:    c: Unicode1 character
                
  Returns:      TRUE if it is L'/' (0x2F) or L'\\' (0x5C).
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FSi_IsUnicodeSlash(u16 c)
{
    return (c == L'/') || (c == L'\\');
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePosition

  Description:  Returns one character of the Unicode string reference position.

  Arguments:    str: Pointer indicating the start of the Unicode string
                pos: Current string reference position (in bytes)

  Returns:      pos is either one returned character from the referenced position or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePosition(const u16 *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePositionToSlash

  Description:  Returns reference position of Unicode string to either directory delimiter or beginning.
                

  Arguments:    str: Pointer indicating the start of the Unicode string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears before pos or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitConditionChange

  Description:  Sleeps until any of the specified bits takes a predetermined state.

  Arguments:    flags: Group of bits to monitor
                on: Bit that might take the value '1'
                off: Bit that might take the value '0'
                queue: Sleep queue or NULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FSi_WaitConditionChange(u32 *flags, u32 on, u32 off, OSThreadQueue *queue)
{
    OSIntrMode bak = OS_DisableInterrupts();
    while ((!on || ((*flags & on) == 0)) &&
           (!off || ((*flags & off) != 0)))
    {
        OS_SleepThread(queue);
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitConditionOn

  Description:  Sleeps until the specified bit is 1.

  Arguments:    flags: Group of bits to monitor
                bits: Bit that should change to 1
                queue: Sleep queue or NULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FSi_WaitConditionOn(u32 *flags, u32 bits, OSThreadQueue *queue)
{
    FSi_WaitConditionChange(flags, bits, 0, queue);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitConditionOff

  Description:  Sleeps until the specified bit is 0.

  Arguments:    flags: Group of bits to monitor
                bits: Bit that should change to 0
                queue: Sleep queue or NULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FSi_WaitConditionOff(u32 *flags, u32 bits, OSThreadQueue *queue)
{
    FSi_WaitConditionChange(flags, 0, bits, queue);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFileLengthIfProc

  Description:  If the specified file is an archive procedure, gets the size.

  Arguments:    file: File handle
                length: Storage destination of the size

  Returns:      If the specified file is an archive procedure, returns its size.
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFileLengthIfProc(FSFile *file, u32 *length);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFilePositionIfProc

  Description:  If the specified file is an archive procedure, gets the current position.

  Arguments:    file: File handle
                length: Storage destination of the size

  Returns:      If the specified file is an archive procedure, returns its current position.
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFilePositionIfProc(FSFile *file, u32 *length);

/*---------------------------------------------------------------------------*
  Name:         FSi_SeekFileIfProc

  Description:  If the specified file is an archive procedure, move the file pointer.

  Arguments:    file: File handle
                offset: Movement amount
                from: Starting point to seek from

  Returns:      TRUE if successful, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL FSi_SeekFileIfProc(FSFile *file, s32 offset, FSSeekFileMode from);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_UTIL_H_ */
