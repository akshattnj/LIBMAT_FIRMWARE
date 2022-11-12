#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "definations.h"
extern "C"
{
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

class I2CHandler
{
public:
    const i2c_port_t portNum;
    i2c_config_t config;
    const int sdaPin;
    const int sclPin;

    I2CHandler(i2c_port_t portNum, int sdaPin, int sclPin, uint32_t clock) : portNum(portNum), sdaPin(sdaPin), sclPin(sclPin)
    {
        this->config.mode = I2C_MODE_MASTER;
        this->config.sda_io_num = sdaPin;
        this->config.scl_io_num = sclPin;
        this->config.sda_pullup_en = GPIO_PULLUP_ENABLE;
        this->config.scl_pullup_en = GPIO_PULLUP_ENABLE;
        this->config.master.clk_speed = clock;
        return;
    }

    void setup()
    {
        vTaskDelay(100 / portTICK_RATE_MS); // I2C devices initialisation
        i2c_param_config(this->portNum, &this->config);
        i2c_driver_install(this->portNum, I2C_MODE_MASTER, 0, 0, 0);
        this->setupAHT(AHT_ADDRESS);
        this->setupMPU(MPU_ADDRESS);
    }

    void setupAHT(uint8_t address)
    {
        uint8_t writeBuffer[] = {0x71};
        uint8_t readBuffer[BUF_SIZE];
        memset(readBuffer, 0, sizeof(readBuffer));
        esp_err_t espError = i2c_master_write_read_device(I2C_NUM_0, 0x38, writeBuffer, sizeof(writeBuffer), readBuffer, 1, 100 / portTICK_RATE_MS);
        if (espError > 0)
        {
            ESP_LOGE(MAIN_TAG, "I2C read-write error: %d", espError);
        }
        else
        {
            for (uint i = 0; i < 1024; i++)
            {
                printf("%d ", readBuffer[i]);
            }
            printf("\n");
        }
    }

    void setupMPU(uint8_t address)
    {
    }

    void updateI2C(void *args)
    {
    }
};

#endif