#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - camera
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

SUBDIRS			 =

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

TWL_CODEGEN 	?= ALL
TWL_PROC		 = ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR			 = src ../common/src ../ARM9/src
INCDIR			 = ../common/include include

SRCS			 = camera_api.c \
				   camera.c \
				   camera_util.c


TARGET_LIB		 = libcamera$(TWL_LIBSUFFIX).a

# Implemented DEBUG-version library so DEBUG builds are built like RELEASE-version builds
# 

ifdef	TARGET_DEBUG
TWL_BUILD_TYPE	 = RELEASE
else
ifdef	TWL_DEBUG
TWL_BUILD_TYPE	 = RELEASE
else
ifdef	NITRO_DEBUG
TWL_BUILD_TYPE	 = RELEASE
endif
endif
endif

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs


INSTALL_TARGETS	 = $(TARGETS)

INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)


#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
