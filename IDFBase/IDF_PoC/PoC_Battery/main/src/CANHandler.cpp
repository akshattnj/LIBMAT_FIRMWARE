#include "CANHandler.h"

namespace CANHandler
{
    QueueHandle_t txTaskQueue;

    gpio_num_t txTWAI = TWAI_TX;
    gpio_num_t rxTWAI = TWAI_RX;

    uint32_t identifierHeader = 0x0B0;
    int64_t bmsTimeout = 0x00;

    typedef struct TWAITaskParameters{
        uint32_t expectedIdentifier;
        uint8_t taskType;
    } TWAITaskParameters;

    esp_err_t startTWAI()
    {
        ESP_LOGI(TWAI_TAG, "Starting TWAI");
        txTaskQueue = xQueueCreate(1, sizeof(TWAITaskParameters));
        gpio_reset_pin(txTWAI);
        gpio_reset_pin(rxTWAI);
        const twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
        const twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        const twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(txTWAI, rxTWAI, TWAI_MODE_NORMAL);
        twai_reconfigure_alerts(TWAI_ALERT_ALL, NULL);
        esp_err_t errorCode = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
        if(errorCode != ESP_OK)
        {
            ESP_LOGE(TWAI_TAG, "Error installing TWAI driver: %s", esp_err_to_name(errorCode));
            return errorCode;
        }

        xTaskCreate(taskReceiveTWAI, "CAN Receive", 4096, NULL, 10, NULL);
        xTaskCreate(taskSendTWAI, "CAN Send", 4096, NULL, 10, NULL);

        return errorCode;
    }

    esp_err_t endTWAI()
    {
        ESP_LOGI(TWAI_TAG, "Ending TWAI");
        vQueueDelete(txTaskQueue);
        ESP_ERROR_CHECK(twai_stop());
        return twai_driver_uninstall();
    }

    void taskReceiveTWAI(void *params)
    {
        TWAITaskParameters parameters;
        twai_message_t rxMessage;
        ESP_ERROR_CHECK(twai_start());

        while (1)
        {
            memset(&rxMessage, 0, sizeof(twai_message_t));
            if(twai_receive(&rxMessage, 1000 / portTICK_PERIOD_MS) == ESP_OK)
            {
                Commons::animationSelection = 0;
                // ESP_LOGI(TWAI_TAG, "Received message with identifier: 0x%08x", rxMessage.identifier);
                // if(rxMessage.data_length_code != 0)
                // {
                //     for(int i = 0; i < rxMessage.data_length_code; i++)
                //         printf("0x%02x ", rxMessage.data[i]);
                //     printf("\n");
                // }

                if(identifierHeader == 0x00)
                {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    continue;
                }
                if (rxMessage.identifier == (ID_MASTER_PING | identifierHeader))
                {
                    ESP_LOGI(TWAI_TAG, "Received master ping");
                    parameters.expectedIdentifier = ID_PING_RESP;
                    parameters.taskType = 0;
                    xQueueSend(txTaskQueue, &parameters, portMAX_DELAY);
                    continue;
                }
                if(rxMessage.identifier == BMS_STATE_ID)
                {
                    bmsTimeout = esp_timer_get_time();
                    Commons::batteryPercentage = (uint8_t)((float)(rxMessage.data[0] << 8 | rxMessage.data[1]) * 0.01);
                    ESP_LOGI(TWAI_TAG, "Got battery percent: %d", Commons::batteryPercentage);
                    // WiFi scan only if CAN data is valid and battery is less then 40%
                    if(Commons::batteryPercentage > 40.00 || Commons::animationSelection == 3)
                    {
                        Commons::WiFiFlags = Commons::WiFiFlags | BIT2; // Stop WiFi Scan
                    }
                    else
                    {
                        Commons::WiFiFlags = Commons::WiFiFlags & (~BIT2); // Start WiFi Scan
                    }
                    continue;
                }
                if (rxMessage.identifier == BMS_VOLTAGE_ID)
                {
                    bmsTimeout = esp_timer_get_time();
                    uint32_t voltage = ((uint32_t)rxMessage.data[0] << 24) | ((uint32_t)rxMessage.data[1] << 16) | ((uint32_t)rxMessage.data[2] << 8) | rxMessage.data[3];
                    Commons::batteryVoltage = (float)voltage * 0.001;
                    ESP_LOGI(TWAI_TAG, "Got battery voltage: %.3f", Commons::batteryVoltage);
                    continue;
                }
            }
            else
            {
                if(esp_timer_get_time() - bmsTimeout > 10000000)
                {
                    Commons::batteryPercentage = 0;
                    Commons::batteryVoltage = 0.00;
                    Commons::animationSelection = 3;
                }
            }
            
        }
        vTaskDelete(NULL);
    }

    void taskSendTWAI(void *params)
    {
        TWAITaskParameters parameters;
        while (1)
        {
            if (xQueueReceive(txTaskQueue, &parameters, portMAX_DELAY) == pdTRUE)
            {
                if (parameters.taskType == 0)
                {
                    ESP_LOGI(TWAI_TAG, "Sending ping request");
                    twai_message_t pingMessage = {.ss = 1, .identifier = parameters.expectedIdentifier, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
                    esp_err_t error = twai_transmit(&pingMessage, portMAX_DELAY);
                    if(error != 0)
                    {
                        ESP_LOGE(TWAI_TAG, "Error sending ping request: %d", error);
                    }
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }
            }
        }
        vTaskDelete(NULL);
    }
}
