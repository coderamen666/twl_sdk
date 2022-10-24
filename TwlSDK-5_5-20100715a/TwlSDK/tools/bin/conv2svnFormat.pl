#!/usr/bin/perl -s
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     conv2svnFormat.pl
#
# Copyright 2007 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2007-09-19#$
# $Rev: 997 $
# $Author: ooshimay $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Get the current year
#----------------------------------------------------------------------------
sub GetThisYear
{
	my	@array	=	localtime(time);

	return (@array[5] + 1900);
}

#----------------------------------------------------------------------------
# Main routine
#----------------------------------------------------------------------------
{
	my	$input_file;
	my	$output_file;
	my	$line;

	# Copy input file to separately named file 
	$output_file	=	@ARGV[0];
	$input_file		=	$output_file . ".convbackup";
	system `cp -rf $output_file $input_file`;

	open INPUTFILE, "$input_file" or die "ERROR: Cannot open file \'$input_file\'\n";
	open OUTPUTFILE, ">$output_file" or die "ERROR: Cannot create file \'$output_file\'\n";

	while ($line = <INPUTFILE>)
	{
		# Convert project character string 
		if ($line =~ m/.*Project: *NitroSDK.*$/)
		{
			$line =~ s/NitroSDK/TwlSDK/;
		}
	
		# Convery copyright year character string 
		if ($line =~ m/.*Copyright [0-9]+.*$/)
		{
			my	$start	=	$line;
			my	$now	=	GetThisYear();
		
			$start	=~	s/.*Copyright ([0-9]+).*\n/$1/;
			if ($start >= $now)
			{
				$line	=~	s/Copyright [0-9\-,]+/Copyright $now/;
			}
			else
			{
				$line	=~	s/Copyright [0-9\-,]+/Copyright $start/;
			}
		}
	
		# Replace log line with automatically converted string 
		if ($line =~ m/\${1}Log.*\$/)	# Verbose expression so this won't mistakenly convert itself 
		{
			my	$prefix	=	$line;
			my	$suffix	=	$line;
		
			$prefix	=~	s/^(.*)\${1}Log.*\$.*\n/$1/;	# Verbose expression so this won't mistakenly convert itself 
			#$suffix	=~	s/.*(\r?\n)$/$1/;
			if ($line =~ m/.*\r\n$/)
			{
				$suffix	=	"\r\n";
			}
			else
			{
				$suffix	=	"\n";
			}
		
			print OUTPUTFILE $prefix . "\$" . "Date::            \$" . $suffix;
			print OUTPUTFILE $prefix . "\$" . "Rev:\$" . $suffix;
			print OUTPUTFILE $prefix . "\$" . "Author:\$" . $suffix;
		
			while ($line = <INPUTFILE>)
			{
				if ((index($line, $prefix, 0) == 0) && ($line =~ m/.*Revision [0-9\.]+.*/))
				{
					while ($line = <INPUTFILE>)
					{
						$line	=~	s/\r?\n$//;
						if (length($line) <= length($prefix))
						{
							last;
						}

						# Exit if gone all the way to NoKeywords 
						if ($line =~ m/\${1}NoKeywords.*\$/)
						{
							last;
						}
					}
				}
				else
				{
					last;
				}
			}
		}
	
		# Delete NoKeywords line 
		if ($line =~ m/\${1}NoKeywords.*\$/)	# Redundant expression so this won't mistakenly convert itself 
		{
			next;
		}
	
		# Output lines that do not match patterns as they are 
		print OUTPUTFILE $line;
	}

	close OUTPUTFILE;
	close INPUTFILE;

	# Add sub-version attribute to output file 
	system `svn propset -q --force "svn:keywords" "Id URL Author Date Rev" $output_file`;
	if ($output_file =~ m/.*\.sh$/)
	{
		system `svn propset -q --force "svn:eol-style" "LF" $output_file`;
		system `svn propset -q --force "svn:executable" "1" $output_file`;
	}
	elsif ($output_file =~ m/.*\.pl$/)
	{
		system `svn propset -q --force "svn:eol-style" "CRLF" $output_file`;
		system `svn propset -q --force "svn:executable" "1" $output_file`;
	}
	else
	{
		system `svn propset -q --force "svn:eol-style" "CRLF" $output_file`;
	}

	# Delete temporarily copied files 
	system `rm -rf $input_file`
}
