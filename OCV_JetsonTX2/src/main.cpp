#include "../includes/surveillance.h"

int main()
{	
	cv::CascadeClassifier body, face;	
	body.load(bodycascade);
	face.load(facecascade);
	
	string pipeline = get_tegra_pipeline(width, height, fps);
	VideoCapture cap(pipeline, CAP_GSTREAMER);
	//VideoCapture cap("/home/nvidia/Desktop/ProgramKLT/media/Day_Stable_15fps_1080.avi");
	
	if(cap.isOpened())
		runprocess(face, cap);
	else
		cout<<"Camera cannot be opened."<<endl;
	
	return 0;	
}
