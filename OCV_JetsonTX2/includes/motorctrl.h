#ifndef MOTORCTRL_H
#define MOTORCTRL_H

#include <stdio.h>      
#include <unistd.h>     
#include <fcntl.h>      
#include <errno.h>      
#include <termios.h>    
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

extern const char* portname;
extern int serial_port;
extern int delay;
extern int commandsize; 
extern struct termios serial_options;
extern string centerpos;

int init_motor(const char*);
void run_motor(const void*, int);
void close_motor(int);
void reset_motor(int);

#endif
