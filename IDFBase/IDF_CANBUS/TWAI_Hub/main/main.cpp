extern "C"
{
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "src/TWAIHandler.h"
}
#include "src/definations.h"

TWAIHandler handleTWAI((gpio_num_t)TWAI_TX, (gpio_num_t)TWAI_RX);

extern "C" void app_main(void)
{
    handleTWAI.startTWAI();
    xTaskCreate([](void *params)
                { handleTWAI.taskReceiveTWAI(params); },
                "Receive TWAI", 4096, NULL, 10, NULL);
    xTaskCreate([](void *params)
                { handleTWAI.taskSendTWAI(params); },
                "Send TWAI", 4096, NULL, 10, NULL);
    xTaskCreate([](void *params)
                { handleTWAI.taskControlTWAI(params); },
                "Control TWAI", 4096, NULL, 10, NULL);
}
