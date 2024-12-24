#!/usr/bin/perl
use strict;
use warnings;
use CGI;
use JSON;
use Time::HiRes qw(time);

# Create CGI object
my $cgi = CGI->new;

# Print CGI header
print $cgi->header('application/json');

eval {
    # Sleep for 35 seconds
    sleep(35);
    
    # Prepare response data
    my $response = {
        status => "success",
        message => "Response after 35 second delay",
        timestamp => time(),
        environment => {
            REQUEST_METHOD => $ENV{REQUEST_METHOD} || "",
            QUERY_STRING => $ENV{QUERY_STRING} || "",
            CONTENT_LENGTH => $ENV{CONTENT_LENGTH} || "",
            CONTENT_TYPE => $ENV{CONTENT_TYPE} || "",
            SCRIPT_NAME => $ENV{SCRIPT_NAME} || "",
            PATH_INFO => $ENV{PATH_INFO} || "",
            SERVER_NAME => $ENV{SERVER_NAME} || "",
            SERVER_PORT => $ENV{SERVER_PORT} || "",
            SERVER_PROTOCOL => $ENV{SERVER_PROTOCOL} || ""
        }
    };
    
    # Convert to JSON and print
    my $json = JSON->new->utf8->pretty;
    print $json->encode($response);
};

# Handle any errors
if ($@) {
    my $error_response = {
        status => "error",
        message => $@,
        timestamp => time()
    };
    
    my $json = JSON->new->utf8;
    print $json->encode($error_response);
    exit(1);
}