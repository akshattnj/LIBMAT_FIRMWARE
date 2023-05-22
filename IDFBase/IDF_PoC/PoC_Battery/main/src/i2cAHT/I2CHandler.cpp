#include I2CHandler.h

namespace AHT{
    void setup()
    {
        vTaskDelay(100 / portTICK_PERIOD_MS); // I2C devices initialisation
        i2c_param_config(portNum, &config);
        i2c_driver_install(portNum, I2C_MODE_MASTER, 0, 0, 0);
        ESP_LOGI(I2C_TAG, "I2C Initial Setup Complete");
        setupAHT(AHT_ADDRESS);
        
    }
    void updateI2C(void *args)
    {
        while (1)
        {
            if (ahtSensorStatus & AHT_ENABLED)
            {
                writeI2C(AHT_ADDRESS, AHTMeasureCommand, 3);
                vTaskDelay(80 / portTICK_PERIOD_MS);
            checkStatusAHT:
                
                memset(readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
                readWriteI2C(AHT_ADDRESS, AHTStatusCommand, 1, readBufferAHT, 1);
                if (readBufferAHT[0] & AHT_BUSY)
                {
                    ESP_LOGI(I2C_TAG, "AHT sensor busy");
                    goto checkStatusAHT;
                }

                memset(readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
                if (readI2C(AHT_ADDRESS, readBufferAHT, 6))
                {
                    calculateTemperatureAndHumidity();
                }
            }

            for(int i = 0; i < 4; i++) {
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    bool readWriteI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize, uint8_t* readBuffer, size_t readSize)
    {
        espError = i2c_master_write_read_device(portNum, address, toWrite, writeSize, readBuffer, readSize, 100 / portTICK_PERIOD_MS);
        if (espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C read-write error with code: %d", espError);
            return false;
        }
        return true;
    }
    bool readI2C(uint8_t address, uint8_t* readBuffer, size_t readSize)
    {
        espError = i2c_master_read_from_device(portNum, address, readBuffer, readSize, 100 / portTICK_PERIOD_MS);
        if (espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C read error with code: %d", espError);
            return false;
        }
        return true;
    }

    bool writeI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize)
    {
        espError = i2c_master_write_to_device(portNum, address, toWrite, writeSize, 100 / portTICK_PERIOD_MS);
        if (espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C write failure with code:%d", espError);
            return false;
        }
        return true;
    }

    void setupAHT(uint8_t address)
    {
        vTaskDelay(40 / portTICK_PERIOD_MS);
        memset(readBufferAHT, 0, AHT_READ_BUFFER * sizeof(uint8_t));
        if (!(readWriteI2C(address, AHTStatusCommand, 1, readBufferAHT, 1)))
            return;

        if (!(readBufferAHT[0] & AHT_CALIB))
        {
            ESP_LOGI(I2C_TAG, "AHT is uncalibrated with response: %d. Initialising...", readBufferAHT[0]);
            if (!(writeI2C(address, AHTCalibrateCommand, 3)))
                return;
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ahtSensorStatus = ahtSensorStatus | AHT_ENABLED;
        ESP_LOGI(I2C_TAG, "AHT21B Setup Complete Status %X", ahtSensorStatus);
    }

    void calculateTemperatureAndHumidity()
    {
        //uint32_t humidityRaw = (readBufferAHT[1] << 12) | (readBufferAHT[2] << 4) | (readBufferAHT[3] >> 4);
        uint32_t temperatureRaw = ((readBufferAHT[3] & 0x0F) << 16) | (readBufferAHT[4] << 8) | (readBufferAHT[5]);
        //humidity = humidityRaw * inv2Pow20 * 100;
        temperature = (temperatureRaw * inv2Pow20 * 200) - 50;
        ESP_LOGI(I2C_TAG, "Got temperature %f", temperature);
    }
}

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
