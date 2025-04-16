#ifndef I2C_HELPERS_H
#define I2C_HELPERS_H

#include <Wire.h> 

// I2C ADDRESS
#define SENSOR_ADDRESS   0x4C

// I2C0 PINS
#define SDA_Pin 21
#define SCL_Pin 22

// Ensure you declare the global I2C object
extern TwoWire ESP32Wemo_I2C;

// Function to write to an I2C register
void writeI2cReg(uint8_t RegAddr, uint8_t Value) {
    ESP32Wemo_I2C.beginTransmission(SENSOR_ADDRESS);
    ESP32Wemo_I2C.write(RegAddr);
    ESP32Wemo_I2C.write(Value);
    if (ESP32Wemo_I2C.endTransmission(true) != 0) {
        Serial.println("Problem writing to I2C device");
        exit(0);
    }
}

// Function to read from an I2C register
uint8_t readI2cReg(uint8_t RegAddr) {
    ESP32Wemo_I2C.beginTransmission(SENSOR_ADDRESS);
    ESP32Wemo_I2C.write(RegAddr);
    if (ESP32Wemo_I2C.endTransmission(false)) { // If != 0
        Serial.println("Problem writing without stop");
        exit(0);
    }
    ESP32Wemo_I2C.requestFrom(SENSOR_ADDRESS, 0x01);
    return ESP32Wemo_I2C.read();
}

#endif