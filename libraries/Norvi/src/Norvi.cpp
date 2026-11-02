#include "Norvi.h"

namespace {
bool isWhitespace(char value) { return value == ' ' || value == '\t' || value == '\r' || value == '\n'; }
void skipWhitespace(const char*& cursor) { while (*cursor && isWhitespace(*cursor)) ++cursor; }

bool consumeJsonString(const char*& cursor) {
  if (*cursor++ != '"') return false;
  while (*cursor) {
    const char value = *cursor++;
    if (value == '"') return true;
    if (value == '\\') {
      if (!*cursor) return false;
      const char escaped = *cursor++;
      if (escaped == 'u') {
        for (int index = 0; index < 4; ++index) {
          const char hex = *cursor++;
          if (!((hex >= '0' && hex <= '9') || (hex >= 'a' && hex <= 'f') || (hex >= 'A' && hex <= 'F'))) return false;
        }
      } else if (!(escaped == '"' || escaped == '\\' || escaped == '/' || escaped == 'b' || escaped == 'f' || escaped == 'n' || escaped == 'r' || escaped == 't')) return false;
    } else if (static_cast<unsigned char>(value) < 0x20) return false;
  }
  return false;
}

bool consumeJsonValue(const char*& cursor);
bool consumeJsonObject(const char*& cursor) {
  if (*cursor++ != '{') return false;
  skipWhitespace(cursor);
  if (*cursor == '}') { ++cursor; return true; }
  while (*cursor) {
    if (!consumeJsonString(cursor)) return false;
    skipWhitespace(cursor);
    if (*cursor++ != ':') return false;
    skipWhitespace(cursor);
    if (!consumeJsonValue(cursor)) return false;
    skipWhitespace(cursor);
    if (*cursor == '}') { ++cursor; return true; }
    if (*cursor++ != ',') return false;
    skipWhitespace(cursor);
  }
  return false;
}
bool consumeJsonArray(const char*& cursor) {
  if (*cursor++ != '[') return false;
  skipWhitespace(cursor);
  if (*cursor == ']') { ++cursor; return true; }
  while (*cursor) {
    if (!consumeJsonValue(cursor)) return false;
    skipWhitespace(cursor);
    if (*cursor == ']') { ++cursor; return true; }
    if (*cursor++ != ',') return false;
    skipWhitespace(cursor);
  }
  return false;
}
bool consumeJsonPrimitive(const char*& cursor) {
  const char* start = cursor;
  while (*cursor && !isWhitespace(*cursor) && *cursor != ',' && *cursor != ']' && *cursor != '}') ++cursor;
  if (cursor == start) return false;
  String value;
  while (start != cursor) value += *start++;
  if (value == "true" || value == "false" || value == "null") return true;
  value.toFloat();
  return value.length() > 0 && (value[0] == '-' || (value[0] >= '0' && value[0] <= '9'));
}
bool consumeJsonValue(const char*& cursor) {
  if (*cursor == '"') return consumeJsonString(cursor);
  if (*cursor == '{') return consumeJsonObject(cursor);
  if (*cursor == '[') return consumeJsonArray(cursor);
  return consumeJsonPrimitive(cursor);
}
} // namespace

NorviClass Norvi;

