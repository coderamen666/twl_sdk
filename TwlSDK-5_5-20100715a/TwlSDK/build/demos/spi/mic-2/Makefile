#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - spi - mic-2
# File:     Makefile
#
# Copyright 2003-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM :=  NITRO

SRCS        =   main.c
TARGET_BIN  =   main.srl

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:   $(TARGETS)

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
# Convert sampling data to a WAV file
capture:
	rm -f *.wav
	echo now capturing MIC...
	$(TWL_TOOLSDIR)/bin/loadrun.exe $(BINDIR)/$(TARGET_BIN) > ./log.txt
	$(TWL_TOOLSDIR)/bin/mic2wav.exe ./log.txt


#===== End of Makefile =====
