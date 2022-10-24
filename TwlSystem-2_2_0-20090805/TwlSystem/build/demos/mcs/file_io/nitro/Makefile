#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - demos - mcs - file_io - nitro
# File:     Makefile
#
# Copyright 2004-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Revision: 1155 $
#----------------------------------------------------------------------------

NNS_USELIBS = fnd mcs

SUBDIRS		=


#----------------------------------------------------------------------------

SRCS		=	main.c


TARGET_BIN	=	main.srl

NNS_MCS_DEMOS_FILEIO_DATA_DIR = $(call eupath,$(TWLSYSTEM_ROOT)/build/demos/mcs/file_io/nitro/data)

include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)
# Copy when data/test.txt does not exist.
	if [ ! data -ef $(NNS_MCS_DEMOS_FILEIO_DATA_DIR) ] ;	\
	then													\
		mkdir -p $(NNS_MCS_DEMOS_FILEIO_DATA_DIR) ;			\
		cp data/test.txt $(NNS_MCS_DEMOS_FILEIO_DATA_DIR) ;	\
	fi

include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
