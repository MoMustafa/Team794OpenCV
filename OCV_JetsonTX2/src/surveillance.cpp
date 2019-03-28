#include "../includes/surveillance.h"
#include "../includes/motorctrl.h"

//VARIABLES
//Footage Settings
string width = "1280";
string height = "720";
string fps = "10";

//KLT Parameters 
const int MAX_COUNT = 500;
double quality = 0.001;
double k = 0.04; 
int minDist = 1;
int blockSize = 3;
bool useHarris = false;
int maxLevel = 3;
int flags = 0;
int minThreshold = 0.501;

TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
Size subPixWinSize(10,10), winSize(31,31);

//Detection Parameters 
int minNeighbors = 4;
double scaleFactor = 1.05;

String bodycascade = "/home/nvidia/Desktop/ProgramKLT/Cascades/haarcascades/haarcascade_fullbody.xml";
String facecascade = "/home/nvidia/Desktop/ProgramKLT/Cascades/haarcascades/haarcascade_frontalface_alt.xml";
cv::CascadeClassifier body, face;

Point2f point;
Point mid(665,750);
Point motorpos;
Point screencenter;
double integ_x = 0.0;
double integ_y = 0.0;


//Drawing Parameters 
int radius = 1;
int thickness = -1;
int lineType = 8;
Scalar red(0,0,255);
Scalar green(0,255,0);
Scalar blue(255, 0, 0);
Scalar white(255,255,255);
Scalar black(0,0,0);

//Efficiency Parameters 
bool needToInit = false;
int detectionFreq = 100;

//Motor Variables
const char* port_name;
int serialport;
const void* command;

int runprocess(CascadeClassifier& cascade, VideoCapture cap)
{
	port_name = "/dev/ttyUSB0"; 
	serialport = init_motor(port_name);

	Mat gray, prevGray, image, frame;
	vector<Point2f> features[2];
	vector<uchar> status;
    vector<float> err;
    
    //cout<<cap.get(3)<<" x "<<cap.get(4)<<endl;
	screencenter = Point(cap.get(3)/2, cap.get(4)/2);
	Point center = screencenter;
	motorpos = mid;
	
	int timer = 0;
	int displaced = 0;
	int tries = 0;
	bool reset = false;
	Rect ROI = Rect();
	
	do
	{
		if(!cap.read(frame))
			break;
		
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		
		Mat mask(frame.size(), CV_8U, Scalar(0));	
		
		if(timer%detectionFreq == 0 || timer == 0)
		{
			//cout<<"Detecting\t";
			ROI = detect(frame, gray, cascade, green);
			if(ROI.size() != frame.size())
			{
				//cout<<"FOUND!"<<endl;
				displaced = 0;
				rectangle(mask, ROI, Scalar(255), -1);
				needToInit = true;
				tries = 0;
			}
			else
				tries++;
				
			//cout<<"try :"<<tries<<endl;
			
			if(tries > 5)
			{
				//cout<<"Resetting.  Center:\t";
				reset = true;
				center = screencenter;
				displaced = 0;
				tries = 0;
				reset_motor(serialport);
				usleep(50000);
				//cout<<center.x<<" x "<<center.y<<endl;
			}
		}
		
		if(needToInit)
		{
			goodFeaturesToTrack(gray, features[1], MAX_COUNT, quality, minDist, mask, blockSize, useHarris, k);
			cornerSubPix(gray, features[1], subPixWinSize, Size(-1,-1), termcrit);
			needToInit = false;
			reset = false;
		}
		
		else if(!features[0].empty() && !reset)
		{
    		if(prevGray.empty())
    			gray.copyTo(prevGray);
    	
			calcOpticalFlowPyrLK(prevGray, gray, features[0], features[1], status, err, winSize, maxLevel, termcrit, flags, minThreshold);
			
			displaced += DrawPoints(frame, ROI, center, features[1], status);
		}
		//cout<<"Crosshair: "<<center.x<<" x "<<center.y<<"\t";
		//cout<<"Displaced points: "<<displaced<<endl;
		if(displaced > 100)
			timer=0;
		else
			timer++;
			
    	needToInit = false;
    	
    	if(!reset)
			motor_control(center);
		
		
		std::swap(features[1], features[0]);
    	cv::swap(prevGray, gray);
		imshow("Footage", frame);
		
		//usleep(50000);
		
	}while(waitKey(1)!=27);
	
	destroyAllWindows();
	
	usleep(50000);
	
	close_motor(serialport);
	return 0;
}

int DrawPoints(Mat& frame, Rect ROI, Point& center, vector<Point2f>& features, vector<uchar> status)
{
	size_t i, k;	
	Point totals;
	int count = 0;

	for(i=k=0; i<features.size(); i++)
	{
		totals.x += features[i].x;
		totals.y += features[i].y;
		
		if(!status[i])
		{
			count++;
			continue;
		}
		
		features[k++] = features[i];
		circle(frame, features[i], radius, red, thickness, lineType);
	}
	
	center.x = (int) totals.x/features.size();
	center.y = (int) totals.y/features.size();
	
	// RESET NEEDS FIXING
	//else
	//{
	//	center.x = ROI.x + ROI.width/2;
	//	center.y = ROI.y + ROI.height/2;
	//}
	
	line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), green, 2);
	line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), green, 2);
		
	return count;
}

void motor_control(Point center)
{
	string command;
	
	Point error;
	int minwindow = 50;
	int maxwindow = 500;
	double p_scale = 0.024;
	double t_scale = 0.024;
	double int_p = 0.006;
    double int_t = 0.006;
    double err_scale = 50.0;
	///////////////////////////////////////////////////////////////////////////////////////
	error = (center - screencenter);	
	integ_x += error.x/err_scale;
	integ_y += error.y/err_scale;
	
	//cout<<"INTEG "<<integ_x<<" x "<<integ_y<<"\t";
	
	
	if(abs(error.x) > minwindow && abs(error.x) < maxwindow)
		motorpos.x -= (p_scale)*(error.x)+(int_p*integ_x);
	
	if(abs(error.y) > minwindow && abs(error.y) < maxwindow)
		motorpos.y += (t_scale)*(error.y)+(int_t*integ_y);
	
	stringstream ss;
	ss << motorpos.y << "P" << motorpos.x << "T0L\n";
	command = ss.str();
	
	//cout<<command<<endl;
	///preverror = error;
	
	run_motor(command.c_str(), serialport);
}

Rect detect(Mat& frame, Mat& gray, CascadeClassifier& cascade, Scalar color)
{	
	//cout<<"Detect"<<endl;
	vector<Rect> ROIs;
	equalizeHist(gray, gray);
	cascade.detectMultiScale(gray, ROIs, scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(90,90), Size(170,170));
	
	if(ROIs.size() != 0)
		return ROIs[0];
	
	return Rect(0, 0, gray.size().width, gray.size().height);
}

string get_tegra_pipeline(string width, string height, string fps)
{      
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)"+width+", height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int) "+width+" , height=(int) "+height+", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR, framerate=(fraction) "+fps+"/1 ! appsink";

}
