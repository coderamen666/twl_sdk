#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - os - reset-1
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-09#$
# $Rev: 10728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	=	TWL NITRO

TWL_ARCHGEN		= HYBRID

SRCS			=	main.c	\
					font.c	\
					screen.c
TARGET_BIN		=	main.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
