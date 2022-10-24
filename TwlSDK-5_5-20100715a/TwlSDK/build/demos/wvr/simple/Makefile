#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - wvr - simple
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
# $Date:: 2009-06-24#$
# $Rev: 10808 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM :=  NITRO TWL
TWL_ARCHGEN		:=	HYBRID

SRCS        =   main.c font.c

LLIBRARIES  =   libwvr$(TWL_LIBSUFFIX).a

# Use the WH sample module
WH_DIR      =   $(ROOT)/build/demos/wireless_shared/wh
SRCS        +=  $(WH_DIR)/wh.c
LINCLUDES   =   $(WH_DIR)

#------------------------------------------------------#
# Component definition where the wireless library is positioned inside VRAM #

DEFAULT_COMP_ARM7	=	ichneumon

#------------------------------------------------------#

TARGET_BIN  =   main.srl

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

# ichneumon.TWL is included only in the CWforDSi version because of its size. Therefore, do not build demos that use it.
ifneq	($(SDK_CW_BUILD_VERSION_CC),)
ifeq	($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" "<=" 3.0),1)
TARGET_PLATFORM_LIST		:= $(filter-out TWL, $(TARGET_PLATFORM_LIST))
endif
endif

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
