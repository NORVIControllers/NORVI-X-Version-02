#include <Norvi.h>

void setup() {
  Serial.begin(115200);
  Norvi.publish("temperature", 23.5f);
  Norvi.publish("running", true);
  Norvi.publish("device_name", String("bench"));
  Norvi.publish("{\"pressure\":1012,\"alarm\":false}");
}

void loop() {}
