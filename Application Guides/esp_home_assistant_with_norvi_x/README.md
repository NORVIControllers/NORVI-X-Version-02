---
type: Example_program
function: NORVI X, DI4, R4 and AI4 and states the tested ESPHome version(NORVI X on-board I/O, X-DI4 digital inputs, X-R4 relay outputs, X-AI4 4-20 mA analog inputs, Home Assistant integration, combined package config)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1 (V2, N16R2)]
compatible_modules: [NORVI X-DI4, NORVI X-R4, NORVI X-AI4]
status: tested (see "Tested versions")
last_updated: 2026-11-16
---

# NORVI X ESPHome Examples

Ready-to-use ESPHome YAML configs for the **NORVI X** industrial controller (CPU-ESPS3-X1) and its **X-DI4**, **X-R4** and **X-AI4** expansion modules, so the board and modules appear in Home Assistant with no custom code.

## What this does
Each file configures one part of the NORVI X in ESPHome and exposes it to Home Assistant as native entities over the ESPHome API (encrypted), with Wi-Fi and over-the-air (OTA) updates:

- **On-board I/O**: the two front buttons and the RUN and ERROR LEDs.
- **X-DI4**: four isolated 24 V digital inputs, with contact-bounce filtering.
- **X-R4**: four relay outputs, off at start-up.
- **X-AI4**: four 4-20 mA analog inputs, read by the ADS1115 and scaled to a 0-10 value.

