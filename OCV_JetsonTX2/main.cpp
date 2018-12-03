#include <iostream>
#include <cmath>
#include <time.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/cuda.hpp>

using namespace std;
using namespace cv;
using namespace cuda;

/*SETTINGS*/
int history = 20;
int limit = 500;
bool shadows = false;
int minkeypoints = 5;
int ROIupdate = 10;
int reset = 100;

Scalar red(0,0,255);
Scalar green(0,255,0);
Scalar white(255,255,255);
Scalar black(0,0,0);

int w = 100;
int h = 100;
int thick = 3;

int timer = 0;
int resettime = 0;
int mindist = 150;

char timestring[24];
time_t rawtime;

Point2f prevcenter(0.0,0.0);

String bodycascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_upperbody.xml";
String facecascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_frontalface_alt2.xml";
CascadeClassifier body, face;

void processVideo(Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast);
void getROI(Point &center, vector <Point2f> &coords);
void detect(Mat& frame, Mat& gray, CascadeClassifier& body, CascadeClassifier& face);
vector<Point2f> motiondetect(Mat& frame, Mat& gray, Point center, Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast);
float getdist(Point2f center);
string get_tegra_pipeline(string width, string height, string fps);

int main()
{
    Ptr <BackgroundSubtractor> MOG2;
    Ptr <FastFeatureDetector> fast;

    MOG2 = createBackgroundSubtractorMOG2(history, limit, shadows);
    fast = FastFeatureDetector::create();

    processVideo(MOG2, fast);

    destroyAllWindows();
    return 0;
}

void processVideo(Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast)
{	
	body.load(bodycascade);
	face.load(facecascade);	
	
	vector<Rect> faces, bodies;
	
	string width = "1280";
	string height = "720";
	string fps = "30";

	cout<<width<<"x"<<height<<" at "<<fps<<"fps"<<endl;

	string pipeline = get_tegra_pipeline(width, height, fps);
    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        cout<<"Cannot start camera.\n";
        return;
    }

    Point center(cap.get(3)/2, cap.get(4)/2);
    prevcenter = center;
    
    do
    {
    	time(&rawtime);
        timer++;
        resettime++;
        Mat frame, gray;
        vector<Point2f> coords;

        if(!cap.read(frame))
            break;        

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        
		//For facial and body detection
		//detect(frame, gray, body, face);
		
        coords = motiondetect(frame, gray, center, MOG2, fast);
        
        if(coords.size()>minkeypoints && timer%ROIupdate==0)
		{
		    getROI(center, coords);
		    resettime = 0;
		}

		if(resettime>reset)
		{
		    cout<<"Resetting camera"<<endl;
		    center.x = cap.get(3)/2;
		    center.y = cap.get(4)/2;
		    prevcenter = center;
		    resettime = 0;
		}
        
    	line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), red, 2);
    	line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), red, 2);
        
        //drawKeypoints(frame, kpts, frame);
        
        struct tm *timeinfo = localtime(&rawtime);
        sprintf(timestring, "%s", asctime(timeinfo));
        putText(frame, timestring, Point2f(0,cap.get(4)*0.025), FONT_HERSHEY_PLAIN, 1, red, 1);

        imshow("Footage", frame);

    }while(waitKey(1)!=27);

    cap.release();
}

void detect(Mat& frame, Mat& gray, CascadeClassifier& body, CascadeClassifier& face)
{	
	vector<Rect> faces, bodies;
	equalizeHist(gray, gray);
	face.detectMultiScale(gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
	body.detectMultiScale(gray, bodies, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
	
	for(int i=0; i<faces.size(); i++)
	{
		rectangle(frame, faces[i], green);
	}
	
	for(int i=0; i<bodies.size(); i++)
	{
		rectangle(frame, bodies[i], white);
	}
	
}

vector <Point2f> motiondetect(Mat& frame, Mat& gray, Point center, Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast)
{
	Mat fgmaskMOG2;

	vector <Point2f> coords;
	vector <KeyPoint> kpts;

	MOG2->apply(gray, fgmaskMOG2);
    fast->detect(fgmaskMOG2, kpts);

    KeyPoint::convert(kpts, coords);
    
    return coords;
}

void getROI(Point &center, vector <Point2f> &coords)
{
    Point totals;

    for (int i=0; i<coords.size(); i++)
    {
        totals.x += coords[i].x;
        totals.y += coords[i].y;
    }
    center.x = (int) totals.x/coords.size();
    center.y = (int) totals.y/coords.size();
    
    //float dist = getdist(center);
    
    float hdist = abs(center.x-prevcenter.x);
    float vdist = abs(center.y-prevcenter.y);
    
    if(hdist > mindist)
    {
    	if((center.x - prevcenter.x)>0)
    		cout<<"Move right"<<endl;
    	else
    		cout<<"Move left"<<endl;
    	prevcenter = center;
    }
    
    if(vdist > mindist)
    {	
    	if((center.y - prevcenter.y)>0)
    		cout<<"Move down"<<endl;
    	else
    		cout<<"Move up"<<endl;
    	prevcenter = center;
    }
}

float getdist(Point2f center)
{		
	float x = pow((prevcenter.x - center.x) ,2);
	float y = pow((prevcenter.y - center.y), 2);
	return sqrt(x+y);
}

string get_tegra_pipeline(string width, string height, string fps)
{
    return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int) "+width+" , height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

