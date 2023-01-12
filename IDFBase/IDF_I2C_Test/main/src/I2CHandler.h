#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "definations.h"
#include <initializer_list>

extern "C"
{
#include <stdio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

class I2CHandler
{
public:
    double humidity;
    double temperature;

    /**
     * @brief Construct a new I2CHandler object
     *
     * @param portNum I2C Port number
     * @param sdaPin I2C SDA Pin
     * @param sclPin I2C SCL Pin
     * @param clock I2C Clock Speed
     */
    I2CHandler(i2c_port_t portNum, int sdaPin, int sclPin, uint32_t clock) : portNum(portNum), sdaPin(sdaPin), sclPin(sclPin)
    {
        this->config.mode = I2C_MODE_MASTER;
        this->config.sda_io_num = sdaPin;
        this->config.scl_io_num = sclPin;
        this->config.sda_pullup_en = GPIO_PULLUP_ENABLE;
        this->config.scl_pullup_en = GPIO_PULLUP_ENABLE;
        this->config.master.clk_speed = clock;
        this->ahtSensorStatus = 0;
        return;
    }

    /**
     * @brief Initial I2C Setup
     *
     */
    void setup()
    {
        vTaskDelay(100 / portTICK_RATE_MS); // I2C devices initialisation
        i2c_param_config(this->portNum, &this->config);
        i2c_driver_install(this->portNum, I2C_MODE_MASTER, 0, 0, 0);
        ESP_LOGI(I2C_TAG, "I2C Initial Setup Complete");
        this->setupAHT(AHT_ADDRESS);
    }


    /**
     * @brief Update data from connected I2C device. To be used inside task.
     *
     * @param args Task Parameters
     */
    void updateI2C(void *args)
    {
        while (1)
        {
            if (this->ahtSensorStatus & AHT_ENABLED)
            {
                this->writeI2C(AHT_ADDRESS, this->AHTMeasureCommand, 3);
                vTaskDelay(80 / portTICK_RATE_MS);
            checkStatusAHT:
                this->readWriteI2C(AHT_ADDRESS, this->AHTStatusCommand, 1, 1);
                if (this->readBuffer[0] & AHT_BUSY)
                {
                    ESP_LOGI(I2C_TAG, "AHT sensor busy");
                    goto checkStatusAHT;
                }
                if (this->readI2C(AHT_ADDRESS, 7))
                {
                    this->calculateTemperatureAndHumidity();
                }
            }
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }

private:
    // General I2C variables
    const i2c_port_t portNum;
    i2c_config_t config;
    const int sdaPin;
    const int sclPin;
    uint8_t ahtSensorStatus; // 7 - AHT ON, 6 - Free, 5 - Free, 4 - Free, 3- Free, 2 - Free, 1 - Free, 0 - Free

    // I2C Communication Variables
    uint8_t readBuffer[I2C_READ_BUFFER];
    esp_err_t espError;

    // AHT Variable
    const uint8_t AHTStatusCommand[1] = {0x71};
    const uint8_t AHTCalibrateCommand[3] = {0xBE, 0x08, 0x00};
    const uint8_t AHTMeasureCommand[3] = {0xAC, 0x33, 0x00};
    const double inv2Pow20 = 1.0 / 1048576.0;

    /**
     * @brief Select I2C register to read form and wait for response
     *
     * @param address Address of I2C device
     * @param toWrite Pointer to array that is to be written to I2C device
     * @param writeSize Size of data to be written to device
     * @param readSize Size of data to be read from device
     * @return true if communication successful
     * @return false if communication failed
     */
    bool readWriteI2C(uint8_t address, const uint8_t* toWrite, size_t writeSize, size_t readSize)
    {
        this->espError = i2c_master_write_read_device(I2C_NUM_0, address, toWrite, writeSize, readBuffer, readSize, 100 / portTICK_RATE_MS);
        if (espError > 0)
        {
            ESP_LOGE(I2C_TAG, "I2C read-write error on AHT21B with code: %d", espError);
            this->ahtSensorStatus = this->ahtSensorStatus & ~AHT_ENABLED;
            return false;
        }
        return true;
    }

    /**
     * @brief Read from I2C device
     *
     * @param address Address of I2C device
     * @param readSize Size of data to be read from device
     * @return true if communication successful
     * @return false if communication failed
     */
    bool readI2C(uint8_t address, size_t readSize)
    {
        this->espError = i2c_master_read_from_device(portNum, address, readBuffer, 6, 100 / portTICK_RATE_MS);
        if (this->espError > 0)
        {
            ESP_LOGE(I2C_TAG, "I2C read error on AHT21B with code: %d", espError);
            return false;
        }
        return true;
    }

    /**
     * @brief Write to I2C device
     *
     * @param address Address of I2C device
     * @param writeSize Size of data to be writted
     * @return true if communication is successful
     * @return false if communication failed
     */
    bool writeI2C(uint8_t address, const uint8_t* toWrite, size_t writeSize)
    {
        this->espError = i2c_master_write_to_device(this->portNum, address, (uint8_t*)toWrite, writeSize, 100 / portTICK_RATE_MS);
        if (this->espError > 0)
        {
            ESP_LOGE(I2C_TAG, "Communication failure with AHT with code:%d", this->espError);
            return false;
        }
        return true;
    }

    /**
     * @brief Initial setup for AHT21B
     *
     * @param address
     */
    void setupAHT(uint8_t address)
    {
        if (!(this->readWriteI2C(address, this->AHTStatusCommand, 1, 1)))
            return;

        if (!(readBuffer[0] & AHT_CALIB))
        {
            ESP_LOGI(I2C_TAG, "AHT is uncalibrated with response: %d. Initialising...", readBuffer[0]);
            if (!(this->writeI2C(address, this->AHTCalibrateCommand, 1)))
                return;
        }
        this->ahtSensorStatus = this->ahtSensorStatus | AHT_ENABLED;
        ESP_LOGI(I2C_TAG, "AHT21B Setup Complete Status %d", this->ahtSensorStatus);
    }

    /**
     * @brief Initial setup for MPU6050
     *
     * @param address
     */
    void setupMPU(uint8_t address)
    {
        ESP_LOGI(I2C_TAG, "MPU6050 Setup Complete");
    }

    /**
     * @brief Calculate temperature and humidity for the AHT21B sensor. Refer to documentation folder for details.
     * Uses data stored in the readBuffer.
     *
     */
    void calculateTemperatureAndHumidity()
    {
        uint32_t humidityRaw = (this->readBuffer[1] << 12) | (this->readBuffer[2] << 4) | (this->readBuffer[3] >> 4);
        uint32_t temperatureRaw = ((this->readBuffer[3] & 0x0F) << 16) | (this->readBuffer[4] << 8) | (this->readBuffer[5]);
        this->humidity = humidityRaw * inv2Pow20 * 100;
        this->temperature = (temperatureRaw * inv2Pow20 * 200) - 50;
        ESP_LOGI(I2C_TAG, "Got temperature %f and humidity %f\nDebug: %X %X", this->temperature, this->humidity, humidityRaw, temperatureRaw);
    }
};

#endif
