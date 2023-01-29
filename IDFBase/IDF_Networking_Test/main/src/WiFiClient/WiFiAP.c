#include "WiFiAP.h"

uint8_t WiFiFlags = 0; // {0 - WiFi connecting, 1 - WiFi Connected}

/**
 * @brief Initialise Non Volatile Storage. Used by BLE and WiFi client to store keys and calibration data
 * 
 */
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

static void WiFiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    { 
        esp_wifi_connect();
        WiFiFlags = WiFiFlags | BIT0;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        WiFiFlags = WiFiFlags & (~BIT1);
        if(WiFiFlags & BIT0)
        {
            ESP_LOGI(WIFI_TAG, "WiFi connection failed");
            ESP_LOGI(WIFI_TAG, "WiFi connect failed. Retrying...");
            esp_wifi_connect();
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
        WiFiFlags = (WiFiFlags | BIT1) & (~BIT0);
    }
}

void initWiFiAP()
{
    // Initialise WiFi settings
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifiInitial = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitial));

    // Register WiFi events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiEventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiEventHandler, NULL));

    // Configure WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "WiFi initialised");
}

bool isWiFiConnecter() {
    return ((WiFiFlags & BIT1) > 0);
}