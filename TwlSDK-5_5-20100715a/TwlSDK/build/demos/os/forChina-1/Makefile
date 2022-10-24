#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - OS - forChina-1
# File:     Makefile
#
# Copyright 2005-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-05-28#$
# $Rev: 10640 $
# $Author: mizutani_nakaba $
#----------------------------------------------------------------------------
TARGET_PLATFORM	    = TWL NITRO

SUBDIRS             =

#----------------------------------------------------------------------------

TARGET_BIN          = main.srl
SRCS                = main.c

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC            = forChina.TWL.rsf
else
ROM_SPEC            = forChina.rsf
endif

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
