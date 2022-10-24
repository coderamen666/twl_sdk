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

my( $src_a_file, $src_p_file, $tlen, $key_file, $nnc_file, $dst_file ) = @ARGV;

die "invalid key file size\n" if -s $key_file != 16;
die "Adata size must be a multiple of 16\n" if (-s $key_file) % 16 != 0;

srand;

my $rAdata = &demo::read_file($src_a_file);
my $rPdata = &demo::read_file($src_p_file);
my $rKey   = &demo::read_file($key_file);
my $rOutput;

if( length($$rPdata) % 16 != 0 )
{
	my $padBytes = 16 - (length($$rPdata) % 16);
	$$rPdata .= pack("x$padBytes");
}

if( $dec_mode )
{
	my $result;
	my $rNonce = &demo::read_file($nnc_file);
	($rOutput, $result) = &aes::ccm_decrypt($rKey, $rNonce, $rAdata, $rPdata, $tlen);
	print $result, "\n";
}
else
{
	my $rNonce = &demo::generate_rand(12);
	$rOutput = &aes::ccm_encrypt($rKey, $rNonce, $rAdata, $rPdata, $tlen);
	&demo::write_file($nnc_file, $rNonce);
}

&demo::write_file($dst_file, $rOutput);

exit;


