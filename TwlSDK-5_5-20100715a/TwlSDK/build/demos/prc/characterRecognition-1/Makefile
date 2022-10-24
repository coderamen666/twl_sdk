#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - PRC
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
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS_P	= ../patterns

#----------------------------------------------------------------------------
TARGET_PLATFORM	= TWL NITRO

TARGET_BIN	= main.srl
#TARGET_NEF	= main.nef

SRCDIR		= src ../patterns/src
INCDIR		= include ../patterns/include

SRCS		= main.c patterns.c

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
