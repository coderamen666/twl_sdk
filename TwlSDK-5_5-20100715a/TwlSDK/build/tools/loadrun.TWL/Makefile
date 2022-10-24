#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - loadrun.TWL
#  File:     Makefile
#
#  Copyright 2007-2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2007-12-10#$
#  $Rev: 2964 $
#  $Author: yada $
#---------------------------------------------------------------------------
TARGET_PLATFORM		=	TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/nitro/commondefs.emtype.IS

ISTD_ROOT	 = $(ISDBG_ROOT)

SRCPATH		 = $(ISTD_ROOT)/Samples/download/X86

LINCLUDES	+= $(ISDBG_ROOT_NOSPACE)/X86/include ./ $(ROOT)/include

TARGETS		 = loadrun.TWL.exe

SOURCES_CC	 = loadrun.c isd_api.c version.c
HEADERS_CC	 = isd_api.h

OBJECTS		 = loadrun.o isd_api.o version.o

NITRORUN	 = nitrorun.TWL



INSTALL_DIR     = $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS) $(NITRORUN)

LDIRT_CLEAN		= $(OBJECTS) $(TARGETS) version.c

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

LINCS		 = $(call decode_specialchar,$(foreach DIR,$(LINCLUDES),-I'$(DIR)'))
WARNING		 = -Wall -Wno-unknown-pragmas -Wno-unused-variable
MACROS		+= -DSDK_TWL

%.o:		%.c
		$(CC_X86) $(MACROS) -DSDK_WIN32 $(WARNING) -c -I. $(LINCS) $< -o $@

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:	$(TARGETS)

$(TARGETS):	$(OBJECTS)
		$(CC_X86) $+ -o $@

loadrun.o:	isd_api.h loadrun.c version.c
isd_api.o:	isd_api.h isd_api.c

version.c:	$(filter-out version.c,$(SOURCES_CC)) $(HEADERS_CC) $(MAKEFILE)
		@for i in $^ ; do \
			date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;'; \
		done | sort | tail -1 > $@
