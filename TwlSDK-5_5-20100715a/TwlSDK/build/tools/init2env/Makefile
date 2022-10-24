#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - envconv
#  File:     Makefile
#
#  Copyright 2005-2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-09-18#$
#  $Rev: 8573 $
#  $Author: okubata_ryoma $
#---------------------------------------------------------------------------

override TARGET_PLATFORM    =   NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS		= init2env.exe

SOURCES		= init.l init.y

OBJECTS		= init.tab.o init.yy.o

INSTALL_DIR     = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN	= $(OBJECTS) $(TARGETS) init.yy.c init.tab.c init.tab.h

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

lex.yy.o:		init.yy.c init.tab.c
lex.yy.c:		init.l
y.tab.c:		init.y
