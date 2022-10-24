#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tips - banner - TWLBanner_anim1
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-26#$
# $Rev: 8716 $
# $Author: ooshimay $
#----------------------------------------------------------------------------

SUBDIRS			 = banner \
					$(ROOT)/build/demos/gx/UnitTours/DEMOLib

#-------------------------
#-- Specifies a NAND application build

TWL_NANDAPP		=	TRUE


#----------------------------------------------------------------------------
TARGET_PLATFORM	 = TWL
SRCS			 =	main.c
TARGET_BIN		 =	TWLBanner_anim1.tad

#-------------------------

ROM_SPEC			=	demo.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
