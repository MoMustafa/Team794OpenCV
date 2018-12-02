#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

/*SETTINGS*/
int history = 20;
int limit = 500;
bool shadows = false;
int minkeypoints = 5;
int ROIupdate = 10;
int reset = 100;

Scalar red = Scalar(0,0,255);
Scalar green = Scalar(0,255,0);

int w = 100;
int h = 100;
int thick = 3;

String bodycascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_upperbody.xml";
String facecascade = "/home/nvidia/Desktop/Program/Cascades/haarcascades/haarcascade_frontalface_alt2.xml";
CascadeClassifier body, face;

void processVideo(Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast);
void getROI(Point &center, vector <Point2f> &coords);
void detect(Mat& frame, CascadeClassifier& body, CascadeClassifier& face);
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
    cout<<"Motor Coordinates: ("<<center.x<<","<<center.y<<")"<<endl;
}

void processVideo(Ptr <BackgroundSubtractor> MOG2, Ptr <FastFeatureDetector> fast)
{
	body.load(bodycascade);
	face.load(facecascade);	
	
	vector<Rect> faces, bodies;
	
	string width, height, fps;

	cout<<"Enter width: ";
	cin>>width;
	cout<<"Enter height: ";
	cin>>height;
	cout<<"Enter fps: ";
	cin>>fps;

	string pipeline = get_tegra_pipeline(width, height, fps);
    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        cout<<"Cannot start camera.\n";
        return;
    }

    int timer = 0;
    int resettime = 0;
    Point center(cap.get(3)/2, cap.get(4)/2);
    
    do
    {

        timer++;
        resettime++;
        Mat frame, eqframe, gray, fgmaskMOG2;
        vector <Point2f> coords;
        vector <KeyPoint> kpts;

        if(!cap.read(frame))
            break;        

        cvtColor(frame, gray, COLOR_BGR2GRAY);
		/*
		equalizeHist(gray, eqframe);
		face.detectMultiScale(eqframe, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
		body.detectMultiScale(eqframe, bodies, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
			
		for(int i=0; i<faces.size(); i++)
		{
			rectangle(frame, faces[i], green);
		}	
		
		for(int i=0; i<bodies.size(); i++)
		{
			rectangle(frame, bodies[i], green);
		}
		*/

        MOG2->apply(gray, fgmaskMOG2);
        fast->detect(fgmaskMOG2, kpts);

        KeyPoint::convert(kpts, coords);

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
            resettime = 0;
        }

        Point topleft(center.x-w, center.y+h);
        Point botright(center.x+w, center.y-h);
        Rect ROI(topleft, botright);

        rectangle(frame, ROI, red, thick);

        drawKeypoints(frame, kpts, frame);

        imshow("Footage", frame);

    }while(waitKey(1)!=27);

    cap.release();
}

void detect(Mat& frame, CascadeClassifier& body, CascadeClassifier& face)
{	
	Mat eqframe;
	vector<Rect> faces, bodies;
	cvtColor (frame, eqframe, COLOR_BGR2GRAY);
	equalizeHist(eqframe, eqframe);
	face.detectMultiScale(eqframe, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
	body.detectMultiScale(eqframe, bodies, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));
	
	for(int i=0; i<faces.size(); i++)
	{
		rectangle(frame, faces[i], green);
	}
	
	for(int i=0; i<bodies.size(); i++)
	{
		rectangle(frame, bodies[i], green);
	}
	
}


string get_tegra_pipeline(string width, string height, string fps)
{
    return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int) "+width+" , height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