bool NorviClass::validateFieldName(const char* name) const {
  if (!name || !name[0]) return false;
  size_t length = 0;
  while (name[length]) {
    if (++length > 64) return false;
    const char value = name[length - 1];
    const bool letter = (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
    const bool digit = value >= '0' && value <= '9';
    if ((length == 1 && !letter) || (length > 1 && !letter && !digit && value != '_')) return false;
  }
  String normalized(name);
  normalized.toLowerCase();
  const char* reserved[] = {"ni", "fw", "rst", "cmd", "log", "ota"};
  for (const char* prefix : reserved) if (normalized.startsWith(prefix)) return false;
  return true;
}

String NorviClass::escapeJsonString(const char* value) const {
  String escaped = "\"";
  for (size_t index = 0; value && value[index]; ++index) {
    const char current = value[index];
    if (current == '"' || current == '\\') { escaped += '\\'; escaped += current; }
    else if (current == '\n') escaped += "\\n";
    else if (current == '\r') escaped += "\\r";
    else if (current == '\t') escaped += "\\t";
    else if (static_cast<unsigned char>(current) < 0x20) return String();
    else escaped += current;
  }
  escaped += '"';
  return escaped;
}

bool NorviClass::allowPublish() {
  const unsigned long now = millis();
  if (!clockInitialized_) { clockInitialized_ = true; lastRefillMs_ = now; tokens_ = TOKEN_CAPACITY; }
  else {
    const unsigned long elapsed = now - lastRefillMs_;
    tokens_ = min(TOKEN_CAPACITY, tokens_ + (static_cast<float>(elapsed) * RATE_PER_SECOND / 1000.0f));
    lastRefillMs_ = now;
  }
  if (tokens_ < 1.0f) { ++dropped_; return false; }
  tokens_ -= 1.0f;
  return true;
}

String NorviClass::appendDropped(const String& jsonObject, unsigned long dropped) const {
  String normalized = jsonObject;
  normalized.trim();
  const int closing = normalized.lastIndexOf('}');
  if (closing < 0) return String();
  String result = normalized.substring(0, closing);
  if (result.length() > 1) result += ',';
  result += "\"_dropped\":";
  result += String(dropped);
  result += '}';
  return result;
}

bool NorviClass::publishLine(const String& line) {
  if (line.length() > MAX_LINE_BYTES) return false;
  Serial.print(line);
  return true;
}

bool NorviClass::publishNamed(const char* name, const String& serializedValue) {
  if (!validateFieldName(name)) return false;
  String line;
  if (dropped_ > 0) {
    String json = "{";
    json += escapeJsonString(name);
    json += ':';
    json += serializedValue;
    json += '}';
    line = "NI:JSON=" + appendDropped(json, dropped_) + '\n';
  } else line = "NI:" + String(name) + '=' + serializedValue + '\n';
  if (line.length() > MAX_LINE_BYTES || !allowPublish()) return false;
  if (!publishLine(line)) return false;
  dropped_ = 0;
  return true;
}

bool NorviClass::validateJsonObject(const String& jsonObject) const {
  const char* cursor = jsonObject.c_str();
  skipWhitespace(cursor);
  if (*cursor++ != '{') return false;
  skipWhitespace(cursor);
  if (*cursor == '}') { ++cursor; skipWhitespace(cursor); return *cursor == '\0'; }
  while (*cursor) {
    const char* keyStart = cursor;
    if (!consumeJsonString(cursor)) return false;
    String key;
    const char* keyCursor = keyStart + 1;
    while (*keyCursor && *keyCursor != '"') { if (*keyCursor == '\\') ++keyCursor; key += *keyCursor++; }
    if (!validateFieldName(key.c_str())) return false;
    skipWhitespace(cursor);
    if (*cursor++ != ':') return false;
    skipWhitespace(cursor);
    if (!consumeJsonValue(cursor)) return false;
    skipWhitespace(cursor);
    if (*cursor == '}') { ++cursor; skipWhitespace(cursor); return *cursor == '\0'; }
    if (*cursor++ != ',') return false;
    skipWhitespace(cursor);
  }
  return false;
}

bool NorviClass::publishJsonObject(const String& jsonObject) {
  String normalized = jsonObject;
  normalized.trim();
  if (!validateJsonObject(normalized)) return false;
  String payload = dropped_ > 0 ? appendDropped(normalized, dropped_) : normalized;
  if (payload.length() == 0) return false;
  String line = "NI:JSON=" + payload + '\n';
  if (line.length() > MAX_LINE_BYTES || !allowPublish()) return false;
  if (!publishLine(line)) return false;
  dropped_ = 0;
  return true;
}

bool NorviClass::publish(const char* name, int value) { return publishNamed(name, String(value)); }
bool NorviClass::publish(const char* name, float value) { return publishNamed(name, String(value, 6)); }
bool NorviClass::publish(const char* name, bool value) { return publishNamed(name, value ? "true" : "false"); }
bool NorviClass::publish(const char* name, const String& value) { return publishNamed(name, escapeJsonString(value.c_str())); }
bool NorviClass::publish(const String& jsonObject) { return publishJsonObject(jsonObject); }
