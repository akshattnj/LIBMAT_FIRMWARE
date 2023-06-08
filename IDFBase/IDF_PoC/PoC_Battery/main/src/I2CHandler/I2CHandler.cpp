#include "I2CHandler.h"

namespace I2C
{
    const i2c_port_t portNum = I2C_NUM_0;

    uint8_t ahtSensorStatus; // 7 - Free, 6 - Free, 5 - Free, 4 - Free, 3- Free, 2 - Free, 1 - Free, 0 - AHT ON

    const uint8_t AHTStatusCommand[1] = {0x71};
    const uint8_t AHTCalibrateCommand[3] = {0xBE, 0x08, 0x00};
    const uint8_t AHTMeasureCommand[3] = {0xAC, 0x33, 0x00};
    const double inv2Pow20 = 1.0 / 1048576.0;
    uint8_t readBufferAHT[AHT_READ_BUFFER];
    ssd1306_handle_t ssdHandle = NULL;
    esp_err_t espError;

    void setup()
    {
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = SDA_0_PIN;
        conf.scl_io_num = SCL_0_PIN;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = I2C_0_CLOCK;
        i2c_param_config(portNum, &conf);

        i2c_driver_install(portNum, I2C_MODE_MASTER, 0, 0, 0);
        ESP_LOGI(I2C_TAG, "I2C Initial Setup Complete");

        setupAHT(AHT_ADDRESS);
        setupSSD();
    }

    void updateI2C(void *args)
    {
        while (1)
        {
            // AHT related work
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
                // OLEDTask(NULL);
            }

            //ssd1306 related work
            char buffer[100];
            size_t len = snprintf(buffer, 100, "SoC: %u%%", Commons::batteryPercentage);
            ssd1306_clear_screen(ssdHandle, false);
            ssd1306_draw_string(ssdHandle, 70, 16, (const uint8_t *)buffer, 16, 1);
            ssd1306_refresh_gram(ssdHandle);

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    bool readWriteI2C(uint8_t address, const uint8_t *toWrite, size_t writeSize, uint8_t *readBuffer, size_t readSize)
    {
        espError = i2c_master_write_read_device(portNum, address, toWrite, writeSize, readBuffer, readSize, 100 / portTICK_PERIOD_MS);
        if (espError != ESP_OK)
        {
            ESP_LOGE(I2C_TAG, "I2C read-write error with code: %d", espError);
            return false;
        }
        return true;
    }
    bool readI2C(uint8_t address, uint8_t *readBuffer, size_t readSize)
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

    void setupSSD()
    {
        ssdHandle = ssd1306_create(portNum, SSD1306_I2C_ADDRESS);
        ssd1306_refresh_gram(ssdHandle);
        ssd1306_clear_screen(ssdHandle, false);
        ESP_LOGI(I2C_TAG, "SSD1306 setup complete");
    }

    void calculateTemperatureAndHumidity()
    {
        uint32_t humidityRaw = (readBufferAHT[1] << 12) | (readBufferAHT[2] << 4) | (readBufferAHT[3] >> 4);
        uint32_t temperatureRaw = ((readBufferAHT[3] & 0x0F) << 16) | (readBufferAHT[4] << 8) | (readBufferAHT[5]);
        Commons::humidity = humidityRaw * inv2Pow20 * 100;
        Commons::temperature = (temperatureRaw * inv2Pow20 * 200) - 50;
        ESP_LOGI(I2C_TAG, "Got Temperature: %0.2f, Humidity: %0.2f", Commons::temperature, Commons::humidity);
    }
}