Every file works on its own, and the four can also be combined into a single config (see [Using all modules together](#using-all-modules-together)).

## Use cases
| Part | Typical uses |
|---|---|
| On-board buttons and LEDs | Local start/stop button; RUN and ERROR LEDs as status lights driven from Home Assistant |
| X-DI4 inputs | Door or window contact, push button, float switch, limit switch, flow switch, or any 24 V sensor output |
| X-R4 relays | Switch a fan, light, pump, heater or door lock (5 A per relay; use a contactor for larger loads) |
| X-AI4 inputs | 4-20 mA tank level, pressure, temperature or flow transmitters, scaled to a 0-10 value |
| All together | Tank control: an AI4 level sensor starts an R4 pump when the level drops below 20 %, and a DI4 float switch raises a dry-run alarm (build the logic as a Home Assistant automation) |

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1, **V2 "N16R2"** (ESP32-S3, 16 MB flash, 2 MB quad PSRAM)
- Expansion module(s), as needed:
  - NORVI X-DI4 (4 digital inputs, direct GPIO)
  - NORVI X-R4 (4 relays, PCA9538 I2C expander)
  - NORVI X-AI4 (4 x 4-20 mA inputs, ADS1115 I2C ADC)
- 24 V DC regulated supply, 1 A or higher. The modules take their power through the expansion port.
- USB cable for the first flash
- A computer with ESPHome (command line or Device Builder) and a Home Assistant instance on the same network
- For testing: a switch or 24 V source (DI4), a multimeter or low-voltage load (R4), and a calibrated 4-20 mA source (AI4)

## Files in this repository

| File | Hardware | Entities created in Home Assistant |
|---|---|---|
| `norvi-x-cpu-esps3-x1.yaml` | NORVI X CPU (PCA9536 at 0x41) | Binary sensors: `Button 1`, `Button 2`. Switches: `RUN LED`, `ERROR LED` |
| `norvi-x-di4.yaml` | X-DI4 (IO5, IO6, IO7, IO10) | Binary sensors: `DI4 Input 1` to `DI4 Input 4` |
| `norvi-x-r4.yaml` | X-R4 (PCA9538 at 0x73) | Switches: `R4 Relay 1` to `R4 Relay 4` (the reset line on IO21 is handled internally and is not exposed) |
| `norvi-x-ai4.yaml` | X-AI4 (ADS1115 at 0x49) | Sensors: `AI4 Channel 1` to `AI4 Channel 4` (4-20 mA scaled to 0-10) |
| `norvi-x-combined.yaml` | CPU + DI4 + R4 + AI4 | All of the above, combined with ESPHome `packages` (needs the four files above in the same folder) |
| `secrets.yaml.example` | - | Template for Wi-Fi and API key |

```
norvi-x-esphome/
  README.md
  LICENSE
  secrets.yaml.example
  norvi-x-cpu-esps3-x1.yaml
  norvi-x-di4.yaml
  norvi-x-r4.yaml
  norvi-x-ai4.yaml
  norvi-x-combined.yaml
```


- **Power off the whole system before attaching or removing any module.** Seat each module firmly on the expansion bus until it locks.
- Power the CPU from a regulated 24 V DC supply (1 A or higher). Modules draw power through the expansion port, so they have no separate power terminals.
- All I2C modules share one bus: **SDA = IO8, SCL = IO9**. Each I2C module on the bus needs a unique address.

| Module | Interface | Address / pins | DIP settings |
|---|---|---|---|
| On-board buttons and LEDs | I2C (PCA9536) | 0x41 | fixed |
| X-R4 | I2C (PCA9538) | 0x73 default, range 0x70 to 0x73 | DIP 1 and 2 OFF = 0x73 |
| X-AI4 | I2C (ADS1115) | 0x49 default, options 0x48, 0x4A, 0x4B | DIP 1 ON, others OFF = 0x49 |
| X-DI4 | Direct GPIO | IN1 = IO5, IN2 = IO6, IN3 = IO7, IN4 = IO10 | none |

Other on-board devices seen in the I2C scan: 0x15 (touch controller), 0x68 (DS3231 RTC), 0x75 (PCA9539).

**X-DI4 wiring:** positive through a switch to `INx`, and negative to `DCOM` (one shared common for all four inputs). Logic OFF is below 5 V and logic ON is above 15 V.

**X-R4 wiring:** each relay is a single-pole normally-open contact. Connect one side of the field supply to `COM` and the load between `NO` and the supply return. Rated 5 A at 250 V AC or 30 V DC per channel.

**X-AI4 wiring:** connect the 4-20 mA signal between `AINx` and `AGND`. Keep every channel within 0-20 mA.

## Software required
| Item | Version |
|---|---|
| ESPHome | 2026.8.0 (tested) |
| ESPHome Device Builder | 1.12.2 (tested) |
| ESP32 framework | ESP-IDF (set in the YAML) |
| Home Assistant |  |

**Board settings used in the YAML** (from the NORVI X CPU guide): board `esp32-s3-devkitc-1`, variant `esp32s3`, flash 16 MB, `psram: mode: quad`, logger on the USB serial/JTAG port.

## Key settings
These substitutions sit at the top of each file. Change them there.

| File | Setting | Default | Meaning |
|---|---|---|---|
| all | `name`, `friendly_name` | per file | Device name on the network and in Home Assistant |
| `norvi-x-di4.yaml` | `di_inverted` | `"true"` | Flip to `"false"` if an input reads OFF while 24 V is applied |
| `norvi-x-di4.yaml` | `debounce` | `20ms` | Contact-bounce filter. Remove it for fast pulse signals (up to 5 kHz) |
| `norvi-x-r4.yaml` | `r4_address` | `"0x73"` | Must match the module's DIP switches |
| `norvi-x-ai4.yaml` | `ai4_address` | `"0x49"` | Must match the module's DIP switches |
| `norvi-x-ai4.yaml` | `ma_per_volt` | `"10"` | Volts-to-mA factor (5 ohm shunt x20 amplifier). Calibrate with a known current |
| `norvi-x-ai4.yaml` | `scale_min`, `scale_max` | `"0.0"`, `"10.0"` | Value shown at 4 mA and at 20 mA |

## How to use
1. Install ESPHome (the Device Builder is the easiest route) and have Home Assistant running on the same network.
2. Copy `secrets.yaml.example` to `secrets.yaml` and fill in your values:
   ```yaml
   wifi_ssid: "YourWifiName"
   wifi_password: "YourWifiPassword"
   api_encryption_key: "<base64 32-byte key>"   
   ```
   In the Device Builder, add the same entries under **Secrets**.
3. Pick the file for your hardware and open it in ESPHome (or `esphome run norvi-x-ai4.yaml` on the command line).
4. **Validate**, then **Install**. Connect the NORVI X over USB for the first flash (hold BOOT while pressing reset if the port does not appear). Later updates can go over Wi-Fi.
5. In Home Assistant, go to **Settings > Devices & Services**. The device appears under *Discovered*. Click **Configure** and enter the API encryption key.


## Using all modules together
Use `norvi-x-combined.yaml`, which includes the four files as packages (keep all files in the same folder). The file contains:

```yaml
substitutions:
  name: norvi-x-full
  friendly_name: "NORVI X Full"

packages:
  onboard: !include norvi-x-cpu-esps3-x1.yaml
  di4: !include norvi-x-di4.yaml
  r4: !include norvi-x-r4.yaml
  ai4: !include norvi-x-ai4.yaml
```

Settings in the main file override those in the included files. Lists such as `switch` and `sensor` are joined, and the expander IDs are different (`pca9536_io`, `pca9538_relays`), so there are no clashes.

In the ESPHome Device Builder, create four devices named exactly `norvi-x-cpu-esps3-x1`, `norvi-x-di4`, `norvi-x-r4` and `norvi-x-ai4` (skip the install step), paste each file into its device, then paste `norvi-x-combined.yaml` into a fifth device and install that one. The file names must match the `!include` lines.

A NORVI X runs one firmware at a time, so flash either a single-module file or the combined config.

## Adding another expansion
The NORVI X series has more expansion modules than the three covered here. To add one to Home Assistant:

1. **Check the module's datasheet** on norvi.io: does it use an I2C chip (note the chip and the address set by its DIP switches) or direct GPIO pins (note the pins)? Make sure it does not share pins or an address with a module you already use (see the Module Mixing Guide).
2. **Create a device.** In the ESPHome Device Builder click **+ New Device**, name it `norvi-x-<module>` (for example `norvi-x-ai4`), choose **ESP32-S3**, and choose **Skip** at the install step.
3. **Write its file.** Click **Edit**, delete the generated YAML and paste in the closest existing file as a template (`norvi-x-ai4.yaml` for an I2C chip, `norvi-x-di4.yaml` for direct GPIO). Keep the top part (substitutions, board, Wi-Fi, API) and replace the bus, chip and entities with those of the new module. Put the I2C address in a substitution. Click **Save**.
4. **Test it alone.** Install it, check the I2C scan in the log for the new address, and check the entities in Home Assistant.
5. **Add it to the combined config.** Open the `norvi-x-combined` device and add one line under `packages:`
   ```yaml
   packages:
     onboard: !include norvi-x-cpu-esps3-x1.yaml
     di4: !include norvi-x-di4.yaml
     r4: !include norvi-x-r4.yaml
     ai4: !include norvi-x-ai4.yaml
     newmodule: !include norvi-x-<module>.yaml    # new line
   ```
   Then **Validate** and **Install > Wirelessly**.
6. **Check in Home Assistant.** The new entities appear on the existing device page. If they do not, reload the ESPHome integration under **Settings > Devices & Services**.

Rules for a clean merge: every I2C address, expander `id`, GPIO pin and entity `name` must be unique across all included files, and the file name in `!include` must match the device name exactly.

## How it works
- **On-board I/O:** the PCA9536 has the same register layout as the PCA9554, so ESPHome's `pca9554` component drives it. Buttons are on pins 0 and 3 and read low when pressed. The LEDs are on pins 2 (RUN) and 1 (ERROR) and are **active low**, so they use `inverted: true`.
- **X-DI4:** inputs are optically isolated and go straight to the ESP32-S3 GPIO.
- **X-R4:** the PCA9538 reset pin is wired to **IO21**. The config holds IO21 high with an internal switch that starts before the expander, otherwise the module can stay in reset and not show up in the I2C scan. Relay `n` is expander pin `n-1`, and all relays start OFF (`restore_mode: ALWAYS_OFF`).
- **X-AI4:** each AINx terminal feeds an ADS1115 input (A0 to A3, single-ended) through a 5 ohm shunt and an amplifier. With the ±2.048 V gain range, 20 mA is about 2.0 V, so `mA = volts x 10`. The filters then run in this order: volts to mA, mA to the 0-10 scale (`calibrate_linear`), clamp to the scale, and a 5-sample moving average. The units are removed so Home Assistant shows a plain number.

## Expected output
In the ESPHome log at boot, the I2C scan should list the devices that are fitted:

| Address | Device |
|---|---|
| 0x15 | Touch controller |
| 0x41 | PCA9536 (buttons, LEDs) |
| 0x49 | ADS1115 (X-AI4) |
| 0x68 | DS3231 RTC |
| 0x73 | PCA9538 (X-R4) |
| 0x75 | PCA9539 |

In Home Assistant the device page shows the entities from the table above. With nothing wired to the AI4, the channels read about 0.

## Testing checklist
Record the date, ESPHome version and result for each row before relying on a config.

| Config | Test | Pass condition | Result |
|---|---|---|---|
| All | Flash over USB, then update over OTA | Device boots, joins Wi-Fi, shows in Home Assistant, OTA update succeeds | ☐ |
| On-board I/O | Press each button, toggle each LED | Button states change; LEDs light when the switch is ON | ☐ |
| X-DI4 | Apply and remove 24 V on each input | Each channel reports ON and OFF correctly, no false triggers | ☐ |
| X-R4 | Toggle each relay from Home Assistant | Each relay clicks and the contact changes as measured; all OFF after a reboot | ☐ |
| X-AI4 | Apply at least two known currents per channel (for example 4 mA and 20 mA) | Readings match the source (4 mA reads `scale_min`, 20 mA reads `scale_max`) | ☐ |
| All | Power-cycle the unit | Device reconnects by itself and entities return to a safe state | ☐ |

## Common issues
| Symptom | Likely cause | Fix |
|---|---|---|
| R4 missing from the I2C scan | PCA9538 held in reset, or module not seated | Use `norvi-x-r4.yaml` (it holds IO21 high); power off and reseat the module |
| AI4 missing from the scan, sensors show Unknown, log says `ads1115 ... FAILED` | X-AI4 not seated, wrong DIP address, or weak supply | Power off and reseat it; DIP 1 ON for 0x49; use a regulated 24 V, 1 A supply |
| DI4 inputs read the wrong way round | Polarity | Set `di_inverted` to `"false"` |
| One DI4 channel flickers | Floating or weak signal (below 15 V), noise from relays, loose DCOM | Check wiring and voltage; increase `debounce`; set `use_interrupt: false` on that input |
| LED lights when its switch is OFF | LED polarity | The on-board LEDs are active low; keep `inverted: true` |
| AI4 shows "V" or a voltage class in Home Assistant | Default ADS1115 unit | The config sets `unit_of_measurement: ""` and `device_class: ""`; clear the old unit on the entity in Home Assistant if it persists |
| AI4 readings off by a constant factor | Shunt and amplifier scaling differs from `ma_per_volt` | Measure with a calibrated source and update `ma_per_volt` |
| `4-Way Handshake Timeout` in the log | Wrong Wi-Fi password, or weak signal | Check the secrets (quote the values); attach the external antenna and move closer |
| Device not found in Home Assistant | Different network, or hotspot client isolation | Put Home Assistant and the device on the same network |

## Known limits
- Tested only on the **V2 N16R2** CPU. A different PSRAM variant (for example N16R8) needs `psram: mode: octal`.
- The X-DI4 and the X-UART-IO module share IO5, IO6, IO7 and IO10 and cannot be used together.
- The `pca9554` component is used for the PCA9536 and PCA9538 because they share its register layout. Only the pins listed above are used.
- The X-AI4 front end is documented as a shunt plus amplifier, but the scaling factor is not stated in the datasheet. Verify `ma_per_volt` on your unit.
- The NORVI documents describe the DI4 input type inconsistently (sinking, sourcing PNP and NPN). Test your sensor type before relying on it.
- Interrupt-mode inputs with a debounce filter are not suitable for fast pulse counting. Use ESPHome's `pulse_counter` for that.
- The on-board RTC (DS3231), RS-485, Ethernet (W5500), display and SD card are not covered by these examples.

## Possible extensions
- RTC: the DS3231 at 0x68 for timekeeping.
- RS-485 (TX IO15, RX IO16, automatic direction control) for Modbus.
- Ethernet through the W5500 (CS IO1, RST IO2, INT IO4).
- Use the 4-20 mA scaling for a real unit, such as 0-16 bar from a pressure transmitter, by changing `scale_max`.

## Related links
- NORVI product page: https://norvi.io/
- X-AI4 datasheet: https://norvi.io/docs/norvi-x-ai4-datasheet/
- X-R4 datasheet: https://norvi.io/docs/norvi-x-r4-datasheet/
- Module Mixing Guide: https://norvi.io/norvi-x-configuration-comparison-tool/#module-mixing
- 4-20 mA pressure example (Arduino): https://norvi.io/docs/esp32-4-20ma-input-pressure-x-ai4/
- Arduino test sketches and libraries: https://github.com/NORVIControllers/NORVI-X-Version-02
- Technical support: support@norvi.io

## License
This example is provided for **development and educational purposes**.
