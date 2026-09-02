/*
   NPD-NORVI X-R4-N16R2
   2026.09.01
*/


#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <PCA9538.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h"

// I2C
#define SDA 8
#define SCL 9

// SPI
#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

// PCA RESET
#define PCA_RESET 21

// PCA9538 ADDRESSES
#define PCA9538_ADDR1 0x70
#define PCA9538_ADDR2 0x71
#define PCA9538_ADDR3 0x72
#define PCA9538_ADDR4 0x73

// RELAY PINS
const int relayPins[] = {1, 2, 3, 4};

// PCA9538 OBJECTS
PCA9538 io1(PCA9538_ADDR1);
PCA9538 io2(PCA9538_ADDR2);
PCA9538 io3(PCA9538_ADDR3);
PCA9538 io4(PCA9538_ADDR4);


CST816S touch(8, 9, 47, 48);


TFT_eSPI tft = TFT_eSPI();



// DEVICE DETECTION
bool devicePresent[4] = {
  false,
  false,
  false,
  false
};


// DETECTED ADDRESS LIST

byte detectedAddress[4];

int deviceCount = 0;


// CURRENT PAGE
int page = 0;


// RELAY STATE
bool relayState[4][4] = {{false, false, false, false}, {false, false, false, false},
  {false, false, false, false}, {false, false, false, false}
};

// TOUCH
#define TOUCH_ADDR 0x15

unsigned long lastTouchTime = 0;


void setup() {

  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("--------------------------------");
  Serial.println("NPD-NORVI X-R4 TEST");
  Serial.println("--------------------------------");

  pinMode(PCA_RESET, OUTPUT);
  delay(100);

  digitalWrite(PCA_RESET, HIGH );
  delay(100);

  // I2C
  Wire.begin(SDA, SCL );
  delay(100);

  // SCAN DEVICES
  I2C_SCAN();
  delay(100);


  // INITIALIZE 0x70
  if (devicePresent[0]) {
    initPCA(io1, PCA9538_ADDR1);
  }

  // INITIALIZE 0x71
  if (devicePresent[1]) {
    initPCA(io2, PCA9538_ADDR2);
  }



  // INITIALIZE 0x72
  if (devicePresent[2]) {
    initPCA(io3,PCA9538_ADDR3);
  }

  // INITIALIZE 0x73
  if (devicePresent[3]) {
    initPCA(io4, PCA9538_ADDR4);
  }

  SPI.begin(SCLK, MISO, MOSI);
  delay(300);

  // TFT
  tft.init();
  tft.begin();
  tft.setRotation(0);
  
  LOGO_PRINT();
  delay(100);

  // FIRST PAGE
  if (deviceCount > 0) {page = 0;displayPage();}
  else {
    drawNoDevice();
  }


  Serial.println();
  Serial.print("TOTAL PCA9538 = ");
  Serial.println(deviceCount);
}


void loop() {

  if (deviceCount > 0) { touchCheck(); }
  delay(10);
}



// I2C SCAN
void I2C_SCAN() {

  byte error;
  byte address;
  deviceCount = 0;

  // Clear detection list

  for (int i = 0; i < 4; i++) {
    devicePresent[i] = false;
  }

  Serial.println();
  Serial.println("I2C SCANNING...");


  // SCAN ALL I2C ADDRESSES

  for (address = 1; address < 127;address++) 
  {
    Wire.beginTransmission(address);
    error =
      Wire.endTransmission();
    if (error == 0) {

      Serial.print(
        "I2C DEVICE : 0x"
      );


      if (address < 0x10) {

        Serial.print(
          "0"
        );

      }


      Serial.println(
        address,
        HEX
      );


      // ======================================================
      // 0x70
      // ======================================================

      if (address == 0x70) {

        devicePresent[0] = true;

        detectedAddress[deviceCount] =
          0x70;

        deviceCount++;

      }


      // ======================================================
      // 0x71
      // ======================================================

      else if (address == 0x71) {

        devicePresent[1] = true;

        detectedAddress[deviceCount] =
          0x71;

        deviceCount++;

      }


      // ======================================================
      // 0x72
      // ======================================================

      else if (address == 0x72) {

        devicePresent[2] = true;

        detectedAddress[deviceCount] =
          0x72;

        deviceCount++;

      }


      // ======================================================
      // 0x73
      // ======================================================

      else if (address == 0x73) {

        devicePresent[3] = true;

        detectedAddress[deviceCount] =
          0x73;

        deviceCount++;

      }

    }

  }


  // ==========================================================
  // RESULT
  // ==========================================================

  Serial.println();

  Serial.print(
    "PCA9538 FOUND : "
  );

  Serial.println(
    deviceCount
  );


  for (
    int i = 0;
    i < deviceCount;
    i++
  ) {

    Serial.print(
      "PAGE "
    );

    Serial.print(
      i + 1
    );

    Serial.print(
      " = 0x"
    );

    Serial.println(
      detectedAddress[i],
      HEX
    );

  }

}


