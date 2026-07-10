#ifndef PCA9538_H
#define PCA9538_H

#include <Arduino.h>

// PCA9538 register addresses
#define PCA9538_INPUT_REG 0x00
#define PCA9538_OUTPUT_REG 0x01
#define PCA9538_POLARITY_REG 0x02
#define PCA9538_CONFIG_REG 0x03

// GPIO pins
#define GPIO1 0x01
#define GPIO2 0x02
#define GPIO3 0x04
#define GPIO4 0x08
#define GPIO5 0x10
#define GPIO6 0x20
#define GPIO7 0x40
#define GPIO8 0x80

class PCA9538 {
  public:
    PCA9538(uint8_t address);
    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t value);
    void toggleOutput(uint8_t pin);
    uint8_t digitalRead(uint8_t pin);

  private:
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value); 
    uint8_t portMap(uint8_t pin);

  uint8_t _address; 
};

#endif
