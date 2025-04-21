#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

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

const char* ssid = "iOT-LAB";
const char* password = "photon999";

// Initialize matrix
MatrixPanel_I2S_DMA *dma_display = nullptr;
const size_t SIZE = 64;

uint16_t myWHITE;

int array[SIZE][SIZE] = {0};

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

void setup() {
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(64, 64, 1, _pins);

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->clearScreen();

  // Declare some basic colors
  myWHITE = dma_display->color565(255, 255, 255);

  // Make square
  for (int i = 16; i < 48; i++) {
    for (int j = 16; j < 48; j++) {
        array[i][j] = 1;
    }
  }
  
  drawMap(array);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  drawMap(array);

  HTTPClient http;

  http.begin("https://api.particle.io/v1/devices/thinky/speed?access_token=78a99eb4943d042f674bedd4ab8095af43702e39");

  int httpResponseCode = http.GET();
  
  String result = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    result = http.getString();
  }

  http.end();

  JSONVar json = JSON.parse(result);
    
  Serial.print("1 = ");
  Serial.println(json["result"]);

  Serial.println(http.GET());
  Serial.println(json);
  http.end();
  delay(500);
  
}


