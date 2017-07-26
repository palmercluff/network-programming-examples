/****************************************************************************
* Program:
*    Lab WebserverT2, Computer Communication and Networking
*    Brother Jones, CS 460
* Author:
*    Palmer Cluff
* Summary:
*    This program is a Java Web Server that will display html, text, JPEG's,
*    and GIF's. Will display in the directory it was ran in. Part A and B
*    are included in this file.
*****************************************************************************
* Changes made to my code for the re-submission:
*   
*   -I fixed the spacing on line 47, which is a comment. No other changes
*     were made becasue it works.
*
*
*****************************************************************************/

import java.io.*;
import java.net.*;
import java.util.*;

public final class WebServer
{
   public static void main(String argv[]) throws Exception
   {
      // sets the port number to be used (default: 6789; optional: argv[0])
      int port = argv.length > 0 ? Integer.parseInt(argv[0]) : 6789;
      
      try
      {
         // Establish the listen socket.
         ServerSocket serverSocket = new ServerSocket(port);
         
         while(true)
         {
            // Listen for a TCP connection request.
            Socket clientSocket = serverSocket.accept();
            
            // Construct an object to process the HTTP request message.
            HttpRequest request = new HttpRequest(clientSocket);
            
            // Create a new thread to process the request.
            Thread thread = new Thread(request);
            
            // Start the thread.
            thread.start();
         }
      } catch (Exception e){
         System.out.println(e);
      }
   }
}

final class HttpRequest implements Runnable
{
   final static String CRLF = "\r\n";
   Socket socket;
   
   // Constructor
   public HttpRequest(Socket socket) throws Exception
   {
      this.socket = socket;
   }
   
   // Implement the run() method of the Runnable interface.
   public void run()
   {
      //Do Stuff
      try {
         processRequest();
      }
      catch (Exception e){
         System.out.println(e);
      }
   }
   private void processRequest() throws Exception
   {
      // Get a reference to the socket's input and output streams.
      InputStream is = socket.getInputStream();
      
      DataOutputStream os = new DataOutputStream(socket.getOutputStream());
      
      //Set up input stream filters   
      BufferedReader br = new BufferedReader(new InputStreamReader(is));
                                             
      //Get the request line of the HTTP request message.
      String requestLine = br.readLine();

      // Display the request line.
      System.out.println();
      System.out.println(requestLine);

      // Get and display the header lines.
      String headerLine = null;
      
      while ((headerLine = br.readLine()).length() != 0)
      {
         System.out.println(headerLine);
      }
      
      // Extract the filename from the request line.
      StringTokenizer tokens = new StringTokenizer(requestLine);
      tokens.nextToken(); // skip over the method, which should be "GET"
      String fileName = tokens.nextToken();

      // Prepend a "." so that the file request is within the current directory
      fileName = "." + fileName;

      //Open the requested file.
      FileInputStream fis = null;
      boolean fileExists = true;
      
      try
      {
         fis = new FileInputStream(fileName);
      }
      catch (FileNotFoundException e)
      {
         fileExists = false;
      }
    
      // Construct the response message.
      String statusLine = null;
      String contentTypeLine = null;
      String entityBody = null;
      
      if (fileExists)
      {
         statusLine = "HTTP/1.0 200 OK";
          contentTypeLine = "Content-type: " +
             contentType( fileName ) + CRLF;
      } else {
         statusLine = "HTTP/1.0 404 NOT FOUND";
         contentTypeLine = "Content-type:" + contentType( fileName ) + CRLF;
          entityBody = "<HTML>" +
              "<HEAD><TITLE>Not Found</TITLE></HEAD>" +
             "<BODY>Not Found</BODY></HTML>";
      }
      
      // Send the status line.
      os.writeBytes(statusLine);
      
      // Send the content type line.
      os.writeBytes(contentTypeLine);
      
      //Send a a blank line to indicate the end of the header
      os.writeBytes(CRLF);

      // Send the entity body/page that will go to the user
      if (fileExists) {
         sendBytes(fis, os);
         fis.close();
      } else {
         os.writeBytes(entityBody);
      }
      
      // Close streams and socket.
      os.close();
      br.close();
      socket.close();
   }
   private static void sendBytes(FileInputStream fis, OutputStream os)
   throws Exception
   {
      // Construct a 1K buffer to hold bytes on their way to the socket.
      byte[] buffer = new byte[1024];
      int bytes = 0;
      
      // Copy requested file into the socket's output stream.
      while((bytes = fis.read(buffer)) != -1 )
      {
         os.write(buffer, 0, bytes);
      }
   }

   private static String contentType(String fileName)
   {
      if (fileName.endsWith(".htm") || fileName.endsWith(".html"))
      {
         return "text/html";
      }
      
      if(fileName.endsWith(".gif") || fileName.endsWith(".GIF"))
      {
         return "image/gif";
      }
      
      if(fileName.endsWith(".jpeg"))
      {
         return "image/jpeg";
      }
      
      if(fileName.endsWith(".java"))
      {
         return "java file";
      }
      
      if(fileName.endsWith(".sh"))
      {
         return "bourne/awk";
      }
      
      return "application/octet-stream";
   }
}
