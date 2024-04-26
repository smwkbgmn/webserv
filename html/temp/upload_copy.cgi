#!/usr/bin/perl

use strict;
use warnings;
use CGI;

# Create CGI object
my $cgi = CGI->new;

# Print HTTP header with content type
print $cgi->header(-type => 'text/plain');

# Check if file upload is present
if ($cgi->upload('file')) {
    my $filename = $cgi->param('file');

    # Specify the directory where uploaded files will be saved
    my $upload_dir = '/path/to/upload/directory/';

    # Construct the full path to the uploaded file
    my $upload_file = $upload_dir . $filename;

    # Save the uploaded file
    open my $upload_fh, '>', $upload_file or die "Cannot open $upload_file: $!";
    while (my $line = $cgi->upload('file')->getline) {
        print $upload_fh $line;
    }
    close $upload_fh;

    print "File '$filename' uploaded successfully.\n";
} else {
    print "No file uploaded.\n";
}
