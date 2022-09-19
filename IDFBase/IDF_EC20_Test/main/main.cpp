/**
 * @file main.cpp
 * @author Antony Kuruvilla (ajosekuruvilla@gmail.com)
 * @brief Tests the UART functionality with the EC20 Chipset
 * @version 1.0
 * @date 2022-09-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "src/Definations.h"
#include "src/EC20Handler.h"

extern "C"
{
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
}

EC20Handler testEC20;

extern "C" void app_main(void)
{
    testEC20.begin();
    xTaskCreate([](void *args)
                { testEC20.portListner(); },
                "EC20 Reader", 2048, NULL, 10, NULL);
    xTaskCreate([](void *args)
                { testEC20.getGPSData(args); },
                "EC20 GPS", 2048, NULL, 10, NULL);
    testEC20.setup();
    testEC20.connect();
    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        taskYIELD();
    }
}
