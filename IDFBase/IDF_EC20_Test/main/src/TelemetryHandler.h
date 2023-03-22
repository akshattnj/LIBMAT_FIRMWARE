#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "EC20Handler.h"
#include "Definations.h"
#include <cJSON.h>

class TelemetryHandler
{
public:
    /**
     * @brief Initialise Telemetry
     *
     * @param networkHandler GPS data source and network connectivity
     */
    TelemetryHandler(EC20Handler *networkHandler)
    {
        this->networkHandler = networkHandler;
    }

    /**
     * @brief Gather and send telemetry to server
     *
     */
    void sendTelemetry()
    {
        while (1)
        {
            if ((networkHandler->MQTTFlags & MQTT_CONNECT) > 0)
            {
                taskYIELD();
                this->root = cJSON_CreateObject();
#if CONFIG_EC20_ENABLE_GPS
                if (networkHandler->getLocationValid())
                {
                    cJSON_AddNumberToObject(root, "lat", networkHandler->latitude);
                    cJSON_AddNumberToObject(root, "lon", networkHandler->longitude);
                }
#endif
                cJSON_AddNumberToObject(root, "testData", esp_timer_get_time() / 1000);
                char *telemetry = cJSON_PrintUnformatted(root);
                ESP_LOGI(TELEMETRY_TAG, "%s", telemetry);
                networkHandler->pauseGPS = true;
                bool sent = networkHandler->sendTelemetry(telemetry, TELEMETRY_TOPIC);
                networkHandler->pauseGPS = false;
                if (sent)
                    ESP_LOGI(TELEMETRY_TAG, "Send Success");
                else
                {
                    ESP_LOGE(TELEMETRY_TAG, "Send failed");
                }
                cJSON_Delete(root);
                taskYIELD();
            }
            vTaskDelay(3000 / portTICK_RATE_MS);
        }
    }

private:
    EC20Handler *networkHandler;
    cJSON *root;
};

#endif