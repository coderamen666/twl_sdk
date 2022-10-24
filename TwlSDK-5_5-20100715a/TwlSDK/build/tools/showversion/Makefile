#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - showversion
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

TARGET_PLATFORM    =   NITRO

include $(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			 = showversion.exe
SOURCES			 = main.c	\
				   version.c

OBJECTS			 = $(SOURCES:.c=.o)
HEADERS			 = $(filter-out version.h,$(wildcard *.h))

MACROS			+=

INSTALL_DIR		 = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	 = $(TARGETS)

LDIRT_CLEAN		 = $(OBJECTS) $(TARGETS) version.c

include $(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

main.o:			main.c version.c

version.c:		$(filter-out version.c,$(SOURCES)) $(HEADERS) $(MAKEFILE)
			@for i in $^ ;			\
			do						\
				date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;'; \
			done | sort | tail -1 > $@
#
