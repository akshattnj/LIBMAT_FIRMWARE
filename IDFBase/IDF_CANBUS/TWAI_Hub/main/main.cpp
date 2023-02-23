extern "C"
{
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "src/TWAIHandler/TWAIHandler.h"
}
#include "src/definations.h"

extern "C" void app_main(void)
{
    esp_err_t errorCode = startTWAI();
    if (errorCode)
    {
        ESP_LOGE(MAIN_TAG, "TWAI driver install fail with %d", errorCode);
    }
    else
    {
        xTaskCreate(taskSendTWAI, "TWAI Send", 2048, NULL, 10, NULL);
        xTaskCreate(taskReceiveTWAI, "TWAI Recieve", 2048, NULL, 10, NULL);
    }
}
