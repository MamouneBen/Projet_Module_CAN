#include <Arduino.h>
#include <Wire.h>
#include "MCP3X21.h"

const uint8_t address = 0x68; // MCP3221 I2C adresse
const uint16_t ref_voltage = 5000; // 3.3V

MCP3221 mcp3221(address);

void setup() {
    Serial.begin(115200);
    Wire.begin(8, 9); // initialise I2C: SDA pin 8 // SCL pin 9
    mcp3221.init(&Wire);
}

void loop() {
    uint16_t result = mcp3221.read();

    Serial.print(F("ADC: "));
    Serial.print(result);
    Serial.print(F(", mV: "));
    Serial.println(mcp3221.toVoltage(result, ref_voltage));

    delay(1000);
}