// ============================================================
// INITIALIZE PCA9538
// ============================================================

void initPCA(
  PCA9538 &io,
  byte address
) {

  Serial.print(
    "INITIALIZE PCA : 0x"
  );

  Serial.println(
    address,
    HEX
  );


  for (
    int i = 0;
    i < 4;
    i++
  ) {

    io.pinMode(
      relayPins[i],
      OUTPUT
    );


    io.digitalWrite(
      relayPins[i],
      LOW
    );

  }

}


// ============================================================
// GET CURRENT PCA
// ============================================================

PCA9538* getCurrentPCA() {

  if (
    detectedAddress[page] == 0x70
  ) {

    return &io1;

  }


  if (
    detectedAddress[page] == 0x71
  ) {

    return &io2;

  }


  if (
    detectedAddress[page] == 0x72
  ) {

    return &io3;

  }


  if (
    detectedAddress[page] == 0x73
  ) {

    return &io4;

  }


  return NULL;

}


// ============================================================
// DISPLAY PAGE
// ============================================================

void displayPage() {

  // Clear complete screen

  tft.fillScreen(
    TFT_BLACK
  );


  // ==========================================================
  // TOP TITLE
  // ==========================================================

  tft.setFreeFont(
    FSB12
  );

  tft.setTextColor(
    TFT_WHITE,
    TFT_BLACK
  );


  tft.setCursor(
    55,
    20
  );

  tft.print(
    "NORVI-X-R4"
  );

  tft.setFreeFont(
    FSB9
  );
  tft.setTextColor(
    TFT_WHITE,
    TFT_BLACK
  );
  // ==========================================================
  // CONTROLLER NUMBER
  // ==========================================================

  tft.setCursor(
    55,
    42
  );

  tft.print(
    "CONTROLLER "
  );

  tft.print(
    page + 1
  );


  // ==========================================================
  // TOP LINE
  // ==========================================================

  tft.drawFastHLine(
    0,
    55,
    240,
    TFT_WHITE
  );


  // ==========================================================
  // RELAY 1
  // ==========================================================

  drawRelayButton(
    0,
    65
  );


  // ==========================================================
  // RELAY 2
  // ==========================================================

  drawRelayButton(
    1,
    105
  );


  // ==========================================================
  // RELAY 3
  // ==========================================================

  drawRelayButton(
    2,
    145
  );


  // ==========================================================
  // RELAY 4
  // ==========================================================

  drawRelayButton(
    3,
    185
  );


  // ==========================================================
  // PREVIOUS BUTTON
  // ==========================================================

  tft.drawRect(
    5,
    220,
    105,
    30,
    TFT_WHITE
  );


  tft.setFreeFont(
    FSB9
  );

  tft.setTextColor(
    TFT_WHITE,
    TFT_BLACK
  );

  tft.setCursor(
    20,
    241
  );

  tft.print(
    "< PREV"
  );


  // ==========================================================
  // NEXT BUTTON
  // ==========================================================

  tft.drawRect(
    130,
    220,
    105,
    30,
    TFT_WHITE
  );


  tft.setCursor(
    150,
    241
  );

  tft.print(
    "NEXT >"
  );


  // ==========================================================
  // BOTTOM LINE
  // ==========================================================

  tft.drawFastHLine(
    0,
    260,
    240,
    TFT_WHITE
  );


  // ==========================================================
  // ADDRESS
  // ==========================================================

  tft.setTextColor(
    TFT_CYAN,
    TFT_BLACK
  );

  tft.setCursor(
    15,
    285
  );

  tft.print(
    "ADDRESS : 0x"
  );


  if (
    detectedAddress[page] < 0x10
  ) {

    tft.print(
      "0"
    );

  }


  tft.print(
    detectedAddress[page],
    HEX
  );


  // ==========================================================
  // PAGE NUMBER
  // ==========================================================

  tft.setCursor(
    175,
    285
  );

  tft.print(
    page + 1
  );

  tft.print(
    "/"
  );

  tft.print(
    deviceCount
  );


  // ==========================================================
  // SERIAL
  // ==========================================================

  Serial.print(
    "PAGE "
  );

  Serial.print(
    page + 1
  );

  Serial.print(
    "/"
  );

  Serial.print(
    deviceCount
  );

  Serial.print(
    "  ADDRESS : 0x"
  );

  Serial.println(
    detectedAddress[page],
    HEX
  );

}


