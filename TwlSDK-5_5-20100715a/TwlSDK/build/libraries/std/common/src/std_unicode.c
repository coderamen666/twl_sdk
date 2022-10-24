/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - STD
  File:     std_unicode.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#include <nitro/std/unicode.h>


#if defined(SDK_ARM9) || !defined(SDK_NITRO)
// Because of memory restrictions, it is not possible to use the Unicode conversion feature with ARM7 in NITRO mode

#if defined(SDK_ARM9)
#define STD_UNICODE_STATIC_IMPLEMENTATION
#endif

#if defined(STD_UNICODE_STATIC_IMPLEMENTATION)
// ARM9 retains a table itself, and also locates executable codes in the resident module

// The array pointers are weak symbols; in HYBRID mode, the code table is sent to ltdmain.
// As weak symbols, however, they are not overwritten externally if they are placed in the same file. The array pointers have therefore been separated into std_unicode_array.c.
// 
// 
extern const u8    *STD_Unicode2SjisArray;
extern const u16   *STD_Sjis2UnicodeArray;
#else
// ARM7 references ARM9's table and also splits up the executable codes and places them in several locations in the LTD region
static const u8    *STD_Unicode2SjisArray = NULL;
static const u16   *STD_Sjis2UnicodeArray = NULL;
static STDResult STDi_ConvertStringSjisToUnicodeCore(u16 *dst, int *dst_len,
                                                     const char *src, int *src_len,
                                                     STDConvertUnicodeCallback callback)
                                                     __attribute__((never_inline));
static STDResult STDi_ConvertStringUnicodeToSjisCore(char *dst, int *dst_len,
                                                     const u16 *src, int *src_len,
                                                     STDConvertSjisCallback callback)
                                                     __attribute__((never_inline));
#endif


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         STDi_GetUnicodeConversionTable

  Description:  Gets the Unicode conversion table.

  Arguments:    u2s: Storage location of pointer to Unicode -> SJIS conversion table
                s2u: Storage location of pointer to SJIS -> Unicode conversion table

  Returns:      None.
 *---------------------------------------------------------------------------*/
void STDi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u)
{
    if (u2s)
    {
        *u2s = STD_Unicode2SjisArray;
    }
    if (s2u)
    {
        *s2u = STD_Sjis2UnicodeArray;
    }
}

/*---------------------------------------------------------------------------*
  Name:         STDi_AttachUnicodeConversionTable

  Description:  Assigns a Unicode conversion table to the STD library.

  Arguments:    u2s: Unicode -> SJIS conversion table
                s2u: SJIS -> Unicode conversion table

  Returns:      None.
 *---------------------------------------------------------------------------*/
