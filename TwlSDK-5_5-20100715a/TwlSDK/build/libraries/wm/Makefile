#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - wm
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
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

TARGET_PLATFORM		:= 	TWL NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

#SUBDIRS    =   $(GSUBDIRS)

ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
ifeq    ($(TWLSDK_PRIVATE),TRUE)
SUBDIRS_P   +=  ARM9.TWL    # Could build in the environment with "buildtools/@privatedefs".
SUBDIRS_P   +=  ARM7.TWL
endif
endif

ifneq	($(filter NITRO,$(TARGET_PLATFORM_LIST)),)
SUBDIRS_P   +=  ARM9
ifeq    ($(TWLSDK_PRIVATE),TRUE)
SUBDIRS_P   +=  ARM7
endif
endif

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