// ============================================================
// DRAW RELAY BUTTON
// OFF = LEFT
// ON  = RIGHT
// ============================================================

void drawRelayButton(
  int relay,
  int y
) {

  bool state =
    relayState[page][relay];


  // ==========================================================
  // RELAY TEXT
  // ==========================================================

  tft.setFreeFont(
    FSB9
  );

  tft.setTextColor(
    TFT_WHITE,
    TFT_BLACK
  );


  tft.setCursor(
    10,
    y + 21
  );

  tft.print(
    "RELAY "
  );

  tft.print(
    relay + 1
  );


  // ==========================================================
  // OFF BUTTON - LEFT
  // ==========================================================

  if (!state) {

    // Active OFF

    tft.fillRect(
      105,
      y,
      55,
      30,
      TFT_RED
    );

    tft.drawRect(
      105,
      y,
      55,
      30,
      TFT_RED
    );

    tft.setTextColor(
      TFT_BLACK
    );

  }
  else {

    // Inactive OFF

    tft.drawRect(
      105,
      y,
      55,
      30,
      TFT_RED
    );

    tft.setTextColor(
      TFT_RED
    );

  }


  tft.setCursor(
    116,
    y + 21
  );

  tft.print(
    "OFF"
  );


  // ==========================================================
  // ON BUTTON - RIGHT
  // ==========================================================

  if (state) {

    // Active ON

    tft.fillRect(
      170,
      y,
      55,
      30,
      TFT_GREEN
    );

    tft.drawRect(
      170,
      y,
      55,
      30,
      TFT_GREEN
    );

    tft.setTextColor(
      TFT_BLACK
    );

  }
  else {

    // Inactive ON

    tft.drawRect(
      170,
      y,
      55,
      30,
      TFT_GREEN
    );

    tft.setTextColor(
      TFT_GREEN
    );

  }


  tft.setCursor(
    183,
    y + 21
  );

  tft.print(
    "ON"
  );

}


// ============================================================
// RELAY ON
// ============================================================

void relayON(
  int relay
) {

  PCA9538 *io =
    getCurrentPCA();


  if (
    io == NULL
  ) {

    return;

  }


  // ==========================================================
  // STATE
  // ==========================================================

  relayState[page][relay] =
    true;


  // ==========================================================
  // PCA OUTPUT
  // ==========================================================

  io->digitalWrite(
    relayPins[relay],
    HIGH
  );


  // ==========================================================
  // SERIAL
  // ==========================================================

  Serial.print(
    "ADDRESS : 0x"
  );

  Serial.print(
    detectedAddress[page],
    HEX
  );

  Serial.print(
    "  RELAY "
  );

  Serial.print(
    relay + 1
  );

  Serial.println(
    " = ON"
  );


  // ==========================================================
  // UPDATE DISPLAY
  // ==========================================================

  displayPage();

}


// ============================================================
// RELAY OFF
// ============================================================

void relayOFF(
  int relay
) {

  PCA9538 *io =
    getCurrentPCA();


  if (
    io == NULL
  ) {

    return;

  }


  // ==========================================================
  // STATE
  // ==========================================================

  relayState[page][relay] =
    false;


  // ==========================================================
  // PCA OUTPUT
  // ==========================================================

  io->digitalWrite(
    relayPins[relay],
    LOW
  );


  // ==========================================================
  // SERIAL
  // ==========================================================

  Serial.print(
    "ADDRESS : 0x"
  );

  Serial.print(
    detectedAddress[page],
    HEX
  );

  Serial.print(
    "  RELAY "
  );

  Serial.print(
    relay + 1
  );

  Serial.println(
    " = OFF"
  );


  // ==========================================================
  // UPDATE DISPLAY
  // ==========================================================

  displayPage();

}


// ============================================================
// TOUCH CHECK
// ============================================================

