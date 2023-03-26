#include "WiFi.h"

namespace WiFi
{
    typedef struct APData
    {
        char *ssid;
        char *password;
        uint8_t bssid[6];
    } APData;

    APData knownAPs[2] = {
        {.ssid = "HUB_1",
         .password = "HUB_1_TEST",
         .bssid = {112, 184, 246, 149, 77, 133}},
        {
            0,
        }};

    APData backupNetAP = {0};
    const uint8_t knownAPCount = 1;
    uint8_t retryCount = 0;
    QueueHandle_t WiFiConnectQueue;

    static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
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
                    Commons::WiFiFlags = Commons::WiFiFlags & (~BIT2);
                }
            }
            else
            {
                ESP_LOGE(WIFI_TAG, "WiFi connection lost");
                Commons::WiFiFlags = Commons::WiFiFlags & (~BIT2);
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(WIFI_TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            Commons::WiFiFlags = (Commons::WiFiFlags | BIT1) & (~BIT0);
            if ((Commons::WiFiFlags & BIT3) > 0)
            {
                Commons::WiFiFlags = (Commons::WiFiFlags) & (~BIT2);
            }
            else
            {
                Commons::WiFiFlags = (Commons::WiFiFlags | BIT2) & (~BIT3);
                Commons::wsFlags = BIT0;
            }
            retryCount = 0;
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
        WiFiConnectQueue = xQueueCreate(2, sizeof(struct APData));

        ESP_LOGI(WIFI_TAG, "WiFi initial setup complete");
    }

    void disconnectWiFi()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }

    void stopWiFi()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        Commons::WiFiFlags = BIT2;
        ESP_ERROR_CHECK(esp_wifi_stop());
    }

    int8_t compareKnownAPs(char *ssid, uint8_t bssid[6])
    {
        for (int i = 0; i < knownAPCount; i++)
        {
            if (strncmp(knownAPs->ssid, ssid, strlen(knownAPs->ssid)) == 0)
            {
                return i;
                // if (memcmp(knownAPs->bssid, bssid, 6) == 0)
                // {
                //     ESP_LOGI(WIFI_TAG, "Known AP found");
                //     return i;
                // }
            }
        }
        return -1;
    }

    void taskAutoConnect(void *args)
    {
        while (1)
        {
            if ((Commons::WiFiFlags & BIT2) == 0)
            {
                wifi_ap_record_t apInfo[MAX_WIFI_LIST_SIZE];
                uint16_t apCount = MAX_WIFI_LIST_SIZE;
                memset(apInfo, 0, sizeof(apInfo));
                ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
                ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, apInfo));
                ESP_LOGI(WIFI_TAG, "Total APs scanned = %u", apCount);
                for (int i = 0; (i < MAX_WIFI_LIST_SIZE) && (i < apCount); i++)
                {
                    int8_t apNum = compareKnownAPs((char *)apInfo[i].ssid, apInfo[i].bssid);
                    if (apNum > -1)
                    {
                        xQueueSend(WiFiConnectQueue, (void *)&(knownAPs[i]), 0);
                        Commons::WiFiFlags = (Commons::WiFiFlags | BIT2) & (~BIT3);
                        break;
                    }
                    ESP_LOGD(WIFI_TAG, "SSID \t\t%s", apInfo[i].ssid);
                    ESP_LOGD(WIFI_TAG, "RSSI \t\t%d", apInfo[i].rssi);
                    ESP_LOGD(WIFI_TAG, "Channel \t\t%d", apInfo[i].primary);
                    ESP_LOGD(WIFI_TAG, "BSSID \t\t%d:%d:%d:%d:%d:%d\n", apInfo[i].bssid[0], apInfo[i].bssid[1], apInfo[i].bssid[2], apInfo[i].bssid[3], apInfo[i].bssid[4], apInfo[i].bssid[5]);
                }
                ESP_ERROR_CHECK(esp_wifi_scan_stop());
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void taskWiFiConnect(void *args)
    {
        struct APData apInfo;

        while (1)
        {
            if (xQueueReceive(WiFiConnectQueue, &apInfo, 0) == pdTRUE)
            {
                ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", apInfo.ssid, apInfo.password);
                if(apInfo.ssid == NULL)
                    continue;
                if ((Commons::WiFiFlags & BIT1) > 0)
                {
                    ESP_ERROR_CHECK(esp_wifi_disconnect());
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    Commons::WiFiFlags = Commons::WiFiFlags & (~BIT1);
                }
                // reconfigure wifi
                wifi_config_t wifiStaConfig = {0};
                memset(&wifiStaConfig, 0, sizeof(wifiStaConfig));
                strcpy((char *)wifiStaConfig.sta.ssid, apInfo.ssid);
                strcpy((char *)wifiStaConfig.sta.password, apInfo.password);
                wifiStaConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiStaConfig));
                Commons::WiFiFlags = Commons::WiFiFlags | BIT0 | BIT2;
                esp_wifi_scan_stop();
                ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", wifiStaConfig.sta.ssid, wifiStaConfig.sta.password);
                ESP_ERROR_CHECK(esp_wifi_connect());
            }
            vTaskDelay(1);
        }   
    }
}