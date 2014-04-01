/*********************************************************************
 * This program will allow on to turn a given port on and off at a user
 * defined rate.  The code was initially developed by Dingo_aus, 7 January 2009
 * email: dingo_aus [at] internode <dot> on /dot/ net
 * From http://www.avrfreaks.net/wiki/index.php/Documentation:LinuxGPIO#gpio_framework
 *
 * Created in AVR32 Studio (version 2.0.2) running on Ubuntu 8.04
 * Modified by Mark A. Yoder, 21-July-2011
 * Refactored and further modified by Walter Schilling, Summer 2012 / Winter 2013-2014.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include <stdint.h>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

// create a variable to keep track of the port that is to be opened as a string.
static char ioPort[56];

static int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/*******************************************************************************
 * This method will setup the GPIO port to allow the user to perform output on the given port.
 *
 * @param uint32_t gpioPort - This is the number of the GPIO port that is to be opened.
 *
 ******************************************************************************/
void setupOutputPort(uint32_t gpioPort) {
	FILE* fp;
	//create a variable to store whether we are sending a '1' or a '0'
	char set_value[5];

	//Using sysfs we need to write the 3 digit gpio number to /sys/class/gpio/export
	//This will create the folder /sys/class/gpio/gpio37
	if ((fp = fopen(SYSFS_GPIO_DIR "/export", "ab")) == NULL) {
		printf("Cannot open export file.\n");
		exit(1);
	}

	//Set pointer to begining of the file
	rewind(fp);

	//Write the value of our GPIO port to the file.
	sprintf(&set_value[0], "%d", gpioPort);
	fwrite(&set_value, sizeof(char), 3, fp);
	fclose(fp);

	printf("...export file accessed, new pin now accessible\n");

	//SET DIRECTION
	//Open the LED's sysfs file in binary for reading and writing, store file pointer in fp
	// Strat by creating a string representing the port that needs to be opened.
	sprintf(&ioPort[0], "%s%s%d%s", SYSFS_GPIO_DIR, "/gpio", gpioPort,
			"/direction");

	if ((fp = fopen(&ioPort[0], "rb+")) == NULL) {
		printf("Cannot open direction file.\n");
		exit(1);
	}

	//Set pointer to begining of the file
	rewind(fp);

	//Write our value of "out" to the file
	strcpy(set_value, "out");
	fwrite(&set_value, sizeof(char), 3, fp);
	fclose(fp);
	printf("...direction set to output\n");
}

/*******************************************************************************
 * This method will setup the GPIO port to allow the user to perform input on the given port.
 *
 * @param uint32_t gpioPort - This is the number of the GPIO port that is to be opened.
 *
 ******************************************************************************/
void setupInputPort(uint32_t gpioPort) {
	FILE* fp;
	//create a variable to store whether we are sending a '1' or a '0'
	char set_value[5];

	//Using sysfs we need to write the 3 digit gpio number to /sys/class/gpio/export
	//This will create the folder /sys/class/gpio/gpio37
	if ((fp = fopen(SYSFS_GPIO_DIR "/export", "ab")) == NULL) {
		printf("Cannot open export file.\n");
		exit(1);
	}

	//Set pointer to begining of the file
	rewind(fp);

	//Write the value of our GPIO port to the file.
	sprintf(&set_value[0], "%d", gpioPort);
	fwrite(&set_value, sizeof(char), 3, fp);
	fclose(fp);

	printf("...export file accessed, new pin now accessible\n");

	//SET DIRECTION
	//Open the LED's sysfs file in binary for reading and writing, store file pointer in fp
	// Strat by creating a string representing the port that needs to be opened.
	sprintf(&ioPort[0], "%s%s%d%s", SYSFS_GPIO_DIR, "/gpio", gpioPort,
			"/direction");

	if ((fp = fopen(&ioPort[0], "rb+")) == NULL) {
		printf("Cannot open direction file.\n");
		exit(1);
	}

	//Set pointer to begining of the file
	rewind(fp);

	//Write our value of "in" to the file
	strcpy(set_value, "in");
	fwrite(&set_value, sizeof(char), 2, fp);
	fclose(fp);
	printf("...direction set to input\n");
}

