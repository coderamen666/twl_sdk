#! make -f
#----------------------------------------------------------------------------
# Project:  TWL-System - buildtools
# File:     commondefs.cctype.CW
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


#----------------------------------------------------------------------------
#  switch by codegen platform TEG/TS

MACRO_FLAGS		+= -DNNS_$(TARGET_PLATFORM)


#----------------------------------------------------------------------------
#  switch by codegen processor ARM7/ARM9

MACRO_FLAGS		+= -DNNS_$(CODEGEN_PROC)


#----------------------------------------------------------------------------
#  switch by DEBUG/RELEASE/FINALROM
#

MACRO_FLAGS		+= -DNNS_$(TWL_BUILD_TYPE)


#----------------------------------------------------------------------------
#  switch by C Compiler CW/ADS/GCC
#

MACRO_FLAGS		+= -DNNS_$(TWL_CCTYPE)


#----------------------------------------------------------------------------
#  switch by codegen target ARM/THUMB

MACRO_FLAGS_CC	+= -DNNS_CODE_$(CODEGEN_CC)
MACRO_FLAGS_AS	+= -DNNS_CODE_$(CODEGEN_AS)


#----- End of commondefs.cctype.CW -----
