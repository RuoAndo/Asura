#!/usr/bin/perl -w

my $filein1 = $ARGV[0];
open(FILEHANDLE1, $filein1);
@list1 = <FILEHANDLE1>;;
@list1 = sort { (split(/\,/,$a))[7] <=> (split(/\,/,$b))[7] } @list1;
print reverse(reverse((@list1)));

close(FILEHANDLE1);
close(OUT1);
