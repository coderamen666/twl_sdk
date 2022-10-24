#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - demos - g3d - samples - SeparateTex
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

NNS_USELIBS     = g3d gfd fnd

G3D_DEMOLIB		=	$(TWLSYSTEM_G3D_ROOT)/build/demos/g3d/demolib

LINCLUDES       =   $(G3D_DEMOLIB)/include
LLIBRARY_DIRS   =   $(G3D_DEMOLIB)/lib/$(NITRO_BUILDTYPE)
LLIBRARIES      =   libg3d_demo.a

#----------------------------------------------------------------------------


#----------------------------------------------------------------------------

SUBDIRS	= 

SRCS		=	main.c
TARGET_BIN	=	main.srl

MAKEROM_ROMROOT  = ./
MAKEROM_ROMFILES = data/*.*

#----------------------------------------------------------------------------
include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs


do-build: $(TARGETS)
$(TARGETS): mdlData texData

mdlData:
	$(TWLSYSTEM_TOOLSDIR)/bin/g3dcvtr.exe data/src/human_run_t.imd -emdl -o data/human_run_t.nsbmd
texData:
	$(TWLSYSTEM_TOOLSDIR)/bin/g3dcvtr.exe data/src/human_run_t.imd -etex -o data/human_run_t.nsbtx


include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
