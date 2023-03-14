#ifndef TWAI_HANDLER_H
#define TWAI_HANDLER_H

extern "C"
{
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/twai.h>
}

#include <string.h>
#include "definations.h"

/**
 * @brief Handler class for TWAI
*/
class TWAIHandler
{
public:
    /**
     * @brief Construct a new TWAI Handler object
    */
    TWAIHandler(gpio_num_t txTwai, gpio_num_t rxTwai)
    {
        this->txTWAI = txTwai;
        this->rxTWAI = rxTwai;
    }

    void setIdentifierHeader(uint8_t identifierHeader)
    {
        this->identifierHeader = identifierHeader << 4;
    }

    /**
     * @brief Build semaphores, queues and install TWAI driver
     */
    esp_err_t startTWAI()
    {
        ESP_LOGI(TWAI_TAG, "Starting TWAI");
        txTaskQueue = xQueueCreate(1, sizeof(TWAITaskParameters));
        gpio_reset_pin(txTWAI);
        gpio_reset_pin(rxTWAI);
        const twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_25KBITS();
        const twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        const twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(txTWAI, rxTWAI, TWAI_MODE_NORMAL);
        twai_reconfigure_alerts(TWAI_ALERT_ALL, NULL);
        return twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
    }

    /**
     * @brief Delete semaphores, queues and delete twai driver
     */
    esp_err_t endTWAI()
    {
        ESP_LOGI(TWAI_TAG, "Ending TWAI");
        vQueueDelete(txTaskQueue);
        ESP_ERROR_CHECK(twai_stop());
        return twai_driver_uninstall();
    }

    /**
     * @brief Task to receive TWAI messages
     */
void taskReceiveTWAI(void *params)
{
    TWAITaskParameters parameters;
    twai_message_t rxMessage;
    ESP_ERROR_CHECK(twai_start());
    while (1)
    {
        memset(&rxMessage, 0, sizeof(twai_message_t));
        twai_receive(&rxMessage, portMAX_DELAY);
        ESP_LOGI(TWAI_TAG, "Received message with identifier: 0x%08x", rxMessage.identifier);
        if(rxMessage.data_length_code != 0)
        {
            for(int i = 0; i < rxMessage.data_length_code; i++)
                printf("0x%02x ", rxMessage.data[i]);
            printf("\n");
        }
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
        else if(rxMessage.identifier == (ID_MASTER_REQUEST | identifierHeader))
        {
            ESP_LOGI(TWAI_TAG, "Received master request");
            parameters.expectedIdentifier = ID_REQUEST_RESP;
            parameters.taskType = 1;
            xQueueSend(txTaskQueue, &parameters, portMAX_DELAY);
            continue;
        }
        else if(rxMessage.identifier == (ID_MASTER_DATA | identifierHeader))
        {
            ESP_LOGI(TWAI_TAG, "Received master data");
            if ((rxMessage.data_length_code >= 8) && (rxMessage.data[0] == 0x00) && (rxMessage.data[1] == 0x00) && ((rxMessage.data[2] & 0xE0) == 0x60))
            {
                uint32_t batteryVoltage = ((rxMessage.data[2] & 0x1F) << 16) | (rxMessage.data[3] << 8) | rxMessage.data[4];
                float voltage = batteryVoltage / 1000.0f;
                printf("Battery voltage = %fV\n", voltage);
            }
            parameters.expectedIdentifier = ID_DATA_RESP;
            parameters.taskType = 2;
            xQueueSend(txTaskQueue, &parameters, portMAX_DELAY);
            continue;
        }
    }
    vTaskDelete(NULL);
}


    /**
     * @brief Task to send TWAI messages
     */
    void taskSendTWAI(void *params)
    {
        TWAITaskParameters parameters;
        while (1)
        {
            if (xQueueReceive(txTaskQueue, &parameters, portMAX_DELAY) == pdTRUE)
            {
                if (parameters.taskType == 0)
                {
                    ESP_LOGI(TWAI_TAG, "Sending ping response");
                    twai_message_t pingMessage = {.identifier = parameters.expectedIdentifier, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
                    esp_err_t error = twai_transmit(&pingMessage, portMAX_DELAY);
                    if(error != 0)
                    {
                        ESP_LOGE(TWAI_TAG, "Error sending ping request: %d", error);
                    }
                }
                else if(parameters.taskType == 1) {
                    ESP_LOGI(TWAI_TAG, "Sending request response");
                    char toTransmit[50];
                    size_t size = sprintf(toTransmit, "{\"CusID\":\"%s\"}", this->assignedCustomer);
                    twai_message_t requestMessage;
                    if(size <= 8)
                    {
                        requestMessage.data_length_code = size;
                        memcpy(requestMessage.data, toTransmit, size);
                        twai_transmit(&requestMessage, portMAX_DELAY);
                    }
                    else
                    {
                        uint8_t chunks = size / 8;
                        uint8_t remainder = size % 8;
                        for(int i = 0; i < chunks; i++)
                        {
                            memset(&requestMessage, 0, sizeof(twai_message_t));
                            requestMessage.identifier = parameters.expectedIdentifier;
                            requestMessage.data_length_code = 8;
                            memcpy(requestMessage.data, toTransmit + (i * 8), 8);
                            twai_transmit(&requestMessage, portMAX_DELAY);
                            ESP_LOGI(TWAI_TAG, "Sent chunk %d %s", i, (char *)requestMessage.data);
                        }
                        memset(&requestMessage, 0, sizeof(twai_message_t));
                        requestMessage.identifier = parameters.expectedIdentifier;
                        requestMessage.data_length_code = remainder;
                        memcpy(requestMessage.data, toTransmit + (chunks * 8), remainder);
                        twai_transmit(&requestMessage, portMAX_DELAY);
                    }
                    memset(&requestMessage, 0, sizeof(twai_message_t));
                    requestMessage.identifier = (ID_MASTER_DONE | identifierHeader);
                    requestMessage.data_length_code = 0;
                    twai_transmit(&requestMessage, portMAX_DELAY);
                }
                else if(parameters.taskType == 2) {
                    ESP_LOGI(TWAI_TAG, "Sending data response");
                    twai_message_t dataMessage = {.identifier = parameters.expectedIdentifier, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
                    esp_err_t error = twai_transmit(&dataMessage, portMAX_DELAY);
                    if(error != 0)
                    {
                        ESP_LOGE(TWAI_TAG, "Error sending data response: %d", error);
                    }
                }
            }
        }
        vTaskDelete(NULL);
    }

private:
    /**
     * @brief Wrapper class for TWAI parameters
    */
    typedef struct TWAITaskParameters{
        uint32_t expectedIdentifier;
        uint8_t taskType;
    } TWAITaskParameters;

    gpio_num_t txTWAI;
    gpio_num_t rxTWAI;
    uint32_t identifierHeader = 0x0B0;
    char *assignedCustomer = "TestCustomer";
    QueueHandle_t txTaskQueue;
};

#endif