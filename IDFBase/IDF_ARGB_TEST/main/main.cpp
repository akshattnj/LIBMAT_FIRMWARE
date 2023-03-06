#include "src/LEDHandler.h"
#include "src/definations.h"

extern "C"
{
    #include <stdio.h>
}

LEDHandler ledHandler;

extern "C" void app_main(void)
{
    ledHandler.init();
    xTaskCreate([](void *params){ledHandler.ledAnimationTask(params);}, "ledAnimationTask", 2048, NULL, 10, NULL);

    while (1)
    {
        ledHandler.setBatteryPercentage(100);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(50);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(25);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(0);
        vTaskDelay(4000 / portTICK_PERIOD_MS);

        ledHandler.setAnimationSelection(1);
        ledHandler.setBatteryPercentage(100);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(50);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(25);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ledHandler.setBatteryPercentage(0);
        vTaskDelay(4000 / portTICK_PERIOD_MS);

        ledHandler.setAnimationSelection(2);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}
