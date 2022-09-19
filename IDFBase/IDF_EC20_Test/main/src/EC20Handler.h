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
    int16_t incomingLen;
    char incomingData[BUF_SIZE];
    bool pauseGPS = true;
    bool stopGPS = false;

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

    void readNetStatus(char *output, bool *enabled, bool *connected)
    {
        ESP_LOGI(EC20_TAG, "Here: %s", output);
        *enabled = output[8] - '0';
        if (*enabled)
            ESP_LOGI(EC20_TAG, "Mode enabled");
        else
            ESP_LOGI(EC20_TAG, "Mode not enabled");
        uint8_t regStatus = output[10] - '0';
        switch (regStatus)
        {
        case 0:
            ESP_LOGI(EC20_TAG, "Not Registered, not searching");
            *connected = false;
            break;
        case 1:
            ESP_LOGI(EC20_TAG, "Registered, Home");
            *connected = true;
            break;
        case 2:
            ESP_LOGI(EC20_TAG, "Not Registered, Searching");
            *connected = false;
            break;
        case 3:
            ESP_LOGI(EC20_TAG, "Not Registered, Denied");
            *connected = false;
            break;
        case 4:
            ESP_LOGI(EC20_TAG, "Unknown, Sim may not support mode");
            *connected = false;
            break;
        case 5:
            ESP_LOGI(EC20_TAG, "Registered, Roaming");
            *connected = true;
        default:
            break;
        }
    }

    void portListner()
    {
        while (1)
        {
            incomingLen = uart_read_bytes(EC20_PORT_NUM, incomingData, (BUF_SIZE - 1), 20 / portTICK_RATE_MS);
            if (incomingLen > 0)
            {
                incomingData[incomingLen] = '\0';
                char *output = strstr(incomingData, "OK");
                if (output)
                {
                    gotOk = true;
                }
                output = strstr(incomingData, "ERROR");
                if (output)
                {
                    gotError = true;
                    ESP_LOGE(EC20_TAG, "Got Error communication with EC20:\n%s", incomingData);
                }
                output = strstr(incomingData, "+CGREG:");
                if (output)
                {
                    readNetStatus(output, &GSMEnabled, &GSMConnected);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+CEREG:");
                if (output)
                {
                    readNetStatus(output, &LTEEnabled, &LTEConnected);
                    goto chkEnd;
                }
            chkEnd:
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
            if (gotError)
                return false;
            taskYIELD();
        }
        gotOk = false;
        return true;
    }

    bool sendAT(char *data, bool isCritical = true)
    {
        uint16_t dataLen = strlen(data);
        char sendBuffer[dataLen + 5];
        sprintf(sendBuffer, "AT%s\r\n", data);
        ESP_LOGI(EC20_TAG, "Sending Data: %s", sendBuffer);
        uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
        if (isCritical == true)
        {
            while (!waitForOk())
            {
                if (gotError)
                {
                    gotError = false;
                    vTaskDelay(100 / portTICK_RATE_MS);
                }
                ESP_LOGE(EC20_TAG, "Failed to communicate with EC20. Sending %s again", data);
                uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
                vTaskDelay(10 / portTICK_RATE_MS);
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
                if (gotError)
                {
                    gotError = false;
                    return false;
                }
                ESP_LOGE(EC20_TAG, "Failed to communicate with EC20. Sending %s again", data);
                uart_write_bytes(EC20_PORT_NUM, data, strlen(data));
                vTaskDelay(10 / portTICK_RATE_MS);
                taskYIELD();
            }
            ESP_LOGE(EC20_TAG, "Failed to communicate with EC20");
            return false;
        }
    }

    void setup()
    {
        sendAT("");
        sendAT("V1");
        sendAT("E0");
        sendAT("+CMEE=2");
        sendAT("+CSQ");
        sendAT("+CREG=0"); // Older circuit switched connectivity
        sendAT("+CGREG=1"); // For 2G connectivity
        // sentAT("+CEREG=1") // For 3G/4G/5G connectivity
        sendAT("+QGPSCFG=\"nmeasrc\",1");
        sendAT("+QGPSEND", false);
        sendAT("+QGPS=1");
        sendAT("+CEREG?");
        sendAT("+CGREG?");
        pauseGPS = false;
    }

    void connect()
    {
        if (GSMConnected || LTEConnected)
        {
            pauseGPS = true;
            sendAT("+QIDEACT=1");
            sendAT("+QICSGP=1,1,\"AIRTELGPRS.COM\",\"\",\"\",0");
            sendAT("+QIACT=1");
            sendAT("+QIACT?");
            pauseGPS = false;
        }
        else
        {
            ESP_LOGE(EC20_TAG, "Network not connected. Aborting");
        }
    }

    void getGPSData(void *args)
    {
        while (1)
        {
            if (pauseGPS == false)
            {
                sendAT("+QGPSLOC=1", false);
            }
            if (stopGPS == true)
            {
                sendAT("+QGPSEND");
                vTaskDelete(NULL);
            }
            vTaskDelay(500 / portTICK_RATE_MS);
            taskYIELD();
        }
    }

private:
    bool gotOk = false;
    bool gotError = false;
    bool GSMEnabled = false;
    bool GSMConnected = false;
    bool LTEEnabled = false;
    bool LTEConnected = false;
};

#endif