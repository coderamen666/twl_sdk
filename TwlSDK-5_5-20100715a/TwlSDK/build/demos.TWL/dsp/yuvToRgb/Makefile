#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - dsp - demo - yuvToRgb
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-05-22#$
# $Rev: 6177 $
# $Author: nishimoto_takashi $
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------
TARGET_PLATFORM		:= TWL

TARGET_BIN	=	main.srl

SRCDIR		=	./src

SRCS		=	main.c

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====