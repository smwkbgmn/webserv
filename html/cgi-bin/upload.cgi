use CGI;

my $cgi = CGI->new;

# Get the boundary string
my $boundary = $cgi->boundary();

# Read from standard input
while (my $line = <STDIN>) {
    last if index($line, "--$boundary") == 0; # Check for final boundary
    # Process each part of the request here
}
