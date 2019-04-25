#include "../includes/motorctrl.h"

int delay = 750000;
int commandsize = 11;
string centerpos = "500P500T1L\n";

int init_motor(const char* port_name)
{
	const char* portname = port_name;
	int serial_port = open(portname, O_RDWR| O_NOCTTY);
	usleep(2*delay);
	fcntl(serial_port, F_SETFL, 0);
	
	struct termios serial_options;
	
	tcgetattr(serial_port, &serial_options);
	
	cfsetispeed(&serial_options, B57600);
	cfsetispeed(&serial_options, B57600);
	
	serial_options.c_cflag |= (CLOCAL | CREAD);
	serial_options.c_cflag &= ~PARENB;
	serial_options.c_cflag &= ~CSTOPB;
	serial_options.c_cflag &= ~CSIZE;
	serial_options.c_cflag |= CS8;
	
	tcsetattr(serial_port, TCSANOW, &serial_options);
	
	run_motor(centerpos.c_str(), serial_port);
	usleep(delay);
	cout<<"Motor Successfully Initialized"<<endl;
	return serial_port;
}

void run_motor(const void* motorcommand, int serial_port)
{
	cout<<(char*)motorcommand<<endl;
	if(write(serial_port, motorcommand, commandsize) == -1)
		cout<<"Write error"<<endl;
	usleep(delay/100);
}

void reset_motor(int serial_port)
{
	run_motor(centerpos.c_str(), serial_port);
	usleep(delay);
}

void close_motor(int serial_port)
{
	run_motor(centerpos.c_str(), serial_port);
	usleep(delay);
	close(serial_port);
}
