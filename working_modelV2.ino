#include <DynamixelMotor.h>

// software serial pins
#define SOFT_RX_PIN 3 //not used but can be
#define SOFT_TX_PIN 10

// software serial without tristate buffer
SoftwareDynamixelInterface interface(SOFT_RX_PIN, SOFT_TX_PIN);

// setting global variables 
int16_t motorSpeed=512;
const long unsigned int baudrate = 57600;
const int ledPin = 5; // pin the LED is connected to
int value, tiltGoal = 512, panGoal = 512;
uint8_t led_state=true;

// setting motor interfaces
DynamixelMotor tiltMotor(interface, 001);
DynamixelMotor panMotor(interface, 002);

void setup() {
  Serial.begin(57600);
  pinMode(ledPin, OUTPUT);
  interface.begin(57600);
  delay(100);

  // setting startup of tilt motor
  tiltMotor.enableTorque();  
  tiltMotor.jointMode(204, 820);
  tiltMotor.speed(motorSpeed);

  // setting stratup of pan motor
  panMotor.enableTorque();
  panMotor.jointMode(204, 820);
  panMotor.speed(motorSpeed);
  
  // setting LED to off
  digitalWrite(ledPin, 0);
}

void loop() {
  // check if serial port is open
  if( Serial.available())
  {
    char ch = Serial.read();
    if( isDigit(ch) )// is this an ascii digit between 0 and 9?
    {
      value = (value * 10) + (ch - '0'); // yes, accumulate the value
    }      
    else if (ch == 80)  // is the character = to P
    {
      panMotor.goalPosition(value);
      panGoal = value;
      value = 0;
    }
    else if (ch == 84)  // is the chararacter = to T
    {
      tiltGoal = value;
      tiltMotor.goalPosition(value);
      value = 0;
    }
    else if (ch == 76 ) // is the character = to L
    {
      digitalWrite(ledPin, value);
      value = 0;
    }
  }
}
