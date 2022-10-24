/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_argument.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-17#$
  $Rev: 10762 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/std/string.h>
#ifdef SDK_TWL
#include <twl/os/common/systemWork.h>
#endif

#define OSi_INIVALUE    0xffff

#ifndef OS_NO_ARGUMENT

#include <nitro/version_begin.h>
//---- This area is for argument string, and may be
//     modified from external tools.
static OSArgumentBuffer OSi_ArgumentBuffer = {
    OS_ARGUMENT_ID_STRING,
    OS_ARGUMENT_BUFFER_SIZE,
    {'\0', '\0'},
};
#include <nitro/version_end.h>

//---- pointer to current argument buffer
const char *sCurrentArgBuffer = &OSi_ArgumentBuffer.buffer[0];

static const char *match_opt(int optchar, const char *optstring);
#endif //ifndef OS_NO_ARGUMENT

//---- for TWL
#ifdef SDK_TWL
static u32                  sBufferState = OS_DELIVER_ARG_BUF_INVALID;
static OSDeliverArgInfo*    sBufferPtr = NULL;
static u8*                  sPtr;
static OSTitleId            sEncodedTitleId = 0;
#endif

static int OSi_GetArgc_core(const char* p);
static const char *OSi_GetArgv_core(const char *p, int n);


/*---------------------------------------------------------------------------*
  Name:         ( OSi_GetArgc_core )

  Description:  Get number of valid arguments.

  Arguments:    p: pointer to argument buffer

  Returns:      number of valid arguments.
 *---------------------------------------------------------------------------*/
