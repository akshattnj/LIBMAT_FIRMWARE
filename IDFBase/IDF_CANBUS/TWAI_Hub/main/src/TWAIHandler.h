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

class TWAIHandler
{
public:
    esp_err_t startTWAI()
    {
        return twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
    }
    esp_err_t endTWAI()
    {
        return twai_driver_uninstall();
    }

    void taskReceiveTWAI(void *params)
    {
        twai_message_t rxMessage;
        esp_err_t errorCode;
        while (1)
        {
            memset(&rxMessage, 0, sizeof(twai_message_t));
            errorCode = twai_receive(&rxMessage, portMAX_DELAY);
            if (errorCode)
            {
                // ESP_LOGE(TWAI_TAG, "Got error: %d", errorCode);
            }
            else
            {
                ESP_LOGI(TWAI_TAG, "Recieved message identifier: %u", rxMessage.identifier);
            }
            taskYIELD();
            vTaskDelay(1);
        }
        vTaskDelete(NULL);
    }
    void taskSendTWAI(void *params)
    {
        esp_err_t errorCode;
        while (1)
        {

            errorCode = twai_transmit(&pingMessage, portMAX_DELAY);
            if (errorCode)
            {
                ESP_LOGE(TWAI_TAG, "TWAI Send Error: %d", errorCode);
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        vTaskDelete(NULL);
    }

private:
    const twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_25KBITS();
    const twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    const twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TWAI_TX, (gpio_num_t)TWAI_RX, TWAI_MODE_NORMAL);
    const twai_message_t pingMessage = {.ss = 1, .identifier = ID_MASTER_PING, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
    const twai_message_t requestMessage = {.identifier = ID_MASTER_REQUEST, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
    const twai_message_t doneMessage = {.identifier = ID_MASTER_DONE, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};

    QueueHandle_t txTaskQueue;
    QueueHandle_t rxTaskQueue;
};
#endif