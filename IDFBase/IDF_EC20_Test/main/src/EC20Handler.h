#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "Definations.h"

#include <string.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
}

class EC20Handler
{
public:
    bool gotOk = false;
    int16_t incomingLen;
    char incomingData[BUF_SIZE];

    uart_config_t portConfig = {
        .baud_rate = EC20_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    void begin()
    {
        ESP_ERROR_CHECK(uart_driver_install(EC20_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(EC20_PORT_NUM, &portConfig));
        ESP_ERROR_CHECK(uart_set_pin(EC20_PORT_NUM, EC20_TXD, EC20_RXD, UART_RTS, UART_CTS));
    }

    void portListner()
    {
        while (1)
        {
            incomingLen = uart_read_bytes(EC20_PORT_NUM, incomingData, (BUF_SIZE - 1), 20 / portTICK_RATE_MS);
            if (incomingLen > 0)
            {
                incomingData[incomingLen] = '\0';
                char* output = strstr(incomingData, "OK");
                if(output) {
                    gotOk = true;
                }
                ESP_LOGI(EC20_TAG, "Got data: %s", incomingData);
            }
            taskYIELD();
        }
    }

    bool waitForOk()
    {
        int64_t startWait = esp_timer_get_time();
        while (!gotOk)
        {
            vTaskDelay(1 / portTICK_RATE_MS);
            if (esp_timer_get_time() - startWait >= 1000000L)
                return false;
            taskYIELD();
        }
        gotOk = false;
        return true;
    }

    bool sendAT(char *data, bool isCritical)
    {
        ESP_LOGI(EC20_TAG, "Sending Data: %s", data);
        uart_write_bytes(EC20_PORT_NUM, data, strlen(data));
        if (isCritical)
        {
            while (!waitForOk())
            {
                ESP_LOGE(EC20_TAG, "Failed to communicate with EC20. Sending %s again", data);
                uart_write_bytes(EC20_PORT_NUM, data, strlen(data));
                taskYIELD();
            }
            return true;
        }
        else
        {
            for (int attempts = 0; attempts < 5; attempts++)
            {
                if (waitForOk())
                    return true;
                ESP_LOGE(EC20_TAG, "Failed to communicate with EC20. Sending %s again", data);
                uart_write_bytes(EC20_PORT_NUM, data, strlen(data));
                taskYIELD();
            }
            ESP_LOGE(EC20_TAG, "Failed to communicate with EC20");
            return false;
        }
    }
};

#endif