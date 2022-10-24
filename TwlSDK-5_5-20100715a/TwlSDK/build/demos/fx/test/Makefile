#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - FX - demos - test
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
# $Date:: 2008-12-24#$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS         =

#----------------------------------------------------------------------------
TARGET_PLATFORM =   TWL NITRO

SRCS            =   main.c table.c fx_test_common.c fx_append_vec.c fx_append_mtx.c 


TARGET_BIN      =   main.srl
LINCLUDES       =   ../DEMOLib/include
LLIBRARIES      =   
LLIBRARY_DIRS   =   ../DEMOLib/lib/$(TWL_BUILDTYPE)

#SRCDIR         =   # using default
#LCFILE         =   # using default

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:       $(TARGETS)


include $(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
