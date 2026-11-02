# Norvi Arduino Library

Norvi is the NORVI Controllers Arduino library for publishing named readings from ESP32 firmware. The v1.0.0 API is `Norvi.publish()` and writes newline-terminated `NI:` records to `Serial` for the device communication path.

Data path: your code -> Serial line -> OTA & Communication Manager -> your cloud.

## Install

Copy the `Norvi` folder into your Arduino `libraries` directory, or install the ZIP through the Arduino IDE library manager's **Add .ZIP Library** command. Restart the IDE after installing if the header is not immediately found.

## Quick start

```cpp
#include <Norvi.h>
void setup() {
  Serial.begin(115200);
}
void loop() {
  Norvi.publish("temperature", 23.5f);
  delay(1000);
}
```

## API

All `publish()` overloads return `bool`.

| Form | Value type |
| --- | --- |
| `Norvi.publish(const char* name, int value)` | Integer |
| `Norvi.publish(const char* name, float value)` | Floating-point |
| `Norvi.publish(const char* name, bool value)` | Boolean |
| `Norvi.publish(const char* name, const String& value)` | Arduino `String` |
| `Norvi.publish(const String& jsonObject)` | JSON object batch |

A named value produces `NI:<field>=<JSON value>`. A JSON batch produces `NI:JSON=<JSON object>`.

## Field names and limits

- Field names are 1–64 characters.
- A field name starts with a letter, followed by letters, digits, or `_`.
- Reserved prefixes are `ni`, `fw`, `rst`, `cmd`, `log`, and `ota` (case-insensitive).
- Each encoded line is limited to 1024 bytes.
- The client accepts 10 publishes per second. Excess publishes are dropped; a `_dropped` count is added to a later JSON record.

## JSON batch

```cpp
Norvi.publish(String("{\"temperature\":23.5,\"running\":true}"));
```

## Supported boards

The library targets ESP32 boards (`architectures=esp32`), including the NORVI-XN main CPU based on ESP32-S3.

## Version

Version 1.0.0 is the initial release.

## License

MIT License. See the LICENSE file. Copyright (c) 2026 Norvi Controllers.

More functions will be added in future versions.
