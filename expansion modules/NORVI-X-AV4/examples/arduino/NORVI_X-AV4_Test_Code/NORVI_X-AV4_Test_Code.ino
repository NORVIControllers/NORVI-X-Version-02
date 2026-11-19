/*
 * 2026.06.09
 * NORVI-AT01-BM6-LTE
 * 3 - DIGITAL INPUT
 * 3 - 0-10 ANALOG INPUT
 * RS485
 * GSM
 * RGB
 * PUSH BUTTON-1
 */

 
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads1;

#define VOLTAGE_DIVIDER_RATIO 0.2065

#define BUTTON_PIN   35    

#define LED_PIN 25     
#define NUM_LEDS 1    

#define RXD 23 // 485 DEFINE
#define TXD 13
#define FC  32

#define GSM_RX 4
#define GSM_TX 5
#define GSM_RESET 19

Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
bool lastState = LOW;
int colorIndex = 0;

unsigned long int timer1 = 0;

void setup() {
 
  Serial.begin(115200);

  Wire.begin(21,22); //I2C
 
  Serial1.begin(9600, SERIAL_8N1,RXD,TXD);
  Serial2.begin(115200, SERIAL_8N1, GSM_RX, GSM_TX); 
  
  pinMode(BUTTON_PIN, INPUT);

  pinMode(FC, OUTPUT); 
  digitalWrite(FC, HIGH);
  
  //GSM ON
  pinMode(GSM_RESET, OUTPUT); 
  digitalWrite(GSM_RESET, HIGH);
  delay(500);

  pinMode(18, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);

  if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS1.");
    while (1);
  }

  
  pixel.begin();
  pixel.clear();
  pixel.show();
  
  delay(300);
  WiFi.mode(WIFI_MODE_APSTA);
  delay(300);
  wifi_test();
  delay(100);
  GSM_TEST();
  delay(500);
  TEST_LED();
  delay(500);

 digitalWrite(FC, HIGH);   // RS-485 
}


void loop() {
  

  Serial.println("");  

 while (Serial.available()) {
    int inByte = Serial.read();
    Serial2.write(inByte);
  }

  while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
  }
  
  Serial.print("BUTTON: ");Serial.println(digitalRead(BUTTON_PIN));

  Serial.print("I1: ");Serial.println(digitalRead(18));
  Serial.print("I2: ");Serial.println(digitalRead(26));
  Serial.print("I3: ");Serial.println(digitalRead(27));
  delay(200);
  
   for (int i = 0; i < 3; i++) {
    int16_t adc = ads1.readADC_SingleEnded(i);
    float voltage = adc * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
    Serial.printf("V %d: %.2f V\n", i, voltage);
  }
  
  delay(100);
  RGB_TEST();
  delay(100);
  
  digitalWrite(FC, HIGH);                   
  delay(100);
  Serial1.println(F("RS485 01 SUCCESS"));    
  delay(100);                               
  digitalWrite(FC, LOW) ;                 

                                             
  delay(100);     
  
  while (Serial1.available()) { 
    char c = Serial1.read();     
    Serial.write(c);             
  }
 delay(100); 
 
 Serial.println("-------------------");  
}


void wifi_test(){
    Serial.println("");
String str_macAddress;
byte mac[6];

WiFi.macAddress(mac);

   str_macAddress = (String(mac[0] >> 4,  HEX) + String(mac[0] & 0x0F, HEX)) + (":") +
                   (String(mac[1] >> 4,  HEX) + String(mac[1] & 0x0F, HEX)) + (":") +
                   (String(mac[2] >> 4,  HEX) + String(mac[2] & 0x0F, HEX)) + (":") +
                   (String(mac[3] >> 4,  HEX) + String(mac[3] & 0x0F, HEX)) + (":") +
                   (String(mac[4] >> 4,  HEX) + String(mac[4] & 0x0F, HEX)) + (":") +
                   (String(mac[5] >> 4,  HEX) + String(mac[5] & 0x0F, HEX));
  str_macAddress.toUpperCase();
  
  Serial.println("MAC Address: " + str_macAddress);

  String ssid =  str_macAddress; 
  const char* password = "12345678";   

  Serial.println("Setting up the Wi-Fi Access Point...");
  WiFi.softAP(ssid.c_str(), password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP address: ");
  Serial.println(IP);

  Serial.print("Access Point SSID: ");
  Serial.println(ssid);

  Serial.println("Wi-Fi Access Point is active!");
  Serial.println("");
}


void RGB_TEST() {

  bool currentState = digitalRead(BUTTON_PIN);

  // Rising edge: LOW -> HIGH
  if (currentState == HIGH && lastState == LOW) {

    colorIndex++;

    if (colorIndex > 2) colorIndex = 0;

    switch (colorIndex) {
      case 0:
        pixel.setPixelColor(0, pixel.Color(255, 0, 0));   // Red
        break;

      case 1:
        pixel.setPixelColor(0, pixel.Color(0, 255, 0));   // Green
        break;

      case 2:
        pixel.setPixelColor(0, pixel.Color(0, 0, 255));   // Blue
        break;
    }

    pixel.show();
  }

  // Input LOW = OFF
  if (currentState == LOW) {
    pixel.clear();
    pixel.show();
  }

  lastState = currentState;
}


void GSM_TEST() {
  
 Serial.println("GSM TEST ");
  delay(1000);  

  timer1 = millis();
  Serial2.println("AT");
  while(millis()<timer1+2000){
    while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    }
  }
  timer1 = millis();
  Serial2.println("AT+CPIN?");
  while(millis()<timer1+1000){
    while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    }
  }

   timer1 = millis();
  Serial2.println("AT+CSQ");
  while(millis()<timer1+1000){
    while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    }
  }
  timer1 = millis();
  Serial2.println("AT+GSN");
  while(millis()<timer1+1000){
    while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    }
  }
  timer1 = millis();
  Serial2.println("ATD776836382;");
  while(millis()<timer1+1000){
    while (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.write(inByte);
    }
  }

 Serial.println("GSM TEST DONE");
 }

 void TEST_LED() {
  // Cycle through colors
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(255, 255, 0)); // Yellow
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(255, 0, 255)); // Magenta
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(255, 255, 255)); // White
  strip.show();
  delay(100); // Wait for 1 second

  strip.setPixelColor(0, strip.Color(0, 0, 0)); // Off
  strip.show();
  delay(100); // Wait for 1 second
}
