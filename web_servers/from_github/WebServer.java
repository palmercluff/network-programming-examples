/****************************************************************************
* Program:
*    Lab Webserver, Computer Communication and Networking
*    Brother Jones, CS 460
* Author:
*    Tyler Jones
* Summary:
*    This program creates and runs a webserver capable of returning 
*	  requested webpages as well as handling 404 errors all while
*	  determining content type and handling multiple clients.
*	  
************************************************************************** */
/* Changes made to my code for the re-submission:
*   Added an extra CRLF to the ouput so that 404s were handled correctly.
* 
*
* Please give a score, on a scale of 0 to 10, to yourself and your group
* partners on their participation in the group work:
*
*   Yourself: Tyler Jones______________ Score: _10_
*
*   Student: Ron Moore_________________ Score: _10_
*
*   Student: Joe Dzado_________________ Score: _10_
*
*****************************************************************************/

import java.io.* ; 
import java.net.* ; 
import java.util.* ;

public final class WebServer 
{
	public static void main(String argv[]) throws Exception 
	{
		int port=6789;
		// Establish the listen socket.
		ServerSocket serverSocket = null;
		try
		{
			serverSocket = new ServerSocket(port);
		}
		catch (IOException e)
		{
			System.err.println("Could not listen on port." + e);
			System.exit(-1);
		}
		// Process HTTP service requests in an infinite loop. 
		while (true) 
		{
			// Listen for a TCP connection request. 
			try
			{
				new HttpRequest(serverSocket.accept()).run();
			}
			catch(Exception e)
			{System.out.println(e);}
		}
	}
}
final class HttpRequest implements Runnable 
{

	Socket socket;
	final static String CRLF = "\r\n"; 
	// Constructor 
	public HttpRequest(Socket socket) throws Exception 
	{
		this.socket = socket;
	}
	// Implement the run() method of the Runnable interface. 
	public void run() 
	{
		try 
		{ 
			processRequest();
		} 
		catch (Exception e) 
		{ 
			System.out.println(e);
		}
	}
	private void processRequest() throws Exception 
	{
// Get a reference to the socket's input and output streams. 
		InputStream is = socket.getInputStream();
		DataOutputStream os = new DataOutputStream(socket.getOutputStream());
// Set up input stream filters. 
		BufferedReader br = new BufferedReader(
											new InputStreamReader(
													socket.getInputStream()));

		//Get the request line of the HTTP request message
		String requestLine = br.readLine();

		//Display the request line.
		System.out.println();
		System.out.println(requestLine);

		// Get and display header lines.
		String headerLine = null;
		while ((headerLine = br.readLine()).length() != 0)
		{
			System.out.println(headerLine);
		}

		StringTokenizer tokens = new StringTokenizer(requestLine);
		tokens.nextToken(); //skip over the method, which should be "GET"
		String fileName = tokens.nextToken();
		
		// prepend a "." so that file request is within the current directory.
		fileName = "." + fileName;

		//open the requested file.
		FileInputStream fis = null;
		boolean fileExists = true;
		try
		{
			fis = new FileInputStream(fileName);
		}
		catch(FileNotFoundException e)
		{
			fileExists = false;
		}

		//construct the response message
		String statusLine = null;
		String contentTypeLine = null;
		String entityBody = null;
		if(fileExists)
		{
			statusLine = "200";
			contentTypeLine = "Content-type: " +
				contentType( fileName ) + CRLF + CRLF;
		}
		else
		{
			statusLine = "404"+ CRLF;
			contentTypeLine = "Content-type: " +
				contentType( fileName ) + CRLF + CRLF;		
			entityBody = "<HTML>" +
				"<HEAD><TITLE>Not Found</TITLE></HEAD>" +
				"<BODY>Not Found</BODY></HTML>";
		}

		//send the status line.
		os.writeBytes(statusLine);
		//send the content type line.
		os.writeBytes(contentTypeLine);
		//send a blank line to indicate the end of the header lines.
		os.writeBytes(CRLF);

		//send the entity body.
		if(fileExists)
		{
			sendBytes(fis, os);
			fis.close();
		}
		else
		{
			os.writeBytes("Oh no. Something gave us the slip Mr Charles.");
		}

		os.close();
		br.close();
		socket.close();		

	}
		private static void sendBytes(FileInputStream fis, OutputStream os)
			throws Exception
		{
			//Construct a 1k buffer to hold bytes on their way to the socket.
			byte[] buffer = new byte[1024];
			int bytes = 0;

			//copy requested file into the socket's output stream.
			while((bytes = fis.read(buffer)) != -1 )
			{
				os.write(buffer, 0, bytes);
			}
		}

		private static String contentType(String fileName)
		{
			if(fileName.endsWith(".htm") || fileName.endsWith(".html"))
			{
				return "text/html";
			}
			if(fileName.endsWith(".gif"))
			{
				return "image/gif";
			}
			if(fileName.endsWith(".png"))
			{
				return "image/png";
			}
			if(fileName.endsWith(".jpg") || fileName.endsWith(".jpeg"))
			{
				return "image/jpeg";
			}
			return "application/pcetet-stream";
		}

}

