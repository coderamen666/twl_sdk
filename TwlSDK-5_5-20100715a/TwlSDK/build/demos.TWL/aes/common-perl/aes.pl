package aes;

use FileHandle;
use IPC::Open2;
use strict;

my $aesconv = "../../../../tools/bin/aesconv.exe";

sub ctr
{	my( $rKey, $rCounter, $rSrc ) = @_;
	my $key = unpack("H*", $$rKey);
	my $ctr = unpack("H*", $$rCounter);
	my $encrypted = "";
	my $buf;

	open2(*PIPE_IN, *PIPE_OUT, "$aesconv -m ctr -k hex:$key -c hex:$ctr") or die "cant exec aesconv\n";
	binmode PIPE_IN;
	binmode PIPE_OUT;
	print PIPE_OUT $$rSrc;
	close(PIPE_OUT);

	while( read(PIPE_IN, $buf, 1024) == 1024 )
	{
		$encrypted .= $buf;
	}

	$encrypted .= $buf;
	close(PIPE_IN);

	return \$encrypted;
}

sub ccm_encrypt
{	my( $rKey, $rNonce, $rAdata, $rPdata, $tlen ) = @_;
	my $key = unpack("H*", $$rKey);
	my $nonce = unpack("H*", $$rNonce);
	my $adata_len = length($$rAdata);
	my $encrypted = "";
	my $buf;

	open2(*PIPE_IN, *PIPE_OUT, "$aesconv -m ccm -k hex:$key -n hex:$nonce -a stdin:$adata_len -t $tlen") or die "cant exec aesconv\n";
	binmode PIPE_IN;
	binmode PIPE_OUT;
	print PIPE_OUT $$rAdata;
	print PIPE_OUT $$rPdata;
	close(PIPE_OUT);

	while( read(PIPE_IN, $buf, 1024) == 1024 )
	{
		$encrypted .= $buf;
	}

	$encrypted .= $buf;
	close(PIPE_IN);

	return \$encrypted;
}


1;
