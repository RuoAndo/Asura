#!/usr/bin/perl -w

my $infile  = $ARGV[0];
my $outfile = "[norepeat]$infile";

open(INPUT, $infile) or die "$infile: $!";
open(OUTPUT, "> $outfile") or die "$outfile: $!";

my %seen;
while(my $line = <INPUT>) {
    unless ($seen{$line}) {
        $seen{$line} = 1;
        print OUTPUT $line;
	print $line;
    }
}
close(INPUT);
close(OUTPUT);

