#ifndef MQTT_H
#define MQTT_H

extern "C"
{
#include <mqtt_client.h>
#include <driver/gpio.h>
#include <cJSON.h>
}

#include "WiFiHandler.h"
#include "definations.h"

namespace MQTT
{

    uint8_t mqttFlags; // {BIT0 - Connected}
    esp_mqtt_client_handle_t client;
    cJSON *root;
    char *data;
    char buffer[30];
    char strBuffer[10];
    int tempPin;
    bool tempState;
    uint8_t gpioFlags = 0x00;

    esp_mqtt_client_config_t mqttConfig = {
        .host = "mqtt://demo.thingsboard.io",
        .uri = "mqtt://demo.thingsboard.io:1883",
        .port = 1883,
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

    char *getGpioValues()
    {
        root = cJSON_CreateObject();
        cJSON_AddBoolToObject(root, "2", ((gpioFlags & BIT0) > 0));
        cJSON_AddBoolToObject(root, "4", ((gpioFlags & BIT1) > 0));
        cJSON_AddBoolToObject(root, "22", ((gpioFlags & BIT2) > 0));
        cJSON_AddBoolToObject(root, "19", ((gpioFlags & BIT3) > 0));
        
        data = cJSON_PrintUnformatted(root);
        ESP_LOGI(MQTT_TAG, "Data: %s", data);
        cJSON_Delete(root);
        return data;
    }

    char *getSubString(char *str, int start, int end)
    {
        int j = 0;
        for (int i = start; i < end; i++)
        {
            strBuffer[j] = str[i];
            j++;
        }
        strBuffer[j] = '\0';
        return strBuffer;
    }

    static void mqttEventHandler(void *args, esp_event_base_t base, int32_t eventId, void *eventData)
    {
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)eventData;
        switch ((esp_mqtt_event_id_t)eventId)
        {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
            mqttFlags = mqttFlags | BIT0;
            esp_mqtt_client_subscribe(client, RPC_TOPIC, 0);
            esp_mqtt_client_publish(client, ATTRIBUTE_TOPIC, getGpioValues(), 0, 0, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
            mqttFlags = mqttFlags & (~BIT0);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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
            root = cJSON_ParseWithLength(event->data, event->data_len);
            data = cJSON_GetStringValue(cJSON_GetObjectItem(root, "method"));
            ESP_LOGI(MQTT_TAG, "Method: %s", data);
            for(int i = event->topic_len - 1; i >= 0; i--)
            {
                if(event->topic[i] == '/')
                {
                    sprintf(buffer, "%s%s", RPC_RESPONSE_TOPIC, getSubString(event->topic, i + 1, event->topic_len));
                    ESP_LOGI(MQTT_TAG, "Buffer: %s", buffer);
                    break;
                }
            }
            if(strncmp(data, "getGpioStatus", 13) == 0)
            {
                cJSON_Delete(root);
                esp_mqtt_client_publish(client, buffer, getGpioValues(), 0, 0, 0);
            }
            else if(strncmp(data, "setGpioStatus", 13) == 0)
            {
                ESP_LOGI(MQTT_TAG, "Setting GPIO Status");
                cJSON_Delete(root);
                root = cJSON_ParseWithLength(event->data, event->data_len);
                tempPin = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItem(cJSON_GetObjectItem(root, "params"), "pin"));
                tempState = cJSON_IsTrue((cJSON_GetObjectItem(cJSON_GetObjectItem(root, "params"), "enabled")));
                ESP_LOGI(MQTT_TAG, "Pin: %d %d", tempPin, tempState);
                gpio_set_level((gpio_num_t)tempPin, tempState);
                cJSON_Delete(root);
                if(tempPin == 2)
                {
                    if(tempState)
                    {
                        gpioFlags = gpioFlags | BIT0;
                    }
                    else
                    {
                        gpioFlags = gpioFlags & (~BIT0);
                    }
                }
                else if(tempPin == 4)
                {
                    if(tempState)
                    {
                        gpioFlags = gpioFlags | BIT1;
                    }
                    else
                    {
                        gpioFlags = gpioFlags & (~BIT1);
                    }
                }
                else if(tempPin == 22)
                {
                    if(tempState)
                    {
                        gpioFlags = gpioFlags | BIT2;
                    }
                    else
                    {
                        gpioFlags = gpioFlags & (~BIT2);
                    }
                }
                else if(tempPin == 19)
                {
                    if(tempState)
                    {
                        gpioFlags = gpioFlags | BIT3;
                    }
                    else
                    {
                        gpioFlags = gpioFlags & (~BIT3);
                    }
                }
                // root = cJSON_CreateObject();
                // cJSON_AddNumberToObject(root, "pin", tempPin);
                // cJSON_AddBoolToObject(root, "enabled", tempState);
                // data = cJSON_PrintUnformatted(root);
                // ESP_LOGI(MQTT_TAG, "Data: %s", data);
                esp_mqtt_client_publish(client, buffer, getGpioValues(), 0, 0, 0);
                // cJSON_Delete(root);
            }
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

        gpio_config_t gpioConfig = {};
        gpioConfig.pin_bit_mask = GPIO_SEL_2 | GPIO_SEL_4 | GPIO_SEL_22 | GPIO_SEL_19;
        gpioConfig.mode = GPIO_MODE_OUTPUT;
        gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
        gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpioConfig.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&gpioConfig);
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