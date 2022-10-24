#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB
# File:     Makefile
#
# Copyright 2005-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-10-15#$
# $Rev: 8948 $
# $Author: nishimoto_takashi $
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------

include	../make_common


###############################################################################
# Settings specific for parents

SRCS			+=	main.c parent.c multiboot.c

# This sample uses the MBP module
MBP_DIR			 =	$(ROOT)/build/demos/wireless_shared/mbp
LINCLUDES		+=	$(MBP_DIR)
SRCS			+=	$(MBP_DIR)/mbp.c

MAKEROM_ROMROOT	 =	.
MAKEROM_ROMFILES =	./data


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
LCFILE_SPEC		 =	main.TWL.lsf
else
LCFILE_SPEC		 =	main.lsf
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====

