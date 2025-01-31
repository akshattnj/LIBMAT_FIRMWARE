#ifndef WIFI_AP_H
#define WIFI_AP_H

#include "../definations.h"

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>

extern uint8_t WiFiFlags;

void startNVS();
void initWiFiAP();
void connectWiFi();

#endif