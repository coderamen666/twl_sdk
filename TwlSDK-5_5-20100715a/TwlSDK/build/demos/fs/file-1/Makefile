#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - FS - demos - file-1
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18 #$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS             =

#----------------------------------------------------------------------------

TARGET_BIN          = main.srl

SRCS                = main.c
SRCS_OVERLAY        = boo.c foo.c woo.c

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq ($(TARGET_PLATFORM),TWL)
LCFILE_SPEC         = main.TWL.lsf
else
LCFILE_SPEC         = main.NTR.lsf
endif

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

do-build:	$(TARGETS)


#===== End of Makefile =====
