/* A simple server in the internet domain using TCP
 The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "gpioInterface.h"
#include <unistd.h>

void setupGpioOutput(uint8_t gpioOutputPort);
void turnLightOn(int outputPin);
void turnLightOff(int outputPin);
void handleClientConnection(int newsockfd, uint8_t gpioOutputPortLight1);
int createServer(int portno);
/*
 * This method will print an error message.
 * char *msg - This is the prelude to the error message that should be printed.
 */
void error(char *msg) {
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
 * argv[2] pin for light 1
 * argv[3] pin for light 2
 */

int main(int argc, char *argv[]) {
	uint32_t clilen;
	uint8_t gpioOutputPortLight1;
	uint8_t gpioOutputPortLight2;
	int newsockfd, portno;
	int sockfd;
	struct sockaddr_in cli_addr;

	// If there are no enough parameters, abort.
	if (argc < 4) {
		fprintf(stderr,
				"ERROR, provide args: port number, pin for light 1, pin for light 2\n");
		exit(1);
	}
	portno = atoi(argv[1]);
	gpioOutputPortLight1 = atoi(argv[2]);
	gpioOutputPortLight2 = atoi(argv[3]);

	setupGpioOutput(gpioOutputPortLight1);
	setupGpioOutput(gpioOutputPortLight2);
	// Create a socket.
	sockfd = createServer(portno);

	clilen = sizeof(cli_addr);
	while (1) {
		fprintf(stderr, "Waiting for client...\n");
		// Block until a client has connected to the server.  This returns a file descriptor for the connection.
		newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

		// If the return is less than 0l, there is an error.
		if (newsockfd < 0) {
			error("ERROR on accept");
		} else {
			fprintf(stderr, "Client connected\n");
			handleClientConnection(newsockfd, gpioOutputPortLight1);
		}

		sleep(1);
	}

	gpio_unexport(gpioOutputPortLight1);
	gpio_unexport(gpioOutputPortLight2);
	return 0;
}

void setupGpioOutput(uint8_t gpioOutputPort) {
	(void) gpio_export(gpioOutputPort);
	(void) gpio_set_dir(gpioOutputPort, 1);
	(void) gpio_fd_open(gpioOutputPort);
}

int createServer(int portno) {
	int sockfd;
	struct sockaddr_in serv_addr;
	// Create a socket.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// If the return is less than 0, then the socket failed to create.
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	// Initialize the buffer to all zeros.
	memset((void*) &serv_addr, 0, sizeof(serv_addr));
	// Obtain the port number as an integer.
	// Setup the server address structure.
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	// Bind the socket appropriately.
	if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}
	// Listen on the socket for an incoming connection.  The parameter is the number of connections that can be waiting / queued up.  5 is the maximum allowed by most systems.
	listen(sockfd, 5);
	return sockfd;
}

void handleClientConnection(int newsockfd, uint8_t gpioOutputPortLight1) {
	int n;
	int index;
	uint64_t bytesReceived = 0;
	char *buffer = malloc(BLOCKSIZE);
	// If the return is less than 0l, there is an error.

	// Fill the buffer with all zeros.
	memset(&buffer[0], 0, BLOCKSIZE);

	// Read from the buffer when data arrives.
	n = read(newsockfd, buffer, BLOCKSIZE);

	if (n < 0) {
		error("ERROR reading from socket");
	} else {
		bytesReceived += n;
	}

	// Print the message.
	printf("Here is the value sent: %d\n", atoi(buffer));
	if (atoi(buffer) == 1) {
		turnLightOn(gpioOutputPortLight1);
	} else {
		turnLightOff(gpioOutputPortLight1);
	}
	printf("Bytes received: %llu\n", bytesReceived);

	close(newsockfd);
}

void turnLightOn(int outputPin) {
	gpio_set_value(outputPin, 1);
}

void turnLightOff(int outputPin) {
	gpio_set_value(outputPin, 0);
}
