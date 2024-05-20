import http.client

def send_chunked_post(host, port, url, chunks):
    # Establish a connection to the host
    connection = http.client.HTTPConnection(host, port)
    
    # Begin the POST request with headers
    headers = {
        'Transfer-Encoding': 'chunked',
        'Content-Type': 'application/json'
    }
    connection.putrequest('POST', url)
    for header, value in headers.items():
        connection.putheader(header, value)
    connection.endheaders()

    # Send the chunks
    for chunk in chunks:
        chunk_size = '{:X}\r\n'.format(len(chunk))
        connection.send(chunk_size.encode('utf-8'))
        connection.send(chunk.encode('utf-8'))
        connection.send(b'\r\n')
    
    # Signal the end of the request
    connection.send(b'0\r\n\r\n')
    
    # Get the response
    response = connection.getresponse()
    print(response.status, response.reason)
    print(response.read().decode())

    # Close the connection
    connection.close()

# Example usage
host = 'localhost'
port = 8080
url = '/freeboard.txt'
chunks = ['{"chunk": "data1"}', '{"chunk": "data2"}', '{"chunk": "data3"}']

send_chunked_post(host, port, url, chunks)
