#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - stubs - isd/ARM7
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
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

SUBDIRS			=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN 	?= ALL
NITRO_PROC		= ARM7

SRCDIR			= ../common/src

SRCS			= stubs_isd.c

TARGET_LIB		= libstubsisd_sp$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
# If it is a DEBUG-version build, build it like a RELEASE-version build and give it the DEBUG-version libraries
# 
ifdef	TARGET_DEBUG
NITRO_BUILD_TYPE	 = RELEASE
else
ifdef	NITRO_DEBUG
NITRO_BUILD_TYPE	 = RELEASE
else
ifdef	TWL_DEBUG
NITRO_BUILD_TYPE	 = RELEASE
endif
endif
endif

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	= $(TARGETS)
INSTALL_DIR		= $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
