#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
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
# $Date:: 2008-11-14#$
# $Rev: 9315 $
# $Author: kitase_hirotake $
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL

TARGET_BIN		 =	main.srl
MAKEROM_ROMROOT  = $(TWLSDK_ROOT)/data/shutter_sound
MAKEROM_ROMFILES = *.wav

SRCS			 =	main.c snd_data.c

#SRCDIR			 =	# using default
#LCFILE			 =	# using default



include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