void STDi_AttachUnicodeConversionTable(const u8 *u2s, const u16 *s2u)
{
#if defined(STD_UNICODE_STATIC_IMPLEMENTATION)
    (void)u2s;
    (void)s2u;
#else
    STD_Unicode2SjisArray = u2s;
    STD_Sjis2UnicodeArray = s2u;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertStringSjisToUnicode

  Description:  Converts a ShiftJIS character string to a Unicode character string.

  Arguments:    dst:               Conversion destination buffer
                                  The storage process is ignored if NULL is specified.
                dst_len:           Pointer which stores and passes the maximum number of characters for the conversion destination buffer, then receives the number of characters that were actually stored.
                                  Ignored when NULL is given.
                                  
                src:               Conversion source buffer
                src_len           Pointer which stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                                  
                                  The end-of-string position takes priority over this specification.
                                  When a either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                callback:          The callback to be called if there are any characters that can't be converted.
                                  When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
STDResult STD_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    // If the table is not resolved, substitute with ASCII conversion
    if (STD_Unicode2SjisArray == NULL)
    {
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u8 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (u16)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
    }
    else
    {
        return STDi_ConvertStringSjisToUnicodeCore(dst, dst_len, src, src_len, callback);
    }
}
#include <twl/ltdmain_begin.h>
static STDResult STDi_ConvertStringSjisToUnicodeCore(u16 *dst, int *dst_len,
                                                     const char *src, int *src_len,
                                                     STDConvertUnicodeCallback callback)
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    STDResult retval = STD_RESULT_SUCCESS;
    if (!src)
    {
        retval = STD_RESULT_INVALID_PARAM;
    }
    else
    {
        int     src_pos = 0;
        int     dst_pos = 0;
        int     src_max = (src_len && (*src_len >= 0)) ? *src_len : 0x7FFFFFFF;
        int     dst_max = (dst && dst_len && (*dst_len >= 0)) ? *dst_len : 0x7FFFFFFF;

        /* Until either buffer length reaches its terminus, process one character at a time. */
        while ((dst_pos < dst_max) && (src_pos < src_max))
        {
            u16     dst_tmp[4];
            int     dst_count = 0;
            int     src_count;
            u32     c1 = (u8)src[src_pos];
            /* Detection of end-of-string has precedence over the string length */
            if (!c1)
            {
                break;
            }
            /* ASCII [00, 7E] */
            else if (c1 <= 0x7E)
            {
                dst_tmp[0] = (u16)c1;
                src_count = 1;
                dst_count = 1;
            }
            /* Half-width kana [A1, DF] */
            else if ((c1 >= 0xA1) && (c1 <= 0xDF))
            {
                dst_tmp[0] = (u16)(c1 + (0xFF61 - 0xA1));
                src_count = 1;
                dst_count = 1;
            }
            /* ShiftJIS */
            else if (STD_IsSjisCharacter(&src[src_pos]))
            {
                src_count = 2;
                if (src_pos + src_count <= src_max)
                {
                    /*
                     * The range of the conversion table is:
                     * First byte: {81 - 9F, E0 - FC}, second byte: {40 - FF}.
                     * In order to avoid division to streamline calculation of the table,
                     * the second byte {7F, FD, FE, FF} is left in full.
                     */
                    u32     c2 = (u8)src[src_pos + 1];
                    c1 -= 0x81 + ((c1 >= 0xE0) ? (0xE0 - 0xA0) : 0);
                    dst_tmp[0] = STD_Sjis2UnicodeArray[c1 * 0xC0 + (c2 - 0x40)];
                    dst_count = (dst_tmp[0] ? 1 : 0);
                }
            }
            /* Calls the callback if a character appears which cannot be converted. */
            if (dst_count == 0)
            {
                if (!callback)
                {
                    retval = STD_RESULT_CONVERSION_FAILED;
                }
                else
                {
                    src_count = src_max - src_pos;
                    dst_count = sizeof(dst_tmp) / sizeof(*dst_tmp);
                    retval = (*callback) (dst_tmp, &dst_count, &src[src_pos], &src_count);
                }
                if (retval != STD_RESULT_SUCCESS)
                {
                    break;
                }
            }
            /* Terminate here if either the conversion source or the conversion destination passes their respective ends. */
            if ((src_pos + src_count > src_max) || (dst_pos + dst_count > dst_max))
            {
                break;
            }
            /* Stores the conversion result if the buffer is valid */
            if (dst)
            {
                int     i;
                for (i = 0; i < dst_count; ++i)
                {
                    MI_StoreLE16(&dst[dst_pos + i], (u16)dst_tmp[i]);
                }
            }
            src_pos += src_count;
            dst_pos += dst_count;
        }
        /* Returns the number of converted characters */
        if (src_len)
        {
            *src_len = src_pos;
        }
        if (dst_len)
        {
            *dst_len = dst_pos;
        }
    }
    return retval;
}
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
#include <twl/ltdmain_end.h>
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertStringUnicodeToSjis

  Description:  Converts a Unicode character string into a ShiftJIS character string.

  Arguments:    dst:               Conversion destination buffer
                                  The storage process is ignored if NULL is specified.
                dst_len:           Pointer which stores and passes the maximum number of characters for the conversion destination buffer, then receives the number of characters that were actually stored.
                                  Ignored when NULL is given.
                                  
                src:               Conversion source buffer
                src_len           Pointer which stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                                  
                                  The end-of-string position takes priority over this specification.
                                  When either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                callback:          The callback to be called if there are any characters that can't be converted.
                                  When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
