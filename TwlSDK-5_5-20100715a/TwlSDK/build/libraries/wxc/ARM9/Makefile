#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - wxc
# File:     Makefile
#
# Copyright 2005-2009 Nintendo. All rights reserved.
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
override TARGET_PLATFORM = NITRO

NITRO_CODEGEN	 	?= ALL
NITRO_PROC		 = ARM9

SRCDIR			 = ./src

# Basic structure of WXC library itself (driver/scheduler/API layer)
SRCS			= wxc_common.c wxc_protocol.c wxc_driver.c wxc_scheduler.c wxc_api.c

# Automatically install all existing protocols
SRCS			+= $(wildcard $(SRCDIR)/wxc_protocol_impl_*.c)

TARGET_LIB		 = libwxc$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		 = $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
