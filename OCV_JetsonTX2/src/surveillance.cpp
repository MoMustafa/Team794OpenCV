#include "../includes/surveillance.h"
#include "../includes/motorctrl.h"
#include "../includes/server.h"

#include <csignal>

//VARIABLES
//Footage Settings
string width = "1280";
string height = "720";
string fps = "10";

//KLT Parameters 
const int MAX_COUNT = 250;
double quality = 0.001;
double k = 0.04; 
int minDist = 1;
int blockSize = 3;
bool useHarris = true;
int maxLevel = 3;
int flags = 0;
int minThreshold = 0.501;

TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
Size subPixWinSize(10,10), winSize(31,31);

//Detection Parameters 
int minNeighbors = 7;
double scaleFactor = 1.05;

String bodycascade = "/home/nvidia/Desktop/Camera/Cascades/haarcascades/haarcascade_upperbody.xml";
String facecascade = "/home/nvidia/Desktop/Camera/Cascades/haarcascades/haarcascade_frontalface_alt.xml";
String sidecascade = "/home/nvidia/Desktop/Camera/Cascades/haarcascades/haarcascade_profileface.xml";
cv::CascadeClassifier body, face, side;

Point2f point;
Point mid(500,500);
Point motorpos;
Point screencenter;
Point2f prevError(0.0, 0.0);
Point2f integ(0.0, 0.0);

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
string light = "1L";

int minwindow = 50;
int maxwindow = 1000;
int panmin = 200;
int panmax = 800;
int tiltmin = 375;
int tiltmax = 800;
double err_scale = 58.0;
double err_p = 0.02;
double err_t = 0.02;
double int_p = 0.012;
double int_t = 0.012;
double drv_p = -0.10;
double drv_t = -0.10;

//Server Variables
char *myfifo = "/tmp/myfifo";

//Exit Prototype
void signalHandler(int);

int runprocess(CascadeClassifier& cascade1, CascadeClassifier& cascade2, VideoCapture cap)
{
	port_name = "/dev/ttyUSB0"; 
	serialport = init_motor(port_name);

	Mat gray, prevGray, image, frame;
	vector<Point2f> features[2];
	vector<uchar> status;
    vector<float> err;
    
	screencenter = Point(cap.get(3)/2, cap.get(4)/2);
	Point center = screencenter;
	motorpos = mid;
	
	signal(SIGINT, signalHandler);
	
	VideoWriter output("footage.avi", VideoWriter::fourcc('M','J','P','G'), 2, Size(cap.get(3), cap.get(4)), true);
	
	string dt;
	
	int timer = 0;
	int displaced = 0;
	int tries = 0;
	bool reset = false;
	Rect ROI = Rect();
	
	vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    //compression_params.push_back(9);
	
	int count = 0;
	mkfifo(myfifo, 0666);
	
	do
	{
		time_t now = time(0);
		dt = ctime(&now);
		dt = dt.substr(0, dt.length()-1);
		if(!cap.read(frame))
			break;
		
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		
		Mat mask(frame.size(), CV_8U, Scalar(0));	
		
		if(timer%detectionFreq == 0 || timer == 0)
		{
			ROI = detect(frame, gray, cascade1, cascade2, green);
			//ROI.x += ROI.width*0.40;
			//ROI.y += ROI.height*0.25;
			//ROI.height -= ROI.height*0.35;
			//ROI.width -= ROI.width*0.75;
			
			if(ROI.size() != frame.size())
			{
				light = "0L";
				displaced = 0;
				rectangle(mask, ROI, Scalar(255), -1);
				needToInit = true;
				tries = 0;
				usleep(50000);
			}
			else
				tries++;
			
			if(tries > 5)
			{
				reset = true;
				center = screencenter;
				displaced = 0;
				tries = 0;
				reset_motor(serialport);
				integ = Point2f(0.0, 0.0);
				motorpos = mid;
				light = "1L";
				usleep(50000);
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
			cout << "d: " << displaced << endl;
		}

		if(displaced > 0)
			timer=0;
		else
			timer++;
			
    	needToInit = false;
    	
    	if(!reset)
			motor_control(center);
		
		
		std::swap(features[1], features[0]);
    	cv::swap(prevGray, gray);
    	rectangle(frame, 
    		Point(screencenter.x-320, screencenter.y-180), 
    		Point(screencenter.x+320, screencenter.y+180), green, 1);
    	
    	putText(frame, dt, Point2f(10,50), FONT_HERSHEY_SIMPLEX, 1, white, 2);
    	
    	if(timer%1 == 0)
    	{
    		send2server(frame, count);
    		count++;
			if(count > 9)
				count = 0;
		}
		
		//output.write(frame);
		//imshow("Footage", frame);		
		//sleep(1);
	}while(1);
	
	return 0;
}

void send2server(Mat frame, int count)
{
	int fd = open(myfifo, O_WRONLY);
	
	size_t s = 320;
	Mat resized;
	
	resize(frame, resized, Size(0,0), 0.5, 0.5, INTER_NEAREST);

	string filename = "/home/nvidia/Desktop/Camera/Server";
	char str[1];
	sprintf(str, "%d", count);
	filename = filename + str + ".jpg"; 
	
	vector<uchar> buff; //buffer
	vector<int> param(2);
	param[0] = IMWRITE_JPEG_QUALITY;
	param[1] = 95; //default(95) 0-100
	imencode(".jpg", resized, buff, param);
	
	write(fd, buff.data(), buff.size());
		
	close(fd);
}

int DrawPoints(Mat& frame, Rect ROI, Point& center, vector<Point2f>& features, vector<uchar> status)
{
	size_t i, k;	
	Point totals;
	int count = 0;

	cout << "M: " << features.size() <<"\t";

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
		circle(frame, features[i], radius, red, 2, lineType);
	}
	
	center.x = (int) totals.x/features.size();
	center.y = (int) totals.y/features.size();
	
	line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), green, 2);
	line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), green, 2);
		
	return count;
}

