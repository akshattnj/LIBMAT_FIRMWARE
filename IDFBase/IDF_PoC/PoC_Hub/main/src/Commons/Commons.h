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
}

namespace Commons 
{
    extern uint8_t WiFiFlags;
    extern uint8_t WSFlags;

    void startNVS();
}


#endif