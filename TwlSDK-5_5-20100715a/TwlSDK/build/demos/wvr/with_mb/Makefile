#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - wvr - with_mb
# File:     Makefile
#
# Copyright 2003-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-24#$
# $Rev: 10808 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM :=  NITRO TWL
TWL_ARCHGEN		:=	HYBRID

# Creates child boot binary
SUBDIRS     =   child


# Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
# 
# The mb_child.bin program in the directory $NitroSDK/bin/ARM9-TS/Release/ is a sample providing the same features as the multiboot child in the final commercial unit. Load this binary into other development units using the same method as for sample programs, and execute them together.
# 
# 
# 
# 


#----------------------------------------------------------------------------

INCDIR      =   common/include parent/include
SRCDIR      =   common/src parent/src
SRCS        =   main.c      \
                common.c    \
                disp.c      \
                font.c      \
                gmain.c

LLIBRARIES  =   libwvr$(TWL_LIBSUFFIX).a

# WH module was moved to wireless_shared

WH_DIR      =   $(ROOT)/build/demos/wireless_shared/wh
SRCDIR      +=  $(WH_DIR)
INCDIR      +=  $(WH_DIR)
SRCS        +=  $(WH_DIR)/wh.c

# MBP was moved to wireless_shared

MBP_DIR     =   $(ROOT)/build/demos/wireless_shared/mbp
SRCDIR      +=  $(MBP_DIR)
INCDIR      +=  $(MBP_DIR)
SRCS        +=  $(MBP_DIR)/mbp.c

TARGET_NEF	 =	main.nef
TARGET_BIN  =   main.srl

# It is safer to set a slightly large IRQ stack size in the LCF file when running complex processes within the MB callback.
# 
# Because OS_Printf() in particular consumes a large amount of stack, try using the lighter version, OS_TPrintf(), in callbacks whenever possible.
# 

# makerom settings
MAKEROM_ROMROOT     =   ./data
MAKEROM_ROMFILES    =   *.srl

#------------------------------------------------------#
# Component definition where the wireless library is positioned inside VRAM #

DEFAULT_COMP_ARM7	=	ichneumon

#------------------------------------------------------#

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

# ichneumon.TWL is included only in the CWforDSi version because of its size. Therefore, do not build demos that use it.
ifneq	($(SDK_CW_BUILD_VERSION_CC),)
ifeq	($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" "<=" 3.0),1)
TARGET_PLATFORM_LIST		:= $(filter-out TWL, $(TARGET_PLATFORM_LIST))
endif
endif

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			 =	main.TWL.rsf
else
ROM_SPEC			 =	main.rsf
endif

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN

#===== End of Makefile =====
