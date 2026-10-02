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
     Blue  blinking       : joining the network
     Green solid (2 s)    : joined successfully
     Green short flash    : uplink sent OK
     Blue  short flash    : uplink failed
     Green + Blue solid   : configuration error in setup()
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
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))  // Set the network join mode to OTAA
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
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");  // Check Duty Cycle status
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED"); // Check Confirm status
  uint8_t assigned_dev_addr[4] = { 0 };
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);  // Check Device Address
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
