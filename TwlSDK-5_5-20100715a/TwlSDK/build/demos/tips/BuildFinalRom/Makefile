#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - GX - demos - tips/BuildFinalRom
# File:	    Makefile
#
# Copyright 2003,2004 Nintendo.	 All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-12-24#$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM	= NITRO TWL

SUBDIRS			= etc/banner


#----------------------------------------------------------------------------

SRCS			= main.c data.c
TARGET_BIN		= main.srl
LINCLUDES		= ../../gx/UnitTours/DEMOLib/include

LLIBRARY_DIRS   = $(ROOT)/build/demos/gx/UnitTours/DEMOLib/lib/$(TWL_BUILDTYPE)
LLIBRARIES      = libDEMO$(TWL_LIBSUFFIX).a

#SRCDIR			= # using default
#LCFILE			= # using default

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs


###--------------------------------------------------------------------------
###  Setting for finalrom
###--------------------------------------------------------------------------
#### BEGIN
####
####  Please get your "libsyscall.a" and "rom header template" from Nintendo.
####  And put them in ./etc directory.
####
#
# ROM SPEC FILE
#
ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC	= ./ROM-TS.TWL.rsf
else
ROM_SPEC	= ./ROM-TS.rsf
endif
#
# LIBSYSCALL: Provided to every product by NINTENDO
#
LIBSYSCALL		= ./etc/libsyscall.a
###
###
###
#### END

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
