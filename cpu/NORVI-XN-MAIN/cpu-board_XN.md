# NORVI XN-MAIN: Hardware Reference

|  |  |
| :---- | :---- |
| **Product** | NORVI XN-MAIN |
| **Document type** | Hardware reference (GPIO map, peripherals, communication interfaces) |
| **Main controller** | ESP32-S3-WROOM-1U-N16R2 |
| **User-programmable controller** | ESP32-S3 only |

&nbsp;

---

## 1\. Product Overview

| Component | Part / Specification | Interface |
| :---- | :---- | :---- |
| Main controller | ESP32-S3-WROOM-1U-N16R2 | N/A |
| Ethernet controller | W5500 | SPI |
| Real-time clock | DS3231 | I2C |
| GPIO expander | PCA9539APW | I2C |
| RS-485 transceiver | SN65HVD72 | UART (IO15 / IO16) |
| Display | 2.0" TFT, ST7789 driver, with touch | SPI (display), I2C (touch) |
| Storage | MicroSD | SPI |
| Cellular modem | Included (managed by OTA manager, not user-accessible) | N/A |
| Expansion | 50-pin expansion port | N/A |
| Programming / debug | USB | USB (IO19 / IO20) |
| Terminals | RS-485 (B, A), Power (−, \+) | N/A |

&nbsp;

## 2\. GPIO Map

| GPIO | Net Name | Function |
| :---- | :---- | :---- |
| IO0 | NRST | BOOT pin |
| IO1 | SCSN | Ethernet module |
| IO2 | RSTN | Ethernet module |
| IO4 | INTN | Ethernet module |
| IO8 | SDA | I2C bus |
| IO9 | SCL | I2C bus |
| IO11 | MOSI | SPI bus |
| IO12 | SCLK | SPI bus |
| IO13 | MISO | SPI bus |
| IO15 | RS485\_TX | RS-485 communication |
| IO16 | RS485\_RX | RS-485 communication |
| IO19 | D\_N | USB (programming and debugging) |
| IO20 | D\_P | USB (programming and debugging) |
| IO38 | RST\_PCA | GPIO expander reset |
| IO42 | SD\_CS | SD card chip select |
| IO45 | DIS\_CS | TFT display chip select |
| IO46 | DIS\_DC | TFT display data/command |
| IO47 | TP\_RST / DIS\_RST | Touch and display reset (shared) |
| IO48 | TP\_INT | Touch interrupt |

&nbsp;

---

## 3\. Shared Buses

| Bus | Signals | Connected Devices |
| :---- | :---- | :---- |
| SPI | MOSI IO11, MISO IO13, SCLK IO12 | Ethernet (W5500), MicroSD, TFT display |
| I2C | SDA IO8, SCL IO9 | RTC (DS3231), touch screen, GPIO expander (PCA9539APW) |

&nbsp;

Each SPI device has a dedicated chip-select line:

| Device | Chip Select |
| :---- | :---- |
| Ethernet (W5500) | IO1 |
| MicroSD | IO42 |
| TFT display | IO45 |

&nbsp;

---

## 4\. Peripherals

### 4.1 Ethernet (W5500)

| Signal | GPIO |
| :---- | :---- |
| SCSN (chip select) | IO1 |
| RSTN (reset) | IO2 |
| INTN (interrupt) | IO4 |
| MOSI | IO11 |
| MISO | IO13 |
| SCLK | IO12 |

&nbsp;

### 4.2 MicroSD Card

| Parameter | Value |
| :---- | :---- |
| Card type | MicroSD |
| Interface | SPI |
| CS | IO42 |
| MISO | IO13 |
| MOSI | IO11 |
| SCLK | IO12 |
| SD detect | Not connected |

&nbsp;

### 4.3 TFT Display and Touch

| Parameter | Value |
| :---- | :---- |
| Display driver | ST7789 |
| Display size | 2.0 inch |
| MOSI | IO11 |
| MISO | IO13 |
| SCLK | IO12 |
| DIS\_CS | IO45 |
| DIS\_DC | IO46 |
| TP\_RST / DIS\_RST | IO47 |
| TP\_INT | IO48 |
| Touch SCL | IO9 |
| Touch SDA | IO8 |
| Reset pin | Not connected |

&nbsp;

### 4.4 GPIO Expander (PCA9539APW)

| Signal | GPIO |
| :---- | :---- |
| SDA | IO8 |
| SCL | IO9 |
| RST\_PCA | IO38 |

&nbsp;

### 4.5 Built-in Buttons

Both buttons are connected through the PCA9539APW GPIO expander.

| Button | Board Label | Expander Pin |
| :---- | :---- | :---- |
| Button 1 | P0 | P0\_2 |
| Button 2 | P3 | P0\_1 |

&nbsp;

### 4.6 Built-in Indicators

Both indicators are connected through the PCA9539APW GPIO expander.

| Indicator | Board Label | Expander Pin |
| :---- | :---- | :---- |
| RUN | P2 | P0\_3 |
| ERROR | P1 | P0\_4 |

&nbsp;

---

## 5\. Communication Interfaces

### 5.1 RS-485

| Parameter | Value |
| :---- | :---- |
| Communication mode | Half-duplex |
| Transceiver | SN65HVD72 |
| Unit load | 1/4 |
| TX pin | IO15 |
| RX pin | IO16 |

&nbsp;

**Terminal arrangement**

| Index | Terminal | Label |
| :---- | :---- | :---- |
| 1 | RS-485 | B |
| 2 | RS-485 | A |
| 3 | Power | − |
| 4 | Power | \+ |

&nbsp;

### 5.2 USB

| Signal | GPIO |
| :---- | :---- |
| D\_N | IO19 |
| D\_P | IO20 |

&nbsp;

Used for programming and debugging.

### 5.3 Cellular Modem

This device includes a cellular modem. It is managed by the OTA manager and cannot be accessed or programmed by the user. No user action or configuration is required.