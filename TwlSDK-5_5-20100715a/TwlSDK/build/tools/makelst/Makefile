#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - makelst
#  File:     Makefile
#
#  Copyright 2006-2009 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2009-06-04#$
#  $Rev: 10698 $
#  $Author: okubata_ryoma $
#---------------------------------------------------------------------------
TARGET_PLATFORM	 = NITRO TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			 = makelst.exe

SOURCES_C		 = makelst.c arch.c elf.c loader_subset.c elf_loader.c searcharg.c version.c
HEADERS_C		 = arch.h elf.h loader_subset.h elf_loader.h searcharg.h types.h

SOURCES			 = $(SOURCES_C)
HEADERS			 = $(HEADERS_C)

OBJECTS			 = $(SOURCES_C:.c=.o)

INSTALL_DIR      = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	 = $(TARGETS)

LDIRT_CLEAN		 = $(OBJECTS) $(TARGETS) version.c

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:			$(TARGETS)

$(TARGETS):			$(OBJECTS)
				$(CC_X86) $+ -o $@

arch.o:				arch.h
elf.o:				types.h elf.h
elf_loader.o:		types.h elf.h elf_loader.h arch.h loader_subset.h
loader_subset.o:	loader_subset.h
makelst.o:			types.h elf.h elf_loader.h searcharg.h
searcharg.o:		searcharg.h version.c

version.c:  $(filter-out version.c,$(SOURCES)) $(HEADERS) $(MAKEFILE)
		@for i in $^ ; do \
			date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;'; \
		done | sort | tail -1 > $@

