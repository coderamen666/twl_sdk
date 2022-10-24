#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - FS - overlay
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
TARGET_PLATFORM	 = TWL NITRO

SUBDIRS          =

BASEDIR          = $(ROOT)/build/demos/fs/overlay
SRCDIR           = $(BASEDIR)/src
INCDIR           = $(BASEDIR)/include

#----------------------------------------------------------------------------

TARGET_BIN       = main.srl

SRCS             = main.c
SRCS_OVERLAY     = func_1.c func_2.c func_3.c

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

ifeq ($(TARGET_PLATFORM),TWL)
LCFILE_SPEC      = $(BASEDIR)/ARM9.TWL-TS.lsf
else
LCFILE_SPEC      = $(BASEDIR)/ARM9-TS.lsf
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
