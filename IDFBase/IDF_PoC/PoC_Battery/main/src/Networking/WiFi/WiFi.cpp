#include "WiFi.h"

namespace WiFi
{
    typedef struct APData
    {
        char *ssid;
        char *password;
        uint8_t bssid[6];
    } APData;

    /**
     * @brief Known Access Points
    */
    APData knownAPs[2] = {
        {.ssid = "HUB_1",
         .password = "HUB_1_TEST",
         .bssid = {112, 184, 246, 149, 77, 133}},
        {
            0,
        }};

    const uint8_t knownAPCount = 1;

    // Retuy count kept global to be used in the event handler
    uint8_t retryCount = 0;
    QueueHandle_t WiFiConnectQueue;

    /**
     * @brief WiFi event handler
     * @param arg Arguemnts specific to event - relavent to custom events
     * @param event_base Event base - What group of event
     * @param event_id Event ID - What event tyoe within the group
     * @param event_data Event data - Event specific data
     * 
     * The event handler needs to mainly deal with only two groupes of events : WiFi Events and IP Events
     * 1. WiFi Disconnect event can mean either WiFi connection failed or disconnection. So in case of disconnect event
     *     1.1. Resect the WiFi connected bit : Bit 1
     *     1.2. Check if the WiFi Connecting bit is set : Bit 0
     *     1.3. If the WiFi Connecting Bit is Set, attempt to reconnect to the WiFi network. Else, reset the WiFi Stop Scan Bit : Bit 2
     *     1.4. If the WiFi Connecting Bit is Set, check if the retry count exceeds the maximum retry count. If yes, reset the WiFi Connecting Bit : Bit 0
     * 2. An IP event means that the WiFi connection is successful. So in case of IP event
     *     2.1. Set the WiFi Connected Bit : Bit 1
     *     2.2. Set the WiFi Stop Scan Bit : Bit 2
     *     2.3. Reset the WiFi Connecting Bit : Bit 0
     */
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
            Commons::WiFiFlags = (Commons::WiFiFlags | BIT1 | BIT2) & (~BIT0);
            Commons::wsFlags = BIT0;
            retryCount = 0;
        }
    }

    /**
     * @brief WiFi initialisation function.
     * 
     * This function initialises the WiFi interface, sets up the WiFi event handler, creates the WiFi connect queue and autoconnect tasks.
    */
    void initialiseWiFiSTA()
    {
        // Set up network interface and event handler
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        // Create WiFi Station
        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);

        // Set up WiFi configuration
        wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&config));

        // Set up WiFi event handler
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL));

        // Set WiFi mode to Station and start WiFi
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());

        // Create WiFi Connect Queues and Tasks
        WiFiConnectQueue = xQueueCreate(2, sizeof(struct APData));
        xTaskCreate(taskWiFiConnect, "WiFiConnect", 4096, NULL, 5, NULL);
        xTaskCreate(taskAutoConnect, "AutoConnect", 4096, NULL, 5, NULL);

        ESP_LOGI(WIFI_TAG, "WiFi initial setup complete");
    }

    /**
     * @brief WiFi disconnect function
    */
    void disconnectWiFi()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }

    /**
     * @brief WiFi disconnect and turn off
    */
    void stopWiFi()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        Commons::WiFiFlags = BIT2;
        ESP_ERROR_CHECK(esp_wifi_stop());
    }

    /**
     * @brief Compare incoming WiFi AP with known WiFi APs
     * @param ssid SSID of the incoming WiFi AP
     * @param bssid BSSID of the incoming WiFi AP
     * 
     * @return int8_t Index of the known WiFi AP in the knownAPs array
     * 
    */
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

    /**
     * @brief WiFi autoconnect task
     * @param args Task arguments
     * 
     * The code above does the following:
     * 1. Check if the WiFi Stop Scan bit is set. If not, then continue. 
     * 2. Scan for WiFi access points and store the results in the apInfo array.
     * 3. Search through the apInfo array for any known access points. If found, then connect to the access point.
     * 4. If not found, then wait 1 second and repeat the process.
    */
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
                        // Setting Stop Scan bit to prevent issues during connect process
                        Commons::WiFiFlags = Commons::WiFiFlags | BIT2;
                        xQueueSend(WiFiConnectQueue, (void *)&(knownAPs[i]), 0);
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

    /**
     * @brief WiFi connect task
     * @param args Task arguments
     * 
     * The code below does the following:
     * 1. The task calls xQueueReceive(WiFiConnectQueue, &apInfo, portMAX_DELAY) to block on the queue. The task will remain blocked until it receives a message on the queue. The message will be copied to the apInfo variable.
     * 2. The task then calls esp_wifi_disconnect() to disconnect from the current AP if the WiFi is connected.
     * 3. The task then calls esp_wifi_set_config() to configure the WiFi station with the SSID and password from the APData object.
     * 4. The WiFi Connecting and Stop Scan bits are set.
     * 4. The task then calls esp_wifi_connect() to connect to the AP.
     * 
    */
    void taskWiFiConnect(void *args)
    {
        struct APData apInfo;

        while (1)
        {
            if (xQueueReceive(WiFiConnectQueue, &apInfo, portMAX_DELAY) == pdTRUE)
            {
                ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", apInfo.ssid, apInfo.password);
                // Check for valid AP info
                if(apInfo.ssid == NULL)
                    continue;

                // disconnect from current AP if connected
                if ((Commons::WiFiFlags & BIT1) > 0)
                {
                    ESP_ERROR_CHECK(esp_wifi_disconnect());
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    Commons::WiFiFlags = Commons::WiFiFlags & (~BIT1);
                }
                // Reconfigure wifi
                wifi_config_t wifiStaConfig = {0};
                memset(&wifiStaConfig, 0, sizeof(wifiStaConfig));
                strcpy((char *)wifiStaConfig.sta.ssid, apInfo.ssid);
                strcpy((char *)wifiStaConfig.sta.password, apInfo.password);
                wifiStaConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiStaConfig));
                
                // Stop scan and set connecting bits
                Commons::WiFiFlags = Commons::WiFiFlags | BIT0 | BIT2;
                esp_wifi_scan_stop();
                
                // Connect to WiFi
                ESP_LOGI(WIFI_TAG, "Connecting to Wifi SSID: %s, Password: %s", wifiStaConfig.sta.ssid, wifiStaConfig.sta.password);
                ESP_ERROR_CHECK(esp_wifi_connect());
            }
        }
    }
}