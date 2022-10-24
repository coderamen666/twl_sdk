/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - STD
  File:     std_string.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

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

// Speed optimization macros for string operation functions
//   (Although these are platform-independent, components will overflow if they are enabled on ARM7, so they are enabled only on ARM9.)
//    
#if defined(SDK_ARM9)
// Because at least the high-speed version of GetStringLength does not consider big endians, just in case, implementation of all functions will be limited to little endians until their operation can be confirmed. 
// 
#if (PLATFORM_BYTES_ENDIAN == PLATFORM_ENDIAN_LITTLE)
#define STRLEN_4BYTE_SKIP		// 4-byte evaluation is for non-zero checks only
#define STRLCPY_4BYTE           // A copy evaluation every 4 bytes 
#define STRNLEN_4BYTE_SKIP      // Measurement every 4 bytes 
#define STRCMP_4BYTE            // Comparison every 4 bytes 
#define STRNCMP_4BYTE           // Comparison every 4 bytes <- Disabled because of a bug
//#define STRNICMP_DIFF         // Improve the comparison 
#endif
#endif

/*---------------------------------------------------------------------------*
  Name:         STD_CopyString

  Description:  same to strcpy

  Arguments:    destp : destination pointer
                srcp : src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
char   *STD_CopyString(char *destp, const char *srcp)
{
    char   *retval = destp;

    SDK_ASSERT(destp && srcp);

    while (*srcp)
    {
        *destp++ = (char)*srcp++;
    }
    *destp = '\0';

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLStringZeroFill

  Description:  do not correspond with strlcpy

  Arguments:    destp : destination pointer
                srcp : src pointer
                n: copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
int STD_CopyLStringZeroFill(char *destp, const char *srcp, int n)
{
    int     i;
    const char *s = srcp;

    SDK_ASSERT(destp && srcp && n >= 0);

    for (i = 0; i < n - 1; i++)
    {
        destp[i] = (char)*s;
        if (*s)
        {
            s++;
        }
    }
    if( i < n )
    {
        destp[i] = '\0';
    }

    return STD_GetStringLength(srcp);
}

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLString

  Description:  same to strlcpy

  Arguments:    destp : destination pointer
                srcp : src pointer
                siz   : copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
int STD_CopyLString(char *destp, const char *srcp, int siz)
{
#if defined(STRLCPY_4BYTE)
	unsigned long  dstalign, srcalign;
	int            i;
	unsigned long  val32, y;
    const char *s = srcp;

    SDK_ASSERT(destp && srcp && siz >= 0);

	// 4-byte alignment check 
	dstalign = (unsigned long)destp & 0x3;
	srcalign = (unsigned long)srcp  & 0x3;

	i = 0;
	// If it does not match, copy normally 
	if( dstalign ^ srcalign )
	{
		goto strlcpy_normal;
	}

	// Check copy of the leading 4 bytes 
	if( dstalign )
	{
		for( ; (i + dstalign < 4) && (i < siz - 1); i++ )
		{
	        *(destp + i) = (char)*s;
    	    if (*s)
        	{
            	s++;
        	}
        	else
        	{
            	goto terminate;
        	}
		}
	}

	// Copy 4 bytes 
	for( ; i < siz - 1 - 4 + 1; i += 4 )
	{
		val32 = *(unsigned long*)(srcp + i);
		y = (val32 & 0x7F7F7F7F) + 0x7F7F7F7F;
		y = ~(y | val32 | 0x7F7F7F7F);
		
		if( y != 0 )
			break;

		*(unsigned long*)(destp + i) = val32;
	}

	// Copy remaining bytes 
	s = srcp + i;

strlcpy_normal:
	// Copy byte 
    for ( ; i < siz - 1; i++ )
    {
        destp[i] = (char)*s;
        if (*s)
        {
            s++;
        }
        else
        {
            break;
        }
    }

terminate:
	// Add terminating NULL 
    if ((i >= siz - 1) && (siz > 0))
    {
        destp[i] = '\0';
    }

	if( !*s )
    	return (int)i;
	else
		return (int)i + STD_GetStringLength( s );
#else  // original
    int     i;
    const char *s = srcp;

    SDK_ASSERT(destp && srcp && siz >= 0);

    for (i = 0; i < siz - 1; i++)
    {
        destp[i] = (char)*s;
        if (*s)
        {
            s++;
        }
        else
        {
            break;
        }
    }

    if ((i >= siz - 1) && (siz > 0))
    {
        destp[i] = '\0';
    }

    return STD_GetStringLength(srcp);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_SearchChar

  Description:  Same as strchr.

  Arguments:    srcp : src string
                c: character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
char    *STD_SearchChar(const char *srcp, int c)
{
    SDK_POINTER_ASSERT(srcp);

    for (;;srcp++)
    {
        if (c == *srcp)
        {
            return (char*)srcp;
        }
        if ('\0' == *srcp)
        {
            return NULL;
        }
    }
    // not reach here.
}

/*---------------------------------------------------------------------------*
  Name:         STD_SearchCharReverse

  Description:  Same as strrchr.

  Arguments:    srcp : src string
                c: character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
char    *STD_SearchCharReverse(const char *srcp, int c)
{
    const char* found = NULL;

    SDK_POINTER_ASSERT(srcp);

    for (;;srcp++)
    {
        if( c == *srcp )
        {
            found = srcp;
        }

        if( '\0' == *srcp)
        {
            return (char*)found;
        }
    }
    // not reach here.
}

/*---------------------------------------------------------------------------*
  Name:         STD_SearchString

  Description:  same to strstr

  Arguments:    srcp : src string
                str : string to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
char   *STD_SearchString(const char *srcp, const char *str)
{
    int     i;
    int     n;

    SDK_ASSERT(srcp && str);

    for (i = 0; srcp[i]; i++)
    {
        n = 0;
        while (str[n] && srcp[i + n] == str[n])
        {
            n++;
        }

        if (str[n] == '\0')
        {
            return (char *)&srcp[i];
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringLength

  Description:  get string length. same to strlen

  Arguments:    str : string

  Returns:      string length
 *---------------------------------------------------------------------------*/
