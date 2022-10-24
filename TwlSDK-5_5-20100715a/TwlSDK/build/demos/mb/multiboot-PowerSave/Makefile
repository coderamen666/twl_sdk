#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB - multiboot-PowerSave
# File:     Makefile
#
# Copyright 2005-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS				 =	

# Because the MB library samples use wireless features, multiple development units with the same communications environment (wireless or wired LAN) are required.
# 
# The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/Rom/ directory provide the same features as those present with a commercial system's IPL for a child during DS Download Play. Load these binaries on other systems just like any sample program and then run them together.
# 
# 
# 
# 

#----------------------------------------------------------------------------
TARGET_PLATFORM		 =	TWL NITRO

SRCS				 =	common.c data.c dispfunc.c menu.c parent.c main.c

TARGET_NEF			 =	main.nef
TARGET_BIN			 =	main.srl

# makerom settings
MAKEROM_ROMROOT		 =	./data
MAKEROM_ROMFILES	 =	*.srl


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			 =	main.TWL.rsf
LCFILE_SPEC			 =	main.TWL.lsf
else
ROM_SPEC			 =	main.rsf
LCFILE_SPEC			 =	main.lsf
endif

#----------------------------------------------------------------------------

do-build:			$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN


#===== End of Makefile =====
