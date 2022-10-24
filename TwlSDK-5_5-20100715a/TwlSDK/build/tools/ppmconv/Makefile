#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - ppmconv
#  File:     Makefile
#
#  Copyright 2003-2008 Nintendo.  All rights reserved.
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
TARGET_PLATFORM	= NITRO TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			= ppmconv.exe ppmconv8.exe ppmconvbg.exe
SOURCES			= $(TARGETS:.exe=.c)
OBJECTS			= $(SOURCES:.c=.o)
HEADERS			=

INSTALL_DIR     = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN		= $(OBJECTS) $(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

%.exe:			%.o
			$(CC_X86) $+ -o $@

#--- ppmconv
ppmconv.o:		ppmconv.c
ppmconv8.o:		ppmconv8.c
ppmconvbg.o:	ppmconvbg.c

#
