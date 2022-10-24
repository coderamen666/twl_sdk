#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - dsp
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM     :=  TWL
TARGET_PROC         :=  ARM9

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS_P	 = $(GSUBDIRS)
SUBDIRS_P	+= g711 \
               graphics

SUBDIRS_P	+= $(PRIVATE_LIBRARIES_DSP)

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
