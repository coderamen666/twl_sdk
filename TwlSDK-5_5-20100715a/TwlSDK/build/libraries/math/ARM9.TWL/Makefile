#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - math
# File:     Makefile
#
# Copyright 2003-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			 =


#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

# Build ARM & THUMB libraries
TWL_CODEGEN		?= ALL
TWL_PROC		 = ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

# SHA-256 uses the C version
# override TWLSDK_MATH_NOT_USE_SHA256_ASM = TRUE

LINCLUDES		 = ../common/include
SRCDIR			 = ../common/src src

SRCS			 = math.c 			\
				   dgt_md5.c		\
				   dgt_sha1.c		\
				   dgt_hmac.c		\
				   crc.c			\
				   checksum.c		\
				   qsort.c			\
				   fft.c			\

ifneq	($(TWLSDK_MATH_NOT_USE_SHA1_ASM),TRUE)
SRCS                    += asm/sha1.s
MACRO_FLAGS             += -DMATH_SHA1_ASM
endif

TARGET_LIB		 = libmath$(TWL_LIBSUFFIX).a


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#SRCS			+=	dgt_sha256.c
SRCS			+=	net_sha256.c

ifneq	($(TWLSDK_MATH_NOT_USE_SHA256_ASM),TRUE)
SRCS			+=	asm/sha256.s
MACRO_FLAGS             +=      -DMATH_SHA256_ASM
endif

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
