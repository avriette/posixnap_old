#!/usr/bin/perl -w
use strict;
require "pr0n.pl";





my ($url,$desc,$date);

my $host= "http://fhwl.com";
my @sites = qw{ /};

my @data;

foreach (@sites) {
#	foreach (    split /<LI>/, join ('', pr0n::fetchsite("$host$_"))       ) {
	foreach (pr0n::fetchsite("$host$_")) {
#	print "*******************\n$_\n";
			if ( ($url,$desc) = /<a href="(http[^"]+)">([^<]+)<\/a><!--POST-->$/) {
#				next unless $url =~  s/.*url=//;
				$date="now";
				#$desc =~ y/A-Z/a-z/;
				$desc =~ s/[0-9]+ pi\w+//g;
				$desc =~ s/\.$//g;
				$desc =~ s/ $//g;
				$desc =~ s/^ //g;
				$desc =~ s/\([^\)]+\)//;
				pr0n::consider( ($url,$desc,$date) ) unless $ARGV[0];	
				print "pr0n::consider( ($url,$desc,$date) ) \n" if $ARGV[0];
			}	
#		}
	}	
}

pr0n::fin();

