#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - build
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
# $Date:: 2008-10-06#$
# $Rev: 8866 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM			 = TWL NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------
ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)

ifeq	($(TWLSDK_PRIVATE),TRUE)
SUBMAKES.COMPONENTS		+= components/Makefile
SUBMAKES.ADDINS.TOOLS	+= ../add-ins/Makefile.tools
SUBMAKES.ADDINS.LIBS	+= ../add-ins/Makefile.libs
SUBMAKES.ADDINS.DEMOS_	+= ../add-ins/Makefile.demos
endif	# ifeq	($(TWLSDK_PRIVATE),TRUE)

ifeq	($(TWLSDK_HIDDEN),TRUE)
SUBMAKES.TARGET_TOOLS	+= tools/_Makefile.target
endif	# ifeq	($(TWLSDK_HIDDEN),TRUE)

ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
SUBMAKES.DEMOS			+= demos/Makefile
SUBMAKES.DEMOS			+= demos.TWL/Makefile

ifeq	($(TWLSDK_HIDDEN),TRUE)
SUBMAKES.TESTS			+= tests/Makefile
endif	# ifeq	($(TWLSDK_HIDDEN),TRUE)

else	# ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
SUBMAKES.DEMOS			+= demos/gx/UnitTours/DEMOLib/Makefile	\
						   demos/wm/wmDEMOLib/Makefile

ifeq	($(TWLSDK_HIDDEN),TRUE)
SUBMAKES.DEMOS			+= demos.TWL/userInfo/Makefile
endif	# ifeq	($(TWLSDK_HIDDEN),TRUE)
endif	# ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),) else
endif	# ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)

#----------------------------------------------------------------------------
ifneq	($(filter NITRO,$(TARGET_PLATFORM_LIST)),)

ifeq	($(TWLSDK_PRIVATE),TRUE)
SUBMAKES.COMPONENTS		+= components/Makefile
SUBMAKES.ADDINS.TOOLS	+= ../add-ins/Makefile.tools
SUBMAKES.ADDINS.LIBS	+= ../add-ins/Makefile.libs
SUBMAKES.ADDINS.DEMOS_	+= ../add-ins/Makefile.demos
endif	# ifeq	($(TWLSDK_PRIVATE),TRUE)

ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
#
# build demo if not NITRO_WITHOUT_DEMOS
#
SUBMAKES.DEMOS			+= demos/Makefile
#SUBMAKES.ADDINS.DEMOS	+= $(SUBMAKES.ADDINS.DEMOS_)
ifeq	($(TWLSDK_HIDDEN),TRUE)
SUBMAKES.TESTS			+= tests/Makefile
endif	# ifeq	($(TWLSDK_HIDDEN),TRUE)

else	# ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
#
# build demo-libs only if NITRO_WITHOUT_DEMOS
#
SUBMAKES.DEMOS			+= demos/gx/UnitTours/DEMOLib/Makefile	\
						   demos/wm/wmDEMOLib/Makefile

endif	# ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),) else
endif	# ifneq	($(filter NITRO,$(TARGET_PLATFORM_LIST)),)

#----------------------------------------------------------------------------

SUBMAKES 		 = buildsetup/Makefile					\
				   $(sort $(SUBMAKES.ADDINS.TOOLS))		\
				   tools/Makefile						\
				   $(sort $(SUBMAKES.ADDINS.LIBS))		\
				   libraries/Makefile					\
				   $(sort $(SUBMAKES.COMPONENTS))		\
				   $(sort $(SUBMAKES.TARGET_TOOLS))		\
				   $(sort $(SUBMAKES.DEMOS))			\
				   $(sort $(SUBMAKES.ADDINS.DEMOS))		\
				   $(sort $(SUBMAKES.TESTS))

LDIRT_CLOBBER	 = ../ARM7-TS/Debug         \
				   ../ARM7-TS/Release       \
				   ../ARM7-TS/Rom           \
				   ../ARM9-TS/Debug         \
				   ../ARM9-TS/Release       \
				   ../ARM9-TS/Rom           \


#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
