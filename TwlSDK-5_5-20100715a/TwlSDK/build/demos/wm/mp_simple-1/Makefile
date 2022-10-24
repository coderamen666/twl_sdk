#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - WM - demos
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
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------
TARGET_PLATFORM	=	TWL NITRO

WH_DIR			=	$(ROOT)/build/demos/wireless_shared/wh

SRCS			=	main.c font.c $(WH_DIR)/wh.c

LINCLUDES		=	$(WH_DIR)

TARGET_BIN		=	main.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
