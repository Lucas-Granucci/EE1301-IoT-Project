// Include Particle Device OS APIs
#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);
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


/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////

// setup() runs once, when the device is first turned on
void setup() {
  pinMode(SIGNAL_A, INPUT);
  pinMode(SIGNAL_B, INPUT);

  attachInterrupt(SIGNAL_A, readEncoder, CHANGE);
  attachInterrupt(SIGNAL_B, readEncoder, CHANGE);

  // Particle.variable("speed", speedRadiansPerSecond);
  // Particle.variable("position", cloudEncoderPositionRadians);


  /////////////////////////////////////////////////////////////////////////////

  // setup particle to connect to esp32's AP and not cloud
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


  Serial.begin(9600);
}

void readEncoder() {
  if (digitalRead(SIGNAL_A) && digitalRead(SIGNAL_B)) {
    encoderPosition++;
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // cloudEncoderPositionRadians = (double)encoderPosition / 120.0 * 2 * 3.141592653589793;
  // pastPositions[indexToReplace] = ((double) cloudEncoderPositionRadians);
  // pastTimestamps[indexToReplace] = micros();
  // // Serial.println(millis());
  // indexToReplace++;
  // if (indexToReplace >= NUM_TIMESTAMPS) indexToReplace = 0; // go back to replacing the first element once it is the furthest one back

  // int mostRecentIndex;
  // if (indexToReplace - 1 < 0) {
  //   mostRecentIndex = NUM_TIMESTAMPS - 1;
  // } else {
  //   mostRecentIndex = indexToReplace - 1;
  // }

  // speedRadiansPerSecond = (pastPositions[mostRecentIndex] - pastPositions[indexToReplace]) / (pastTimestamps[mostRecentIndex] - pastTimestamps[indexToReplace]) * 1000000.0;





  ///////////////////////////////////////////////////////////////////////
  if (!connected) {
    // reconnect if disconnected
    if (!WiFi.ready()) {
      Serial.print("Reconnecting to ESP32 AP...");
      // WiFi.setCredentials(ESP_SSID, ESP_PASSWORD, WPA2);
      WiFi.connect();
      waitFor(WiFi.ready, 5000);

      if (WiFi.ready()) {
        connected = true;
        udp.begin(localPort);
        Serial.println("We have Reconnected");
      }

    }
    delay(1000);
    return;
  }


  // send message every interval
  static unsigned long int lastSendTime = 0;
  if (millis() - lastSendTime > 10) {
    udp.beginPacket(serverIP, serverPort);
    String message = "Good soup, it works";
    message += millis();
    udp.write(message);
    udp.endPacket();

    Serial.printlnf("Sent message to ESP32 at %s:%d", serverIP.toString().c_str(), serverPort);
    lastSendTime = millis();
  }

  ///////////////////////////////////////////////////////////////////////

  

  // if(millis() % 500 == 9){
  //   Serial.print("most recent: ");
  // Serial.println(pastPositions[mostRecentIndex]);
  // Serial.print("new recent ");
  // Serial.println(pastPositions[indexToReplace]);
  // Serial.print("most time ");
  // Serial.println(pastTimestamps[mostRecentIndex]);
  // Serial.print("new time ");
  // Serial.println(pastTimestamps[indexToReplace]);
  //Serial.println(speedRadiansPerSecond);
  // }
  
}
