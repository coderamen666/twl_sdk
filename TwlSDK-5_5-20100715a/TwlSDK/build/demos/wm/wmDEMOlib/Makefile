#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - WM - demos - wmDEMOlib
# File:     Makefile
#
# Copyright 2004-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-12-24#$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			=


#----------------------------------------------------------------------------
TARGET_PLATFORM	=	TWL NITRO

SRCS			=	wm_lib.c wm_tool.c

TARGET_LIB		=	libwmDEMO$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifdef	TWLSDK_INSTALL_ROOT
INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR		=	$(TWL_INSTALL_ROOT)/build/demos/wm/wmDEMOlib/lib/$(TWL_BUILDTYPE)
endif # TWLSDK_INSTALL_ROOT

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
