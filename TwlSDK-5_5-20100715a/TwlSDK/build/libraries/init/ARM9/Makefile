#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
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

override NITRO_CODEGEN	= ARM
NITRO_PROC		= ARM9

SRCDIR			= ../common/src src
INCDIR			= ../common/include

SRCS			= crt0.c
TARGET_OBJ		= crt0.o

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(NITRO_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
