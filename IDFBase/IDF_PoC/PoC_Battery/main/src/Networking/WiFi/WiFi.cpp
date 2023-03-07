#include "WiFi.h"

namespace WiFi
{
    uint8_t retryCount = 0;

    void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        if (event_id == WIFI_EVENT_AP_STACONNECTED)
        {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(WIFI_TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
        }
        else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
        {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(WIFI_TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            Commons::WiFiFlags = Commons::WiFiFlags & (~BIT1);
            if (Commons::WiFiFlags & BIT0)
            {
                wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGI(WIFI_TAG, "WiFi connection failed with reason %d", event->reason);
                ESP_LOGI(WIFI_TAG, "WiFi connect failed. Retrying...");
                ESP_ERROR_CHECK(esp_wifi_connect());
                retryCount++;
                if (retryCount > ESP_MAXIMUM_RETRY)
                {
                    Commons::WiFiFlags = Commons::WiFiFlags & (~BIT0);
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
            Commons::WiFiFlags = (Commons::WiFiFlags | BIT1) & (~BIT0);
            retryCount = 0;
        }
    }

    void initialiseWiFiSoftAP()
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        esp_netif_t *p_netif = esp_netif_create_default_wifi_ap();
        assert(p_netif);
        esp_netif_ip_info_t if_info;
        ESP_ERROR_CHECK(esp_netif_get_ip_info(p_netif, &if_info));
        ESP_LOGI(WIFI_TAG, "ESP32 IP:" IPSTR, IP2STR(&if_info.ip));

        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);

        wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&config));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL));

        wifi_config_t wifiConfig = {0};
        strncpy((char *)wifiConfig.ap.ssid, ESP_WIFI_SSID, strlen(ESP_WIFI_SSID) + 1);
        strncpy((char *)wifiConfig.ap.password, ESP_WIFI_PASS, strlen(ESP_WIFI_PASS) + 1);
        wifiConfig.ap.ssid_len = strlen(ESP_WIFI_SSID);
        wifiConfig.ap.channel = ESP_WIFI_CHANNEL;
        wifiConfig.ap.max_connection = MAX_STA_CONN;
        wifiConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    void connectToWiFi(char *SSID, char *password)
    {
        ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", SSID, password);
        // reconfigure wifi
        wifi_config_t wifiStaConfig = {0};
        memset(&wifiStaConfig, 0, sizeof(wifiStaConfig));
        strcpy((char *)wifiStaConfig.sta.ssid, SSID);
        strcpy((char *)wifiStaConfig.sta.password, password);
        wifiStaConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiStaConfig));
        ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", wifiStaConfig.sta.ssid, wifiStaConfig.sta.password);
        Commons::WiFiFlags = Commons::WiFiFlags | BIT0;
        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    void disconnectWiFi()
    {
        Commons::WiFiFlags = Commons::WiFiFlags & (~BIT0);
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }
}