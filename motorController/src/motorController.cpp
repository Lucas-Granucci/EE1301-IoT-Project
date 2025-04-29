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
#define BUTTON_PIN D5

#define NUM_TIMESTAMPS 100

volatile int encoderPosition = 0;
double cloudEncoderPositionRadians = 0;
double speedRadiansPerSecond;
double pastPositions[NUM_TIMESTAMPS] = {0}; // 100 so we can take the average speed over a longer period of time than 1 loop (i think it's prolly more accurate)
double pastTimestamps[NUM_TIMESTAMPS] = {0};
int indexToReplace = 0;

void readEncoder();

String color;

const char* ESP_SSID = "ESP32_AP";
const char* ESP_PASSWORD = "esp32password";

// server details
IPAddress serverIP(192, 168, 4, 1);  // check serials logs of esp32
unsigned int serverPort = 4210;
unsigned int localPort = 4210;

UDP udp;

// buffers for sending and recieving data
char packetBuffer[255];
char replyBuffer[255];

// connection state
bool connected = false;

String getRandomRGBValue() {
  int ret = random(256);
  if (ret < 100 && ret > 9) {
    return "0" + (String)ret;
  } else if (ret < 10) {
    return "00" + (String)ret;
  }
  return (String)ret;
}

// setup() runs once, when the device is first turned on
void setup() {
  Serial.begin(9600);

  WiFi.clearCredentials();

  pinMode(SIGNAL_A, INPUT);
  pinMode(SIGNAL_B, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  attachInterrupt(SIGNAL_A, readEncoder, CHANGE);
  attachInterrupt(SIGNAL_B, readEncoder, CHANGE);

  Particle.disconnect();
  WiFi.disconnect();
  delay(1000);  // let WiFi chill a bit

  // connect to esp32 AP
  Serial.printlnf("Connecting to %s...", ESP_SSID);
  WiFi.setCredentials(ESP_SSID, ESP_PASSWORD, WPA2);
  WiFi.connect();

  // wait for connection
  waitFor(WiFi.ready, 10000);

  if (WiFi.ready()) {
    Serial.println("Connected to ESP32 AP!");
    Serial.printlnf("IP Address: %s", WiFi.localIP().toString().c_str());

    connected = true;
  
    // start udp
    udp.begin(localPort);
    Serial.printlnf("UDP initialized on port %d", localPort);
  } else {
    Serial.println("Failed to connect to ESP32 AP");
  }

  // Particle.variable("speed", speedRadiansPerSecond);
  // Particle.variable("position", cloudEncoderPositionRadians);
  color = getRandomRGBValue() + getRandomRGBValue() + getRandomRGBValue();
}

void readEncoder() {
  if (digitalRead(SIGNAL_A) && digitalRead(SIGNAL_B)) {
    encoderPosition++;
  }
}

String generateNewColor() {
  return getRandomRGBValue() + getRandomRGBValue() + getRandomRGBValue();
}

int lastPressed = LOW;

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (lastPressed == LOW) {
      color = generateNewColor();
    }
    lastPressed = HIGH;
  } else {
    lastPressed = LOW;
  }

  if (!connected) {
    // reconnect if disconnected
    if (!WiFi.ready()) {
      Serial.print("Reconnecting to ESP32 AP...");
      // WiFi.setCredentials(ESP_SSID, ESP_PASSWORD, WPA2);
      WiFi.connect();
      waitFor(WiFi.ready, 5000);

      if (WiFi.ready() && !connected) {
        connected = true;
        udp.begin(localPort);
        Serial.println("We have Reconnected");
      }

    }
    delay(100);
    return;
  }

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
  
  static unsigned long int lastSendTime = 0;
  if (millis() - lastSendTime > 10) {
    udp.beginPacket(serverIP, serverPort);
    String message = color; // + speedRadiansPerSecond;
    udp.write(message);
    if (!udp.endPacket()) {
      Serial.println("UDP packet failed to send!");
    }

    // Serial.printlnf("Sent message to ESP32 at %s:%d", serverIP.toString().c_str(), serverPort);
    Serial.println(message);
    lastSendTime = millis();
  }

  // Serial.println(speedRadiansPerSecond);

}
