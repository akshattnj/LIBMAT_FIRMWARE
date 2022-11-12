extern "C"
{
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
}
#include "src/I2C_Scanner.h"

extern "C" void app_main(void)
{
    xTaskCreatePinnedToCore(task, I2C_TAG, configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}
