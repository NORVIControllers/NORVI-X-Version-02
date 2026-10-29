---
type: Test_program
function: lorawan_otaa_serial_bridge (LoRaWAN OTAA uplink of newline-terminated JSON received on Serial1, with LED status indication)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1, NORVI X-CPU-ESPS3-X2, NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-LW-915]
status: completed
last_updated: 2026-10-26
---

# Example: NORVI X-LW LoRaWAN Test

## What this does
Joins a LoRaWAN network using OTAA and then forwards every newline-terminated text line (e.g. a JSON string, max 64 bytes) received on Serial1 as a confirmed uplink on port 2. Two on-board LEDs show the join and uplink status, and all events are logged to the Serial Monitor

## Hardware required
- Expansion module(s): NORVI X-LW
- Serial data output device with newline-terminated text line
- Wiring / pin map (from firmware `#define`s and the `LGFX` class):

| Signal | GPIO |
|---|---|
| UART RX | P0.15 |
| UART TX | P0.16 |
| LED1 | P1.03 |
| LED1 | P1.04 |

## Libraries required
| Library | Version |
|---|---|

**RAKwireless RUI nRF Board Package version:** 4.2.1 version.

**Arduino IDE Board/Tools settings:**
| Setting | Value |
|---|---|
| Board | WisBlock Core RAK4631 |
| Debug | Level 0 |
| Bootloader | 0.3.2 SoftDevices140 7.2.0 |
| Support LoRaWAN Region AS923 | On |
| Support AT | On |
| Support LoRaWAN Region AU915 | On |
| Support LoRaWAN Region CN470| Off |
| Support LoRaWAN Region EU433 | Off |
| Support LoRaWAN Region EU868 | On |
| Support LoRaWAN Region IN865 | On |
| Support LoRaWAN Region KR920 | On |
| Support LoRaWAN Region LA915 | On |
| Support LoRaWAN | Support LoRaWAN and LoRa P2P |
| Support NFC | Disable |
| Support LoRaWAN Region RU864 | On |
| Support LoRaWAN Region US915 | On |

