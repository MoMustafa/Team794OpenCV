#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main()
{

	FILE *filePtr;
	
	char therm_path[64];
	char readBuf[16];
	float temp;
	unsigned int curZone = 5;
	
	while(true)
	{
	
		unsigned int readBytes = 0;
		sprintf(therm_path, "/sys/devices/virtual/thermal/thermal_zone%u/temp", curZone);
		
		printf("Reading Zone %u Temp:\t", curZone);
		
		filePtr = fopen(therm_path, "r");
		
		if(!filePtr)
		{
			printf("Error, failed to open file\n");
			
			continue;
		}
		
		while(readBytes <16)
		{
			int curRead = fread(readBuf + readBytes, 1, 16 - readBytes, filePtr);
			if(curRead > 0)
				readBytes += curRead;
			else
			break;	
		}
		
		fclose(filePtr);
		
		if(readBytes>0)
			*(readBuf + readBytes - 1) = 0;
		
		//printf("[%s]\t", readBuf);
			
		temp = atof(readBuf);
		temp /= 1000.f;
		printf("%.02f\n", temp);	
		
	}
	exit(0);
}
