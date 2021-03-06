#ifndef SURVEILLANCE_H
#define SURVEILLANCE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <ctype.h>
#include <cmath>
#include <ctime>
#include <string>

using namespace std;
using namespace cv;

//VARIABLES
//Footage Settings
extern string width, height, fps;

//KLT Parameters 
extern const int MAX_COUNT;
extern double quality, k; 
extern int minDist, blockSize, maxLevel, flags, minThreshold;
extern bool useHarris;

extern TermCriteria termcrit;
extern Size subPixWinSize, winSize;

//Detection Parameters 
extern int minNeighbors;
extern double scaleFactor;

extern String bodycascade, facecascade, sidecascade;
extern cv::CascadeClassifier body, face, side;

extern Point2f point;

//Drawing Parameters 
extern int radius, thickness, lineType;
extern Scalar red, green, blue, white, black;

//Efficiency Parameters 
extern bool needToInit;
extern int detectionFreq;

//Motor Control Parameters
extern int minwindow;
extern int maxwindow;
extern int panmin;
extern int panmax;
extern int tiltmin;
extern int tiltmax;
extern double err_scale;
extern double err_p;
extern double err_t;
extern double int_p;
extern double int_t;
extern double drv_p;
extern double drv_t;

//PROTOTYPES
string get_tegra_pipeline(string, string, string);

Rect detect(Mat&, Mat&, CascadeClassifier&, CascadeClassifier&, Scalar);

void motor_control(Point);

int runprocess(CascadeClassifier&, CascadeClassifier&, VideoCapture);

void getCenter(Point&, vector<Point2f>&);

int DrawPoints(Mat&, Rect, Point&, vector<Point2f>&, vector<uchar>);

#endif
