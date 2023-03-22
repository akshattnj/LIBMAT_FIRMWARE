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
#include "src/TelemetryHandler.h"

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
TelemetryHandler telemetryHandler(&testEC20);

extern "C" void app_main(void)
{
    testEC20.begin();
    xTaskCreate([](void *args)
                { testEC20.portListner(); },
                "EC20 Reader", 4096, NULL, 10, NULL);
    testEC20.setup();

#if CONFIG_EC20_ENABLE_GPS
    xTaskCreate([](void *args)
                { testEC20.getGPSData(args); },
                "EC20 GPS", 4096, NULL, 10, NULL);
#endif

#if CONFIG_EC20_ENABLE_MQTT
    testEC20.connect();
    xTaskCreate([](void *args)
                { telemetryHandler.sendTelemetry(); },
                "Telemetry", 4096, NULL, 10, NULL);
    
    xTaskCreate([](void *args)
                { testEC20.MQTTConnectionManager(); },
                "MQTT Connector", 2048, NULL, 10, NULL);
#endif

    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        taskYIELD();
    }
}