void touchCheck() {

  uint16_t x;
  uint16_t y;


  // ==========================================================
  // TOUCH DELAY
  // ==========================================================

  if (
    millis() - lastTouchTime < 400
  ) {

    return;

  }


  // ==========================================================
  // READ TOUCH
  // ==========================================================

  if (
    readTouch(
      x,
      y
    ) == false
  ) {

    return;

  }


  lastTouchTime =
    millis();


  Serial.print(
    "TOUCH X : "
  );

  Serial.print(
    x
  );

  Serial.print(
    "  Y : "
  );

  Serial.println(
    y
  );


  // ==========================================================
  // RELAY 1
  // OFF LEFT / ON RIGHT
  // ==========================================================

  if (
    y >= 65 &&
    y < 95
  ) {

    // OFF

    if (
      x >= 105 &&
      x < 160
    ) {

      relayOFF(
        0
      );

      return;

    }


    // ON

    if (
      x >= 170 &&
      x < 225
    ) {

      relayON(
        0
      );

      return;

    }

  }


  // ==========================================================
  // RELAY 2
  // ==========================================================

  if (
    y >= 105 &&
    y < 135
  ) {

    // OFF

    if (
      x >= 105 &&
      x < 160
    ) {

      relayOFF(
        1
      );

      return;

    }


    // ON

    if (
      x >= 170 &&
      x < 225
    ) {

      relayON(
        1
      );

      return;

    }

  }


  // ==========================================================
  // RELAY 3
  // ==========================================================

  if (
    y >= 145 &&
    y < 175
  ) {

    // OFF

    if (
      x >= 105 &&
      x < 160
    ) {

      relayOFF(
        2
      );

      return;

    }


    // ON

    if (
      x >= 170 &&
      x < 225
    ) {

      relayON(
        2
      );

      return;

    }

  }


  // ==========================================================
  // RELAY 4
  // ==========================================================

  if (
    y >= 185 &&
    y < 215
  ) {

    // OFF

    if (
      x >= 105 &&
      x < 160
    ) {

      relayOFF(
        3
      );

      return;

    }


    // ON

    if (
      x >= 170 &&
      x < 225
    ) {

      relayON(
        3
      );

      return;

    }

  }


  // ==========================================================
  // PREVIOUS PAGE
  // ==========================================================

  if (
    y >= 220 &&
    y < 250 &&
    x < 120
  ) {

    page--;


    if (
      page < 0
    ) {

      page =
        deviceCount - 1;

    }


    displayPage();

    return;

  }


  // ==========================================================
  // NEXT PAGE
  // ==========================================================

  if (
    y >= 220 &&
    y < 250 &&
    x >= 120
  ) {

    page++;


    if (
      page >= deviceCount
    ) {

      page = 0;

    }


    displayPage();

    return;

  }

}


// ============================================================
// CST816S TOUCH READ
// ============================================================

bool readTouch(
  uint16_t &x,
  uint16_t &y
) {

  Wire.beginTransmission(
    TOUCH_ADDR
  );

  Wire.write(
    0x01
  );


  if (
    Wire.endTransmission(false) != 0
  ) {

    return false;

  }


  uint8_t count =
    Wire.requestFrom(
      TOUCH_ADDR,
      (uint8_t)6
    );


  if (
    count != 6
  ) {

    return false;

  }


  uint8_t data[6];


  for (
    int i = 0;
    i < 6;
    i++
  ) {

    data[i] =
      Wire.read();

  }


  uint8_t points =
    data[1] & 0x0F;


  if (
    points == 0
  ) {

    return false;

  }


  x =
    ((uint16_t)(data[2] & 0x0F) << 8)
    | data[3];


  y =
    ((uint16_t)(data[4] & 0x0F) << 8)
    | data[5];


  return true;

}


// ============================================================
// NO DEVICE SCREEN
// ============================================================

void drawNoDevice() {

  tft.fillScreen(
    TFT_BLACK
  );


  // ==========================================================
  // TITLE
  // ==========================================================

  tft.setFreeFont(
    FSB9
  );

  tft.setTextColor(
    TFT_GREEN,
    TFT_BLACK
  );


  tft.setCursor(
    55,
    30
  );

  tft.print(
    "NORVI-X-R4"
  );


  tft.setTextColor(
    TFT_CYAN,
    TFT_BLACK
  );
  tft.setCursor(
    15,
    296
  );

  tft.print(
    "CONTROLLER"
  );


  // ==========================================================
  // LINE
  // ==========================================================

  tft.drawFastHLine(
    0,
    55,
    240,
    TFT_WHITE
  );


  // ==========================================================
  // NO DEVICE
  // ==========================================================

  tft.setFreeFont(
    FSB12
  );

  tft.setTextColor(
    TFT_RED,
    TFT_BLACK
  );


  tft.setCursor(
    55,
    110
  );

  tft.print(
    "NO DEVICE"
  );



  // CHECK ADDRESS

  tft.setFreeFont(FSB9 );

  tft.setCursor(45, 150);

  tft.print(
    "CHECK 0x70-0x73"
  );

}



void LOGO_PRINT() {
  tft.setSwapBytes(true);

  // BLACK BACKGROUND
  tft.fillScreen(TFT_WHITE);

  // DISPLAY IMAGE
  tft.pushImage(
    0,
    0,
    IMAGE_WIDTH,
    IMAGE_HEIGHT,
    image_data
  );
delay(2000);
  tft.fillScreen(TFT_BLACK);
}
