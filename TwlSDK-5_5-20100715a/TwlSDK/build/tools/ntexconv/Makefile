#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - ntexconv
#  File:     Makefile
#
#  Copyright 2006-2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2007-11-08#$
#  $Rev: 2178 $
#  $Author: terui $
#---------------------------------------------------------------------------
TARGET_PLATFORM		= NITRO TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			= ntexconv.exe TXLib.dll
NO_STRIP		= TRUE

INSTALL_DIR     = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

do-build:		$(TARGETS)
