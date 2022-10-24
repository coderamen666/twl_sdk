/*---------------------------------------------------------------------------*
  Project:  TwlSDK - - types definition
  File:     types.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_TYPES_H_
#define NITRO_TYPES_H_

// Because the SDK will not operate correctly if enum is not the same size as the int type
#ifdef __MWERKS__
#pragma enumsalwaysint on
#endif

// Endian
#define SDK_LITTLE_ENDIAN
#define SDK_IS_LITTLE_ENDIAN	1
#define SDK_IS_BIG_ENDIAN	0

#ifdef  SDK_ASM
#else  //SDK_ASM

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;

#ifdef SDK_HAS_NO_LONG_LONG_INT_
typedef unsigned __int64 u64;
#else
typedef unsigned long long int u64;
#endif

typedef signed char s8;
typedef signed short int s16;
typedef signed long s32;

#ifdef SDK_HAS_NO_LONG_LONG_INT_
typedef signed __int64 s64;
#else
typedef signed long long int s64;
#endif

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float f32;
typedef volatile f32 vf32;

/*
    Macro and types used with io_register_list_XX.h
 */

typedef u8 REGType8;
typedef u16 REGType16;
typedef u32 REGType32;
typedef u64 REGType64;

typedef vu8 REGType8v;
typedef vu16 REGType16v;
typedef vu32 REGType32v;
typedef vu64 REGType64v;


#ifndef SDK_BOOL_ALREADY_DEFINED_
#ifndef BOOL
typedef int BOOL;
#endif //BOOL
#endif //SDK_BOOL_ALREADY_DEFINED_

#ifndef TRUE
// Any non-zero value is considered TRUE
#define TRUE                    1
#endif //TRUE

#ifndef FALSE
#define FALSE                   0
#endif // FALSE


#ifndef NULL
#ifdef  __cplusplus
#define NULL                    0
#else  // __cplusplus
#define NULL                ((void *)0)
#endif // __cplusplus
#endif // NULL

// For compatibility with GameCube
#if     defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(num) __attribute__ ((aligned(num)))
#endif
#endif

// Weak symbol
#if     defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#define SDK_WEAK_SYMBOL  __declspec(weak)
#elif   defined(SDK_PRODG)
#define SDK_WEAK_SYMBOL
#endif

/* Option for the compiler that deals with dead-stripping */
#ifdef  SDK_CW_FORCE_EXPORT_SUPPORT
#define SDK_FORCE_EXPORT   __declspec(force_export)
#else
#define SDK_FORCE_EXPORT   
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif //SDK_ASM


/* When you want to make static inline into inline and reduce the code size, change the SDK_INLINE macro.
   (When there is a declaration that makes static inline into inline, also change SDK_DECL_INLINE.)
   Rebuild the library if changes are made. 
   
   However, using a version of the compiler in which the inline function instantiation bug has been fixed, code size can be reduced if this macro is used to change static inline to inline only in functions where static was changed to static inline to avoid the bug.
   
   
   
   Note: These macros are currently valid with only the FX_, VEC_, and MTX_ series.*/
#define SDK_INLINE      static inline
#define SDK_DECL_INLINE static


/* NITRO_TYPES_H_ */
#endif
