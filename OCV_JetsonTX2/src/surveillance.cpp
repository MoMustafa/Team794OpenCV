#include "../includes/surveillance.h"
#include "../includes/motorctrl.h"
#include "../includes/server.h"

//VARIABLES
//Footage Settings
string width = "1280";
string height = "720";
string fps = "10";

//KLT Parameters 
const int MAX_COUNT = 100;
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
int minNeighbors = 9;
double scaleFactor = 1.05;

String bodycascade = "/home/nvidia/Desktop/SilasProgram/Cascades/haarcascades/haarcascade_upperbody.xml";
String facecascade = "/home/nvidia/Desktop/SilasProgram/Cascades/haarcascades/haarcascade_frontalface_alt.xml";
cv::CascadeClassifier body, face;

Point2f point;
Point mid(500,500);
Point motorpos;
Point screencenter;
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

//Server Variables
char *myfifo = "/tmp/myfifo";

int runprocess(CascadeClassifier& cascade, VideoCapture cap)
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
			ROI = detect(frame, gray, cascade, green);
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
		}

		if(displaced > 100)
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
		
		output.write(frame);
		//imshow("Footage", frame);
		
	}while(waitKey(1)!=27);
	
	destroyAllWindows();
	
	usleep(50000);
	
	close_motor(serialport);
	return 0;
}

void send2server(Mat frame, int count)
{
	int fd = open(myfifo, O_WRONLY);
	
	size_t s = 320;
	Mat resized;
	
	resize(frame, resized, Size(0,0), 0.5, 0.5, INTER_NEAREST);

	string filename = "/home/nvidia/WebServer/flask-video-streaming/";
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
	int panmin = 200;
	int panmax = 800;
	int tiltmin = 375;
	int tiltmax = 800;
	
	Point2f error;
	int minwindow = 50;
	int maxwindow = 500;
	double p_scale = 0.026;
	double t_scale = 0.026;
	double int_p = 0.01;
	double int_t = 0.01;
    double err_scale = 50.0;
    
	error = (center - screencenter);
	cout<<"Ex: "<<error.x<<" Ey: "<<error.y<<endl;
	
	//Bound Check 2.0
	if((motorpos.x > panmax && error.x > 0) || (motorpos.x < panmin && error.x < 0))
		error.x = 0;
	
	if((motorpos.y > tiltmax && error.y > 0) || (motorpos.y < tiltmin && error.y < 0))
		error.y = 0;
	
	integ.x += error.x/err_scale;	
	integ.y += error.y/err_scale;
	
	if(abs(error.x) > minwindow && abs(error.x) < maxwindow)
		motorpos.x += (p_scale)*(error.x)+(int_p*integ.x);
	
	if(abs(error.y) > minwindow && abs(error.y) < maxwindow)
		motorpos.y += (t_scale)*(error.y)+(int_t*integ.y);
	
	stringstream ss;
	ss << motorpos.x << "P" << motorpos.y << "T" << light << "\n";
	command = ss.str();
	
	run_motor(command.c_str(), serialport);
}

Rect detect(Mat& frame, Mat& gray, CascadeClassifier& cascade, Scalar color)
{	
	vector<Rect> ROIs;
	equalizeHist(gray, gray);
	cascade.detectMultiScale(gray, ROIs, scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(40,40));
	
	if(ROIs.size() != 0)
		return ROIs[0];
	
	return Rect(0, 0, gray.size().width, gray.size().height);
}

string get_tegra_pipeline(string width, string height, string fps)
{      
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)"+width+", height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int) "+width+" , height=(int) "+height+", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR, framerate=(fraction) "+fps+"/1 ! appsink";

}
