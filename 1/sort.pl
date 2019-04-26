#!/usr/bin/perl -w

my $filein1 = $ARGV[0];
open(FILEHANDLE1, $filein1);
@list1 = <FILEHANDLE1>;;
@list1 = reverse sort { (split(/\,/,$a))[5] <=> (split(/\,/,$b))[5] } @list1;
print @list1;
close(FILEHANDLE1);
close(OUT1);
