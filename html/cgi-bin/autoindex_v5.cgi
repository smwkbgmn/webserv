#!/opt/homebrew/bin/php
<?php

// Get the path information from PATH_INFO
$path_info = isset($_SERVER['PATH_INFO']) ? $_SERVER['PATH_INFO'] : '';

// Set the base directory to search
$base_directory = 'html/'; // Change this to your desired directory

// Construct the full directory path
$directory = rtrim($base_directory . $path_info, '/');

// Check if the directory exists and is readable
if (is_dir($directory) && is_readable($directory)) {
    // Open the directory
    if ($handle = opendir($directory)) {
        // Count the number of entries
        $num_entries = 0;
        while (false !== readdir($handle)) {
            $num_entries++;
        }

        // Calculate content length
        $content_length = $num_entries * strlen("<li><a href=\"/\">Sample</a></li>");

        // Set and print HTTP headers
        // header("Content-Type: text/html");
        // header("Content-Length: $content_length");

		echo "Content-Type: text/html\r\n";
        echo "Content-Length: $content_length\r\n\r\n";
        
        // Output HTML for the autoindex
        echo "<html><head><title>Autoindex</title></head><body>";
        echo "<h1>Index of $path_info</h1>";
        echo "<ul>";

        // Rewind the directory handle
        rewinddir($handle);

        // Loop through each entry in the directory
        while (false !== ($entry = readdir($handle))) {
            // Skip current and parent directory entries
            if ($entry != "." && $entry != "..") {
                // Output a link to the entry
                echo "<li><a href=\"/$entry/\">$entry</a></li>";
            }
        }
        
        // Close the directory handle
        closedir($handle);
        
        echo "</ul></body></html>";
    } else {
        // Error opening directory
        echo "Error opening directory.";
    }
} else {
    // Directory not found or not readable
    echo "Directory not found or not readable.";
}
?>
