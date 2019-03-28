#include "../includes/surveillance.h"
#include "../includes/motorctrl.h"

int main()
{	
	
	cv::CascadeClassifier body, face;	
	body.load(bodycascade);
	face.load(facecascade);
	
	string pipeline = get_tegra_pipeline(width, height, fps);
	VideoCapture cap(pipeline, CAP_GSTREAMER);
	//VideoCapture cap("/home/nvidia/Desktop/ProgramKLT/media/Day_Stable_15fps_1080.avi");
	
		
	/*
	//MOTOR CALIBRATION
	const char* port_name = "/dev/ttyUSB0"; 
	int serialport = init_motor(port_name);
	
	string command;
	
	while(true)
	{
		cin >> command;
		command = command+"0L\n";
		run_motor(command.c_str(), serialport);
	}
	*/
	
	
	if(cap.isOpened())
		runprocess(face, cap);
	else
		cout<<"Camera cannot be opened."<<endl;
	
	return 0;	
}
