#ifndef COMMONS_H
#define COMMONS_H

#include "../definations.h"
#include <string.h>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <cJSON.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_http_client.h>
}

namespace Commons 
{
    extern uint8_t WiFiFlags; // {BIT0 - Connect flag, BIT1 - connected flag, BIT2 - Stop scan bit}
    extern uint8_t wsFlags;
    extern uint8_t batteryPercentage;
    extern uint8_t animationSelection;
    extern float batteryVoltage;
    extern double longitude;
    extern double latitude;

    extern SemaphoreHandle_t semaphoreCAN;
    extern QueueHandle_t queueCAN;

    void startNVS();
}


#endif