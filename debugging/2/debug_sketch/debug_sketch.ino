// --------- //
// Libraries //
// --------- //
#include <Servo.h>      // Steering and motor
#include <Wire.h>       // Sonars
#include <SonarSRF08.h> // Sonars

// --------- //
// Constants //
// --------- //
int counter = 0;
//SERVO
const int rcPinSteer = 3; // rc steering
const int rcPinESC = 9;  // rc motor

//MOTOR -- Digital
const int servoPin = 5;   // pin to which the servo motor is attached
const int escPin = 6;     // pin to which the ESC is attached

//SONARS  -- I2C
//Change these to Constants instead as it's preferred on the arduino
#define FC_08_ADDRESS (0xE6 >> 1) // Front Center Sonar
#define FR_08_ADDRESS (0xE0 >> 1) // Front Right Sonar
#define GAIN_REGISTER 0x09        // Analog Gain
#define LOCATION_REGISTER 0x8C    // 1 meter

char unit = 'c'; // 'i' for inches, 'c' for centimeters, 'm' for micro-seconds

//INFRAREDS  -- Analog
const int irFrontRightPin = 0;     // pin to which the front right infrared sensor is attached
const int irRearRightPin = 1;      // pin to which the rear right infrared sensor is attached
const int irRearCenterPin  = 2;    // pin to which the rear infrared sensor is attached

//WHEEL ENCODERS
const int pulsesPerMeter = 150; // TBD
const int encoderRightPin = 18;
const int encoderLeftPin = 19;

// ----------------------- //
// Instatiation of objects //
// ----------------------- //
Servo motor, steering;
unsigned long rcControllerFlag;
int steer, velocity, controlFlag;
const int fifoSize = 3;
int velocityArray[fifoSize] = {0};
int steerArray[fifoSize] = {0};
int frCSArray[fifoSize] = {0};
int frRSArray[fifoSize] = {0};
int iRFRArray[fifoSize] = {0};
int iRRRArray[fifoSize] = {0};
int iRRCArray[fifoSize] = {0};

//SONARS
SonarSRF08 FrontCenterSonar;
SonarSRF08 FrontRightSonar;

void setup() {
  //SERIAL CONNECTION
  Serial.begin(57600);

  motor.attach(escPin);
  motor.writeMicroseconds(1500);  // set motor to neutral
  steering.attach(servoPin);
  steering.write(90);  // set servo to neutral
  attachInterrupt(digitalPinToInterrupt(3), rcControllerInterrupt, RISING); // interupts from rc controller

  rcControllerFlag = 0; // Set to 1 if RC controller is turned on (interupt)
  controlFlag = 1; // Set to 0 when the RC takes over, used to set steering and speed to neutral when RC controller is turned off

  // Setting up the sonars and limiting the range to 1 meter.
  FrontCenterSonar.connect(FC_08_ADDRESS, GAIN_REGISTER, LOCATION_REGISTER);
  FrontRightSonar.connect(FR_08_ADDRESS, GAIN_REGISTER, LOCATION_REGISTER);
}

void loop() {
  Serial.println(encodeNetstring(getUSData() + getIRData())); // encode as a netstring and send over serial
  counter++;

  //Serial.println(US);
  if (Serial.available() > 0){
    Serial.read();
    //Serial.println("HELLO!");
  }
  delay(100);
}
/*
 * Function for manual control with an RC-Controller
 */
void rcControl() {
  Serial.println("RC Control took over!");
  velocity = pulseIn(rcPinESC, HIGH, 25000); // get a value from the RC-Controller
  velocity = constrain(velocity, 1020, 1900); // we dont want any values aoutside this range
  velocity = map(velocity, 1020, 1900, -100, 100); // map values for easier use
  Serial.print("velocity ------- ");
  Serial.println(velocity);
  velocity = fifo(velocityArray, velocity);
  steer = pulseIn(rcPinSteer, HIGH, 25000);
  steer = constrain(velocity, 1090, 1750); // check these values first
  steer = map(steer, 1090, 1750, 0, 180); // map values for easier use
  steer = fifo(steerArray, steer);
  //Serial.print("steer ");
  //Serial.println(steer);
  Serial.print("velocity ");
  Serial.println(velocity);
  //steering.write(steer);
  if (velocity > -5 && velocity < 5) {
    motor.writeMicroseconds(1500);
    //Serial.println(1500);
  } else if (velocity > 5) {
    motor.writeMicroseconds(1550 + velocity);
  } else if (velocity < 95) {
    motor.writeMicroseconds(1200 + velocity);
  }
  //int temp = pulseIn(rcPinSteer, LOW, 25000);
  //Serial.println(temp);
  if (pulseIn(rcPinSteer, HIGH, 25000) == 0) {
    rcControllerFlag = 0;
    Serial.print("RC control set to off!");
  }
}
/*
 * Reads values from the serial port. Reads the int values and sets the steering
 * and motor to what it read. Use 't' for steering anv 'v' for motor
 */
