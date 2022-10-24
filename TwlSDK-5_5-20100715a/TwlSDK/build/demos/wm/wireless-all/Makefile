#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - WM - demo - wireless-all
# File:     Makefile
#
# Copyright 2006-2008 Nintendo. All rights reserved.
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

SUBDIRS		 =
SUBMAKES	 = Makefile.icon


#----------------------------------------------------------------------------
# The source files used with this program are shown below.
#----------------------------------------------------------------------------
# This only runs on NITRO
TARGET_PLATFORM	:= NITRO

SRCDIR		 = ./src
INCDIR		 = ./include

# Common parent processes
SRCS		 =	main.c		\
				common.c	\
				test.c

# Wireless driver processes (simple WH wrapper)
SRCS		+=	wh_measure.c	\
				wh_download.c	\
				wh_datasharing.c

# Uses the WH (wireless sample module)
WH_DIR		 =	$(ROOT)/build/demos/wireless_shared/wh
LINCLUDES	+=	$(WH_DIR)
SRCS		+=	$(WH_DIR)/wh.c

# Uses the MBP (DS Download Play sample module)
MBP_DIR		 =	$(ROOT)/build/demos/wireless_shared/mbp
LINCLUDES	+=	$(MBP_DIR)
SRCS		+=	$(MBP_DIR)/mbp.c

# Uses the WFS (wireless file system sample module)
WFS_DIR		 =	$(ROOT)/build/demos/wireless_shared/wfs
LINCLUDES	+=	$(WFS_DIR)/include
SRCS		+=	$(WFS_DIR)/src/wfs_common.c

#----------------------------------------------------------------------------
# The resources used when building this program are shown below
#----------------------------------------------------------------------------

# The directory that stores the data to convert from .dat -> .o format with bin2obj.exe
DATA_DIR	 =	./data/image

# The wireless link strength icon
WLICON_DIR	 =	$(ROOT)/data/wl_icons
WLICON_OBJS	 =	$(patsubst $(WLICON_DIR)/wireless_strength_level_%_b.bmp,$(OBJDIR)/linkimg%.o,$(WLICON_BMP))
EXT_OBJS	+=	$(WLICON_OBJS)

# BG character for ASCII text display
BGCHAR_OBJS	 =	$(patsubst $(DATA_DIR)/%.dat,$(OBJDIR)/%.o,$(wildcard $(DATA_DIR)/*.dat))
EXT_OBJS	+=	$(BGCHAR_OBJS)


#----------------------------------------------------------------------------
# The general application settings are shown below
#----------------------------------------------------------------------------

TARGET_NEF		 = main.nef
TARGET_BIN		 = main.srl
TWL_MAKEROM		 = TRUE
MAKEROM_ROMROOT	 = ./data/rom
MAKEROM_ROMFILES = *

# To compress each segment of a program, enable this flag.
# With a reduced binary size on the CARD-ROM, compressed segments reduce load times via wireless transfers or CARD-ROM but requires some CPU time to extract their contents at runtime.
# 
# 

TWL_COMPRESS	 = TRUE

# You must always enable TWL_DIGEST for security when using overlays as wireless download children.
# 

TWL_DIGEST		 = TRUE


#----------------------------------------------------------------------------


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
# Special settings for the clone boot parent are shown below
#----------------------------------------------------------------------------

# Specifies the link settings template for clone booting in order to place the .parent section, which is specific to parent devices and used for security purposes
# 

ifeq	($(TARGET_PLATFORM),TWL)
LCFILE_TEMPLATE	= $(TWL_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
else
LCFILE_TEMPLATE	= $(NITRO_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template
endif

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
# In this sample demo, all of the functions for DS Download Play are placed in the .parent section. 
# See the source code for details.


# Official system call libraries, distributed for every retail application, are specified here.
# 
# In this sample, a copy of the version included in the SDK (lib/ARM9-TS/etc/libsyscall.a) has been provisionally placed in this directory in advance.
# 
# When creating an application based on this demo, overwrite these provisional system calls with the libsyscal.a provided by Nintendo.
# 
# 

LIBSYSCALL			 = ./etc/libsyscall.a
LIBSYSCALL_CHILD	 = ./etc/libsyscall_child.bin


#----------------------------------------------------------------------------
# Below is the signature procedure to run the application on the production model
#----------------------------------------------------------------------------

# The following a is rough outline of signature procedure.
# For details, see "DS Download Play."
# (/docs/TechnicalNotes/AboutMultiBoot.pdf)
#
# (0) If clone boot, use the emuchild command to prepare the program to send.
#     If not, use main.srl without modification.
#     In this makefile, the prepared program is bin/sign.srl.
#
# (1) To get the signature code, send tmp/sign.srl, which was prepared in Step 0, to the server.
#     You must do this by hand using your browser.
#     In this makefile, the obtained signature code is bin/sign.sgn.
#
# (2) Use the attachsign command to combine main.srl and bin/sign.sgn, obtained in step (1), and create the final program.
#     
#     In this makefile, the product is main_with_sign.srl.
#

# When clone booting, the program will send a program modified using emuchild, instead of itself, to the server. This differs slightly from an ordinary DS Download Play program.
# 

presign:
	$(EMUCHILD) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN)	\
		$(LIBSYSCALL_CHILD)						\
		bin/sign.srl

# The procedure to include the obtained signature code in the binary is the same in both clone boot and normal operation
postsign:
	$(ATTACHSIGN) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN)	\
		bin/sign.sgn							\
		main_with_sign.srl

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

# All binary data is linked as a constant array
$(OBJDIR)/%.o : $(DATA_DIR)/%.dat
	$(BIN2OBJ) --compatible $< $@ >/dev/null


#===== End of Makefile =====
