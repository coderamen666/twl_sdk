#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - demos - g2d - bg - BgEx
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

SRCS		=	main.c loader.c
TARGET_BIN	=	main.srl

MAKEROM_ROMROOT  = ./
MAKEROM_ROMFILES = data/*.*


G2D_TEST_RES_SRC			= $(wildcard data/src/Screen/*.nsc)
G2D_TEST_RES_SRC_NAMES		= $(notdir $(G2D_TEST_RES_SRC))
G2D_TEST_RES_TARGET			= $(addprefix data/,$(G2D_TEST_RES_SRC_NAMES:.nsc=.NSCR))
G2D_TEST_CVTR_OUTDIR 		= data

BINRES_FILES = $(G2D_TEST_CVTR_OUTDIR)/*.NCER 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NMCR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCGR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCBR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NCLR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NANR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NMAR 
BINRES_FILES := $(BINRES_FILES) $(G2D_TEST_CVTR_OUTDIR)/*.NSCR

LDIRT_CLEAN = $(BINRES_FILES)

#----------------------------------------------------------------------------
include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs

# Normal conversion
data/BG_normal.NSCR: data/src/Screen/BG_normal.nsc data/src/Character/BG_normal.ncg  data/src/ColorPalette/BG_normal.ncl
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $< -o/$(G2D_TEST_CVTR_OUTDIR)

# Does not include parts that do not use a palette
data/BG_cmp.NSCR: data/src/Screen/BG_cmp.nsc data/src/Character/BG_cmp.ncg  data/src/ColorPalette/BG_cmp.ncl
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $< -o/$(G2D_TEST_CVTR_OUTDIR) -pcm

# Extract rectangle (0, 12)-(31, 31) from character
data/BG_part.NSCR: data/src/Screen/BG_part.nsc data/src/Character/BG_part.ncg  data/src/ColorPalette/BG_part.ncl
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $< -o/$(G2D_TEST_CVTR_OUTDIR) -cr/0_12_32_20

# Does not include parts that do not use a palette
# Extract rectangle (0, 0)-(31, 11) from character
data/BG_cmp_part.NSCR: data/src/Screen/BG_cmp_part.nsc data/src/Character/BG_cmp_part.ncg  data/src/ColorPalette/BG_cmp_part.ncl
	$(TWLSYSTEM_TOOLSDIR)/bin/g2dcvtr.exe $< -o/$(G2D_TEST_CVTR_OUTDIR) -pcm -cr/0_0_32_12


reconv: $(G2D_TEST_RES_TARGET)

do-build: $(TARGETS)
$(TARGETS): reconv 

include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
