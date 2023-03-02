#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

extern "C"
{
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
}

#include <string.h>
#include "./definations.h"

namespace WiFi
{

    uint8_t WiFiFlags = 0x00;
    uint8_t retryCount = 0;

    static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            WiFiFlags = WiFiFlags & (~WIFI_CONNECTED);
            if (WiFiFlags & WIFI_CONNECTING)
            {
                wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGI(WIFI_TAG, "WiFi connection failed with reason %d", event->reason);
                ESP_LOGI(WIFI_TAG, "WiFi connect failed. Retrying...");
                ESP_ERROR_CHECK(esp_wifi_connect());
                retryCount++;
                if (retryCount > ESP_MAXIMUM_RETRY)
                {
                    WiFiFlags = WiFiFlags & (~WIFI_CONNECTING);
                    retryCount = 0;
                    ESP_LOGE(WIFI_TAG, "Retry count exceeded");
                }
            }
            else
            {
                ESP_LOGE(WIFI_TAG, "WiFi connection lost");
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(WIFI_TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            WiFiFlags = (WiFiFlags | WIFI_CONNECTED) & (~WIFI_CONNECTING);
        }
    }

    void initialiseWiFiSTA()
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);

        wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&config));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(WIFI_TAG, "WiFi initial setup complete");
    }

    void connectWiFi()
    {
        wifi_config_t wifiStaConfig = {0};
        strcpy((char *)wifiStaConfig.sta.ssid, SSID);
        strcpy((char *)wifiStaConfig.sta.password, PASSWORD);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiStaConfig));
        WiFiFlags = WiFiFlags | BIT0;
        ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", wifiStaConfig.sta.ssid, wifiStaConfig.sta.password);
        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    void startNVS()
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
    }
}
#endif