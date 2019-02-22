#ifndef YOUR_NAME_INCLUDE
#define YOUR_NAME_INCLUDE

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <ctype.h>

#include "surveillance.h"

using namespace std;
using namespace cv;

class surveillance
{
	private:
	Scalar red;
	Scalar green;
	Scalar blue;
	Scalar white;
	Scalar black;

	// SETTINGS
	string width = "1920";
	string height = "1080";
	string fps = "10";
	
	
	// KLT Parameters 
	const int MAX_COUNT = 500;
	double quality = 0.001;
	double k = 0.04; 
	int minDist = 1;
	int blockSize = 3;
	bool useHarris = false;
	int maxLevel = 3;
	int flags = 0;
	int minThreshold = 0.001;

	// Detection Parameters 
	int minNeighbors = 3;
	double scaleFactor = 1.05;

	// Drawing Parameters 
	int radius = 1;
	int thickness = -1;
	int lineType = 8;

	// Efficiency Parameters 
	bool needToInit = false;
	int detectionFreq = 10;
	
	public:	
	
	string get_tegra_pipeline(string, string, string);

	Rect detect(Mat&, Mat&, CascadeClassifier&, Scalar);

	int runprocess(CascadeClassifier&, VideoCapture);

	void getCenter(Point&, vector<Point2f>&);

	void DrawPoints(Mat&, Rect, Point&, vector<Point2f>&, vector<uchar>);
};



#endif
