#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "definations.h"
#include <string.h>

extern "C"
{
#include <stdio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

}


namespace AHT{
    //general I2C variables
    const i2c_port_t portNum;
    i2c_config_t config;
    const int sdaPin;
    const int sclPin;
    uint8_t ahtSensorStatus; // 7 - Free, 6 - Free, 5 - Free, 4 - Free, 3- Free, 2 - Free, 1 - Free, 0 - AHT ON

    // I2C Communication Variables
    uint8_t readBufferAHT[AHT_READ_BUFFER];
    esp_err_t espError;

    // AHT Variable
    const uint8_t AHTStatusCommand[1] = {0x71};
    const uint8_t AHTCalibrateCommand[3] = {0xBE, 0x08, 0x00};
    const uint8_t AHTMeasureCommand[3] = {0xAC, 0x33, 0x00};
    const double inv2Pow20 = 1.0 / 1048576.0;


    double temperature;
    void setup();
    void updateI2C(void *args);
    bool readWriteI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize, uint8_t* readBuffer, size_t readSize);
    bool readI2C(uint8_t address, uint8_t* readBuffer, size_t readSize);
    bool writeI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize);
    void setupAHT(uint8_t address);
    void calculateTemperatureAndHumidity();

}
#endif