#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL - ccm-compatible - build
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-09-24#$
# $Rev: 11063 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------
TARGET_PLATFORM		:= TWL

SRCS		=	main.c
TARGET_BIN	=	main.srl

MAKEROM_ROMROOT  = ./rom_data
MAKEROM_ROMFILES = *.*

LDIRT_CLEAN	=	$(MAKEROM_ROMROOT)/nonce.bin $(MAKEROM_ROMROOT)/encrypted.bin

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

.PHONY:	rom_data

do-build:		$(TARGETS)


$(BINDIR)/main.srl: rom_data

rom_data:
	$(PERL) aes-128-ccm.pl						\
		$(MAKEROM_ROMROOT)/sample_adata.txt		\
		$(MAKEROM_ROMROOT)/sample_pdata.txt		\
		16										\
		$(MAKEROM_ROMROOT)/key.bin				\
		$(MAKEROM_ROMROOT)/nonce.bin			\
		$(MAKEROM_ROMROOT)/encrypted.bin


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
