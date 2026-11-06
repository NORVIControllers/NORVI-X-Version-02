#include <Norvi.h>

void setup() {
  Serial.begin(115200);
  Norvi.publish(String("{\"temperature\":23.5,\"running\":true}"));
}

void loop() {}
