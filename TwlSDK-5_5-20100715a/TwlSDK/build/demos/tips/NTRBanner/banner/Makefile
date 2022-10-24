#! make -f
#---------------------------------------------------------------------------
# Project:  TwlSDK - tips - NTRBanner - banner
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
# $Date:: 2008-09-26#$
# $Rev: 8700 $
# $Author: ooshimay $
#---------------------------------------------------------------------------

TARGET_PLATFORM	 = NITRO

include $(TWLSDK_ROOT)/build/buildtools/commondefs

BANNER_ICON		 = banner.bmp
BANNER_SPEC		 = banner.bsf

TARGETS			 = $(BANNER_SPEC:.bsf=.bnr)

BANNER_ICON_NAME = $(basename $(BANNER_ICON))
BANNER_ICON_MIDDLE	= $(addprefix $(BANNER_ICON_NAME), .nbfs .nbfc .nbfp)
MAKEBANNER_FLAGS	= -N $(BANNER_ICON_NAME)

LDIRT_CLEAN		 = $(TARGETS) \
				   $(BANNER_ICON_MIDDLE) \

#----------------------------------------------------------------------------

do-build:		$(BANNER_ICON_MIDDLE) $(TARGETS)

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
