extern "C"
{
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
}
#include <string.h>
#include "src/I2CHandler.h"
#include "src/definations.h"

I2CHandler handleI2C0(I2C_NUM_0, SDA_0_PIN, SCL_0_PIN, I2C_0_CLOCK);

extern "C" void app_main(void)
{
    handleI2C0.setup();
    handleI2C0.updateI2C(NULL);
}
