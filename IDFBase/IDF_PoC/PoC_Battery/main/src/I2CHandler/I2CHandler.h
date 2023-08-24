#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "definations.h"
#include "Commons/Commons.h"

#include <string.h>
#include <ssd1306.h>

extern "C"
{
#include <stdio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

namespace I2C
{
    extern uint8_t chargingimg[];
    void setup();
    void updateI2C(void *args);
    
    bool readWriteI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize, uint8_t *readBuffer, size_t readSize);
    bool readI2C(uint8_t address, uint8_t *readBuffer, size_t readSize);
    bool writeI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize);
    
    void setupAHT(uint8_t address);
    void calculateTemperatureAndHumidity();
    void setupSSD();
}
#endif