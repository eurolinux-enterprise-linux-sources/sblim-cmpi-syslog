#!/usr/bin/perl

use strict;
use warnings;

my $className="Syslog_MessageLog.instance";
my $INST_FILE;

my $IN=$ARGV[0];

if(defined $IN) {
  if($IN =~ /-rm/) {
    unlink("$className");
    exit 0;
  }
}

my @out=`./messagelog.sh`;

if( !open($INST_FILE,'>', "$className")) {
  print "can not open $className\n"; 
  exit 0;
}

foreach my $out (@out) {
  $out =~ s/\-//;
  print $INST_FILE "$out\n";
}
print $INST_FILE "\n";

