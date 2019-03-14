#include <stdio.h>      
#include <unistd.h>     
#include <fcntl.h>      
#include <errno.h>      
#include <termios.h>    
#include <iostream>
#include <string>

using namespace std;

int main()
{
    int  serial_port = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY );
	usleep(1000000);
    fcntl(serial_port, F_SETFL, 0);
	usleep(1000000);
    /*Define the POSIX structure*/
    struct termios serial_options;

    /*Read the attribute structure*/
    tcgetattr(serial_port, &serial_options);

    /*Set the baud rate of the port  to 9600*/
    cfsetispeed(&serial_options, B57600);
    cfsetospeed(&serial_options, B57600);
            serial_options.c_cflag |= (CLOCAL | CREAD);

    /*Define other parameters in order to  realize the 8N1 standard*/
    serial_options.c_cflag &= ~PARENB;
    serial_options.c_cflag &= ~CSTOPB;
    serial_options.c_cflag &= ~CSIZE;
    serial_options.c_cflag |= CS8;

    /*Apply the new attributes */
    tcsetattr(serial_port, TCSANOW, &serial_options);

    /*Now, we read the first 100 line from the data stream, then we close the port */
		
	if (write(serial_port, "300P300T1L\n", 11) == -1)
	{
        printf("Write error\n");
	}
usleep(1000000);
	if (write(serial_port, "800T800P1L\n", 11) == -1)
	{
        printf("Write error\n");
	}
usleep(1000000);
if (write(serial_port, "300P300T1L\n", 11) == -1)
	{
        printf("Write error\n");
	}
usleep(1000000);
if (write(serial_port, "800T800P1L\n", 11) == -1)
	{
        printf("Write error\n");
	}

    char buf[1000];

 //   for(int i=0; i<100;i++) {
 //       read( serial_port, &buf , VEOL);
  //      std::cout << "Read: " << buf << endl;
  //  }

    close(serial_port);
    return 0;

}
