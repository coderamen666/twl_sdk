/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - utest
  File:	    utest.h

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef	 NITRO_UTEST_H__
#define	 NITRO_UTEST_H__
#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Library for unit function test
//
//  CAUTION
//     This library is an alpha version for internal testing. The specifications are currently in review.
//     Accordingly, the interface could change, and this feature itself could be deleted. Also, the library has not been fully confirmed to operate in all environments.
//      
//      
//
//     Related files
//              $NitroSDK/tools/bin/make_utest_main__
//              $NitroSDK/build/tools/nitload-gcc
//---------------------------------------------------------------------------
#ifdef	SDK_ARM9

#include <nitro/os/common/system.h>
#include <nitro/os/common/context.h>

//---------------------------------------------------------------------------
// Test module structure
//
typedef void (*UTiCallback) (void);

typedef struct UTiModule_
{
    char   *filename;
    char   *testname;
    UTiCallback callback;
    UTiCallback callback_begin;
    UTiCallback callback_end;
}
UTiModule;

//---------------------------------------------------------------------------
// Test result values
//
typedef enum UTiResult_
{
    UT_RESULT_OK = 1,
    UT_RESULT_NG = 0,
    UT_RESULT_UNTESTED = -1
}
UTiResult;

extern UTiResult UTi_Result;

static inline void UTi_SetResult(UTiResult r)
{
    UTi_Result = r;
}
static inline UTiResult UTi_GetResult(void)
{
    return UTi_Result;
}

//---------------------------------------------------------------------------
void    UTi_Main(int, UTiModule *);
void    UTi_Printf(const char *, ...);
void    UTi_PrintfWithFloat(const char *, ...);
void    UTi_PutString(const char *str);
void    UTi_ClearMessage(void);
char   *UTi_GetMessage(void);
void    UTi_SetMessage(const char *, int, const char *, ...);
void    UTi_SetMessageWithFloat(const char *, int, const char *, ...);

//---------------------------------------------------------------------------
#define UT_ASSERTMESSAGE_MAXLEN	  1024

void    UTi_Assert(const char *, int, const char *, int);
void    UTi_AssertEq(const char *, int, const char *, int, const char *, int);
void    UTi_AssertNe(const char *, int, const char *, int, const char *, int);

#define UT_Assert(ex)	     UTi_Assert(  __FILE__, __LINE__, #ex,  (int)(ex))
#define UT_AssertEq(ex1,ex2) UTi_AssertEq(__FILE__, __LINE__, #ex1, (int)(ex1), #ex2, (int)(ex2))
#define UT_AssertNe(ex1,ex2) UTi_AssertNe(__FILE__, __LINE__, #ex1, (int)(ex1), #ex2, (int)(ex2))

void    UTi_AssertMemEq(const char *file, int line,
                        const char *exp_str1, const void *exp_ptr1,
                        const char *exp_str2, const void *exp_ptr2, int exp_len);
void    UTi_AssertMemNe(const char *file, int line,
                        const char *exp_str1, const void *exp_ptr1,
                        const char *exp_str2, const void *exp_ptr2, int exp_len);

#define UT_AssertMemPtrEq(ex1,ex2,len) \
    UTi_AssertMemEq(__FILE__, __LINE__, #ex1, (const void*)(ex1), #ex2, (const void*)(ex2), (int)(len))
#define UT_AssertMemPtrNe(ex1,ex2,len) \
    UTi_AssertMemNe(__FILE__, __LINE__, #ex1, (const void*)(ex1), #ex2, (const void*)(ex2), (int)(len))
#define UT_AssertMemEq(ex1,ex2) \
    UTi_AssertMemEq(__FILE__, __LINE__, #ex1, (const void*)&(ex1), #ex2, (const void*)&(ex2), sizeof(ex1))
#define UT_AssertMemNe(ex1,ex2) \
    UTi_AssertMemNe(__FILE__, __LINE__, #ex1, (const void*)&(ex1), #ex2, (const void*)&(ex2), sizeof(ex1))

void    UTi_AssertAsserted(const char *, int, const char *, int);
void    UTi_AssertNotAsserted(const char *, int, const char *, int);

int     UTi_MemCmp(const void *ptr1, const void *ptr2, int len);

//---------------------------------------------------------------------------
// Context for LONG JUMP / LONG JUMP during ASSERT
//
#ifdef	SDK_DEBUG
extern OSContext UTi_AssertContext;
extern void (*OSi_FuncTerminate) (void);
void    UTi_Terminate(void);

#define UT_AssertAsserted(ex)					\
{								\
    int __n;							\
    OSi_FuncTerminate = UTi_Terminate;				\
    __n = OS_SaveContext( &UTi_AssertContext );			\
    if (!__n) (void)(ex);					\
    UTi_AssertAsserted(__FILE__, __LINE__, #ex, __n );		\
    OSi_FuncTerminate = OS_Terminate;				\
}
#define UT_AssertNotAsserted(ex)				\
{								\
    int __n;							\
    OSi_FuncTerminate = UTi_Terminate;				\
    __n = OS_SaveContext( &UTi_AssertContext );			\
    if (!__n) (void)(ex);					\
    UTi_AssertNotAsserted(__FILE__, __LINE__, #ex, __n );	\
    OSi_FuncTerminate = OS_Terminate;				\
}
#else
#define UT_AssertAsserted(ex)		((void)0)
#define UT_AssertNotAsserted(ex)	((void)0)
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#else  //SDK_ARM7

// UnitTest is NOT supported on ARM7 devenv.
#define UT_Assert(ex)			((void)0)
#define UT_AssertEq(ex1,ex2)		((void)0)
#define UT_AssertNe(ex1,ex2)		((void)0)
#define UT_AssertMemPtrEq(ex1,ex2,len)	((void)0)
#define UT_AssertMemPtrNe(ex1,ex2,len)	((void)0)
#define UT_AssertMemEq(ex1,ex2)		((void)0)
#define UT_AssertMemNe(ex1,ex2)		((void)0)
#define UT_AssertAsserted(ex)		((void)0)
#define UT_AssertNotAsserted(ex)	((void)0)

#endif //SDK_ARM9/SDK_ARM7
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif //NITRO_UTEST_H__
