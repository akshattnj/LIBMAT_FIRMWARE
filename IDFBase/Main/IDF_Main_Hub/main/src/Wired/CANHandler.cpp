/**
 * CAN RX GPIO: ??
 * CAN TX GPIO: ??
 *
 * TODO:
 * 1) CAN general scan and pass results to GPIO handler
 * 2) CAN ping and response for incoming battery
 * 3) CAN ping and response loss for outgoing battery
 * 4) CAN transmit to battery
 * 5) CAN telemetry scan and pass to telemetry handler
 */

#include "CANHandler.h"
namespace CAN
{
    std::set<uint8_t> detectedIDs;

    // Control queue will always be populated from outside the system
    QueueHandle_t controlQueue = xQueueCreate(7, sizeof(ControlQueueParams));
    QueueHandle_t recieveQueue = xQueueCreate(1, sizeof(ControlQueueParams));
    QueueHandle_t transmitQueue = xQueueCreate(1, sizeof(ControlQueueParams));

    SemaphoreHandle_t controlSemaphore = xSemaphoreCreateBinary();
    SemaphoreHandle_t transmitSemaphore = xSemaphoreCreateBinary();

    void intitialiseTWAI()
    {
        gpio_reset_pin(TWAI_RX);
        gpio_reset_pin(TWAI_TX);
        const twai_general_config_t generalCofig = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX, TWAI_RX, TWAI_MODE_NORMAL);
        const twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
        const twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        twai_driver_install(&generalCofig, &timingConfig, &filterConfig);
    }

    void recieveTask(void *args)
    {
        twai_message_t rxMessage;
        ControlQueueParams params;

        while (1)
        {
            if (xQueueReceive(recieveQueue, &params, portMAX_DELAY) == pdTRUE)
            {
                if (params.instruction == GENERAL_PING)
                {
                    // Exit as soon as semaphore is given
                    while (xSemaphoreTake(controlSemaphore, 0) != pdTRUE)
                    {
                        memset(&rxMessage, 0, sizeof(twai_message_t));
                        twai_receive(&rxMessage, portMAX_DELAY);
                        // Ignore all CAN IDs that are bigger than 0xFF
                        if (rxMessage.identifier > 0xFF)
                        {
                            continue;
                        }
                        // Insert detected IDs into a set
                        detectedIDs.insert((uint8_t)rxMessage.identifier);
                    }
                }
            }
        }
    }

    void transmitTask(void *args)
    {
        ControlQueueParams params;
        while (1)
        {
            if (xQueueReceive(transmitQueue, &params, portMAX_DELAY) == pdTRUE)
            {
                if (params.instruction == GENERAL_PING)
                {
                    twai_message_t pingMessage = {.ss = 1, .identifier = 0xFFF, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
                    // Send a general ping every 0.5 seconds and stop as soon as semaphore is given
                    while (xSemaphoreTake(transmitSemaphore, 0) != pdTRUE)
                    {
                        twai_transmit(&pingMessage, portMAX_DELAY);
                        vTaskDelay(500 / portTICK_PERIOD_MS);
                    }
                }
            }
        }
    }

    void controlTask(void *args)
    {
        ControlQueueParams params;
        while (1)
        {
            if (xQueueReceive(controlQueue, &params, portMAX_DELAY))
            {
                // Populate the detected battery IDs
                if (params.instruction == GENERAL_PING)
                {
                    xQueueSend(recieveQueue, &params, portMAX_DELAY);
                    xQueueSend(transmitQueue, &params, portMAX_DELAY);
                    vTaskDelay(5000 / portTICK_PERIOD_MS);
                    xSemaphoreGive(controlSemaphore);
                    xSemaphoreGive(transmitSemaphore);
                }
            }
        }
    }
}