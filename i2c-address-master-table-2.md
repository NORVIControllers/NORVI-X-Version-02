---
type: master_reference
purpose: single lookup table across the entire NORVI X expansion lineup, used for conflict-checking
status: Released
last_updated: 2026-09-04
---

# NORVI X — Master I2C Address Table

Every expansion module's I2C address lives here in one place. Purpose: a user stacking
multiple expansions (or the GPT answering "can I run these three together?") can check
for collisions in one lookup instead of cross-referencing individual datasheets.

## How to use this table
- If a module has a fixed address, it appears once.
- If a module's address is selectable (DIP switch / jumper), all options in its range are listed.
- "Default" marks the out-of-box address before any jumper change.

## Table

| Module | Default Addr | Selectable Range | Selection Method | Notes |
|---|---|---|---|---|
| CPU board onboard RTC (X1/X2/X3) | 0x68 | fixed | — | reserved, do not reuse |
| CPU board onboard IO Expander (X1/X2/X3) | 0x75 | fixed | - | reserved — carries some GPIO Map pins, see cpu-board_X1.md/cpu-board_X2.md/cpu-board_X3.md |
| CPU board onboard IO Expander (X1/X2/X3) | 0x41 | fixed | - | reserved — do not reuse |
| CPU board onboard touch panel (X1/X2/X3)| 0x15 | fixed | - | reserved — do not reuse |
| X-R4 4-Ch Relay Output | 0x73 | 0x70-0x73 | DIP |  |
| X-AV4 4-Ch Voltage Input | 0x49 | 0x48,0x49,0x4A,0x4B | DIP |  |
| X-AI4 4-Ch Current Input | 0x49 | 0x48,0x49,0x4A,0x4B | DIP |  |
| X-AQ4 4-Ch Analog Output | 0x5F | 0x50-0x5F | DIP |  |
| X-DI8 8-Ch Digital Input | 0x73 | 0x70-0x73 | DIP |  |
| X-DI16 16-Ch Digital Input | 0x27 | 0x20-0x27 | DIP |  |
| X-R8 8-Ch Relay Output | 0x72 | 0x70-0x73 | DIP |  |
| X-Q16 16-Ch Transistor Output | 0x27 | 0x20-0x27 | DIP |  |
| X-TC4 4-Ch Thermocouple Input | 0x18 |   0x08-0x0F,0x10-0x18 | DIP |  |
| X-Q8 8-Ch Transistor Output | 0x73 | 0x70-0x73 | DIP |  |
| X-RTD4 4-Ch RTD Input | 0x3F | 0x30-0x3F | DIP |  |
| X-DA8 8-Ch Digital AC Input | 0x73 | 0x70-0x73 | DIP |  |
| X-T4 4-Ch Thermistor Input | 0x48 | 0x48,0x49,0x4A,0x4B | DIP |  |
| X-MX4 — 4-Channel Digital Input, 4-Channel Analog Input, 4-Channel Transistor Output | 0X4A, 0X71 | 0x48,0x49,0x4A,0x4B, 0x70,0x71,0x72,0x73 | DIP | Has two DIP switches|


## Known Conflict Pairs
X-R4, X-DI8, X-Q8, X-DA8 are shipped with the same I2C address configuration.
X-DI16, X-Q16 are shipped with the same I2C address configuration.
X-AI4, X-AV4 are shipped with the same I2C address configuration.

## Reserved / Do Not Use
| Address | Reserved For |
|---|---|
| 0x68 | RTC |
| 0x15  | TOUCH PANEL |
| 0x75  | CPU GPIO EXPANDER |
| 0x41 | PCA9536 |
