extern "C"
{
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
}
#include <string.h>
#include "src/I2C_Scanner.h"
#include "src/definations.h"

extern "C" void app_main(void)
{
    i2c_config_t I2C_Config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_0_PIN,
        .scl_io_num = SCL_0_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE};
    I2C_Config.master.clk_speed = 100000;
    i2c_param_config(I2C_NUM_0, &I2C_Config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

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
    vTaskDelay(100 / portTICK_RATE_MS); // I2C devices initialisation
}
