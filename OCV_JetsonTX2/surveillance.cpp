#include "surveillance.h"

surveillance::surveillance() : 
red(0,0,255), green(0,255,0), blue(255, 0, 0), white(255,255,255), black(0,0,0)
{}

int surveillance::runprocess(CascadeClassifier& cascade, VideoCapture cap)
{

	Mat gray, prevGray, image, frame;
	vector<Point2f> features[2];
	vector<uchar> status;
    vector<float> err;
	Point center(cap.get(3)/2, cap.get(4)/2);
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
    		if(prevGray.empty())
    			gray.copyTo(prevGray);
    	
			calcOpticalFlowPyrLK(prevGray, gray, features[0], features[1], status, err, winSize, maxLevel, termcrit, flags, minThreshold);
			
			DrawPoints(frame, ROI, center, features[1], status);
		}
    	needToInit = false;
		
		std::swap(features[1], features[0]);
    	cv::swap(prevGray, gray);
    	
		imshow("Footage", frame);
	}while(waitKey(1)!=27);
	
	destroyAllWindows();
	return 0;
}

void surveillance::DrawPoints(Mat& frame, Rect ROI, Point& center, vector<Point2f>& features, vector<uchar> status)
{
	size_t i, k;	
	Point totals;

	
	for(i=k=0; i<features.size(); i++)
	{
		totals.x += features[i].x;
		totals.y += features[i].y;
	
		if(!status[i])
			continue;
		
		features[k++] = features[i];
		circle(frame, features[i], radius, red, thickness, lineType);
	}
	
	
		center.x = (int) totals.x/features.size();
		center.y = (int) totals.y/features.size();
	
	/* RESET NEEDS FIXING
	else
	{
		center.x = ROI.x + ROI.width/2;
		center.y = ROI.y + ROI.height/2;
	}
	*/
	line(frame, Point(center.x-20, center.y), Point(center.x+20, center.y), green, 2);
	line(frame, Point(center.x, center.y-20), Point(center.x, center.y+20), green, 2);
}

Rect surveillance::detect(Mat& frame, Mat& gray, CascadeClassifier& cascade, Scalar color)
{	
	vector<Rect> ROIs;
	equalizeHist(gray, gray);
	cascade.detectMultiScale(gray, ROIs, scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(170,170));
	
	if(ROIs.size() != 0)
		return ROIs[0];
	
	return Rect(0, 0, gray.size().width, gray.size().height);
}

string surveillance::get_tegra_pipeline(string width, string height, string fps)
{      
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)"+width+", height=(int) "+height+", format=(string)I420, framerate=(fraction) "+fps+"/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int) "+width+" , height=(int) "+height+", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR, framerate=(fraction) "+fps+"/1 ! appsink";

}


