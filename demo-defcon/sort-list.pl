#!/usr/bin/perl -w

my $filein1 = $ARGV[0];
open(FILEHANDLE1, $filein1);
@list1 = <FILEHANDLE1>;;
@list1 = sort { (split(/\,/,$a))[1] <=> (split(/\,/,$b))[1] } @list1;
print @list1;

close(FILEHANDLE1);
close(OUT1);
