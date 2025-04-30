#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <cmath>

const char* ssid = "ESP32_AP";
const char* password = "esp32password";

// UDP settings
WiFiUDP udp;
unsigned int localUdpPort = 4210;
char packetBuffer[255]; // Buffer for incoming packets

// DO NOT CHANGE
#define R1_PIN 13
#define G1_PIN 12
#define B1_PIN 11
#define R2_PIN 10
#define G2_PIN 9
#define B2_PIN 8
#define A_PIN 7
#define B_PIN 6
#define C_PIN 5
#define D_PIN 4
#define E_PIN 3
#define LAT_PIN 2
#define OE_PIN 41
#define CLK_PIN 40

// const char* ssid = "iOT-LAB";
// const char* password = "photon999";

// Initialize matrix
MatrixPanel_I2S_DMA* dma_display = nullptr;
const size_t SIZE = 56;

uint16_t myWHITE;

bool array[SIZE][SIZE][SIZE] = { 0 };  // x, y, z
int offset = 0;

enum Shape {
  CUBE_OUTLINE,
  SPHERE,
  CYLINDER
};

Shape shape = CUBE_OUTLINE;

void drawMap(bool array[SIZE][SIZE][SIZE], double angRad) {

  for (int i = 0; i < SIZE; i++) {
    for (int z = 0; z < SIZE; z++) {
      int realI = i - SIZE/2;
      int x = realI * cos(angRad) + SIZE/2;
      int y = realI * sin(angRad) + SIZE/2;

      if (x >= SIZE) x = SIZE - 1;
      if (y >= SIZE) y = SIZE - 1;

      if (x < 0) x = 0;
      if (y < 0) y = 0;

      bool lightUp = array[x][y][z];

      if (lightUp) {
        dma_display->drawPixel((64 - SIZE)/2 + i, (64 - SIZE)/2 + z, myWHITE);
      } else {
        dma_display->drawPixelRGB888((64 - SIZE)/2 + i, (64 - SIZE)/2 + z, 0, 0, 0);
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

void cubeOutline() {
  int thickness = 2;
  for (signed int offset = 16; offset <= 20; offset++) {
    for (signed int x = offset; x < SIZE - offset; x++) {
      for (signed int y = offset; y < SIZE - offset; y++) {
        for (signed int z = offset; z < SIZE - offset; z++) {
          int cnt = 0;
          if (abs(x - offset) <= thickness || abs(x - ((int)SIZE-1-offset)) <= thickness) cnt++;
          if (abs(y - offset) <= thickness || abs(y - ((int)SIZE-1-offset)) <= thickness) cnt++;
          if (abs(z - offset) <= thickness || abs(z - ((int)SIZE-1-offset)) <= thickness) cnt++;
          if (cnt >= 2) { // Only edges and corners (like a wireframe)
            array[x][y][z] = true;
          }
        }
      }
    }
  }
}

void solidCube() {
  for (int x = 10; x < SIZE - 10; x++) {
    for (int y = 10; y < SIZE-10; y++) {
      for (int z = 10; z < SIZE-10; z++) {
        array[x][y][z] = true;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  HUB75_I2S_CFG::i2s_pins _pins = { R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN };
  HUB75_I2S_CFG mxconfig(64, 64, 1, _pins);

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->clearScreen();

  Serial.print(1);


  // Declare some basic colors
  myWHITE = dma_display->color565(200, 200, 0);
  dma_display->setBrightness8(150);
  Serial.print(2);
  // dma_display->fillScreen(myWHITE);

  // Configure ESP32 as access point
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start UDP
  udp.begin(localUdpPort);
  Serial.printf("UDP server listening on port %d\n", localUdpPort);

  cubeOutline();
  
  Serial.println(3);

}

double lastAngle = 0;
double lastReadAngle = 0;
long int lastTime = 0;
double speed = 58.5;
double angle = 0.0;
int oldR = 0;

void loop() {
  
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Serial.print("Recieved %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());

    // Read packet into buffer
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0; // null-terminate the string
    }
    Serial.printf("UDP packet contents: %s\n", packetBuffer);

    // gets rgb values from the received string
    char rStr[4], gStr[4], bStr[4], speedStr[7];
    strncpy(rStr, packetBuffer, 3);
    strncpy(gStr, packetBuffer + 3, 3);
    strncpy(bStr, packetBuffer + 6, 3);
    strncpy(speedStr, packetBuffer + 9, 6);

    rStr[3] = '\0';
    gStr[3] = '\0';
    bStr[3] = '\0';
    speedStr[6] = '\0';

    int r = atoi(rStr);
    int g = atoi(gStr);
    int b = atoi(bStr);

    if (r != oldR) {
      switch (rand() % 3) {
        case 1:
          shape = CUBE_OUTLINE;
          break;
        case 2:
          shape = SPHERE;
          break;
        case 0:
          shape = CYLINDER;
          break;
      }
    }
    oldR = r;
    Serial.println(speedStr);
    // speed = atof(speedStr);

    myWHITE = dma_display->color565(r, g, b);
  }
  
  speed = 64.0;

  // if (lastReadAngle != angle) { // like if we are rate limited
  long int time = micros();
  if (lastTime == 0) {
    lastTime = time;
  }

  angle = lastAngle + speed * (double)(time - lastTime) / 1000000;
  // Serial.println(lastTime - time);
  // Serial.println(speed * (lastTime - time) / 1000000);
  // Serial.println(angle);
  if (angle >= 2 * PI) angle -= 2 * PI;
  if (angle < 0) angle += 2 * PI;

  // }
  lastAngle = angle;
  lastTime = time;

  if (!dma_display) {
    Serial.println("Display not initialized!");
    return;
  }

  switch (shape) {
    case CUBE_OUTLINE:
      drawMap(array, angle); 
      break;
    case SPHERE:
      dma_display->clearScreen();
      dma_display->drawCircle(32, 32, 10, myWHITE);
      break;
    case CYLINDER:
      dma_display->clearScreen();
      dma_display->fillRect(22, 22, 20, 20, myWHITE);
      break;
  }
  
  // 
}