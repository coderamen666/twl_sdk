#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - makelcf.TWL
#  File:     Makefile
#
#  Copyright 2007-2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-09-18#$
#  $Rev: 8573 $
#  $Author: okubata_ryoma $
#---------------------------------------------------------------------------

TARGET_PLATFORM    =   TWL

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#---------------------------------------------------------------------------

TARGET_BIN  =   makelcf.TWL.exe
TARGETS     =   $(BINDIR)/$(TARGET_BIN)

SRCS        =   makelcf.c           \
                defval.c            \
                misc.c              \
                container.c         \
                createlcf.c         \
                spec.tab.c          \
                spec.yy.patched.c   \
                tlcf.tab.c          \
                tlcf.yy.c           \
                version.c

HEADERS     =   defval.h    \
                makelcf.h   \
                misc.h

OBJDIR      =   obj
BINDIR      =   bin

OBJS        =   $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

MACROS      +=  -DSDK_TWL
NEWDIRS     =   $(OBJDIR) $(BINDIR)
LDIRT_CLEAN +=  $(NEWDIRS) version.c                                \
                spec.tab.c spec.tab.h spec.yy.c spec.yy.patched.c   \
                tlcf.tab.c tlcf.tab.h tlcf.yy.c

INSTALL_DIR     =   $(TWL_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS =   $(TARGETS)

#---------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules.x86

do-build:   $(TARGETS)

$(TARGETS): $(OBJS) $(MAKEFILE)
	$(CC_X86) $(OBJS) -o $@

$(OBJS):%.o:
	$(COMPILE_C)

$(OBJDIR)/makelcf.o:            makelcf.c makelcf.h version.c
$(OBJDIR)/container.o:          container.c makelcf.h
$(OBJDIR)/createlcf.o:          createlcf.c makelcf.h tlcf.tab.c
$(OBJDIR)/defval.o:             defval.c misc.h defval.h makelcf.h
$(OBJDIR)/misc.o:               misc.c misc.h defval.h makelcf.h

$(OBJDIR)/spec.tab.o:           spec.tab.c makelcf.h
$(OBJDIR)/spec.yy.patched.o:    spec.yy.patched.c spec.tab.c makelcf.h
$(OBJDIR)/spec.tab.o:           spec.tab.c
$(OBJDIR)/tlcf.yy.o:            tlcf.yy.c tlcf.tab.c makelcf.h
$(OBJDIR)/tlcf.tab.o:           tlcf.tab.c
$(OBJDIR)/version.o:            version.c

spec.yy.c:      spec.l spec.tab.c
spec.tab.c:     spec.y
tlcf.yy.c:      tlcf.l
tlcf.tab.c:     tlcf.y

$(OBJDIR)/spec.yy.patched.o:    WARNING += -Wno-unused-label -Wno-unused-function

spec.yy.patched.c:  spec.yy.c
	sed 's/^#define YY_BUF_SIZE 16384/#define YY_BUF_SIZE (1024*1024)/' $< > $@

version.c:      $(filter-out version.c,$(SOURCES)) $(HEADERS) $(MAKEFILE)
	@for i in $^ ;                                                  \
    do                                                              \
        date -r $$i +'const unsigned long SDK_DATE_OF_LATEST_FILE=%Y%m%dUL;';    \
    done | sort | tail -1 > $@

#===== End of Makefile =====
