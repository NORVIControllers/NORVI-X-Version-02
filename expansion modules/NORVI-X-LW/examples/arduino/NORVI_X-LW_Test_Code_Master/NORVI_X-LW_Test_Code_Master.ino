#include <ArduinoJson.h>

#define LoRaSerial Serial1

// ESP32-S3 UART pins
#define LORA_RX_PIN 14
#define LORA_TX_PIN 40

void setup()
{
  // UART connection to RAK4631
  LoRaSerial.begin(115200, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);

  // USB/debug serial
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("ESP32-S3 UART JSON Sender");
  Serial.println("-------------------------");

  randomSeed(micros());
}

void loop()
{
  // Generate random values
  int Anlaog1 = random(0, 101);
  int Anlaog2 = random(0, 101);

  // Create JSON
  JsonDocument jsonDocument;

  jsonDocument["Anlaog1"] = Anlaog1;
  jsonDocument["Anlaog2"] = Anlaog2;

  // Convert JSON to String
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Send JSON to RAK4631
  LoRaSerial.println(jsonString);

  // Show what was transmitted
  Serial.print("TX -> ");
  Serial.println(jsonString);

  // Check for response from RAK4631
  while (LoRaSerial.available())
  {
    String receivedString = LoRaSerial.readStringUntil('\n');

    Serial.print("RX <- ");
    Serial.println(receivedString);
  }

  delay(30000);
}
