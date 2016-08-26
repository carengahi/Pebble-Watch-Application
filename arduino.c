#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

char buf[100];
char message[100];

int main () {
  // open
  int fd = open("/dev/ttyUSB10", O_RDWR);
  if (fd == -1) {
    perror("cannot open");
    return 0;
  }
  // configure
  struct termios options; // struct to hold options 
  tcgetattr(fd, &options); // associate with this fd 
  cfsetispeed(&options, 9600); // set input baud rate 
  cfsetospeed(&options, 9600); // set output baud rate 
  tcsetattr(fd, TCSANOW, &options); // set options 
  // read
  
  while (1) {
    int bytes_read = read(fd, buf, 100);
    if (bytes_read != 0) {
      buf[bytes_read] = '\0';
      // append buf to message
      strcat(message, buf);
      if (buf[bytes_read - 1] == '\n') {
	// buf is the end of the message, print message and reset message
	printf("%s", message);
	strcpy(message, "");
      }
    }
  }
  close(fd);
  return 0;
}