#ifndef MQTT_H
#define MQTT_H

extern "C"
{
#include <mqtt_client.h>
}

#include "WiFiHandler.h"
#include "definations.h"

namespace MQTT
{

    uint8_t mqttFlags; // {BIT0 - Connected}
    esp_mqtt_client_handle_t client;

    esp_mqtt_client_config_t mqttConfig = {
        .host = "mqtt://demo.thingsboard.io",
        .uri = "mqtt://demo.thingsboard.io:1883",
        .port = 1883,
        .client_id = "test",
        .username = DEVICE_ACCESS_ID,
        .password = NULL,
    };

    static void log_error_if_nonzero(const char *message, int error_code)
    {
        if (error_code != 0)
        {
            ESP_LOGE(MQTT_TAG, "Last error %s: 0x%x", message, error_code);
        }
    }

    static void mqttEventHandler(void *args, esp_event_base_t base, int32_t eventId, void *eventData)
    {
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)eventData;
        switch ((esp_mqtt_event_id_t)eventId)
        {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
            mqttFlags = mqttFlags | BIT0;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
            mqttFlags = mqttFlags & (~BIT0);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            // ESP_LOGI(MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(MQTT_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
            break;
        }
    }

    void mqttSetup()
    {
        mqttFlags = 0x00;
        client = esp_mqtt_client_init(&mqttConfig);
        esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqttEventHandler, NULL);
    }

    void connectMQTT()
    {
        xTaskCreate([](void *args) {
            while(1)
            {
                ESP_LOGD(MQTT_TAG, "%X", WiFi::WiFiFlags);
                if((WiFi::WiFiFlags & WIFI_CONNECTED) > 0) 
                {
                    esp_mqtt_client_start((esp_mqtt_client_handle_t)args);
                    break;
                }
            vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            vTaskDelete(NULL); 
        }, "MQTT Connect check", 2048, client, 10, NULL);
    }
}

#endif