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

class UARTHandler
{
public:
    int8_t RXD;
    int8_t TXD;
    uint8_t incomingData[BUF_SIZE];
    uint16_t incomingLen;
    uart_port_t portNum;

    uart_config_t portConfig = {
                .baud_rate = UART_BAUD_RATE,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_APB,
            };

    UARTHandler(uart_port_t portNum, int8_t RXD, int8_t TXD)
    {
        this->portNum = portNum;
        this->RXD = RXD;
        this->TXD = TXD;
    }

    void begin()
    {
        ESP_ERROR_CHECK(uart_driver_install(portNum, BUF_SIZE * 2, 0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(portNum, &portConfig));
        ESP_ERROR_CHECK(uart_set_pin(portNum, TXD, RXD, UART_RTS, UART_CTS));
    }

    void readData()
    {
        while (1)
        {
            incomingLen = uart_read_bytes(portNum, incomingData, (BUF_SIZE - 1), 20 / portTICK_RATE_MS);
            if (incomingLen > 0)
            {
                incomingData[incomingLen] = '\0';
                ESP_LOGI(UART_TAG, "Got data: %s", incomingData);
            }
            taskYIELD();
        }
    }

    void writeData(char *data)
    {
        ESP_LOGI(UART_TAG, "Sending Data: %s", data);
        uart_write_bytes(portNum, data, strlen(data));
    }
};

#endif