#!/usr/bin/perl

use strict;
use warnings;

# Print CGI headers
print "Content-Type: text/plain\r\n\r\n";

# Read and print all input from STDIN (raw HTTP request data)
while (my $line = <STDIN>) {
    print $line;
}