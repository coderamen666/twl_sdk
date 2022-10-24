#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tips - NTRBanner
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
# $Date:: 2008-09-26#$
# $Rev: 8700 $
# $Author: ooshimay $
#----------------------------------------------------------------------------

TARGET_PLATFORM	:= NITRO

SUBDIRS			 = banner \
					$(ROOT)/build/demos/gx/UnitTours/DEMOLib

#----------------------------------------------------------------------------

SRCS			 =	main.c
TARGET_BIN		 =	main.srl

#-------------------------

ROM_SPEC		 =	ROM-TS.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
