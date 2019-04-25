#include "../includes/surveillance.h"
#include "../includes/motorctrl.h"

int main()
{	
/*	//Motor Calibration
	const char* port_name = "/dev/ttyUSB0"; 
	int serialport = init_motor(port_name);
	
	while(1)
	{
		string command;
		cin >> command;
		run_motor(command.c_str(), serialport);
	}
	
*/	
	cv::CascadeClassifier body, face;	
	body.load(bodycascade);
	face.load(facecascade);
	
	string pipeline = get_tegra_pipeline(width, height, fps);
	VideoCapture cap(pipeline, CAP_GSTREAMER);
	
	if(cap.isOpened())
		runprocess(face, cap);
	else
		cout<<"Camera cannot be opened."<<endl;
	
}