/*******************************************************************************
 * This method will read the given pin and perform processing on it.
 *
 * @param FILE *fp - This is a pointer to the io device driver which will be read to determine the state of the pin.
 ******************************************************************************/
int processPin(int gpio_fd, FILE *ofp, uint32_t gpioInputPort) {
	struct pollfd fdset[2];
	int timeout, rc;
	int nfds = 2;
	char buf[MAX_BUF];
	int len;

	//create a variable to store whether we are sending a '1' or a '0'
	char set_value[5];
	char prevState;

	prevState = '?';
	timeout = POLL_TIMEOUT;
	printf("AAAA");

	while (keepgoing) {
		memset((void*) fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;

		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}

		if (rc == 0) {
			printf(".");
		}

		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[1].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
					gpioInputPort, buf[0], len);

			if (buf[0] != prevState) {
				if (buf[0] == '0') {
					//printf("The button is pressed.\n");

					//Write our value of "1" to the file
					strcpy(set_value, "0");
				} else {
					//printf("The button is not pressed.\n");

					//Write our value of "1" to the file
					strcpy(set_value, "1");
				}

				fwrite(&set_value, sizeof(char), 1, ofp);
				fflush(ofp);

				prevState = buf[0];
			}
		}
		if (fdset[0].revents & POLLIN) {
			(void) read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
	}
	return 0;
}

/****************************************************************
 * signal_handler
 ****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig) {
	printf("Ctrl-C pressed, cleaning up and exiting..\n");
	keepgoing = 0;
}

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio) {
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio) {
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag) {
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction", gpio);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value) {
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value) {
	int fd;
	char buf[MAX_BUF];
	char ch;

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}

	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge) {
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}

	write(fd, edge, strlen(edge) + 1);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio) {
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd) {
	return close(fd);
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp) {
	/* This variable is a file pointer to the file interface for the GPIO device driver
	 which controls the input pin. */
	FILE *ofp;
	uint8_t gpioInputPort; // This is the GPIO port that is to be used for input into this program.
	uint8_t gpioOutputPort;	//  This is the GPIO port that is to be used for output from this program.

	int gpio_fd; //, rc;

	if (argc < 3) {
		printf("Usage: %s <input port number> <output port number>\n\n",
				argv[0]);
		printf(
				"Waits for a change in the GPIO pin voltage level or input on stdin\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	// Convert the input into the appropriate parameters.
	gpioInputPort = atoi(argv[1]);
	gpioOutputPort = atoi(argv[2]);

	printf("\n*********************************\n"
			"*  Welcome to LED follower program Demo program. *\n"
			"*  ....Reading gpio %d Controlling GPIO %d       *\n"
			"**********************************\n", gpioInputPort,
			gpioOutputPort);

	gpio_export(gpioInputPort);
	gpio_set_dir(gpioInputPort, 0);
	gpio_set_edge(gpioInputPort, "both");  // Can be rising, falling or both
	gpio_fd = gpio_fd_open(gpioInputPort);

	// Setup the IO ports.
	setupOutputPort(gpioOutputPort);

	// Open the device driver for processing.
	sprintf(&ioPort[0], "%s%s%d%s", SYSFS_GPIO_DIR, "/gpio", gpioOutputPort,
			"/value");

	if ((ofp = fopen(&ioPort[0], "rb+")) == NULL) {
		printf("Cannot open value file.\n");
		exit(1);
	}

	processPin(gpio_fd, ofp, gpioInputPort);

//***********************************************************************

	gpio_fd_close(gpio_fd);
	fclose(ofp);

	return 0;
}