int STD_GetStringLength(const char *str)
{
#if defined(STRLEN_4BYTE_SKIP)
    int     n;
	unsigned long  *p32, y;

	for( n=0; ((unsigned long)(str + n) & 3) != 0; n++ )
	{
		if(!str[n])
			return n;
	}

	p32 = (unsigned long*)(str + n);
	for( ; ; n+=4 )
    {
		y = (*p32 & 0x7F7F7F7F) + 0x7F7F7F7F;
		y = ~(y | *p32 | 0x7F7F7F7F);

		if( y != 0 )
			break;

		p32++;
	}

#if 1
	while (str[n])
    {
        n++;
    }
    return n;
#else
	if( y & 0x000000FF )
		return n;
	else if( y & 0x0000FF00 )
		return n + 1;
	else if( y & 0x00FF0000 )
		return n + 2;
	else
		return n + 3;
	// It is not possible to reach here
	return -1;
#endif

#else    // original
    int     n = 0;
    while (str[n])
    {
        n++;
    }
    return n;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringNLength

  Description:  Gets string length below len. Same as strnlen.

  Arguments:    str : string
                len: max length

  Returns:      string length
 *---------------------------------------------------------------------------*/
int STD_GetStringNLength(const char *str, int len)
{
#if defined(STRNLEN_4BYTE_SKIP)
    int     n;
	unsigned long  *p32, y;

	for( n=0; (((unsigned long)(str + n) & 3) != 0) && (n < len); n++ )
	{
		if(!str[n])
			return n;
	}

	p32 = (unsigned long*)(str + n);
	for( ; n < len - 4; n+=4 )
    {
		y = (*p32 & 0x7F7F7F7F) + 0x7F7F7F7F;
		y = ~(y | *p32 | 0x7F7F7F7F);

		if( y != 0 )
			break;

		p32++;
	}

    while( str[n] && (n < len) )
    {
        n++;
    }

	return n;

#else    // original
	int    n = 0;

    while( str[n] && (n < len) )
    {
        n++;
    }

	return n;
#endif
}


/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateString

  Description:  concatenate strings. same to strcat

  Arguments:    str1 : original string
                str2 : string to concatenate

  Returns:      concatenated string
 *---------------------------------------------------------------------------*/
char   *STD_ConcatenateString(char *str1, const char *str2)
{
    int     n = STD_GetStringLength(str1);
    (void)STD_CopyString(&str1[n], str2);
    return str1;
}

/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateLString

  Description:  Concatenates strings. Same as strlcat.

  Arguments:    str1 : original string
                str2 : string to concatenate
                size: buffer size of str1

  Returns:      length of str1 + length of str2
 *---------------------------------------------------------------------------*/
int STD_ConcatenateLString(char *str1, const char *str2, int size)
{
    int n;
    int str2_len;

    // If 'str1' is the length of 'size' or smaller and not terminated by NULL, [size + strlen(str2)] must be returned. 
    for( n = 0; n < size; ++n )
    {
        if( str1[n] == '\0' )
        {
            break;
        }
    }

    if( n >= size )
    {
        return n + STD_GetStringLength(str2);
    }

    str2_len = STD_CopyLString(&str1[n], str2, size - n);

    return n + str2_len;
}

/*---------------------------------------------------------------------------*
  Name:         STD_CompareString

  Description:  compare strings. same to strcmp

  Arguments:    str1, str2 : strings

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
int STD_CompareString(const char *str1, const char *str2)
{
#if defined(STRCMP_4BYTE)
	unsigned long  dstalign, srcalign;
	unsigned char  chr1, chr2;
	unsigned long  lng1, lng2, y;
    int            i;

	// 4-byte alignment check 
	dstalign = (unsigned long)str1 & 0x3;
	srcalign = (unsigned long)str2 & 0x3;

	i = 0;
	// If it does not match, compare normally 
	if( dstalign ^ srcalign )
	{
		goto strcmp_normal;
	}

	// Compare the leading 4 bytes 
	if( dstalign )
	{
		for( ; i + dstalign < 4; i++ )
		{
			chr1 = (unsigned char)*(str1 + i);
			chr2 = (unsigned char)*(str2 + i);
			if(chr1 != chr2 || !chr1)
			{
				return (int)(chr1 - chr2);
			}
		}
	}
	// Compare 4 bytes 
	for( ; ; i += 4 )
	{
		lng1 = *(unsigned long*)(str1 + i);
		lng2 = *(unsigned long*)(str2 + i);

		if( lng1 != lng2 )
			break;

		y = (lng1 & 0x7F7F7F7F) + 0x7F7F7F7F;
		y = ~(y | lng1 | 0x7F7F7F7F);

		if( y != 0 )
			break;
	}

strcmp_normal:
	// Compare normally 
	for(;;i++)
    {
		chr1 = (unsigned char)*(str1 + i);
		chr2 = (unsigned char)*(str2 + i);
		if(chr1 != chr2 || !chr1)
		{
			return (int)(chr1 - chr2);
		}
	}

	// This will not be reached
	return -1;

#else  // original
    while (*str1 == *str2 && *str1)
    {
        str1++;
        str2++;
    }
    return (*str1 - *str2);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  same to strncmp

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
int STD_CompareNString(const char *str1, const char *str2, int len)
{
#if defined(STRNCMP_4BYTE)
	unsigned long  dstalign, srcalign;
	unsigned char  chr1, chr2;
	unsigned long  lng1, lng2;
	unsigned long  y;
    int            i;

	if( len == 0 )
		goto end;

	// 4-byte alignment check 
	dstalign = (unsigned long)str1 & 0x3;
	srcalign = (unsigned long)str2 & 0x3;

	i = 0;
	// If it does not match, compare normally 
	if( dstalign ^ srcalign )
	{
		goto strncmp_normal;
	}

	// Compare the leading 4 bytes 
	if( dstalign )
	{
		for( ; (i + dstalign < 4) && (i < len); i++ )
		{
			chr1 = (unsigned char)*(str1 + i);
			chr2 = (unsigned char)*(str2 + i);
			if(chr1 != chr2 || !chr1)
			{
				return (int)(chr1 - chr2);
			}
		}
	}
	// Compare 4 bytes 
	for( ; i <= len - 4; i += 4 )
	{
		lng1 = *(unsigned long*)(str1 + i);
		lng2 = *(unsigned long*)(str2 + i);

		// Check comparison 
		if( lng1 != lng2 )
			break;

		// NULL check
		y = (lng1 & 0x7F7F7F7F) + 0x7F7F7F7F;
		y = ~(y | lng1 | 0x7F7F7F7F);

		if( y != 0 )
			break;
	}

strncmp_normal:
	// Compare normally 
    for (; i < len; ++i)
    {
		chr1 = (unsigned char)*(str1 + i);
		chr2 = (unsigned char)*(str2 + i);
		if(chr1 != chr2 || !chr1)
		{
			return (int)(chr1 - chr2);
		}
    }

end:
	// Return value
	return 0;
#else
    if (len)
    {
        int     i;
        for (i = 0; i < len; ++i)
        {
            int     c = (int)(MI_ReadByte(str1 + i));
            int     d = (int)(MI_ReadByte(str2 + i));
            if (c != d || !c)
            {
                return (int)(c - d);
            }
        }
    }
    return 0;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_CompareLString

  Description:  same to strlcmp

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
int STD_CompareLString(const char *str1, const char *str2, int len)
{
	int     c, d;

	while(len-- && *str2 != '\0')
	{
        c = (int)(MI_ReadByte(str1));
        d = (int)(MI_ReadByte(str2));
		if( c != d )
		{
			return c - d;
		}
		str1++;
		str2++;
	}
	return (int)(MI_ReadByte(str1));
}

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  Same as strncasecmp.

  Arguments:    str1, str2 : strings
                len: max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
int STD_CompareNIString(const char *str1, const char *str2, int len)
{
#if defined(STRNICMP_DIFF)
	int   i;
	char  c, d, diff;
	int   retval = 0;
#define TO_UPPER(x)		( ((x >= 'a') && (x <= 'z')) ? (x - 0x20) : x )

	if( (str1 == str2) || (len == 0) )
	{
		return 0;
	}

	for( i = 0; ; i++ )
	{
		c = *(str1 + i);
		d = *(str2 + i);
		diff = (char)(c - d);

		if( diff == 0 )
		{
			goto check_end;
		}
		else
		{
			if( diff == 'a' - 'A' )
			{
				if( (d >= 'A') && (d <= 'Z') )
				{
					goto check_end;
				}
			}
			else if( diff == 'A' - 'a' )
			{
				if( (c >= 'A') && (c <= 'Z') )
				{
					goto check_end;
				}
			}
			retval = TO_UPPER(c) - TO_UPPER(d);
			break;
		}
check_end:
		if( c == '\0' )
			break;
		if( i >= len - 1 )
			break;
	}

	return retval;
#else
    int     retval = 0;
#define TO_UPPER(x)		( ((x >= 'a') && (x <= 'z')) ? (x - 0x20) : x )

	if( (str1 == str2) || (len == 0) )
	{
		return 0;
	}

	while( !(retval = TO_UPPER(*str1) - TO_UPPER(*str2)) )
	{
		if( (*str1 == '\0') || (--len == 0) )
		{
			break;
		}
		str1++;
		str2++;
	}

    return retval;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_TSScanf

  Description:  This is sscanf with size reduced.
                Supports basic format specifications "%(*?)([lh]{,2})([diouxXpn])".

  Arguments:    src: Input string
                fmt: Format control string

  Returns:      Total number of substituted values.
                Returns -1 if the function ends without a substitution or an error is detected.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int STD_TSScanf(const char *src, const char *fmt, ...)
{
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = STD_TVSScanf(src, fmt, va);
    va_end(va);
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         STDi_IsSpace

  Description:  Determines if the specified character is a whitespace character.

  Arguments:    c: Character to evaluate.

  Returns:      TRUE if whitespace.
 *---------------------------------------------------------------------------*/
static BOOL STDi_IsSpace(int c)
{
    return (c == '\f') || (c == '\n') || (c == '\r') || (c == '\t') || (c == '\v') || (c == ' ');
}

/*---------------------------------------------------------------------------*
  Name:         STDi_GetBitset

  Description:  Gets the contents of the specified position of the bit set

  Arguments:    bitet: Bitset array.
                i: Position of the information to be gotten.

  Returns:      TRUE if ON, FALSE if OFF
 *---------------------------------------------------------------------------*/
inline static u32 STDi_GetBitset(const u32 *bitset, u32 i)
{
    const u32 pos = (u32)(i >> 5UL);
    const u32 bit = (u32)(i & 31UL);
    return ((bitset[pos] >> bit) & 1UL);
}

/*---------------------------------------------------------------------------*
  Name:         STDi_SetBitset

  Description:  Sets the specified position of the bitset to ON.

  Arguments:    bitet: Bitset array.
                i: Position to be turned ON

  Returns:      None.
 *---------------------------------------------------------------------------*/
inline static void STDi_SetBitset(u32 *bitset, u32 i)
{
    const u32 pos = (i >> 5UL);
    const u32 bit = (i & 31UL);
    bitset[pos] |= (1UL << bit);
}

/*---------------------------------------------------------------------------*
  Name:         STDi_FillBitset

  Description:  Sets the specified range [a,b) of the bitset to ON.

  Arguments:    bitet: Bitset array.
                a: Start position
                b: End position

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void STDi_FillBitset(u32 *bitset, u32 a, u32 b)
{
    while (a < b)
    {
        const u32 pos = (u32)(a >> 5UL);
        const u32 bit = (u32)(a & 31UL);
        u32     mask = (~0UL << bit);
        a = (pos + 1UL) * 32UL;
        if (a > b)
        {
            mask &= (u32)((1UL << (b & 31UL)) - 1UL);
        }
        bitset[pos] |= mask;
    }
}

/*---------------------------------------------------------------------------*
  Name:         STD_TVSScanf

  Description:  This version supports va_list used with STD_TSScanf.
                Supports basic format specifications.

  Arguments:    src: Input string
                fmt: Format control string
                vlist: Parameters

  Returns:      Total number of substituted values.
                Returns -1 if the function ends without a substitution or an error is detected.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int STD_TVSScanf(const char *src, const char *fmt, va_list vlist)
{
    /* Start of verification string (required for %n) */
    const char *src_base = src;
    /* Has the format been matched even once? (return -1 if not) */
    BOOL    matched = FALSE;
    /* Number of substitutions made. (return >=0 in this case) */
    int     stored = 0;

    /* Parse format string one character at a time */
    while (*fmt)
    {
        char    c = *fmt;
        /* Skip both formatting/verification if whitespace */
        if (STDi_IsSpace(c))
        {
            while (STDi_IsSpace(*fmt))
            {
                ++fmt;
            }
            while (STDi_IsSpace(*src))
            {
                ++src;
            }
            continue;
        }
        /* Conventional characters must match exactly */
        else if (c != '%')
        {
            if (c != *src)
            {
                break;
            }
            /* Check next byte if SHIFT-JIS  */
            if ((unsigned int)(((unsigned char)c ^ 0x20) - 0xA1) < 0x3C)
            {
                if (c != *++src)
                {
                    break;
                }
            }
            ++src;
            ++fmt;
        }
        /* Simply compare if the escape character '%' is detected */
        else if (fmt[1] == '%')
        {
            if (c != *src)
            {
                break;
            }
            fmt += 2;
        }
        /* Start format analysis if this is a conversion specification */
        else
        {
            enum
            {
                flag_plus = 000002,    /* '+' */
                flag_minus = 000010,   /* '-' */
                flag_l1 = 000040,      /* "l" */
                flag_h1 = 000100,      /* "h" */
                flag_l2 = 000200,      /* "ll" */
                flag_h2 = 000400,      /* "hh" */
                flag_unsigned = 010000, /* 'o', 'u', ... */
                flag_ignored = 020000, /* '*' */
                flag_end
            };
            u64     val = 0;
            int     flag = 0, width = 0, radix = 10;
            int     digit;

            /* Prohibit substitution flag */
            c = *++fmt;
            if (c == '*')
            {
                flag |= flag_ignored;
                c = *++fmt;
            }
            /* Specify width */
            while ((c >= '0') && (c <= '9'))
            {
                width = (width * 10) + c - '0';
                c = *++fmt;
            }
            /* Modify conversion */
            switch (c)
            {
            case 'h':
                c = *++fmt;
                if (c != 'h')
                {
                    flag |= flag_h1;
                }
                else
                    flag |= flag_h2, c = *++fmt;
                break;
            case 'l':
                c = *++fmt;
                if (c != 'l')
                    flag |= flag_l1;
                else
                    flag |= flag_l2, c = *++fmt;
                break;
            }
            /* Conversion type */
            switch (c)
            {
            case 'd':                 /* Signed decimal number */
                radix = 10;
                goto get_integer;
            case 'i':                 /* Signed base-8/10/16 number */
                radix = -1;
                goto get_integer;
            case 'o':                 /* Signed octal number */
                radix = 8;
                goto get_integer;
            case 'u':                 /* Unsigned decimal number */
                radix = 10;
                flag |= flag_unsigned;
                goto get_integer;
            case 'X':                 /* Unsigned hexadecimal number */
            case 'x':                 /* Unsigned hexadecimal number */
            case 'p':                 /* Pointer conversion (unsigned hexadecimal) */
                radix = 16;
                flag |= flag_unsigned;
                goto get_integer;
            case 's':                 /* Character string up to non-whitespace */
            case 'c':                 /* Character string of specified width only */
                goto store_string;

            case '[':                 /* Character class */
                goto store_char_class;

            case 'n':                 /* Store match location */
                /* Since this doesn't contribute to the substitution count, subtract it here. */
                if (!(flag & flag_ignored))
                {
                    --stored;
                }
                val = src - src_base;
                c = *++fmt;
                goto store_integer;
            }

//        invalid:
            /* Parsing ended in an unfortunate result */
            break;

          get_integer:
            /* Integer input processing */
            ++fmt;
            c = *src;
            /* Skip whitespace */
            while (STDi_IsSpace(c))
            {
                c = *++src;
            }
            /* Get sign */
            for (;; c = *++src)
            {
                if (c == '+')
                {
                    flag |= flag_plus;
                }
                else if (c == '-')
                {
                    /* '-' in an unsigned type is an error */
                    if ((flag & flag_unsigned) != 0)
                    {
                        /*
                         * ...would be a good way to do things, but since this is ignored with gcc and CW, the SDK currently follows their lead.
                         
                         
                         */
                        //goto invalid;
                    }
                    flag |= flag_minus;
                }
                else
                {
                    break;
                }
            }
            /* Automatic detection of integers (%i) */
            if (radix == -1)
            {
                if (c != '0')
                {
                    radix = 10;
                }
                else if ((src[1] == 'x') || (src[1] == 'X'))
                {
                    radix = 16;
                }
                else
                {
                    radix = 8;
                }
            }
            /* If hexadecimal, skip "^(0[xX])?" */
            if ((radix == 16) && (c == '0') && ((src[1] == 'x') || (src[1] == 'X')))
            {
                src += 2;
                c = *src;
            }
            /* Get integer */
            if (width == 0)
            {
                width = 0x7FFFFFFF;
            }
            for (digit = 0; digit < width; ++digit)
            {
                u32     d = (u32)(c - '0');
                if (d >= 10)
                {
                    d -= (u32)('a' - '0');
                    if (d < 6)
                    {
                        d += 10;
                    }
                    else
                    {
                        d -= (u32)('A' - 'a');
                        if (d < 6)
                        {
                            d += 10;
                        }
                    }
                }
                if (d >= radix)
                {
                    break;
                }
                c = *++src;
                val = val * radix + d;
            }
            /* No input is an error */
            if (digit == 0)
            {
                break;
            }
            /* Adjust sign */
            if (flag & flag_minus)
            {
                val = (u64)(val * -1);
            }
            /* At least conversion succeeded */
            matched = TRUE;
          store_integer:
            /* Substitute */
            if (!(flag & flag_ignored))
            {
                ++stored;
                if (flag & flag_h2)
                {
                    *va_arg(vlist, u8 *) = (u8)val;
                }
                else if (flag & flag_h1)
                {
                    *va_arg(vlist, u16 *) = (u16)val;
                }
                else if (flag & flag_l2)
                {
                    *va_arg(vlist, u64 *) = (u64)val;
                }
                else
                {
                    *va_arg(vlist, u32 *) = (u32)val;
                }
            }
            continue;

          store_string:
            /* Substitute character string */
            {
                char   *dst = NULL, *dst_bak = NULL;
                ++fmt;
                if (!(flag & flag_ignored))
                {
                    dst = va_arg(vlist, char *);
                    dst_bak = dst;
                }
                /*
                 * If s, search until you find whitespace and append '\0'
                 * If c, search for the specified width
                 */
                if (c == 's')
                {
                    if (width == 0)
                    {
                        width = 0x7FFFFFFF;
                    }
                    for (c = *src; STDi_IsSpace(c); c = *++src)
                    {
                    }
                    for (; c && !STDi_IsSpace(c) && (width > 0); --width, c = *++src)
                    {
                        if (dst)
                        {
                            *dst++ = c;
                        }
                    }
                    /*
                     * If [dst! = dst_bak], it is assumed that more than one character was substituted 
                     * Append NULL to the end, make [matched = TRUE], and ++ the substitute success number
                     */
                    if (dst != dst_bak)
                    {
                        *dst++ = '\0';
                        ++stored;
                        matched = TRUE;
                    }
                }
                else
                {
                    if (width == 0)
                    {
                        width = 1;
                    }
                    for (c = *src; c && (width > 0); --width, c = *++src)
                    {
                        if (dst)
                        {
                            *dst++ = c;
                        }
                    }
                    /*
                     * If [dst! = dst_bak], it is assumed that more than one character was substituted 
                     * Append NULL to the end, make [matched = TRUE], and ++ the substitute success number
                     */
                    if (dst != dst_bak)
                    {
                        ++stored;
                        matched = TRUE;
                        if(width > 0)
                        {
                            *dst++ = '\0';
                        }
                    }
                }
            }
            continue;

          store_char_class:
            ++fmt;
            /* Character class substitution processing */
            {
                char   *dst = NULL;

                u32     bitset[256 / (8 * sizeof(u32))];
                u32     matchcond = 1;
                u32     from = 0;
                BOOL    in_range = FALSE;
                MI_CpuFill32(bitset, 0x00000000UL, sizeof(bitset));
                if (*fmt == '^')
                {
                    matchcond = 0;
                    ++fmt;
                }
                /* Start-of-line escape */
                if (*fmt == ']')
                {
                    STDi_SetBitset(bitset, (u8)*fmt);
                    ++fmt;
                }
                /* Character class analysis */
                for (;; ++fmt)
                {
                    /* Terminus detection */
                    if (!*fmt || (*fmt == ']'))
                    {
                        /* A terminus whose range is being specified is treated as a simple character */
                        if (in_range)
                        {
                            STDi_SetBitset(bitset, from);
                            STDi_SetBitset(bitset, (u32)'-');
                        }
                        if (*fmt == ']')
                        {
                            ++fmt;
                        }
                        break;
                    }
                    /* Beginning of the simple character or specified range */
                    else if (!in_range)
                    {
                        /* Begin range specification */
                        if ((from != 0) && (*fmt == '-'))
                        {
                            in_range = TRUE;
                        }
                        else
                        {
                            STDi_SetBitset(bitset, (u8)*fmt);
                            from = (u8)*fmt;
                        }
                    }
                    /* End range specification */
                    else
                    {
                        u32     to = (u8)*fmt;
                        /* Illegal specified ranges are treated as separate specified characters */
                        if (from > to)
                        {
                            STDi_SetBitset(bitset, from);
                            STDi_SetBitset(bitset, (u32)'-');
                            STDi_SetBitset(bitset, to);
                        }
                        /* Also includes and collectively sets terminal characters */
                        else
                        {
                            STDi_FillBitset(bitset, from, to + 1UL);
                        }
                        in_range = FALSE;
                        from = 0;
                    }
                }
                /* Verification of character class and string */
                /* At least conversion has been successful to this point */
                matched = TRUE;
                if (!(flag & flag_ignored))
                {
                    ++stored;
                    dst = va_arg(vlist, char *);
                }
                if (width == 0)
                {
                    width = 0x7FFFFFFF;
                }
                for (c = *src; c && (width > 0); --width, c = *++src)
                {
                    if (STDi_GetBitset(bitset, (u8)c) != matchcond)
                    {
                        break;
                    }
                    if (dst)
                    {
                        *dst++ = c;
                    }
                }
                if (dst)
                {
                    *dst++ = '\0';
                }
            }
            continue;

        }
    }

    return (*src || matched) ? stored : -1;

}
