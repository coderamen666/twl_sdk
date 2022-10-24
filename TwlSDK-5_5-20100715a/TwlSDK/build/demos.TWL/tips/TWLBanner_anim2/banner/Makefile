#! make -f
#---------------------------------------------------------------------------
# Project:  TwlSDK - tips - TWLBanner_anim2 - banner
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
# $Date:: 2008-09-29#$
# $Rev: 8733 $
# $Author: ooshimay $
#---------------------------------------------------------------------------

TARGET_PLATFORM	 = TWL

include $(TWLSDK_ROOT)/build/buildtools/commondefs

BANNER_BASE		 = NBF
BANNER_BMP		 = $(wildcard $(BANNER_BASE)/*.bmp)
BANNER_SPEC		 = banner.bsf
BANNER_XML		 = $(BANNER_BASE)/sample.xml

TARGETS			 = $(BANNER_SPEC:.bsf=.bnr)

# ntexconv conversion

BANNER_ICON_NBF		= $(patsubst %.bmp, %.nbfs,$(BANNER_BMP))
BANNER_NBF_FILES	= $(patsubst %.bmp, %.nbfp,$(BANNER_BMP)) \
					  $(patsubst %.bmp, %.nbfc,$(BANNER_BMP))
NTEXCONV_FLAGS		= -f palette16

# bannercvtr conversion

BANNER_ICON_MIDDLE	= $(addprefix $(basename $(BANNER_SPEC)), .ban)
MAKEBANNER_FLAGS	= -A $(BANNER_ICON_MIDDLE)

BANNERCVTR_SRCS		= $(BANNER_XML)

LDIRT_CLEAN		 = $(TARGETS) \
				   $(BANNER_ICON_NBF) \
				   $(BANNER_ICON_MIDDLE) \
				   $(BANNER_NBF_FILES)

#----------------------------------------------------------------------------

do-build:		$(BANNER_ICON_NBF) $(BANNER_ICON_MIDDLE) $(TARGETS)

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
