#ifndef SERVER_H
#define SERVER_H

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <stdio.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//PIPE VARIABLES
extern int fd;
extern char *myfifo;

void send2server(Mat, int);

#endif
