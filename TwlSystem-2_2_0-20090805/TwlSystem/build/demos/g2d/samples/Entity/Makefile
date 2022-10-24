#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - demos - g2d - samples - Entity
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

NNS_USELIBS     = g2d gfd fnd

G2D_DEMOLIB		=	$(TWLSYSTEM_G2D_ROOT)/build/demos/g2d/demolib

LINCLUDES       =   $(G2D_DEMOLIB)/include
LLIBRARY_DIRS   =   $(G2D_DEMOLIB)/lib/$(NITRO_BUILDTYPE)
LLIBRARIES      =   libg2d_demo.a

#----------------------------------------------------------------------------

SRCS		=	main.c
TARGET_BIN	=	main.srl

MAKEROM_ROMROOT  = ./
MAKEROM_ROMFILES = data/*.*

#
#Resource name for reconversion
# Specify G2D_TEST_CVTR_SRCDIR as root
#
G2D_TEST_RES_SRC = Entity.nce

G2D_TEST_CVTR_OUTDIR = data/
G2D_TEST_CVTR_SRCDIR = data/src/

G2D_TEST_RES_SRC := $(G2D_TEST_CVTR_SRCDIR)$(G2D_TEST_RES_SRC)

BINRES_FILES = $(G2D_TEST_CVTR_OUTDIR)/*.NCER 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NMCR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCGR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCBR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCLR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NANR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NMAR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NSCR
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NENR


LDIRT_CLEAN = $(BINRES_FILES)


#----------------------------------------------------------------------------
include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs

reconv: $(G2D_TEST_RES_SRC)
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $(G2D_TEST_RES_SRC) -o/$(G2D_TEST_CVTR_OUTDIR) $(G2D_TEST_CVTR_OPTS)
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $(G2D_TEST_RES_SRC) -o/$(G2D_TEST_CVTR_OUTDIR) -bmp $(G2D_TEST_CVTR_OPTS)
	$(TWLSYSTEM_TOOLSDIR)/bin/BuildNENR.exe $(G2D_TEST_CVTR_SRCDIR)Entity.txt -o/$(G2D_TEST_CVTR_OUTDIR)
do-build: $(TARGETS)
$(TARGETS): reconv




include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
