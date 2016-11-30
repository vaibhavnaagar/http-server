

# Concurrent HTTP-Server #

* Server supports the GET method to retrieve files from it.
* It can handle requests from both protocols HTTP/1.1 and HTTP/1.0 .
* Multiple clients can connect and make request to it at the same time.
* Server perfectly provides appropriate Status-code and Response-Phrase values in response to errors or incorrect requests from client.
* Server makes persistent connections with clients upto certain extent i.e. once the connection is established between server and client, server then starts handling requests from it using same connection and process until client closes the connection.
* Server is designed such that it can run continuously until an unrecoverable error occurs.


### How to use it ###

* Run makefile using command "make" or "make all".
* It creates one execuatble file `http-server`.
* Run server using command: ./http-server [-p port-number] [-b base_directory]
* Arguments are optional 
* IMPORTANT: Base directory is default set to `webfiles` folder present in this same folder.


