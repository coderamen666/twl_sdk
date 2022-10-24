#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - MB - demos - cloneboot
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-12-24#$
# $Rev: 9728 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS		=

# Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
# The mb_child.bin sample program in the $TwlSDK/bin/ARM9-TS/Release directory provides the same functionality as a commercial system that is a multiboot child. Load this binary on other systems just like a sample program and run them together.
# 
# 
# 
# 
# 

#----------------------------------------------------------------------------

TWL_ARCHGEN	:=	HYBRID

#----------------------------------------------------------------------------
# The source files used with this program are shown below
#----------------------------------------------------------------------------

SRCDIR		 = ./src
INCDIR		 = ./include

SRCS		 = main.c		\
			   common.c		\
			   disp.c		\
			   font.c		\
			   child.c		\
			   parent.c		\
			   gmain.c

# To simplify the samples, use WH modules

WH_DIR		 = $(ROOT)/build/demos/wireless_shared/wh
SRCDIR		+= $(WH_DIR)
INCDIR		+= $(WH_DIR)
SRCS		+= $(WH_DIR)/wh.c

# To simplify the samples, use MBP modules

MBP_DIR		 = $(ROOT)/build/demos/wireless_shared/mbp
SRCDIR		+= $(MBP_DIR)
INCDIR		+= $(MBP_DIR)
SRCS		+= $(MBP_DIR)/mbp.c


#----------------------------------------------------------------------------
# The general application settings are shown below
#----------------------------------------------------------------------------

TARGET_NEF		 = main.nef
TARGET_BIN		 = main.srl

TWL_MAKEROM		 = TRUE

# To compress each segment of a program, enable this flag.
# With a reduced binary size on the CARD-ROM, compressed segments reduce load times via wireless transfers or CARD-ROM but requires some CPU time to extract their contents at runtime.
# 
# 
TWL_COMPRESS	 = TRUE

# You must enable NITRO_DIGEST when using overlays as wireless download children
# 
TWL_DIGEST		 = TRUE

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC		 = main.TWL.rsf
#LCFILE_SPEC	 = main.TWL.lsf
else
ROM_SPEC		 = main.rsf
#LCFILE_SPEC	 = main.lsf
endif

# It is safer to set a slightly large IRQ stack size in the LCF file when running complex processes within the MB callback.
# Because OS_Printf() in particular consumes a large amount of stack, try using the lighter version, OS_TPrintf(), in callbacks whenever possible.
# 
# 

#----------------------------------------------------------------------------
# Special settings for the clone boot parent are shown below
#----------------------------------------------------------------------------

# Specifies the link settings template for clone booting in order to place the .parent section, which is specific to parent devices and used for security purposes.
# 
#
# The contents of the .parent section are not sent to the child machine in DS Download Play.
# Placing essential and parent-specific functions in this section makes it impossible to duplicate the parent device from intercepted transmission data.
# 
# When positioning the code, please note the following:
#   -Do not include anything other than the .text/.rodata section
#   -Include important process required only by the parent
# 
# Code in the .parent section is arranged as follows.
# 
#     #include <nitro/parent_begin.h>
#
#     void  ForParentOnly(void)
#     {
#         /* Processing exclusive to the parent goes here */
#     }
#
#     #include <nitro/parent_end.h>
#
# In this sample demo, all the functions in parent.c are placed in the .parent section. 
# For details, see the source code.

ifeq	($(TARGET_PLATFORM),TWL)
LCFILE_TEMPLATE	= $(TWL_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
else
LCFILE_TEMPLATE	= $(NITRO_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
endif

# Official system call libraries, distributed for every retail application, are specified here.
# In this sample, a copy of the version included in the SDK (lib/ARM9-TS/etc/libsyscall.a) has been provisionally placed in this directory in advance.
# When creating an application based on this demo, overwrite these provisional system calls with the libsyscal.a provided by Nintendo.
# 
# 
# 
# 

LIBSYSCALL			 = ./etc/libsyscall.a
LIBSYSCALL_CHILD	 = ./etc/libsyscall_child.bin

#----------------------------------------------------------------------------
# Here we describe the pre-authentication process used to run the application on the production model
#----------------------------------------------------------------------------

# The following a is rough outline of pre-authentication.
# For details, see "DS Download Play".
# (/docs/TechnicalNotes/AboutMultiBoot.doc)
#
# (0) If clone boot, use the emuchild command to prepare the program to send.
#     If not, use main.srl without modification.
#     In this makefile, the prepared program is bin/sign.srl.
#
# (1) To obtain the authentication code, send tmp/sign.srl, which was prepared in Step 0, to the server.
#     You must do this by hand using your browser.
#     In this makefile, the received authentication code is bin/sign.sgn.
#
# (2) Use the attachsign command to combine main.srl and bin/sign.sgn, obtained in step (1), and create the final program.
#     
#     In this makefile, the product is main_with_sign.srl.
#

# When clone booting, the program will send a program modified using emuchild, instead of itself, to the server; this differs slightly from an ordinary DS Download Play program.
# 

presign:
	$(EMUCHILD) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		$(LIBSYSCALL_CHILD) \
		bin/sign.srl

# The process of including the obtained authentication code in the binary is the same in both clone boot and normal operation
postsign:
	$(ATTACHSIGN) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		bin/sign.sgn  \
		main_with_sign.srl

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN


#===== End of Makefile =====
