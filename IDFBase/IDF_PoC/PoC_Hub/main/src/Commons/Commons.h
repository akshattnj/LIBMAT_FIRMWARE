#ifndef COMMONS_H
#define COMMONS_H

#include "../definations.h"
#include "../BatteryHandler.h"
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
    extern uint8_t WiFiFlags;
    extern uint8_t WSFlags;
    extern bool activeDoor;
    extern uint8_t batteryPercentage[4];
    extern uint8_t animationSelection[4];

    extern SemaphoreHandle_t semaphoreCAN;
    extern QueueHandle_t queueCAN;

    void startNVS();
    void sendPostMessage();
}


#endif