#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL - DSP - g711
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
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM    := TWL
TWL_PROC           := ARM9

SRCS                = main.c

# Reference the SND library's sample audio source files
TARGET_BIN          = main.srl
MAKEROM_ROMROOT     = $(ROOT)/build/demos/snd/stream/files
MAKEROM_ROMFILES    = fanfare.32.wav

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
