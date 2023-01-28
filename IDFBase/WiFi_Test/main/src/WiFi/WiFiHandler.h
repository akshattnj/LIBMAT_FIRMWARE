#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#include "../definations.h"

void initialiseNVS();
void initialiseWiFiSoftAP();


#endif