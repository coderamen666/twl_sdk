#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - makelcf
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
TARGET_PLATFORM		= TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGET			 =	simple1.lcf simple2.lcf test.autogen.lcf ARM9-TEG.autogen.lcf overlay.autogen.lcf
TARGET			+=	test2.lcf

LCFILE_TEMPLATE	 =	$(NITRO_INCDIR)/nitro/specfiles/$(NITRO_PROC)-$(NITRO_PLATFORM).lcf.template

%.autogen.lcf:		%.lsf $(MAKELCF) $(LCFILE_TEMPLATE)
			$(MAKELCF) $(MAKELCF_FLAGS) $< $(LCFILE_TEMPLATE) $@

simple1.lcf:		simple.lsf $(MAKELCF) simple.lcf.template
			$(MAKELCF) $(MAKELCF_FLAGS) -V1 $< simple.lcf.template $@

simple2.lcf:		simple.lsf $(MAKELCF) simple.lcf.template
			$(MAKELCF) $(MAKELCF_FLAGS) -V2 $< simple.lcf.template $@

test2.lcf:		test2.lsf $(MAKELCF) test2.def
			$(MAKELCF) $(MAKELCF_FLAGS) -Mtest2.def $< $(LCFILE_TEMPLATE) $@

MAKELCF =		$(NITRO_TOOLSDIR)/bin/makelcf.exe

.PHONY:			build install do-autotest clean clobber

define ECHO_CURDIR
			echo "==== $(CURDIR)";
endef

build:			
			@$(ECHO_CURDIR)
			@$(MAKE) $(TARGET)

install do-autotest:
			@$(ECHO_CURDIR)

clean clobber super-clobber:
			@$(ECHO_CURDIR)
			-rm -f $(TARGET) *~
