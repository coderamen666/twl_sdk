#! make -f
#---------------------------------------------------------------------------
# Project:  TwlSDK - tips - TWLBanner_anim1 - banner
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
# $Date:: 2008-12-04#$
# $Rev: 9509 $
# $Author: ooshimay $
#---------------------------------------------------------------------------

TARGET_PLATFORM	 = TWL

include $(TWLSDK_ROOT)/build/buildtools/commondefs

BANNER_SPEC		 = banner.bsf
BANNER_BASE		 = SDK_demo

TARGETS			 = $(BANNER_SPEC:.bsf=.bnr)

BANNER_ICON_NAME = $(basename $(BANNER_SPEC))
BANNER_ICON_MIDDLE	= $(addprefix $(BANNER_ICON_NAME), .ban)
MAKEBANNER_FLAGS	= -A $(BANNER_ICON_MIDDLE)
BANNERCVTR_SRCS	  = $(BANNER_BASE)/Cell/$(BANNER_BASE).nce

LDIRT_CLEAN		 = $(TARGETS) \
				   $(BANNER_ICON_MIDDLE) \

#----------------------------------------------------------------------------

do-build:		$(BANNER_ICON_MIDDLE) $(TARGETS)

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
