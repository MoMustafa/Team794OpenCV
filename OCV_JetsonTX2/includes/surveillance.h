#ifndef SURVEILLANCE_H
#define SURVEILLANCE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <ctype.h>
#include <cmath>

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

extern String bodycascade, facecascade;
extern cv::CascadeClassifier body, face;

extern Point2f point;

//Drawing Parameters 
extern int radius, thickness, lineType;
extern Scalar red, green, blue, white, black;

//Efficiency Parameters 
extern bool needToInit;
extern int detectionFreq;

//PROTOTYPES
string get_tegra_pipeline(string, string, string);

Rect detect(Mat&, Mat&, CascadeClassifier&, Scalar);

void motor_control(Point);

int runprocess(CascadeClassifier&, VideoCapture);

void getCenter(Point&, vector<Point2f>&);

int DrawPoints(Mat&, Rect, Point&, vector<Point2f>&, vector<uchar>);

#endif
