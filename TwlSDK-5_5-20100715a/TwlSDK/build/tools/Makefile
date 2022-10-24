#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-07-10#$
# $Rev: 10884 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		:= TWL NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
#----------------------------------------------------------------------------

SUBDIRS_P   =   ntexconv		\
                bin2obj			\
                makelst			\
                ppmconv			\
                stripdebug		\
                mic2wav			\


ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
SUBDIRS_P	+=	buryarg.TWL		\
                loadrun.TWL		\
                makelcf.TWL		\

ifeq	($(TWLSDK_HIDDEN),TRUE)
SUBDIRS_P	+=  makerom.TWL		\
                maketad			\
                cmptad			\
                aesconv         \
                compstatic.TWL	\
                makebanner.TWL
endif
endif

ifneq	($(filter NITRO,$(TARGET_PLATFORM_LIST)),)
SUBDIRS_P   +=  makelcf     \
                loadrun     \
                buryarg     \

ifeq    ($(TWLSDK_HIDDEN),TRUE)
SUBDIRS		+=  makebanner         \
                defval             \

SUBDIRS_P   +=  makerom            \
                compstatic         \
                rc4conv            \
                addbanner          \
                attachsign         \
                init2env           \
                makebanner-default \
                showversion        \
                tad2rom            \
                xml2env            \
                slicerom           \
                emuchild           \
                defval/test        \
                chscope

endif
endif

ifeq    ($(TWLSDK_HIDDEN),TRUE)
SUBDIRS_P	+= compBLZ
endif

ifeq	($(TWLSDK_PRIVATE),TRUE)
SUBDIRS_P	+=  sdkindent			\
				ntrcomp
endif

#------------------------------------------------------------------
#  After installation, change over CRLF to LF in the bash script in tools/bin 
#  
#------------------------------------------------------------------
ifeq	($(MAKECMDGOALS),install)
INSTALL_TARGETS		= TRUE
override DO_INSTALL	= $(REMAKE) changeD2U
endif

changeD2U:
	for i in $(TWL_INSTALL_TOOLSDIR)/bin/*; do				\
	  if [ -f $$i ]; then							\
	    if [ "`sed '1!d;/^#!.*bash/!d' $$i | wc -l`" -eq "1" ]; then		\
	      tr -d '\r' < $$i > $$i.new;	\
	      mv $$i.new $$i;			\
	    fi					\
	  fi					\
	done

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
