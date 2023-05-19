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

class I2CHandler
{
public:
    double temperature;

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

    void setup()
    {
        vTaskDelay(100 / portTICK_PERIOD_MS); // I2C devices initialisation
        i2c_param_config(this->portNum, &this->config);
        i2c_driver_install(this->portNum, I2C_MODE_MASTER, 0, 0, 0);
        ESP_LOGI(I2C_TAG, "I2C Initial Setup Complete");
        this->setupAHT(AHT_ADDRESS);
    }

    void updateI2C(void *args)
    {
        while (1)
        {
            if (this->ahtSensorStatus & AHT_ENABLED)
            {
                this->writeI2C(AHT_ADDRESS, this->AHTMeasureCommand, 3);
                vTaskDelay(80 / portTICK_PERIOD_MS);
            checkStatusAHT:
                
                memset(this->readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
                this->readWriteI2C(AHT_ADDRESS, this->AHTStatusCommand, 1, this->readBufferAHT, 1);
                if (this->readBufferAHT[0] & AHT_BUSY)
                {
                    ESP_LOGI(I2C_TAG, "AHT sensor busy");
                    goto checkStatusAHT;
                }

                memset(this->readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
                if (this->readI2C(AHT_ADDRESS, this->readBufferAHT, 6))
                {
                    this->calculateTemperatureAndHumidity();
                }
            }

            for(int i = 0; i < 4; i++) {
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

private:
    // General I2C variables
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

    bool readWriteI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize, uint8_t* readBuffer, size_t readSize)
    {
        this->espError = i2c_master_write_read_device(this->portNum, address, toWrite, writeSize, readBuffer, readSize, 100 / portTICK_PERIOD_MS);
        if (espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C read-write error with code: %d", espError);
            return false;
        }
        return true;
    }


    bool readI2C(uint8_t address, uint8_t* readBuffer, size_t readSize)
    {
        this->espError = i2c_master_read_from_device(this->portNum, address, readBuffer, readSize, 100 / portTICK_PERIOD_MS);
        if (this->espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C read error with code: %d", espError);
            return false;
        }
        return true;
    }

    bool writeI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize)
    {
        this->espError = i2c_master_write_to_device(this->portNum, address, toWrite, writeSize, 100 / portTICK_PERIOD_MS);
        if (this->espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C write failure with code:%d", this->espError);
            return false;
        }
        return true;
    }

    void setupAHT(uint8_t address)
    {
        vTaskDelay(40 / portTICK_PERIOD_MS);
        memset(this->readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
        if (!(this->readWriteI2C(address, this->AHTStatusCommand, 1, this->readBufferAHT, 1)))
            return;

        if (!(readBufferAHT[0] & AHT_CALIB))
        {
            ESP_LOGI(I2C_TAG, "AHT is uncalibrated with response: %d. Initialising...", readBufferAHT[0]);
            if (!(this->writeI2C(address, this->AHTCalibrateCommand, 3)))
                return;
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        this->ahtSensorStatus = this->ahtSensorStatus | AHT_ENABLED;
        ESP_LOGI(I2C_TAG, "AHT21B Setup Complete Status %X", this->ahtSensorStatus);
    }

    void calculateTemperatureAndHumidity()
    {
        //uint32_t humidityRaw = (this->readBufferAHT[1] << 12) | (this->readBufferAHT[2] << 4) | (this->readBufferAHT[3] >> 4);
        uint32_t temperatureRaw = ((this->readBufferAHT[3] & 0x0F) << 16) | (this->readBufferAHT[4] << 8) | (this->readBufferAHT[5]);
        //this->humidity = humidityRaw * inv2Pow20 * 100;
        this->temperature = (temperatureRaw * inv2Pow20 * 200) - 50;
        ESP_LOGI(I2C_TAG, "Got temperature %f", this->temperature);
    }

};

extern "C" void task(void *ignore)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_0_PIN;
    conf.scl_io_num = SCL_0_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_NUM_0, &conf);

    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}
#endif