static int OSi_GetArgc_core(const char* p)
{
    int     n = 0;
    for (; *p; p++, n++)
    {
        while (*p)
        {
            p++;
        }
    }
    return n;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgc

  Description:  Get number of valid arguments.
                This function is for debug.

  Arguments:    None.

  Returns:      number of valid arguments.
                return 1 if no argument.
                return 0 if not set argument buffer.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGC_AND_ARGV
int OS_GetArgc(void)
{
	return OSi_GetArgc_core( (const char *)sCurrentArgBuffer );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         ( OSi_GetArgv_core )

  Description:  Get the pointer to the specified argument string.

  Arguments:    n: index of argument. n==1 means the first argument.

  Returns:      n must less than value of OS_GetArgc()
 *---------------------------------------------------------------------------*/
static const char *OSi_GetArgv_core(const char *p, int n)
{
    SDK_ASSERT(n >= 0);
	for (; *p && n > 0; p++, n--)
	{
		while (*p)
		{
			p++;
		}
    }
    return (*p) ? p : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgv

  Description:  Get the pointer to the specified argument string.
                This function is for debug.

  Arguments:    n: index of argument. n==1 means the first argument.
                    n must less than value of OS_GetArgc()

  Returns:      pointer to the specified argument string
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGC_AND_ARGV
const char* OS_GetArgv(int n)
{
	return OSi_GetArgv_core( (const char *)sCurrentArgBuffer, n );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_GetOpt

  Description:  getopt() like function to get command line options
  
  Arguments:    optstring: Option character string
                           Internal parameters are reset if NULL.

  Returns:      Option character code.
                '?' Indicates the option character code is unclear.
                If -1, the option does not exist
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
const char *OSi_OptArg = NULL;
int     OSi_OptInd = 1;
int     OSi_OptOpt = 0;

int OS_GetOpt(const char *optstring)
{
    static BOOL end_of_option = FALSE;
    int     optchar;
    const char *arg;
    const char *opt;
    const char *optarg;

    OSi_OptArg = NULL;
    OSi_OptOpt = 0;

    // Reset if optstring is NULL
    if (optstring == NULL)
    {
        OSi_OptInd = 1;
        end_of_option = FALSE;
        return 0;
    }

    // Get command line arguments
    arg = OS_GetArgv(OSi_OptInd);

    if (arg == NULL)
    {
        return -1;
    }

    if (optstring[0] == '-')           // Minus Mode
    {
        OSi_OptInd++;                  // Consume arguments

        // Normal argument when it does not start with '-'
        if (end_of_option || arg[0] != '-')
        {
            OSi_OptArg = arg;          // Normal arguments are also set to OptArg.
            return 1;
        }

        // Option analysis
        optchar = arg[1];

        if (optchar == '-')            // End of options when '--' is encountered
        {
            end_of_option = TRUE;      // Normal arguments from here on
            return OS_GetOpt(optstring);
        }
    }
    else                               // normal mode
    {
        // Normal argument when it does not start with '-'
        if (end_of_option || arg[0] != '-')
        {
            return -1;                 // OptArg remains NULL
        }

        OSi_OptInd++;                  // Consume arguments

        // Option analysis
        optchar = arg[1];

        if (optchar == '-')            // End of options when '--' is encountered
        {
            end_of_option = TRUE;      // Normal arguments from here on
            return -1;
        }

    }

    opt = match_opt(optchar, optstring);

    if (opt == NULL)
    {
        OSi_OptOpt = optchar;          // Unknown option
        return '?';
    }

    if (opt[1] == ':')                 // OptArg search specification?
    {
        optarg = OS_GetArgv(OSi_OptInd);

        if (optarg == NULL || optarg[0] == '-')
        {
            if (opt[2] != ':')         // '::' Or not?
            {
                OSi_OptOpt = optchar;  // OptArg not present.
                return '?';
            }
            // OptArg not present.
            // OSi_OptArg = NULL
        }
        else
        {
            OSi_OptArg = optarg;
            OSi_OptInd++;              // Consume arguments
        }
    }
    return optchar;
}

static const char *match_opt(int optchar, const char *optstring)
{
    if (optstring[0] == '-' || optstring[0] == '+')
    {
        optstring++;
    }

    if (optchar != ':')
    {
        while (*optstring)
        {
            if (optchar == *optstring)
            {
                return optstring;
            }
            optstring++;
        }
    }
    return NULL;
}
#endif // ifndef OS_NO_ARGUMENT

/*---------------------------------------------------------------------------*
  Name:         OS_ConvertToArguments

  Description:  convert string data to arg binary
  
  Arguments:    str : string
                cs      : character to separate
                buffer: buffer to store
                bufSize: max buffer size
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
void OS_ConvertToArguments(const char *str, char cs, char *buffer, u32 bufSize)
{
    char   *p = buffer;
    char   *pEnd = buffer + bufSize;
    BOOL    isQuoted = FALSE;

    while (1)
    {
        //---- skip separator
        while (*str == cs && p < pEnd)
        {
            str++;
        }

        //---- store argument string
        while (*str && p < pEnd)
        {
            //---- check quote
            if (*str == '\"')
            {
                isQuoted = (isQuoted == FALSE);
                str++;
                continue;
            }

            //---- found separator
            else if (*str == cs && isQuoted == FALSE)
            {
                break;
            }

            *p++ = *str++;
        }

        //---- 1) reached to the buffer end
        if (p >= pEnd)
        {
            *(pEnd - 2) = '\0';
            *(pEnd - 1) = '\0';
            break;
        }

        //---- 2) reached to the string end
        if (*str == '\0')
        {
            while ((p + 1) >= pEnd)
            {
                p--;
            }
            *p++ = '\0';
            *p = '\0';
            break;
        }

        //---- 3) separator
        if (*str == cs)
        {
            *p++ = '\0';
        }
    }

    //---- end mark
    if (p < pEnd)
    {
        *p++ = '\0';
    }
}
#endif //ifndef OS_NO_ARGUMENT

/*---------------------------------------------------------------------------*
  Name:         OS_SetArgumentBuffer

  Description:  force to set argument buffer.
  
  Arguments:    buffer: argument buffer
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
void OS_SetArgumentBuffer(const char *buffer)
{
    sCurrentArgBuffer = buffer;
}
#endif //ifndef OS_NO_ARGUMENT

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgumentBuffer

  Description:  get pointer to argument buffer.
  
  Arguments:    None.
 
  Returns:      pointer to argument buffer.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
const char *OS_GetArgumentBuffer(void)
{
    return sCurrentArgBuffer;
}
#endif //ifndef OS_NO_ARGUMENT


//================================================================================
// for TWL
//================================================================================
#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         OS_InitDeliverArgInfo

  Description:  initialize the argument delivery system for TWL
  
  Arguments:    binSize : buffer size for binary area.
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_InitDeliverArgInfo( OSDeliverArgInfo* info, int binSize )
{
    SDK_ASSERT( 2<=(OS_DELIVER_ARG_BUFFER_SIZE-binSize) && binSize>=0);
    SDK_ASSERT( info != NULL );

    sBufferPtr = info;

    //---- clear whole buffer
    MI_CpuClear8( sBufferPtr, HW_PARAM_DELIVER_ARG_SIZE );

    //---- set parameters
    sBufferState = OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE;
    sBufferPtr->header.argBufferSize = (u16)(OS_DELIVER_ARG_BUFFER_SIZE - binSize);
    sBufferPtr->header.binarySize = 0;

    //---- init pointer
    sPtr = &sBufferPtr->buf[0];
    *sPtr = '\0';
    *(sPtr+1) = '\0';
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetStringToDeliverArg

  Description:  write specified string as argument data
  
  Arguments:    str : string to write
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY ... failed. (buffer is not writable)
 *---------------------------------------------------------------------------*/
int OS_SetStringToDeliverArg( const char* str )
{
    int length;

    //---- check buffer state
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    //---- check buffer over
    length = STD_StrLen( str );

    if ( length > sBufferPtr->header.argBufferSize - (sPtr - &sBufferPtr->buf[0]) - 2)
    {
        return OS_DELIVER_ARG_OVER_SIZE;
    }

    //---- copy string
    (void)STD_StrCpy( (char*)sPtr, str );
    sPtr += length;
    *sPtr++ = '\0';
    *sPtr = '\0';

    //---- return result
    return OS_DELIVER_ARG_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         OS_SetBinaryToDeliverArg

  Description:  write specified binary 
  
  Arguments:    bin  : pointer for binary
                size: binary size
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY ... failed. (buffer is not writable)
 *---------------------------------------------------------------------------*/
int OS_SetBinaryToDeliverArg( const void* bin, int size )
{
    u8* dest = &sBufferPtr->buf[ sBufferPtr->header.argBufferSize + sBufferPtr->header.binarySize ];

    //---- check buffer state
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    //---- check buffer over
    if ( size > OS_DELIVER_ARG_BUFFER_SIZE - sBufferPtr->header.argBufferSize - sBufferPtr->header.binarySize )
    {
        return OS_DELIVER_ARG_OVER_SIZE;
    }

    //---- write binary data to buffer
    MI_CpuCopy8( bin, dest, (u32)size );
    sBufferPtr->header.binarySize += (u16)size;

    //---- return result
    return OS_DELIVER_ARG_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ConvertStringToDeliverArg

  Description:  Coverts string to arguments
  
  Arguments:    str : string
                cs      : character to separate
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success 
                OS_DELIVER_ARG_OVER_SIZE ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY ... failed. (buffer is not writable)
 *---------------------------------------------------------------------------*/
int OS_ConvertStringToDeliverArg(const char *str, char cs)
{
    int length;
    const char* endp;

    //---- check buffer state
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    while (1)
    {
        //---- skip separator
        while ( *str == cs )
        {
            str++;
        }

        //---- check string end
        if ( ! *str )
        {
            break;

        }

        //---- search separator
        endp = str;
        while ( *endp != cs && *endp != '\0' )
        {
            endp++;
        }
        length = endp - str;

        //---- check buffer over
        if ( length > sBufferPtr->header.argBufferSize - (sPtr - &sBufferPtr->buf[0]) - 2)
        {
            return OS_DELIVER_ARG_OVER_SIZE;
        }

        //---- copy string
        while( str != endp )
        {
            *sPtr ++ = *(u8*)str ++;
        }
        *sPtr ++ = '\0';
        *sPtr = '\0';
    }

    return OS_DELIVER_ARG_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_EncodeDeliverArg

  Description:  Encodes argument buffer.
  
  Arguments:    None.
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success 
                OS_DELIVER_ARG_NOT_READY  ... failed. (buffer is not accesible)
 *---------------------------------------------------------------------------*/
int OS_EncodeDeliverArg(void)
{
    //---- check buffer state
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }
    
    //---- set makerCode, titleId
    sBufferPtr->header.makerCode = OS_GetMakerCode();
    sBufferPtr->header.titleId = OS_GetTitleId();

    //---- set flag
    sBufferPtr->header.flag = OS_DELIVER_ARG_ENCODE_FLAG | OS_DELIVER_ARG_VALID_FLAG;

    //---- set encoded titleId
    sEncodedTitleId = sBufferPtr->header.titleId;

    //---- calculate crc
    sBufferPtr->header.crc = 0;
    sBufferPtr->header.crc = SVC_GetCRC16( OSi_INIVALUE, (const void*)sBufferPtr, sizeof(OSDeliverArgInfo) );

    //---- copy to HW_PARAM_DELIVER_ARG
    MI_CpuCopy8( sBufferPtr, (void*)HW_PARAM_DELIVER_ARG, sizeof(OSDeliverArgInfo) );

    //---- clearArea
    MI_CpuClear8( sBufferPtr, sizeof(OSDeliverArgInfo) );

    //---- buffer state
    sBufferState = OS_DELIVER_ARG_BUF_INVALID;

    return OS_DELIVER_ARG_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_DecodeDeliverArg

  Description:  Decodes to work buffer from HW_PARAM_DELIVER_ARG
  
  Arguments:    None.
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success 
                OS_DELIVER_ARG_NOT_READY  ... not ready
                OS_DELIVER_ARG_INVALID    ... invalid CRC check
 *---------------------------------------------------------------------------*/
int OS_DecodeDeliverArg(void)
{
    //----check if deliver arg contains valid data
    if ( !OS_IsValidDeliverArg() )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }
    
    //----check buffer
    if ( ! sBufferPtr )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    //---- copy from HW_PARAM_DELIVER_ARG
    MI_CpuCopy8( (void*)HW_PARAM_DELIVER_ARG, sBufferPtr, sizeof(OSDeliverArgInfo) );

    //---- crc check
    {
        u16 crc = sBufferPtr->header.crc;
        sBufferPtr->header.crc = 0;
        sBufferPtr->header.crc = SVC_GetCRC16( OSi_INIVALUE, (const void*)sBufferPtr, sizeof(OSDeliverArgInfo) );
        if ( crc != sBufferPtr->header.crc )
        {
            MI_CpuClear8( sBufferPtr, sizeof(OSDeliverArgInfo) );
            OS_SetDeliverArgStateInvalid();
            return OS_DELIVER_ARG_BUF_INVALID;
        }
    }

    //---- unset encode flag
    sBufferPtr->header.flag &= ~OS_DELIVER_ARG_ENCODE_FLAG;

    //---- buffer state
    sBufferState = OS_DELIVER_ARG_BUF_ACCESSIBLE;

    return OS_DELIVER_ARG_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgState

  Description:  Gets deliver arg state
  
  Arguments:    None.
 
  Returns:      state.
                one of the following:
                  OS_DELIVER_ARG_BUF_INVALID
                  OS_DELIVER_ARG_BUF_ACCESSIBLE
                  OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE
 *---------------------------------------------------------------------------*/
u32 OS_GetDeliverArgState(void)
{
    return sBufferState;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetDeliverArgState

  Description:  Sets deliver arg state
  
  Arguments:    state.
                one of the following:
                  OS_DELIVER_ARG_BUF_INVALID
                  OS_DELIVER_ARG_BUF_ACCESSIBLE
                  OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_SetDeliverArgState(u32 state)
{
    sBufferState = state;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetDeliverArgStateInvalid

  Description:  Sets OS_DELIVER_ARG_BUF_INVALID to deliver arg state
  
  Arguments:    None.
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_SetDeliverArgStateInvalid( void )
{
    sBufferState = OS_DELIVER_ARG_BUF_INVALID;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetBinarySizeFromDeliverArg

  Description:  Reads binary size
  
  Arguments:    None.
 
  Returns:      0> ... size
                -1 ... work area not ready
 *---------------------------------------------------------------------------*/
int OS_GetBinarySizeFromDeliverArg(void)
{
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE) )
    {
        return -1;
    }

    return sBufferPtr->header.binarySize;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetBinaryFromDeliverArg

  Description:  Reads binary data
  
  Arguments:    buffer: pointer for binary
                size: pointer for binary size
                maxSize : Max size
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... success but buffer over.
                OS_DELIVER_ARG_NOT_READY ... 
 *---------------------------------------------------------------------------*/
int OS_GetBinaryFromDeliverArg( void* buffer, int* size, int maxSize )
{
    int retval;
    int copySize;

    //---- check buffer state
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    //---- check buffer over
    if ( maxSize < sBufferPtr->header.binarySize )
    {
        retval = OS_DELIVER_ARG_OVER_SIZE;
        copySize = maxSize;
    }
    else
    {
        retval = OS_DELIVER_ARG_SUCCESS;
        copySize = sBufferPtr->header.binarySize;
    }

    //---- read binary data
    MI_CpuCopy8( &sBufferPtr->buf[sBufferPtr->header.argBufferSize], buffer, (u32)copySize );
    if ( size )
    {
        *size = copySize;
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleIdFromDeliverArg

  Description:  Gets the title ID
  
  Arguments:    None.
 
  Returns:      title ID.
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
OSTitleId OS_GetTitleIdFromDeliverArg( void )
{
    return (sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)? sBufferPtr->header.titleId: 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetGameCodeFromDeliverArg

  Description:  Gets the game code
  
  Arguments:    None.
 
  Returns:      game code
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
u32 OS_GetGameCodeFromDeliverArg( void )
{
    return (sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)? *(u32*)(&sBufferPtr->header.titleId): 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMakerCodeFromDeliverArg

  Description:  Gets the maker code
  
  Arguments:    None.
 
  Returns:      maker code
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
u16 OS_GetMakerCodeFromDeliverArg( void )
{
    return (u16)((sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)? sBufferPtr->header.makerCode: 0);
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsValidDeliverArg

  Description:  Checks if deliver arg contains valid data
  
  Arguments:    None.
 
  Returns:      TRUE if valid
                FALSE if not valid
 *---------------------------------------------------------------------------*/
BOOL OS_IsValidDeliverArg( void )
{
    // TODO: a more reliable verification
    OSDeliverArgInfo* p = (OSDeliverArgInfo*)HW_PARAM_DELIVER_ARG;

    return (p->header.flag & OS_DELIVER_ARG_VALID_FLAG) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsDeliverArgEncoded

  Description:  Gets titleId that is contained in last encoded deliverArg
  
  Arguments:    None.
 
  Returns:      TRUE  if deliverArg has been encoded
                FALSE if deliverArg has never been encoded since application was
                launched
 *---------------------------------------------------------------------------*/
BOOL OS_IsDeliverArgEncoded( void )
{
    OSDeliverArgInfo* p = (OSDeliverArgInfo*)HW_PARAM_DELIVER_ARG;
    return (p->header.flag & (OS_DELIVER_ARG_ENCODE_FLAG | OS_DELIVER_ARG_VALID_FLAG)) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleIdLastEncoded

  Description:  Gets titleId that is contained in last encoded deliverArg
  
  Arguments:    None.
 
  Returns:      TitleID if deliverArg has been encoded
                0 if deliverArg has never been encoded since application was
                launched
 *---------------------------------------------------------------------------*/
OSTitleId OS_GetTitleIdLastEncoded( void )
{
	return sEncodedTitleId;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetSysParamToDeliverArg

  Description:  set sysParam to deliverArg
  
  Arguments:    param: parameter to set
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
int OS_SetSysParamToDeliverArg( u16 param )
{
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE) )
    {
        return OS_DELIVER_ARG_NOT_READY;
    }

    sBufferPtr->header.sysParam = param;
    return OS_DELIVER_ARG_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetSysParamFromDeliverArg

  Description:  Gets sysParam from deliverArg
  
  Arguments:    None.
 
  Returns:      parameter. 0 if FAIL.
 *---------------------------------------------------------------------------*/
u16 OS_GetSysParamFromDeliverArg( void )
{
    if ( !(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE) )
    {
        return 0;
    }

    return sBufferPtr->header.sysParam;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgc

  Description:  GEts number of valid arguments in deliverArg.

  Arguments:    None.

  Returns:      number of valid arguments.
                return 1 if no argument.
                return 0 if buffer is not prepared.
 *---------------------------------------------------------------------------*/
int OS_GetDeliverArgc(void)
{
    if ( sBufferPtr && sBufferState == OS_DELIVER_ARG_BUF_ACCESSIBLE )
    {
		return OSi_GetArgc_core( (const char *)sBufferPtr->buf ) + 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgv

  Description:  Gets the pointer to the specified argument string in deliverArg.

  Arguments:    n: index of argument. n==1 means the first argument.
                    n must be less than value of the OS_GetDeliverArgc function

  Returns:      pointer to string
 *---------------------------------------------------------------------------*/
const char *OS_GetDeliverArgv(int n)
{
    static const char *procName = "main";
    SDK_ASSERT(n >= 0);

    if ( sBufferPtr && sBufferState == OS_DELIVER_ARG_BUF_ACCESSIBLE )
    {   
		return (n==0)? procName: OSi_GetArgv_core( (const char *)sBufferPtr->buf, n-1 );
    }

    return NULL;
}
#endif //ifdef SDK_TWL
