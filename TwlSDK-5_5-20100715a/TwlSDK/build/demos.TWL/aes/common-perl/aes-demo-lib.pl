package demo;


sub read_file
{	my( $path ) = @_;
	my $data;

	open(IN, $path) or die "cant open '$path'\n";
	binmode IN;
	read(IN, $data, -s $path);
	close(IN);

	return \$data;
}

sub write_file
{	my( $path, $rData ) = @_;

	open(OUT, ">$path") or die "can't open '$path'\n";
	binmode OUT;
	print OUT $$rData;
	close(OUT);
}

sub generate_rand
{	my $size = shift;
	my @rand = ();

	for( my $i = 0; $i < $size; ++$i )
	{
		push @rand, ((rand() * 0x100) & 0xFF);
	}

	my $rand = pack("C*", @rand);

	return \$rand;
}


1;
