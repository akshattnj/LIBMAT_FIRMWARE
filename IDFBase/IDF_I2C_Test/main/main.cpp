extern "C"
{
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
}
#include <string.h>
#include "src/definations.h"

#ifdef SCANNER_MODE

#include "src/I2C_Scanner.h"

extern "C" void app_main(void)
{
    xTaskCreatePinnedToCore(task, "Scanner", 2048, NULL, 12, NULL, 1);
}

#else

#include "src/I2CHandler.h"

I2CHandler handleI2C(I2C_NUM_0, SDA_0_PIN, SCL_0_PIN, I2C_0_CLOCK);

extern "C" void app_main(void)
{
    handleI2C.setup();
    xTaskCreate([](void *parameters)
                { handleI2C.updateI2C(parameters); },
                "I2C Updater", 2048, NULL, 12, NULL);
}

#endif