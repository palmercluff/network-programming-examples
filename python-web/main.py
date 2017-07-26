#############################################################################
# Program:
#    Lab PythonWebServerT2, Computer Communication and Networking
#    Brother Jones, CS 460
# Author:
#    Gage Peterson
# Summary:
#    A HTTP server written in python
#############################################################################
# Changes made to my code for the re-submission:
#   - Changed to using python's mimetype guessing
# 
# NOTE: I stuck with the docstrings despite the comments because it's good
#       python style to do so. Refrences: 
#        https://www.python.org/dev/peps/pep-0257/
#        https://google.github.io/styleguide/pyguide.html#Comments
##############################################################################

import sys
import re
import os.path
import mimetypes

from socket import *

DEFAULT_PORT = 6789
CRLF = "\r\n"

def error500(url):
   return "HTTP/1.0 500 Server Error" + CRLF + \
           "Content-type: text/html" + CRLF + CRLF + """
           <HTML>
                     <HEAD><TITLE>Server Error</TITLE></HEAD> +
                     <BODY>500 Server Error</BODY>
              </HTML>
            """

def error404(url):
   return "HTTP/1.0 404 Not Found" + CRLF + \
           "Content-type: text/html" + CRLF + CRLF + """
           <HTML>
                     <HEAD><TITLE>%s Not Found</TITLE></HEAD> +
                     <BODY> "%s" Was Not Found</BODY>
              </HTML>
            """ % (url, url)

def ok200(filetype):
    return "HTTP/1.0 200 OK" + CRLF + \
           "Content-type: " + content_type(filetype) + CRLF + CRLF;


def get_url(header):
   "This will get the URL out of a HTTP header"

   try:
      return header.split("\n")[0].split(" ")[1]

   except IndexError as e:
      print "Bad Header: '%s'" % header
      # this happens sometimes, perhaps a blank header?
      return "/"

def get_file(file_path, content_type):
   "This will figure out the headers for the given filetype"

   if os.path.isfile(file_path):

      # open files that are plain text as text files
      if re.match('text', content_type) != None:
         return open(file_path, "r").read()

      # open as binary otherwise
      else:
         return open(file_path, "rb").read()

   # file wasn't there
   else:
      return False

def content_type(file_name):
   "A simple wrapper around the mimetypes function"
   ( type, _ ) = mimetypes.guess_type(file_name)

   return type


#############################################################################
# This is main
#############################################################################
def main():
   """docstring for main"""

   port = int(sys.argv[1]) if len(sys.argv) == 2 else DEFAULT_PORT

   print "starting server on port: %s." % port

   serverSocket = socket(AF_INET, SOCK_STREAM)
   serverSocket.bind(('', port))
   serverSocket.listen(1)

   try:
      while 1:
         connectionSocket, addr = serverSocket.accept()
         header = connectionSocket.recv(8192)

         print header.split("\n")[0]

         file_name = "." + get_url(header);

         ct = content_type(file_name);

         contents = get_file(file_name, ct)

         if contents:
            response = ok200(file_name) + contents
         else:
            response = error404(file_name);

         connectionSocket.send(response)
         connectionSocket.close()

   except KeyboardInterrupt:
      print "\nClosing Server"
      serverSocket.close()



if __name__ == "__main__":
       main()

