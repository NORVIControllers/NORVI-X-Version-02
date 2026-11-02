#pragma once

#include <Arduino.h>

class NorviClass {
public:
  bool publish(const char* name, int value);
  bool publish(const char* name, float value);
  bool publish(const char* name, bool value);
  bool publish(const char* name, const String& value);
  bool publish(const String& jsonObject);

private:
  static constexpr float RATE_PER_SECOND = 10.0f;
  static constexpr float TOKEN_CAPACITY = 10.0f;
  static constexpr size_t MAX_LINE_BYTES = 1024;
  float tokens_ = TOKEN_CAPACITY;
  unsigned long lastRefillMs_ = 0;
  unsigned long dropped_ = 0;
  bool clockInitialized_ = false;
  bool allowPublish();
  bool publishLine(const String& line);
  bool publishNamed(const char* name, const String& serializedValue);
  bool publishJsonObject(const String& jsonObject);
  bool validateJsonObject(const String& jsonObject) const;
  bool validateFieldName(const char* name) const;
  String escapeJsonString(const char* value) const;
  String appendDropped(const String& jsonObject, unsigned long dropped) const;
};

extern NorviClass Norvi;
