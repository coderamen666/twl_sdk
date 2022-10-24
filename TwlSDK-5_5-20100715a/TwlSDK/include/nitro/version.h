#ifndef	TWLSDK_VERSION_H_
#define	TWLSDK_VERSION_H_
#define	SDK_VERSION_DATE		2020111209
#define	SDK_VERSION_TIME		312
#define	SDK_VERSION_MAJOR		5
#define	SDK_VERSION_MINOR		5
#define	SDK_VERSION_RELSTEP		30005
#define	SDK_BUILDVER_CW_CC		4.0
#define	SDK_BUILDVER_CW_LD		2.0
#define	SDK_BUILDNUM_CW_CC		1028
#define	SDK_BUILDNUM_CW_LD		99
#define	SDK_BUILDVER_CC			4.0
#define	SDK_BUILDVER_LD			2.0
#define	SDK_BUILDNUM_CC			1028
#define	SDK_BUILDNUM_LD			99
#if 0	// for Makefile
TWL_VERSION_DATE_AND_TIME	=	20111209 0312
TWL_VERSION_DATE				=	2020111209
TWL_VERSION_TIME				=	312
TWL_VERSION_MAJOR			=	5
TWL_VERSION_MINOR			=	5
TWL_VERSION_RELSTEP			=	30005
TWL_VERSION_BUILDVER_CW_CC	=	4.0
TWL_VERSION_BUILDVER_CW_LD	=	2.0
TWL_VERSION_BUILDNUM_CW_CC	=	1028
TWL_VERSION_BUILDNUM_CW_LD	=	99
TWL_VERSION_BUILDVER_CC		=	4.0
TWL_VERSION_BUILDVER_LD		=	2.0
TWL_VERSION_BUILDNUM_CC		=	1028
TWL_VERSION_BUILDNUM_LD		=	99
#
#  RELSTEP PR1=10100 PR2=10200 ...
#          RC1=20100 RC2=20200 ...
#          RELEASE=30000
#
#endif

#ifndef SDK_VERSION_NUMBER
#define SDK_VERSION_NUMBER(major, minor, relstep) \
(((major) << 24) | ((minor) << 16) | ((relstep) << 0))
#define SDK_CURRENT_VERSION_NUMBER \
SDK_VERSION_NUMBER(SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_RELSTEP)
#endif

#endif