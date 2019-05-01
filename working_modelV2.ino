#include <DynamixelMotor.h>

// software serial pins
#define SOFT_RX_PIN 3 //not used but can be
#define SOFT_TX_PIN 4

// software serial without tristate buffer
SoftwareDynamixelInterface interface(SOFT_RX_PIN, SOFT_TX_PIN);

// setting global variables 
int16_t motorSpeed=200; //512;
const long unsigned int baudrate = 57600;
const int ledPin = 5; // pin the LED is connected to
const int motorReset = 6;
int value, tiltGoal = 512, panGoal = 512;
uint8_t led_state=true;

// setting motor interfaces
DynamixelMotor tiltMotor(interface, 002);
DynamixelMotor panMotor(interface, 001);

void setup() {
  Serial.begin(57600);
  pinMode(ledPin, OUTPUT);
  pinMode(motorReset, OUTPUT);
  interface.begin(57600);
  delay(100);

  // setting startup of tilt motor
  tiltMotor.enableTorque();  
  tiltMotor.jointMode(375, 800);
  tiltMotor.speed(motorSpeed);

  // setting stratup of pan motor
  panMotor.enableTorque();
  panMotor.jointMode(200, 800);
  panMotor.speed(motorSpeed);
  
  // setting LED to off
  digitalWrite(ledPin, 1);

  // setting motors on
  digitalWrite(motorReset, 1);
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
    else if (ch == 80)  // is the character = to P Send pan move for motor control
    {
      panMotor.goalPosition(value);
      panGoal = value;
      value = 0;
      Serial.println("Y");
    }
    else if (ch == 84)  // is the chararacter = to T Send tilt move for motor control
    {
      tiltGoal = value;
      tiltMotor.goalPosition(value);
      value = 0;
      Serial.println("Y");
    }
    else if (ch == 76 ) // is the character = to L Send light command to relay
    {
      digitalWrite(ledPin, value);
      value = 0;
      Serial.println("Y");
    }
    else if (ch == 77 ) // is the character = to M Reset motors relay
    {
      digitalWrite(motorReset, 0);
      delay (1000);
      digitalWrite(motorReset, 1);
      value = 0;
      Serial.println("Y");
    }
    else if ( ch == 35 ) // set of start character = to # Reset stream
    {
      value = 0;
      Serial.println("Y");
    }
    
  }
}
