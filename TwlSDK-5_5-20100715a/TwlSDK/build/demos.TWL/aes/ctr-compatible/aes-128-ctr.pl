#!/bin/perl

use strict;

require "../common-perl/aes.pl";
require "../common-perl/aes-demo-lib.pl";

my $dec_mode = 0;

if( $ARGV[0] eq "-d" )
{
	$dec_mode = 1;
	shift @ARGV;
}

my( $src_file, $key_file, $ctr_file, $dst_file ) = @ARGV;

die "invalid key file size\n" if -s $key_file != 16;

srand;

my $rSrc = &demo::read_file($src_file);
my $rKey = &demo::read_file($key_file);
my $rCounter;

if( $dec_mode )
{
	$rCounter = &demo::read_file($ctr_file);
}
else
{
	$rCounter = &demo::generate_rand(16);
}

my $encrypted = &aes::ctr($rKey, $rCounter, $rSrc);

&demo::write_file($ctr_file, $rCounter) unless $dec_mode;
&demo::write_file($dst_file, $encrypted);

exit;



