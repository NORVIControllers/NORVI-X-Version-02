# Norvi Library for NORVI-XN

Norvi is the Arduino library for sending readings from your NORVI-XN sketch to your own cloud. You call `Norvi.publish()` with a name and a value, and the OTA & Communication Manager handles the connection underneath.

Your code -> OTA & Communication Manager -> your cloud

This library works with the NORVI-XN only.

## Use in Neural Studio

The Norvi library is already included in Neural Studio. There is nothing to install. Add `#include <Norvi.h>` to your sketch and call `Norvi.publish()`.

## Install in the Arduino IDE

1. Copy the `Norvi` folder into your Arduino `libraries` folder, or choose Sketch > Include Library > Add .ZIP Library and select the ZIP file.
2. Restart the Arduino IDE.
3. Select your NORVI-XN board, then open File > Examples > Norvi.

## Quick start

```cpp
#include <Norvi.h>
void setup() { Serial.begin(115200); }
void loop() {
  Norvi.publish("tank_level", 72.5f); delay(1000);
}
```

`tank_level` is an example. Use any field name you like.

## Functions

All `publish()` functions return `true` when the reading was accepted and `false` when it was not.

| Function | Value |
| --- | --- |
| `Norvi.publish(name, int)` | Whole number |
| `Norvi.publish(name, float)` | Decimal number |
| `Norvi.publish(name, bool)` | True or false |
| `Norvi.publish(name, String)` | Text |
| `Norvi.publish(String json)` | Several fields in one JSON object |

## Sending several values at once

```cpp
Norvi.publish(String("{\"pressure\":1012,\"alarm\":false}"));
```

## Field names and limits

- 1 to 64 characters. Start with a letter, then use letters, digits or `_`.
- Names must not start with `ni`, `fw`, `rst`, `cmd`, `log` or `ota` (any case).
- Up to 10 publishes per second. A message can be up to 1024 bytes.

## Examples

`PublishExample` and `PublishJson` are included under File > Examples > Norvi.

## Version and license

Version 1.0.0. MIT License. See the LICENSE file. Copyright (c) 2026 Norvi Controllers.

More functions will be added in future versions.
