#! bash -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     conv2svnDir.sh
#
# Copyright 2007 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2007-07-31#$
# $Rev: 387 $
# $Author: terui $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Check arguments
#----------------------------------------------------------------------------
if [ -z $1 ];
then
	input_name="."
else
	input_name=$1
fi
if [ ! -d $input_name ];
then
	echo -e "\"$input_name\" is not directory."
	exit -1
fi

#----------------------------------------------------------------------------
# Search files in directory and convert header string 
#----------------------------------------------------------------------------
for file_kind in *.txt *.html *.c *.h *.l *.y *.rsf *.lsf *.lcf *.template *.pl *.sh makefile* Makefile* commondefs* modulerules* @* _*
do
	for file in `find $input_name -name "$file_kind"`
	do
		if [ `expr match $file .*/\.svn/.*` = 0 -a `expr match $file .*/CVS/.*` = 0 ];
		then
			echo -e "converting -> $file"
			perl -s conv2svnFormat.pl $file
		fi
	done
done
