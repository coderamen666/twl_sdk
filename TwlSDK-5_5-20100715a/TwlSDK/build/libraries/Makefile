#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries
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
TARGET_PLATFORM	=	TWL NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

export PARALLEL_BUILD=TRUE

#----------------------------------------------------------------------------

SUBDIRS_P	 = 	fx		\
				gx		\
				os		\
				mi		\
				pxi		\
				init	\
				ext		\
				pad		\
				exi		\
				fs		\
				cp		\
				spi		\
				rtc		\
				wm		\
				wbt		\
				wfs		\
				prc		\
				card	\
				mb		\
				math	\
				wvr		\
				snd		\
				std		\
				env		\
				ctrdg	\
				stubs	\
				wxc		\

SUBDIRS_P	+= $(PRIVATE_LIBRARIES)

ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
SUBDIRS_P	+= cdc camera i2c scfg mcu dsp el

SUBDIRS_P	+= $(PRIVATE_LIBRARIES_TWL)

endif

#----------------------------------------------------------------------------
export	NITRO_BLXCHECKED = yes

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
