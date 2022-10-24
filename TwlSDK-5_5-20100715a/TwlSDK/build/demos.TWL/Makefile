#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	:=	TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
SUBDIRS		 = ../demos/gx/UnitTours/DEMOLib

SUBDIRS_P	 =			\
				camera	\
				os		\
				mi		\
				spi		\
				snd		\
				dsp     \
				aes		\
				el		\
				crypto	\
				nandApp	\
				tips	\
				tcl		\

SUBDIRS_P	+= $(PRIVATE_DEMOS_TWL)

endif	# ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


