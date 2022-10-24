#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - CARD - card-mrom
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
# $Date:: 2009-07-03#$
# $Rev: 10859 $
# $Author: yosizaki $
#----------------------------------------------------------------------------

SUBDIRS          =

#----------------------------------------------------------------------------
TARGET_PLATFORM := NITRO

TARGET_BIN       = main.srl

SRCS             = main.c
SRCDIR           = ./src

# If you explicitly specify NITRO_ROMSPEED, it will be applied to the default RSF file.
# If there is an independently prepared RSF file, use "{1TROM|MROM}" to describe RomSpeedType in the Property section.
#  
# 
NITRO_ROMSPEED   = MROM

# Sample files for measuring transfer speeds
MAKEROM_ROMROOT     = $(ROOT)/build/demos/snd/stream/files
MAKEROM_ROMFILES    = kart_title.32.wav

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
