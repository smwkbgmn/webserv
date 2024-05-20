#!/usr/bin/perl -w
use CGI;

# Get the upload directory from the environment variable, default to "html/upload"
$upload_dir = $ENV{'UPLOAD_DIR'} // "html/upload";

$query = new CGI;

$filename = $query->param("photo");
$email_address = $query->param("email_address");

# Sanitize the filename
$filename =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("photo");

# Open the file for writing
open UPLOADFILE, ">$upload_dir/$filename" or die "Cannot open $upload_dir/$filename: $!";

# Write the file to the specified directory
while (<$upload_filehandle>) {
    print UPLOADFILE;
}

close UPLOADFILE;

# Determine the URL path for the uploaded file
# Assuming the URL base path is the same as the filesystem path
# Adjust this if your URL structure is different
$upload_dir_url = $upload_dir;
$upload_dir_url =~ s/^html\///;

# Print the HTML response
print $query->header();
print <<END_HTML;
<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>
<BODY>
<P>Thanks for uploading your photo!</P>
<P>Your email address: $email_address</P>
<P>Your photo:</P>
<img src="/$upload_dir_url/$filename" border="0">
</BODY>
</HTML>
END_HTML
