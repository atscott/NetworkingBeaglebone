/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"

/*
 * This method will print an error message.
 * char *msg - This is the prelude to the error message that should be printed.
 */
void error(char *msg)
{
  // Print the error to stderr using the perror method.  See page 328 in reference for details.
    perror(msg);
    // Exit the system with a negative 1 return, indicating an error.
    exit(-1);
}

/*
 * This is the main method for the client.
 * There are two parameters.  argc must be 2.
 * argv[0] is the program name.
 * argv[1] is the port number.
 */

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char *buffer = malloc(BLOCKSIZE);
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int index;
	 uint64_t bytesReceived = 0;

     // If there are no enough parameters, abort.
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     // Create a socket.
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     // If the return is less than 0, then the socket failed to create.
     if (sockfd < 0)
     { 
        error("ERROR opening socket");
     }

     // Initialize the buffer to all zeros.
     memset((void*) &serv_addr, 0, sizeof(serv_addr));

     // Obtain the port number as an integer.
     portno = atoi(argv[1]);

     // Setup the server address structure.
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     // Bind the socket appropriately.
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     // Listen on the socket for an incoming connection.  The parameter is the number of connections that can be waiting / queued up.  5 is the maximum allowed by most systems.
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
 
     // Block until a client has connected to the server.  This returns a file descriptor for the connection.
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
	// If the return is less than 0l, there is an error.
    if (newsockfd < 0) 
    {
      error("ERROR on accept");
    }

	for (index = 0; index < ITERATION_COUNT; index++)
	{
	
	 // Fill the buffer with all zeros.
     memset(&buffer[0], 0, BLOCKSIZE);

     // Read from the buffer when data arrives.
     n = read(newsockfd,buffer,BLOCKSIZE);
	 
     if (n < 0)
	 {
		error("ERROR reading from socket");
	 }
	 else
	 {
	   bytesReceived+= n;
	 }
     
     // Print the message.
     printf("Here is the value sent: %d\n",buffer[0]);
	 printf("Bytes received: %llu\n", bytesReceived);

     
     n = write(newsockfd, &buffer[0], BLOCKSIZE);
     if (n < 0)
     {
       error("Error writing to socket.");
     }
	}
	 
     return 0; 
}
