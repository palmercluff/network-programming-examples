#############################################################################
# Program:
#    Lab PythonWebServerT2, Computer Communication and Networking
#    Brother Jones, CS 460
# Author:
#    Palmer Cluff
# Summary:
#    This is a python web server.
#
#    Sources: https://github.com/joshmaker/simple-python-webserver/
#             blob/master/server.py
#
#############################################################################
#
# Changes made to my code for the re-submission:
#
#   -For the first submission, my 404 error message page wasn't working
#    becasue I was trying to reference another student's file found
#    in a different directory. That obviously doesn't work, so I
#    hard-coded a 404 message in my Python Script.
#
##############################################################################


#import socket module
from socket import *
import mimetypes
import sys

serverSocket = socket(AF_INET, SOCK_STREAM)
serverPort = 6789

if (len(sys.argv) == 2):
    serverPort = int(sys.argv[1])

serverSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
serverSocket.bind(('', serverPort)) #set up socket connection


serverSocket.listen(1) #tells the server to try a maximum of one connect request before ending connection    

while True:
    #Establish the connection
    print 'Serving on port',serverPort

    connectionSocket, addr = serverSocket.accept()

    try:
        message = connectionSocket.recv(4096) #Makes it so that you can recieve message from client

        start = message.find('GET ') + 5
        stop  = message.find('HTTP') - 1

        filename = message[start:stop]
        f = open(filename)
        outputdata = f.read()

        content_type = mimetypes.MimeTypes().guess_type(filename)[0]

        #Send one HTTP header line into socket2
        connectionSocket.send('HTTP/1.0 200 OK')
        connectionSocket.send('Content-type: '+ content_type + '\n')
        connectionSocket.send('\n')
        connectionSocket.send(outputdata)
        connectionSocket.close()

    except IOError, e:
        #Send response message for file not found
        connectionSocket.send('HTTP/1.0 404 NOT FOUND')
        connectionSocket.send('Content-type: text/html\n')
        connectionSocket.send('\n')
        connectionSocket.send("<html><head><title>Not Found</title></head><body><h1>404: File not found</h1><br><p>The content you're attempting to retrieve does not exist</p></body></html>")
        connectionSocket.close()

serverSocket.close()
