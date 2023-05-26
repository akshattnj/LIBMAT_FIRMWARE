#ifndef I2CHANDLER_H
#define I2CHANDLER_H

#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

namespace aht {

    class I2CHandler {
    public:
        I2CHandler(i2c_port_t portNum, int sdaPin, int sclPin, uint32_t clock);

        void setup();

        void updateI2C(void* args);

    private:
        // General I2C variables
        const i2c_port_t portNum;
        i2c_config_t config;
        const int sdaPin;
        const int sclPin;
        uint8_t ahtSensorStatus;

        // I2C Communication Variables
        uint8_t readBufferAHT[AHT_READ_BUFFER];
        esp_err_t espError;

        // AHT Variable
        const uint8_t AHTStatusCommand[1] = {0x71};
        const uint8_t AHTCalibrateCommand[3] = {0xBE, 0x08, 0x00};
        const uint8_t AHTMeasureCommand[3] = {0xAC, 0x33, 0x00};
        const double inv2Pow20 = 1.0 / 1048576.0;

        bool readWriteI2C(uint8_t address, const uint8_t* toWrite, size_t writeSize, uint8_t* readBuffer, size_t readSize);

        bool readI2C(uint8_t address, uint8_t* readBuffer, size_t readSize);

        bool writeI2C(uint8_t address, const uint8_t* toWrite, size_t writeSize);

        void setupAHT(uint8_t address);

        void calculateTemperatureAndHumidity();
    };

}  // namespace aht

#endif  // I2C_HANDLER_H
