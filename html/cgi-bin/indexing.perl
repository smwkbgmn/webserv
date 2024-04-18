#!/usr/bin/perl

use strict;
use warnings;
use CGI;
use File::Basename;

# Create CGI object
my $cgi = CGI->new;

# Get the target URI from the query parameters
my $target_uri = $cgi->param('uri');

# Check if the target URI is provided
unless ($target_uri) {
    print $cgi->header(-status => '400 Bad Request');
    print "Error: Target URI not provided!";
    exit;
}

# Extract the directory path from the URI
my ($directory) = $target_uri =~ m|^([^/]+)/?$|;

# Build the absolute path to the directory
my $absolute_path = "./html/$directory";  # Adjust this path according to your server configuration

# Print the HTML header
print $cgi->header;
print $cgi->start_html(-title => 'Autoindex');

# Print the heading
print $cgi->h1("Index of $target_uri");

# Open the directory corresponding to the target URI
opendir(my $dh, $absolute_path) || die "can't opendir $absolute_path: $!";

# List files and directories
print $cgi->start_ul;
while (my $entry = readdir $dh) {
    next if $entry eq '.' || $entry eq '..';
    my $entry_path = "$directory/$entry";
    my $link = -d "$absolute_path/$entry" ? "$target_uri$entry/" : "$target_uri$entry";
    print $cgi->li($cgi->a({-href=>$link}, $entry));
}
print $cgi->end_ul;

# Close the directory handle
closedir $dh;

# Print HTML footer
print $cgi->end_html;