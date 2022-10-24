#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - bin2obj
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
TARGET_PLATFORM	 = NITRO TWL

include $(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			 = bin2obj.exe

SOURCES			 = bin2obj.c	\
				   cookargs.c	\
				   section.c	\
				   object.c		\
				   output.c		\
				   version.c

OBJECTS			 = $(SOURCES:.c=.o)
HEADERS			 = $(filter-out version.h,$(wildcard *.h))

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

bin2obj.o:		bin2obj.c bin2obj.h
cookargs.o:		cookargs.c bin2obj.h version.c
section.o:		section.c bin2obj.h
object.o:		object.c bin2obj.h
output.o:		output.c bin2obj.h
bin2obj.h:		elf.h object.h types.h

version.c:		$(filter-out version.c,$(SOURCES)) $(HEADERS) $(MAKEFILE)
			@for i in $^ ;			\
			do				\
				date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;'; \
			done | sort | tail -1 > $@
#
