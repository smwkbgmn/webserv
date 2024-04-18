#!/usr/bin/php
<?php

// Get the target directory path from TRANSLATED_PATH
$target_directory = isset($_SERVER['PATH_TRANSLATED']) ? $_SERVER['PATH_TRANSLATED'] : '';
$target_directory = rtrim($target_directory, '/');

// Check if the target directory exists and is readable
if (is_dir($target_directory) && is_readable($target_directory)) {
    // Open the directory
    if ($handle = opendir($target_directory)) {
        // Start building the HTTP response message
        $response_line = "HTTP/1.1 200 OK\r\n";
        $content_type = "Content-Type: text/html\r\n";
        
        // Count the total length of the response content
        $content_length = 0;
        
        // Start building the HTML content
        $html_content = "<html><head><title>Index of " . htmlspecialchars($target_directory) . "</title></head><body>";
        $html_content .= "<h1>Index of " . htmlspecialchars($target_directory) . "</h1>";
        $html_content .= "<ul>";
        
        // Loop through each entry in the directory
        while (false !== ($entry = readdir($handle))) {
            // Skip current and parent directory entries
            if ($entry != "." && $entry != "..") {
                // Add an item to the HTML content
                $html_content .= "<li>" . htmlspecialchars($entry) . "</li>";
                
                // Increment the content length
                $content_length += strlen($entry) + 5; // 5 is for <li></li> tags
            }
        }
        
        // Close the directory handle
        closedir($handle);
        
        // Finish building the HTML content
        $html_content .= "</ul></body></html>";
        
        // Update the content length with the HTML content length
        $content_length += strlen($html_content);
        
        // Build the Content-Length header
        $content_length_header = "Content-Length: $content_length\r\n";
        
        // Output the complete HTTP response message
        echo $response_line;
        echo $content_type;
        echo $content_length_header;
        echo "\r\n"; // End of headers
        echo $html_content;
    } else {
        // Error opening directory
        // echo "HTTP/1.1 500 Internal Server Error\r\n";
        // echo "Content-Type: text/plain\r\n";
        // echo "Content-Length: 28\r\n\r\n";
        // echo "Error opening directory.\r\n";
        echo "HTTP/1.1 303 See Other\r\n";
        echo "Location: /50x.html\r\n";
        echo "\r\n";
    }
} else {
    // Directory not found or not readable
    // echo "HTTP/1.1 404 Not Found\r\n";
    // echo "Content-Type: text/plain\r\n";
    // echo "Content-Length: 25\r\n\r\n";
    // echo "Directory not found or not readable.\r\n";
    echo "HTTP/1.1 303 See Other\r\n";
    echo "Location: /40x.html\r\n";
    echo "\r\n";
}
?>
