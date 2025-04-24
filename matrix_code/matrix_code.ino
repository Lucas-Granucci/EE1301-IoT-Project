#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFIUdp.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <cmath>

const char* ssid = "ESP32_AP";
const char* password = "esp32password";

// UDP settings
WiFiUDP udp;
unsigned int localUdpPort = 4210;
char packetBuffer = [255]; // Buffer for incoming packets

// DO NOT CHANGE
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 22
#define C_PIN 5
#define D_PIN 18
#define E_PIN 33 
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 19

// const char* ssid = "iOT-LAB";
// const char* password = "photon999";

// Initialize matrix
MatrixPanel_I2S_DMA *dma_display = nullptr;
const size_t SIZE = 42;

uint16_t myWHITE;

bool array[SIZE][SIZE][SIZE] = {0}; // x, y, z

void drawMap(bool array[SIZE][SIZE][SIZE], int angRad) {
  
  for (int i = 0; i < SIZE; i++) {
    for (int z = 0; z < SIZE; z++) {

      int x = round(i * cos(angRad));
      int y = round(i * sin(angRad));

      bool val = array[x][y][z];

      // switch (val) {
      //   case 0:
      //     break;
      //   case 1:
      //     dma_display->drawPixel(i, z, myWHITE);
      //     break;
      //   default:
      //     break;
      // }
      if (val) {
        dma_display->drawPixel(i, z, myWHITE);
      } else {
        dma_display->drawPixelRGB888(i, z, 0, 0, 0);
      }
    }
  }
}

void drawMap(int array[SIZE][SIZE]) {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {

      int val = array[i][j];

      switch (val) {
        case 0:
          break;
        case 1:
          dma_display->drawPixel(i, j, myWHITE);
          break;
        default:
          break;
      }
    }
  }
}

// String getHTTP(String url) {
//   HTTPClient http;

//   http.begin(url);
//   int httpResponseCode = http.GET();
//   String result = "{}"; 
//   if (httpResponseCode>0) {
//     Serial.print("HTTP Response code: ");
//     Serial.println(httpResponseCode);
//     result = http.getString();
//   }
//   http.end();
//   return result;

// }

void setup() {
  Serial.begin(115200);
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(64, 64, 1, _pins);

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->clearScreen();

  Serial.print(1);

  ///////////////////////////////////////////////////////////////////////////////

  // Configure ESP32 as access point
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFI.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start UDP
  udp.begin(localUdpPort);
  Serial.printf("UDP server listening on port %d\n", localUdpPort);

  ///////////////////////////////////////////////////////////////////////////////
  

  // Declare some basic colors
  myWHITE = dma_display->color565(0, 0, 255);
  dma_display->setBrightness8(150);
  Serial.print(2);
  dma_display->fillScreen(myWHITE);

  // Make square
  // for (int i = 16; i < 48; i++) {
  //   for (int j = 16; j < 48; j++) {
  //       array[i][j] = 1;
  //   }
  // }

  // make cube
  for (int x = 10; x < 32; x++) {
    for (int y = 10; y < 32; y++) {
      for (int z = 10; z < 32; z++) {
        array[x][y][z] = true;
      }
    }
  }
  
  // drawMap(array);

  // WiFi.begin(ssid, password);
  // while(WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.print("ESP32 IP Address: ");
  // Serial.println(WiFi.localIP());
}

double lastAngle = 0;
double lastReadAngle = 0;
int lastTime = 0;

void loop() {

  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Recieved %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotedPort());

    // Read packet into buffer
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0; // null-terminate the string
    }
  }

  Serial.printf("UDP packet contents: %s\n", packetBuffer);
  
  // JSONVar angleJSON = JSON.parse(getHTTP("https://api.particle.io/v1/devices/thinky/position?access_token=78a99eb4943d042f674bedd4ab8095af43702e39"));
  // double angle = angleJSON["result"];
  // Serial.println(angle);

  // if (lastReadAngle != angle) { // like if we are rate limited
  //   int time = micros();
  //   JSONVar speedJSON = JSON.parse(getHTTP("https://api.particle.io/v1/devices/thinky/speed?access_token=78a99eb4943d042f674bedd4ab8095af43702e39"));
  //   double speed = speedJSON["result"];
  //   Serial.println(speed);

  //   angle = lastAngle + speed * (lastTime - time);

  // }
  // lastAngle = angle;

  // drawMap(array, angle);
  
  
}