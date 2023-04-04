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

#include "src/definations.h"
#include "src/EC20/EC20.h"

extern "C"
{
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
}

extern "C" void app_main(void)
{
    
    xTaskCreate(EC20::commandControl, "Command Control", EC20_STACK_SIZE, NULL, 10, NULL);
    
    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        taskYIELD();
    }
}
