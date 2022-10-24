#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - root
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

include		$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

SUBDIRS 	= build


#----------------------------------------------------------------------------

include		$(TWLSDK_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------
.PHONY:		setup

setup:
	+$(MAKE) -C build/buildsetup/libcxx

#----------------------------------------------------------------------------
.PHONY:	do-copyright

SDKDIR_BUILD	= build
SDKDIR_INCLUDE	= include
SDKCOPYRIGHT	= $(TWL_TOOLSDIR)/bin/sdkindent.copyright

do-copyright:
	@echo ===== COPYRIGHT STAGE =====
	/bin/find $(SDKDIR_BUILD)   -type f \
		-name '*' ! -name '*.yy.c' ! -name '*.yy.patched.c' ! -regex ".*\/\.svn\/.*" \
		-size -128k -print \
		-mtime -`/bin/date +%j` \
		\! -exec grep -q 'Copyright [0-9 -]*2009' {} \; \
		-exec $(SDKCOPYRIGHT) {} \;
	/bin/find $(SDKDIR_INCLUDE) -type f \
		-name '*' ! -name '*.yy.c' ! -name '*.yy.patched.c' ! -regex ".*\/\.svn\/.*" \
		-size -128k -print \
		-mtime -`/bin/date +%j` \
		\! -exec grep -q 'Copyright [0-9 -]*2009' {} \; \
		-exec $(SDKCOPYRIGHT) {} \;
#----------------------------------------------------------------------------
.PHONY:	check_code check_lang_en check_copyright_date_2009

CHECKEES	= build include

check_code: check_lang_en

#
#  Check backslash code for language EN
#	Inverse result code of grep (e.g. grep returns 1 if not found)
#	 -> Error if grep finds 0x5c(\) on 2nd byte of 16bit-wide 
#           Japanese kanji code at the end of line
#
CHECK_EXCLUDES	= *.txt *.o *.a *.d *.map *.elf Entries* Repository Root

check_lang_en:
	@echo --- $@ ---
	-@(! grep -I -n -r $(patsubst %, --exclude='%',$(CHECK_EXCLUDES)) '[^[:space:] -~]\\[^ -~]\?$$' $(CHECKEES))

#
#  Check Copyright date for 2009
#
check_copyright_date_2009:
	@echo --- $@ ---
	-@for dir in $(CHECKEES); do \
	    /bin/find $$dir \
		\! \( -type d \( -name '_*' -o \
				 -name \.svn  -o \
				 -exec /bin/test -e {}/_ \; \) -prune \) \
		-type f \
		\! -name '*.exe' \! -name '*.[dao]' \! -name '*.sbin' \
		\! -name '*.nlf' \! -name '*.nef'   \! -name '*.xMAP' \
		-mtime -`/bin/date +%j` \
		-exec grep -q 'Copyright [0-9 ,-]* Nintendo' {} \; \
		\! -exec grep -q 'Copyright [0-9 ,-]*2009' {} \; \
		-printf "Error: Copyright Date: %p\n"; \
	done

#===== End of Makefile =====
