//"NPD-NORVI X-R8-V2 (N16R2)"

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <PCA9538.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h"

// I2C address of PCA9538
#define PCA9538_ADDR 0x72 

#define SDA   8     
#define SCL   9     
#define MISO 13
#define MOSI 11
#define SCLK 12
#define DSP_CS 45
#define PCA_RESET 21

// Relay Pin Definitions for PCA9538
const int relayPins[] = {1, 2, 3, 4, 5, 6, 7, 8};

PCA9538 io(PCA9538_ADDR);
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  
  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH); 
  
  Wire.begin(SDA, SCL);
  
  // Initialize PCA9538 Pins
  for(int i=0; i<8; i++) {
    io.pinMode(relayPins[i], OUTPUT);
    io.digitalWrite(relayPins[i], LOW);
  }

  SPI.begin(SCLK, MISO, MOSI);
  tft.init();
  tft.setRotation(0);
  drawHeader();
}

void loop() {
  // 1. All On / All Off
  displayPatternName("ALL ON / OFF");
  setAllRelays(HIGH);
  delay(2000);
  setAllRelays(LOW);
  delay(1000);

  // 2. Sequential Test (The original pattern)
  displayPatternName("SEQUENTIAL");
  for(int i=0; i<8; i++) {
    updateRelay(i, HIGH);
    delay(300);
    updateRelay(i, LOW);
    delay(100);
  }

  // 3. Odds and Evens
  displayPatternName("ODDS vs EVENS");
  for(int repeat=0; repeat<4; repeat++) {
    toggleOddsEvens(true);  // Odds On
    delay(500);
    toggleOddsEvens(false); // Evens On
    delay(500);
  }
  setAllRelays(LOW);

  // 4. Chaser (Knight Rider style)
  displayPatternName("CHASER");
  for(int i=0; i<8; i++) { 
    updateRelay(i, HIGH); 
    if(i>0) updateRelay(i-1, LOW);
    delay(150); 
  }
  for(int i=7; i>=0; i--) { 
    updateRelay(i, HIGH); 
    if(i<7) updateRelay(i+1, LOW);
    delay(150); 
  }
  setAllRelays(LOW);

  // 5. Binary Counter (Fun visual)
  displayPatternName("BINARY COUNTER");
  for(int count=0; count <= 15; count++) { // Counting to 15 for brevity
    displayBinary(count);
    delay(400);
  }
  setAllRelays(LOW);
}

// --- Helper Functions ---

void drawHeader() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  tft.setCursor(60, 25);
  tft.print("NORVI");
  tft.setCursor(40, 50);
  tft.print("X-R8 TEST");
  tft.drawFastHLine(0, 60, 240, TFT_WHITE);
}

void displayPatternName(String name) {
  tft.fillRect(0, 65, 240, 30, TFT_BLACK); // Clear sub-header area
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);
  tft.setCursor(10, 85);
  tft.print("MODE: " + name);
  Serial.println("Pattern: " + name);
}

// Controls a single relay and updates the TFT
void updateRelay(int index, bool state) {
  io.digitalWrite(relayPins[index], state);
  
  int yPos = 110 + (index * 25);
  tft.setFreeFont(FSB9);
  tft.setCursor(10, yPos);
  
  // Clear the line
  tft.fillRect(0, yPos-15, 240, 20, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE);
  tft.printf("RELAY %d : ", index + 1);
  
  if(state) {
    tft.setTextColor(TFT_RED);
    tft.print("ON");
  } else {
    tft.setTextColor(TFT_BLUE);
    tft.print("OFF");
  }
}

void setAllRelays(bool state) {
  for(int i=0; i<8; i++) {
    io.digitalWrite(relayPins[i], state);
  }
  
  // Update UI
  tft.fillRect(0, 100, 240, 220, TFT_BLACK);
  tft.setCursor(60, 200);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FSB12);
  tft.print(state ? "ALL ON" : "ALL OFF");
}

void toggleOddsEvens(bool odds) {
  for(int i=0; i<8; i++) {
    bool state = (odds) ? (i % 2 == 0) : (i % 2 != 0);
    io.digitalWrite(relayPins[i], state);
  }
}

void displayBinary(int num) {
  for (int i = 0; i < 8; i++) {
    bool bit = bitRead(num, i);
    io.digitalWrite(relayPins[i], bit);
  }
  tft.fillRect(0, 100, 240, 220, TFT_BLACK);
  tft.setCursor(80, 200);
  tft.setFreeFont(FSB12);
  tft.setTextColor(TFT_CYAN);
  tft.print("VAL: ");
  tft.print(num);
}
