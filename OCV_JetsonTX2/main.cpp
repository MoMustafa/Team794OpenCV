#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <ctype.h>

using namespace std;
using namespace cv;

/*SETTINGS*/
Scalar red(0,0,255);
Scalar green(0,255,0);
Scalar blue(255, 0, 0);
Scalar white(255,255,255);
Scalar black(0,0,0);

string width = "1920";
string height = "1080";
string fps = "10";

double quality = 0.001;
int minDist = 1;
int blockSize = 3;
bool useHarris = false;
double k = 0.04; 
int maxLevel = 3;
int flags = 0;
int minThreshold = 0.001;
const int MAX_COUNT = 500;
bool needToInit = false;
int detectionFreq = 10;

TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
Size subPixWinSize(10,10), winSize(31,31);

String bodycascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_upperbody.xml";
String facecascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_frontalface_alt2.xml";
cv::CascadeClassifier body, face;
Point2f point;

string get_tegra_pipeline(string, string, string);
Rect detect(Mat&, Mat&, CascadeClassifier&, Scalar);
int runprocess(CascadeClassifier&, VideoCapture);
void getCenter(Point&, vector<Point2f>&);


int main()
{
	body.load(bodycascade);
	face.load(facecascade);
	
	string pipeline = get_tegra_pipeline(width, height, fps);
	VideoCapture cap(pipeline, CAP_GSTREAMER);
	
	if(cap.isOpened())
		runprocess(face, cap);
	else
		cout<<"Camera cannot be opened."<<endl;
	return 0;
}

int runprocess(CascadeClassifier& cascade, VideoCapture cap)
{

	Mat gray, prevGray, image, frame;
	vector<Point2f> features[2];
	Point center(cap.get(3)/2, cap.get(4)/2);
	size_t i, k;
	int timer = 0;
	Rect ROI = Rect();
	do
	{
		timer++;
		if(!cap.read(frame))
			break;
		
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		
		Mat mask(frame.size(), CV_8U, Scalar(0));	
		
		if(timer%detectionFreq == 0 || timer == 0)
		{
			ROI = detect(frame, gray, cascade, green);
			if(ROI.size() != frame.size())
			{
				rectangle(mask, ROI, Scalar(255), -1);
				center.x = ROI.x + ROI.width/2;
				center.y = ROI.y - ROI.height/2;
				line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), red, 2);
    			line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), red, 2);
				needToInit = true;
			}
		}
		
		if(needToInit)
		{
			goodFeaturesToTrack(gray, features[1], MAX_COUNT, quality, minDist, mask, blockSize, useHarris, k);
			cornerSubPix(gray, features[1], subPixWinSize, Size(-1,-1), termcrit);
			needToInit = false;
		}
		
		else if(!features[0].empty())
		{
			vector<uchar> status;
    		vector<float> err;
    	
    		if(prevGray.empty())
    			gray.copyTo(prevGray);
    	
			calcOpticalFlowPyrLK(prevGray, gray, features[0], features[1], status, err, winSize, maxLevel, termcrit, flags, minThreshold);
			
			
			getCenter(center, features[1]);
			line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), red, 2);
			line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), red, 2);
			
			/*
			for(i=k=0; i<features[1].size(); i++)
			{
				if(!status[i])
					continue;
					
				features[1][k++] = features[1][i];
				circle(frame, features[1][i], 3, red, -1, 8);
			}
			*/
		}
    	needToInit = false;
		
		std::swap(features[1], features[0]);
    	cv::swap(prevGray, gray);
    	
		imshow("Footage", frame);
	}while(waitKey(1)!=27);
	
	destroyAllWindows();
	return 0;
}

void getCenter(Point& center, vector<Point2f> &coords)
{
	Point totals;
	
	for(int i=0; i<coords.size(); i++)
	{
		totals.x += coords[i].x;
		totals.y += coords[i].y;
	}
	center.x = (int) totals.x/coords.size();
	center.y = (int) totals.y/coords.size();
}

Rect detect(Mat& frame, Mat& gray, CascadeClassifier& cascade, Scalar color)
{	
	vector<Rect> ROIs;
	equalizeHist(gray, gray);
	cascade.detectMultiScale(gray, ROIs, 1.2, 4, 0|CASCADE_SCALE_IMAGE, Size(170,170));
	
	if(ROIs.size() != 0)
	{
		return ROIs[0];
	}
	return Rect(0, 0, gray.size().width, gray.size().height);
	
}

string get_tegra_pipeline(string width, string height, string fps)
{      
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)"+width+", height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int) "+width+" , height=(int) "+height+", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR, framerate=(fraction) "+fps+"/1 ! appsink";

}