void motor_control(Point center)
{
	string command;
	
	Point2f error(0.0, 0.0);
	Point2f change(0.0, 0.0);    

	error = center - screencenter;
	cout << "E: " << error << "\t"; 
	
/*	//Added in

	stringstream lighton;
	lighton << "#" << light;
	command  = lighton.str();
	run_motor(command.c_str(), serialport);

	if((motorpos.x >= panmax && error.x > 0) || (motorpos.x <= panmin && error.x < 0))
		error.x = 0;

	integ.x += error.x/err_scale;
	if(abs(error.x) > minwindow && abs(error.x) < maxwindow)
		motorpos.x += (p_scale)*(error.x)+(int_p*integ.x);
	
	stringstream movex;
	movex << "#" << motorpos.x << "P";
	command = movex.str();
	run_motor(command.c_str(), serialport);
		
	if((motorpos.y >= panmax && error.y > 0) || (motorpos.y <= panmin && error.y < 0))
		error.y = 0;

	integ.y += error.y/err_scale;
	if(abs(error.y) > minwindow && abs(error.y) < maxwindow)
		motorpos.y += (p_scale)*(error.y)+(int_p*integ.y);
	
	stringstream movey;
	movey << "#" << motorpos.y << "T";
	command = movey.str();
	run_motor(command.c_str(), serialport);

*/
	//Bound Check 2.0
	if((motorpos.x >= panmax && error.x > 0) || (motorpos.x <= panmin && error.x < 0))
		error.x = 0;
	
	if((motorpos.y >= tiltmax && error.y > 0) || (motorpos.y <= tiltmin && error.y < 0))
		error.y = 0;

	integ.x += error.x/err_scale;	
	integ.y += error.y/err_scale;

	prevError -= error;
	cout << "pE: " << prevError << endl;

	change.x = (err_p)*(error.x) + (int_p)*(integ.x) + (drv_p*prevError.x);
	change.y = (err_t)*(error.y) + (int_t)*(integ.y) + (drv_t*prevError.y);
	
	if(abs(error.x) > minwindow && abs(error.x) < maxwindow)
	{
		motorpos.x += change.x;
		if((motorpos.x > panmax) || (motorpos.x < panmin))
			motorpos.x -= change.x;
	}

	if(abs(error.y) > minwindow && abs(error.y) < maxwindow)
	{
		motorpos.y += change.y;
		if((motorpos.y > tiltmax) || (motorpos.y < tiltmin))
			motorpos.y -= change.y;
	}	
	
	stringstream ss;
	ss << "#" << motorpos.x << "P" << motorpos.y << "T" << light << "\n";
	
//	ss << "#" << motorpos.x << "P";
	command = ss.str();
	run_motor(command.c_str(), serialport);
/*
	ss.str("");

	ss << "#" << motorpos.y << "T";
	command = ss.str();
	run_motor(command.c_str(), serialport);
	ss.str("");

	ss << "#" << light << "\n";
	command = ss.str();
	run_motor(command.c_str(), serialport); 
	ss.str("");
*/
	prevError = error;
}

Rect detect(Mat& frame, Mat& gray, CascadeClassifier& cascade1, CascadeClassifier& cascade2, Scalar color)
{	
	vector<Rect> ROI1;
	vector<Rect> ROI2;
	vector<Rect> ROIs;

	equalizeHist(gray, gray);
	cascade1.detectMultiScale(gray, ROI1, scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(70,70));
	cascade2.detectMultiScale(gray, ROI2, scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(70,70));

	ROIs.reserve(ROI1.size() + ROI2.size());
	ROIs.insert(ROIs.end(), ROI1.begin(), ROI1.end());
	ROIs.insert(ROIs.end(), ROI2.begin(), ROI2.end());	
	
	if(ROIs.size() != 0)
		return ROIs[0];
	
	return Rect(0, 0, gray.size().width, gray.size().height);
}

void signalHandler(int signum)
{
	cout<<"\nClosing Application"<<endl;
	
	destroyAllWindows();
		
	close_motor(serialport);
	
	usleep(50000);

	exit(signum);
}

string get_tegra_pipeline(string width, string height, string fps)
{      
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)"+width+", height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int) "+width+" , height=(int) "+height+", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR, framerate=(fraction) "+fps+"/1 ! appsink";

}
