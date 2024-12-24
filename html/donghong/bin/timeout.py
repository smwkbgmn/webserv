#!/usr/bin/python
import sys
import os
import time
import json

def main():
    # Print CGI header first
    print("Content-Type: application/json")
    print("")  # Empty line to separate headers from body
    
    try:
        # Sleep for 35 seconds
        time.sleep(35)
        
        # Prepare response data
        response = {
            "status": "success",
            "message": "Response after 35 second delay",
            "timestamp": time.time(),
            "environment": {
                "REQUEST_METHOD": os.environ.get("REQUEST_METHOD", ""),
                "QUERY_STRING": os.environ.get("QUERY_STRING", ""),
                "CONTENT_LENGTH": os.environ.get("CONTENT_LENGTH", ""),
                "CONTENT_TYPE": os.environ.get("CONTENT_TYPE", ""),
                "SCRIPT_NAME": os.environ.get("SCRIPT_NAME", ""),
                "PATH_INFO": os.environ.get("PATH_INFO", ""),
                "SERVER_NAME": os.environ.get("SERVER_NAME", ""),
                "SERVER_PORT": os.environ.get("SERVER_PORT", ""),
                "SERVER_PROTOCOL": os.environ.get("SERVER_PROTOCOL", "")
            }
        }
        
        # Send response
        print(json.dumps(response, indent=2))
        
    except Exception as e:
        error_response = {
            "status": "error",
            "message": str(e),
            "timestamp": time.time()
        }
        print(json.dumps(error_response))
        sys.exit(1)

if __name__ == "__main__":
    main()