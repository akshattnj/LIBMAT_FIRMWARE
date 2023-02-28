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
        txTaskQueue = xQueueCreate(1, sizeof(TWAITaskParameters));
        rxTaskQueue = xQueueCreate(1, sizeof(TWAITaskParameters));
        ctrlSem = xSemaphoreCreateBinary();
        doneSem = xSemaphoreCreateBinary();
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
        vQueueDelete(rxTaskQueue);
        vSemaphoreDelete(ctrlSem);
        vSemaphoreDelete(doneSem);
        ESP_ERROR_CHECK(twai_stop());
        return twai_driver_uninstall();
    }

    /**
     * @brief Task to receive TWAI messages
     */
    void taskReceiveTWAI(void *params)
    {
        twai_message_t rxMessage;
        TWAITaskParameters parameters;
        while (1)
        {
            if (xQueueReceive(rxTaskQueue, &parameters, portMAX_DELAY) == pdTRUE)
            {
                if (parameters.taskType == 0)
                {
                    while (1)
                    {
                        ESP_LOGI(TWAI_TAG, "Waiting for ping response");
                        memset(&rxMessage, 0, sizeof(twai_message_t));
                        twai_receive(&rxMessage, portMAX_DELAY);
                        if (rxMessage.identifier == parameters.expectedIdentifier)
                        {
                            xSemaphoreGive(ctrlSem);
                            xSemaphoreGive(doneSem);
                            break;
                        }
                    }
                }
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
                    ESP_LOGI(TWAI_TAG, "Sending ping request");
                    twai_message_t pingMessage = {.ss = 1, .identifier = parameters.expectedIdentifier, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
                    while (xSemaphoreTake(doneSem, 0) != pdTRUE)
                    {
                        esp_err_t error = twai_transmit(&pingMessage, portMAX_DELAY);
                        if(error != 0)
                        {
                            ESP_LOGE(TWAI_TAG, "Error sending ping request: %d", error);
                        }
                        vTaskDelay(500 / portTICK_PERIOD_MS);
                    }
                }
            }
        }
        vTaskDelete(NULL);
    }

    /**
     * @brief Task to control the TWAI communication
     */
    void taskControlTWAI(void *params)
    {
        ESP_ERROR_CHECK(twai_start());
        TWAITaskParameters parameters;
        while (1)
        {
            ESP_LOGI(TWAI_TAG, "Starting TWAI communication");
            parameters.expectedIdentifier = ID_MASTER_PING;
            parameters.taskType = 0;
            xQueueSend(txTaskQueue, &parameters, portMAX_DELAY);
            parameters.expectedIdentifier = ID_PING_RESP;
            parameters.taskType = 0;
            xQueueSend(rxTaskQueue, &parameters, portMAX_DELAY);

            xSemaphoreTake(ctrlSem, portMAX_DELAY);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        endTWAI();
        vTaskDelete(NULL);
    }

private:
    typedef struct TWAITaskParameters{
        uint32_t expectedIdentifier;
        uint8_t taskType;
    } TWAITaskParameters;

    gpio_num_t txTWAI;
    gpio_num_t rxTWAI;
    const twai_message_t requestMessage = {.identifier = ID_MASTER_REQUEST, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
    const twai_message_t doneMessage = {.identifier = ID_MASTER_DONE, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};

    QueueHandle_t txTaskQueue;
    QueueHandle_t rxTaskQueue;
    SemaphoreHandle_t doneSem;
    SemaphoreHandle_t ctrlSem;
};

#endif