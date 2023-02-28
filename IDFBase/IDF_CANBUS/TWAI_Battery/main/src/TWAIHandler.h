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

    /**
     * @brief Build semaphores, queues and install TWAI driver
     */
    esp_err_t startTWAI()
    {
        ESP_LOGI(TWAI_TAG, "Starting TWAI");
        txTaskQueue = xQueueCreate(1, sizeof(TWAIParameters));
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
        TWAIParameters parameters;
        ESP_ERROR_CHECK(twai_start());
        twai_message_t rxMessage;
        while (1)
        {
            ESP_LOGI(TWAI_TAG, "Waiting for master ping");
            memset(&rxMessage, 0, sizeof(twai_message_t));
            twai_receive(&rxMessage, portMAX_DELAY);
            if (rxMessage.identifier == ID_MASTER_PING)
            {
                ESP_LOGI(TWAI_TAG, "Received master ping");
                parameters.setTwaiParameters(ID_PING_RESP, 0);
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
        TWAIParameters parameters;
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
            }
        }
        vTaskDelete(NULL);
    }

private:
    /**
     * @brief Wrapper class for TWAI parameters
    */
    class TWAIParameters
    {
    public:
        uint32_t expectedIdentifier;
        uint8_t taskType;

        void setTwaiParameters(uint32_t id, uint8_t task)
        {
            this->expectedIdentifier = id;
            this->taskType = task;
        }
    };

    gpio_num_t txTWAI;
    gpio_num_t rxTWAI;
    const twai_message_t requestMessage = {.identifier = ID_MASTER_REQUEST, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
    const twai_message_t doneMessage = {.identifier = ID_MASTER_DONE, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};

    QueueHandle_t txTaskQueue;
};

#endif