STDResult STD_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    // If the table is not resolved, substitute with ASCII conversion
    if (STD_Unicode2SjisArray == NULL)
    {
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u16 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (char)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
    }
    else
    {
        return STDi_ConvertStringUnicodeToSjisCore(dst, dst_len, src, src_len, callback);
    }
}
#include <twl/ltdmain_begin.h>
static STDResult STDi_ConvertStringUnicodeToSjisCore(char *dst, int *dst_len,
                                                     const u16 *src, int *src_len,
                                                     STDConvertSjisCallback callback)
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    STDResult retval = STD_RESULT_SUCCESS;
    if (!src)
    {
        retval = STD_RESULT_INVALID_PARAM;
    }
    else
    {
        int     src_pos = 0;
        int     dst_pos = 0;
        int     src_max = (src_len && (*src_len >= 0)) ? *src_len : 0x7FFFFFFF;
        int     dst_max = (dst && dst_len && (*dst_len >= 0)) ? *dst_len : 0x7FFFFFFF;
        /* Until either buffer length reaches its terminus, process one character at a time. */
        while ((dst_pos < dst_max) && (src_pos < src_max))
        {
            char    dst_tmp[4];
            int     dst_count = 0;
            int     src_count = 1;
            u32     w = MI_LoadLE16(&src[src_pos]);
            /* Detection of end-of-string has precedence over the string length */
            if (!w)
            {
                break;
            }
            /* Extended characters (private region) */
            else if ((w >= 0xE000) && (w < 0xF8FF))
            {
                const u32 sjis_page = 188UL;
                const u32 offset = w - 0xE000;
                u32     c1 = offset / sjis_page;
                u32     c2 = offset - c1 * sjis_page;
                dst_tmp[0] = (char)(c1 + 0xF0);
                dst_tmp[1] = (char)(c2 + ((c2 < 0x3F) ? 0x40 : 0x41));
                dst_count = 2;
            }
            else
            {
                /*
                 * The range of the conversion table is:
                 * [0000-0480), [2000-2680), [3000-3400), [4E00-9FA8), [F928-FFE6).
                 * Regions filled with 0000 are simply deleted in descending order (largest first) and packed.
                 */
				/* *INDENT-OFF* */
                static const int table[][2] =
                {
                    {0x0000,0x0480 - 0x0000,},
                    {0x2000,0x2680 - 0x2000,},
                    {0x3000,0x3400 - 0x3000,},
                    {0x4E00,0x9FA8 - 0x4E00,},
                    {0xF928,0xFFE6 - 0xF928,},
                };
				enum { table_max = sizeof(table) / (sizeof(int) * 2) };
				/* *INDENT-ON* */
                int     i;
                int     index = 0;
                for (i = 0; i < table_max; ++i)
                {
                    const int offset = (int)(w - table[i][0]);
                    /* Invalid range */
                    if (offset < 0)
                    {
                        break;
                    }
                    /* Valid range */
                    else if (offset < table[i][1])
                    {
                        index += offset;
                        dst_tmp[0] = (char)STD_Unicode2SjisArray[index * 2 + 0];
                        if (dst_tmp[0])
                        {
                            dst_tmp[1] = (char)STD_Unicode2SjisArray[index * 2 + 1];
                            dst_count = dst_tmp[1] ? 2 : 1;
                        }
                        break;
                    }
                    /* Higher ranges */
                    else
                    {
                        index += table[i][1];
                    }
                }
            }
            /* Calls the callback if a character appears which cannot be converted. */
            if (dst_count == 0)
            {
                if (!callback)
                {
                    retval = STD_RESULT_CONVERSION_FAILED;
                }
                else
                {
                    src_count = src_max - src_pos;
                    dst_count = sizeof(dst_tmp) / sizeof(*dst_tmp);
                    retval = (*callback) (dst_tmp, &dst_count, &src[src_pos], &src_count);
                }
                if (retval != STD_RESULT_SUCCESS)
                {
                    break;
                }
            }
            /* Terminate here if either the conversion source or the conversion destination passes their respective ends. */
            if ((src_pos + src_count > src_max) || (dst_pos + dst_count > dst_max))
            {
                break;
            }
            /* Stores the conversion result if the buffer is valid */
            if (dst)
            {
                int     i;
                for (i = 0; i < dst_count; ++i)
                {
                    MI_StoreLE8(&dst[dst_pos + i], (u8)dst_tmp[i]);
                }
            }
            src_pos += src_count;
            dst_pos += dst_count;
        }
        /* Returns the number of converted characters */
        if (src_len)
        {
            *src_len = src_pos;
        }
        if (dst_len)
        {
            *dst_len = dst_pos;
        }
    }
    return retval;
}
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
#include <twl/ltdmain_end.h>
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)


#endif // defined(SDK_ARM9) || !defined(SDK_NITRO)
