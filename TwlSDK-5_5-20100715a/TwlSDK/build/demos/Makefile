#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos
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
TARGET_PLATFORM	= TWL NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS			= gx/UnitTours/DEMOLib

SUBDIRS_P		= template	\
				  gx		\
				  pad		\
				  os		\
				  fx		\
				  card		\
				  fs		\
				  mi		\
				  spi		\
				  rtc		\
				  cp		\
				  prc		\
				  wm		\
				  wbt		\
				  mb		\
				  wvr		\
				  math		\
				  snd		\
				  env		\
				  std		\
				  cht		\
				  tips		\
				  crypto	\
				  ssp       \
				  wxc       \

SUBDIRS_P	+= $(PRIVATE_DEMOS)

ifneq	($(TWL_ARCHGEN),LIMITED)
SUBDIRS_P		+= ctrdg	\
				   vib
endif

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