## Full Sketch
```cpp



/*
https://docs.rakwireless.com/product-categories/wisduo/rak4630-module/quickstart/

Link for board configuration details

*/

#define OTAA_PERIOD   (1000)
/*************************************
   LoRaWAN band setting:
     RAK_REGION_EU433
     RAK_REGION_CN470
     RAK_REGION_RU864
     RAK_REGION_IN865
     RAK_REGION_EU868
     RAK_REGION_US915
     RAK_REGION_AU915
     RAK_REGION_KR920
     RAK_REGION_AS923

 *************************************/
#define OTAA_BAND     (RAK_REGION_EU868)
#define OTAA_DEVEUI  {0x69, 0x8E, 0xF2, 0x51, 0x4D, 0xC9, 0x2A, 0x26};
#define OTAA_APPEUI   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define OTAA_APPKEY   {0xC9, 0xEB, 0xAD, 0x92, 0x18, 0x5A, 0xCE, 0xF6, 0x26, 0x3D, 0xDC, 0xCC, 0xB3, 0x1C, 0x0C, 0x0F};

/*************************************
   LED indicators (plain digital output, no PWM)
     Green 2  blinking       : joining the network
     Green 1 solid (2 s)    : joined successfully
     Green 1 short flash    : uplink sent OK
     Green 2 short flash    : uplink failed
     Green 1 + Green 2 solid   : configuration error in setup()
 *************************************/
#define LED_ON          HIGH   // swap HIGH/LOW if your LEDs are active-low
#define LED_OFF         LOW
#define JOIN_BLINK_MS   250    // blink period while joining

/** Packet buffer for sending */
uint8_t collected_data[64] = { 0 };

/** Set by the join callback when a join attempt (after all retries) failed */
volatile bool joinFailed = false;

/** State for the non-blocking LED pulse */
static uint8_t  ledPulsePin = 0;
static uint32_t ledOffAt = 0;
static bool     ledPulsing = false;

void ledsOff()
{
  digitalWrite(LED_GREEN, LED_OFF);
  digitalWrite(LED_BLUE, LED_OFF);
}

void ledError()
{
  digitalWrite(LED_GREEN, LED_ON);
  digitalWrite(LED_BLUE, LED_ON);
}

/** Turn a LED on now and off again after 'ms' (handled by ledService() in loop()) */
void ledPulse(uint8_t pin, uint32_t ms)
{
  digitalWrite(pin, LED_ON);
  ledPulsePin = pin;
  ledOffAt = millis() + ms;
  ledPulsing = true;
}

/** Call regularly from loop() to switch a pulsed LED off again */
void ledService()
{
  if (ledPulsing && (int32_t)(millis() - ledOffAt) >= 0) {
    digitalWrite(ledPulsePin, LED_OFF);
    ledPulsing = false;
  }
}

void recvCallback(SERVICE_LORA_RECEIVE_T * data)
{
  if (data->BufferSize > 0) {
    Serial.println("Something received!");
    for (int i = 0; i < data->BufferSize; i++) {
      Serial.printf("%02x", data->Buffer[i]);
    }
    Serial.print("\r\n");
  }
}

void joinCallback(int32_t status)
{
  Serial.printf("Join status: %d\r\n", status);
  if (status != 0) {
    joinFailed = true;
  }
}

void sendCallback(int32_t status)
{
  if (status == 0) {
    Serial.println("Successfully sent");
    ledPulse(LED_GREEN, 150);
  } else {
    Serial.println("Sending failed");
    ledPulse(LED_BLUE, 300);
  }
}

void setup()
{
  Serial.begin(115200, RAK_AT_MODE);
  Serial1.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  ledsOff();

  Serial.println("RAKwireless LoRaWan OTAA Example");
  Serial.println("------------------------------------------------------");

  if (api.lorawan.nwm.get() != 1)
  {
    api.lorawan.nwm.set();   // switch to LoRaWAN mode, then restart
    api.system.reboot();
  }

  // OTAA Device EUI MSB first
  uint8_t node_device_eui[8] = OTAA_DEVEUI;
  // OTAA Application EUI MSB first
  uint8_t node_app_eui[8] = OTAA_APPEUI;
  // OTAA Application Key MSB first
  uint8_t node_app_key[16] = OTAA_APPKEY;

  if (!api.lorawan.appeui.set(node_app_eui, 8)) {
    Serial.printf("LoRaWan OTAA - set application EUI is incorrect! \r\n");
    ledError();
    return;
  }
  if (!api.lorawan.appkey.set(node_app_key, 16)) {
    Serial.printf("LoRaWan OTAA - set application key is incorrect! \r\n");
    ledError();
    return;
  }
  if (!api.lorawan.deui.set(node_device_eui, 8)) {
    Serial.printf("LoRaWan OTAA - set device EUI is incorrect! \r\n");
    ledError();
    return;
  }

  if (!api.lorawan.band.set(OTAA_BAND)) {
    Serial.printf("LoRaWan OTAA - set band is incorrect! \r\n");
    ledError();
    return;
  }
  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
    Serial.printf("LoRaWan OTAA - set device class is incorrect! \r\n");
    ledError();
    return;
  }
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))	// Set the network join mode to OTAA
  {
    Serial.printf("LoRaWan OTAA - set network join mode is incorrect! \r\n");
    ledError();
    return;
  }

  /** Register the callbacks BEFORE joining so the join result is reported */
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);

  Serial.println("Joining LoRaWAN network...");
  /** join, no auto-join, retry every 10 s, up to 8 attempts */
  if (!api.lorawan.join(1, 0, 10, 8))
  {
    Serial.printf("LoRaWan OTAA - join fail! \r\n");
    ledError();
    return;
  }

  /** Wait for join success, blinking the blue LED meanwhile */
  bool blueOn = false;
  uint32_t lastToggle = millis();
  while (api.lorawan.njs.get() == 0) {
    if (millis() - lastToggle >= JOIN_BLINK_MS) {
      lastToggle = millis();
      blueOn = !blueOn;
      digitalWrite(LED_BLUE, blueOn ? LED_ON : LED_OFF);
    }
    if (joinFailed) {
      joinFailed = false;
      Serial.println("Join failed, trying again...");
      api.lorawan.join(1, 0, 10, 8);
    }
    delay(10);
  }
  digitalWrite(LED_BLUE, LED_OFF);
  ledPulse(LED_GREEN, 2000);   // joined: green for 2 s (switched off in loop())

  if (!api.lorawan.adr.set(true)) {
    Serial.printf("LoRaWan OTAA - set adaptive data rate is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rety.set(1)) {
    Serial.printf("LoRaWan OTAA - set retry times is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.cfm.set(1)) {
    Serial.printf("LoRaWan OTAA - set confirm mode is incorrect! \r\n");
    return;
  }

  /** Check LoRaWan Status*/
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");	// Check Duty Cycle status
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED");	// Check Confirm status
  uint8_t assigned_dev_addr[4] = { 0 };
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);	// Check Device Address
  Serial.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
  Serial.println("");
}

void uplink_routine(const char* json_str)
{
  /** Copy the text into the packet buffer, never more than the buffer holds */
  size_t len = strlen(json_str);
  if (len > sizeof(collected_data)) len = sizeof(collected_data);
  if (len == 0) return;
  memcpy(collected_data, json_str, len);
  uint8_t data_len = (uint8_t) len;

  Serial.println("Data Packet:");
  for (int i = 0; i < data_len; i++) {
    Serial.printf("0x%02X ", collected_data[i]);
  }
  Serial.println("");

  /** Send the data package */
  if (api.lorawan.send(data_len, (uint8_t*) &collected_data, 2, true, 1)) {
    Serial.println("Sending is requested");
  } else {
    Serial.println("Sending failed");
    ledPulse(LED_BLUE, 300);
  }
}

void loop()
{
  ledService();   // switches pulsed LEDs off again

  char inputString[70]; // buffer for incoming serial data
  String inputString_Str; // variable to hold incoming serial data as a String object
  bool stringComplete = false; // flag to indicate if a complete string has been received

  // check if there is any serial data available
  while (Serial1.available()) {

    // read incoming serial data as a String object until a newline character is encountered
    inputString_Str = Serial1.readStringUntil('\n');

    // set the stringComplete flag to true since a complete string has been received
    stringComplete = true;
  }

  // check if a complete string has been received
  if (stringComplete) {

    // convert the incoming serial data from a String object to a char array
    inputString_Str.toCharArray(inputString, 69);

    // print the incoming serial data to the Serial monitor
    Serial.println(inputString);

    // pass the incoming serial data to the uplink_routine function
    uplink_routine(inputString);
  }
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- `RAKwireless LoRaWan OTAA Example`
- `Joining LoRaWAN network...`
- `Join status: 0` once the join is accepted (a non-zero status prints `Join failed, trying again...` and the join is retried
- Network status: `Duty cycle is ON/OFF, Packet is CONFIRMED, Device Address is XXXXXXXX, Uplink period is 1000ms`
- For every line received on Serial1: the line itself,`Data Packet:` followed by the hex bytes `0x7B 0x22 ...`, then `Sending is requested`
- `Successfully sent` or `Sending failed` from the send callback
- `Something received!` followed by the hex payload when a downlink arrives

On the LEDs: LED1 blinks while joining, LED2 is solid for 2 s after joining, then LED2 flashes after each successful uplink and LED1 flashes after a failed one.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| Both LEDs solid after reset | A configuration call in setup() failed (EUI, key, band, class or join mode) | failed (EUI, key, band, class or join mode)	Check the Serial Monitor message and verify the EUI / key array lengths (8 / 8 / 16 bytes) and that the band is supported by your module |
| Blue LED blinks forever, Join status is non-zero | Join request not accepted: DevEUI / AppEUI / AppKey do not match the network server, no gateway in range, or region mismatch | Confirm the keys are the same in the server and the sketch, that the gateway is online, and that the server region matches OTAA_BAND |
| Join request seen on the server but shown as an error | Device not registered / profile or key mismatch on the network server | Check the device profile (LoRaWAN band, LoRaWAN version, OTAA) and the AppKey on the server |
| Nothing is uplinked from Serial1 | Sender does not end lines with \n, wrong baud rate, or TX/RX not crossede | Send newline-terminated text at 115200 baud |
| Uplinks fail with Sending failed | Not joined, payload too large for the current data rate (ADR may select DR0), or duty cycle limit | Wait for a successful join, shorten the payload, and allow time between uplinks |


## Related Files
- Module reference: `expansion_module_LW.md`
- CPU board: `cpu_board_X1.md`
