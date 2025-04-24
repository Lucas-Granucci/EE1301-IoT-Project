/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

#define SIGNAL_A D2
#define SIGNAL_B D3

#define NUM_TIMESTAMPS 1000

volatile int encoderPosition = 0;
double cloudEncoderPositionRadians = 0;
double speedRadiansPerSecond;
double pastPositions[NUM_TIMESTAMPS] = {0}; // 100 so we can take the average speed over a longer period of time than 1 loop (i think it's prolly more accurate)
double pastTimestamps[NUM_TIMESTAMPS] = {0};
int indexToReplace = 0;

void readEncoder();

// setup() runs once, when the device is first turned on
void setup() {
  pinMode(SIGNAL_A, INPUT);
  pinMode(SIGNAL_B, INPUT);

  attachInterrupt(SIGNAL_A, readEncoder, CHANGE);
  attachInterrupt(SIGNAL_B, readEncoder, CHANGE);

  Particle.variable("speed", speedRadiansPerSecond);
  Particle.variable("position", cloudEncoderPositionRadians);
  Serial.begin(9600);
}

void readEncoder() {
  if (digitalRead(SIGNAL_A) && digitalRead(SIGNAL_B)) {
    encoderPosition++;
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  cloudEncoderPositionRadians = (double)encoderPosition / 120.0 * 2 * 3.141592653589793;
  pastPositions[indexToReplace] = ((double) cloudEncoderPositionRadians);
  pastTimestamps[indexToReplace] = micros();
  // Serial.println(millis());
  indexToReplace++;
  if (indexToReplace >= NUM_TIMESTAMPS) indexToReplace = 0; // go back to replacing the first element once it is the furthest one back

  int mostRecentIndex;
  if (indexToReplace - 1 < 0) {
    mostRecentIndex = NUM_TIMESTAMPS - 1;
  } else {
    mostRecentIndex = indexToReplace - 1;
  }

  speedRadiansPerSecond = (pastPositions[mostRecentIndex] - pastPositions[indexToReplace]) / (pastTimestamps[mostRecentIndex] - pastTimestamps[indexToReplace]) * 1000000.0;
  

  // if(millis() % 500 == 9){
  //   Serial.print("most recent: ");
  // Serial.println(pastPositions[mostRecentIndex]);
  // Serial.print("new recent ");
  // Serial.println(pastPositions[indexToReplace]);
  // Serial.print("most time ");
  // Serial.println(pastTimestamps[mostRecentIndex]);
  // Serial.print("new time ");
  // Serial.println(pastTimestamps[indexToReplace]);
  Serial.println(speedRadiansPerSecond);
  // }
  
}
