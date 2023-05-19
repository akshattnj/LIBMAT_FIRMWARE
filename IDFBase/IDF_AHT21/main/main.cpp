extern "C"
{
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
}
#include <string.h>
#include "definations.h"

#include "I2CHandler.h"
//#include "I2C_Scanner.h"

I2CHandler handleI2C(I2C_NUM_0, SDA_0_PIN, SCL_0_PIN, I2C_0_CLOCK);

extern "C" void app_main(void)
{
    handleI2C.setup();
    xTaskCreate([](void *parameters)
                { handleI2C.updateI2C(parameters); },
                "I2C Updater", 2048, NULL, 12, NULL);
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