void manualControl() {
  String input;
  if (Serial.available() > 0) {
    input = Serial.readStringUntil('\n');

    if (input.startsWith("t")) { // turning
      steer = input.substring(1).toInt();
      if (steer <= 180 && steer >= 0) { // check that the value is in range
        steering.write(steer);
        Serial.println("Turning set to: ");
        Serial.println(steer);
      }
    } else if (input.startsWith("v")) { // velocity
      velocity = input.substring(1).toInt();
      if (velocity <= 2000 && velocity >= 1000) { // check that the value is in range
        motor.writeMicroseconds(velocity);
        Serial.print("Velocity set to: ");
        Serial.println(velocity);
      }
    }
  }
}
/*
 * Listens for the interupts from the RC-Controller
 */
void rcControllerInterrupt() {
  rcControllerFlag = 1;
}
/*
 * Returns both US sensors value as a string.
 * " USF 'value' USR 'value'"
 */
String getUSData() {
  String USF = "USF ";
  //USF.concat(fifo(frCSArray, FrontCenterSonar.getRange(unit)));
  USF.concat(counter);
  String USR = " USR ";
  //USR.concat(fifo(frRSArray, FrontRightSonar.getRange(unit)));
  USR.concat(counter);

  return USF + USR;
}
/*
 * Returns all 3 IR sensors value as a string.
 * " IRFR 'value' IRRR 'value' IRRC 'value'"
 */
String getIRData() {
  String IRFR = " IRFR ";
  //IRFR.concat(fifo(iRFRArray, irCalc(irFrontRightPin)));
  IRFR.concat(counter);
  String IRRR = " IRRR ";
  //IRRR.concat(fifo(iRRRArray, irCalc(irRearRightPin)));
  IRRR.concat(counter);
  String IRRC = " IRRC ";
  //IRRC.concat(fifo(iRRCArray, irCalc(irRearCenterPin)));
  IRRC.concat(counter);
  return IRFR + IRRR + IRRC;
}

/*
 * Calculates the distance an IR sensor is reporting. Returns the value as
 * centimeters. Returns 0 if the value is outside 5-25.
 * Takes an analog pin as input.
 */
int irCalc(int pin) {
  float volt = analogRead(pin);
  int cm = ((2914 / (volt + 5 )) - 1); // gives the range in centimeters
  if (cm >= 5 && cm <= 25) {
    return cm;
  }
  return 0; // if the value is not in our range
}
/*
 * Takes an array of integers as input and a new integer value
 * the new int value will be added and the oldest value of the
 * array will be removed. Oldest value is at the top.
 */
int fifo(int array[], int newValue) {
  int sum = 0;
  for (int i = 0; i < fifoSize - 1; i++) {
    array[i] = array[i + 1];
    sum += array[i + 1];
  }
  array[fifoSize - 1] = newValue;
  sum += newValue;
  return sum / fifoSize;
}

/*
 * Encoding netsrings. Takes a string and returns it as
 * '5:hello'
 */
String encodeNetstring(String toEncode){
  String str = "";
  if (toEncode.length() < 1){
    return "Netstrings: Nothing to encode";
  }
  return String(toEncode.length()) + ":" + toEncode + ",";
}
/*
 * Decoding netsrings. Takes a string like this '5:hello'
 * and returns it like this 'hello'
 * Also checks that the netstring is of the correct format and size.
 */
String decodeNetstring(String toDecode){
  if (toDecode.length() < 3){ // A netstring can't be shorter than 3 characters
    return "Netstrings: Wrong format";
  }

  // Check that : and , exists at the proper places
  int semicolonIndex = toDecode.indexOf(':');
  int commaIndex = toDecode.lastIndexOf(',');
  if (semicolonIndex < 1 || commaIndex != toDecode.length() - 1) { // the first character has to be a number
    return "Netstrings: No semicolon found, or no comma found";
  }

  // Parse control number
  String number = toDecode.substring(0, semicolonIndex);
  int controlNumber = number.toInt();
  if (controlNumber < 1){ // the netstring has to contain atleast 1 character to be parsed
    return "Netstrings: Control Number is to low";
  }

  // Check that the length of the string is correct
  toDecode = toDecode.substring(semicolonIndex + 1, commaIndex); // the data that we want to parse
  if (controlNumber != toDecode.length()){
    return "Netstrings: Wrong length of data";
  }
  return toDecode;
}
