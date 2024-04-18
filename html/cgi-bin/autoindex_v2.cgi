#!/usr/bin/php
<?php

// Get the path information from PATH_INFO
$path_info = isset($_SERVER['PATH_INFO']) ? $_SERVER['PATH_INFO'] : '';

// Set the base directory to search
$base_directory = './html'; // Change this to your desired directory

// Construct the full directory path
// $directory = rtrim($base_directory, '/') . $path_info;
$directory = rtrim($path_info, '/');

// Check if the directory exists and is readable
if (is_dir($directory) && is_readable($directory)) {
    // Open the directory
    if ($handle = opendir($directory)) {
        // Output HTML for the autoindex
        echo "<html><head><title>Autoindex</title></head><body>";
        echo "<h1>Index of $path_info</h1>";
        echo "<ul>";

        // Loop through each entry in the directory
        while (false !== ($entry = readdir($handle))) {
            // Skip current and parent directory entries
            if ($entry != "." && $entry != "..") {
                // Output a link to the entry
                // echo "<li><a href=\"$path_info/$entry\">$entry</a></li>";
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
