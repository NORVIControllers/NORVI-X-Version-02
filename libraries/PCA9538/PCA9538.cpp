#include "PCA9538.h"
#include "Wire.h"

PCA9538::PCA9538(uint8_t address) {
    _address         = address;        // save the address id
    //_valueRegister   = 0;
    //Wire.begin();                      // start I2C communication
}

void PCA9538::pinMode(uint8_t pin, uint8_t mode) {
  uint8_t config = readRegister(PCA9538_CONFIG_REG);
  uint8_t port;

  port = portMap(pin);
  if (port == 255) Serial.println("Invalid pin number!!!");  // Invalid pin
  
  if (mode == INPUT) {
    config |= port;
  } else {
    config &= ~port;
  }
  writeRegister(PCA9538_CONFIG_REG, config);
}


void PCA9538::digitalWrite(uint8_t pin, uint8_t value) {
  uint8_t output = readRegister(PCA9538_OUTPUT_REG);
  uint8_t port;

  port = portMap(pin);
  if (port == 255) Serial.println("Invalid output pin number!!!");  // Invalid pin
  
  if (value == LOW) {
    output &= ~port;
  } else {
    output |= port;
  }
  writeRegister(PCA9538_OUTPUT_REG, output);
}


void PCA9538::toggleOutput(uint8_t pin) {
  uint8_t output = readRegister(PCA9538_OUTPUT_REG);
  uint8_t port;

  port = portMap(pin);
  if (port == 255) Serial.println("Invalid pin number!!!");  // Invalid pin
  
  output ^= port;
  writeRegister(PCA9538_OUTPUT_REG, output);
}


uint8_t PCA9538::digitalRead(uint8_t pin) {
  uint8_t _inputData = 0;
  uint8_t port;

  port = portMap(pin);
  if (port == 255) return 255;  // Invalid pin
  
  _inputData = readRegister(PCA9538_INPUT_REG);

  if((_inputData & port)>0) return HIGH;
  else return LOW;
}


uint8_t PCA9538::readRegister(uint8_t reg) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(_address, 1);
  return Wire.read();
}


void PCA9538::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t PCA9538::portMap(uint8_t pin) {
  switch (pin) {
    case 1: return 0x01;
    case 2: return 0x02;
    case 3: return 0x04;
    case 4: return 0x08;
    case 5: return 0x10;
    case 6: return 0x20;
    case 7: return 0x40;
    case 8: return 0x80;
    default:
      Serial.println("Invalid pin no!!! (Expected value 1-8)");
      return 255;   
  }
}
