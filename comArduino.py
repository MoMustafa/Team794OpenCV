
import serial
import time
import random

print ("Started")

serPort ="/dev/ttyUSB0" #use for Promini  #"/dev/ttyACM0" #use for UNO
baudRate = 57600
ser = serial.Serial(serPort, baudRate)
print ("Serial port" + serPort + " opened, Baudrate " + str(baudRate))

time.sleep(2)
n=0

while n < 10:
	m=0
#	movement = random.randint(230, 820)
#	print (movement)
	data = [str.encode(str(random.randint(230, 820))+"P"+str(random.randint(230, 820))+"TZ")]
#	data.append()
#	data.append("P")
#	data.append(str(random.randint(230, 820)))
#	data.append("TZ")
	print("Data =" + str(data))
	looptime = len(data)

	while m < looptime :
		sendingData = data[m]
		#print (sendingData)
		ser.write(sendingData)
		#time.sleep(1)
		m=m+1
	n=n+1
	time.sleep(0.5)

print("Done.")
ser.close


