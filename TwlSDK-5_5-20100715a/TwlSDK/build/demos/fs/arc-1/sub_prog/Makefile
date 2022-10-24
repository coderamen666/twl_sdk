#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - FS - demos - arc-1/sub_prog
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
# $Date:: 2008-12-24 #$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS             =

#----------------------------------------------------------------------------

TARGET_BIN          = main.srl
MAKEROM_ROMROOT     = ./data 
MAKEROM_ROMFILES    = * 

SRCS                = main.c

INSTALL_DIR         = ../data
INSTALL_TARGETS     = ./bin/$(TWL_BUILDTYPE)/main.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
