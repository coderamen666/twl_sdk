#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - buryarg.TWL
#  File:     Makefile
#
#  Copyright 2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-05-23#$
#  $Rev: 0 $
#  $Author: nobody $
#---------------------------------------------------------------------------
TARGET_PLATFORM		=	TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS		= buryarg.TWL.exe

OBJECTS		= buryarg.o

INSTALL_DIR     = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN	= $(OBJECTS) $(TARGETS)
MACROS		+= -DSDK_NITRO

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

buryarg.o:		buryarg.c
