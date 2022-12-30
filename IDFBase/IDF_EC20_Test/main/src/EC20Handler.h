#ifndef EC20_HANDLER_H
#define EC20_HANDLER_H

#include "Definations.h"

#include <string.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "nmea.h"
#include "gpgll.h"
#include "gpgga.h"
#include "gprmc.h"
#include "gpgsa.h"
#include "gpvtg.h"
#include "gptxt.h"
#include "gpgsv.h"
}

class EC20Handler
{
public:
    int16_t incomingLen;
    char incomingData[BUF_SIZE];

#if CONFIG_EC20_ENABLE_GPS
    bool pauseGPS = true;
    bool stopGPS = false;
    double longitude;
    double latitude;
#endif

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

    void setup()
    {
        this->sendAT("");
        this->sendAT("V1");
        this->sendAT("E0");
        this->sendAT("+CMEE=2");
        this->sendAT("+CPIN?");
        this->sendAT("+CSQ");
        this->sendAT("+CREG=0");  // Older circuit switched connectivity
        this->sendAT("+CGREG=0"); // For 2G connectivity
        this->sendAT("+CEREG=0"); // For 3G/4G/5G connectivity
#if CONFIG_EC20_ENABLE_GPS
        this->sendAT("+QGPSCFG=\"nmeasrc\",1");
        this->sendAT("+QGPSEND", false);
        this->sendAT("+QGPS=1");
#endif
        this->sendAT("+CEREG?");
        this->sendAT("+CGREG?");
        pauseGPS = false;
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
                    connectFlags = connectFlags | GOT_OK;
                }
                output = strstr(incomingData, "ERROR");
                if (output)
                {
                    connectFlags = connectFlags | GOT_ERROR;
                    ESP_LOGE(EC20_TAG, "Got Error communication with EC20:\n%s", incomingData);
                }
                output = strstr(incomingData, "+CGREG:");
                if (output)
                {
                    this->readNetStatus(output, false);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+CEREG:");
                if (output)
                {
                    this->readNetStatus(output, true);
                    goto chkEnd;
                }

#if CONFIG_EC20_ENABLE_GPS
                output = strstr(incomingData, "+QGPSGNMEA:");
                if (output)
                {
                    this->readGSPResponse(output);
                    goto chkEnd;
                }
#endif

#if CONFIG_EC20_ENABLE_MQTT
                output = strstr(incomingData, "+QMTOPEN:");
                if (output)
                {
                    if (output[12] == '0')
                        MQTTFlags = (MQTTFlags | SERVER_CONNECT) & (~SERVER_ERROR);
                    else
                        MQTTFlags = (MQTTFlags & (~SERVER_CONNECT)) | SERVER_ERROR;
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QMTCONN");
                if (output)
                {
                    if (output[12] == '0')
                        MQTTFlags = (MQTTFlags | MQTT_CONNECT) & (~MQTT_ERROR);
                    else
                        MQTTFlags = (MQTTFlags & (~MQTT_CONNECT)) | MQTT_ERROR;
                    goto chkEnd;
                }
                output = strstr(incomingData, ">");
                if (output)
                {
                    MQTTFlags = MQTTFlags | SEND_READY;
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QMTPUBEX");
                if (output)
                {
                    if (output[15] == '0')
                        MQTTFlags = MQTTFlags | SEND_SUCCESS;
                    else
                    {
                        ESP_LOGE(EC20_TAG, "Failed to sent with code %c", output[15]);
                        MQTTFlags = (MQTTFlags | MQTT_ERROR) & (~MQTT_CONNECT);
                    }
                }
                output = strstr(incomingData, "+QMTSTAT");
                if (output)
                {
                    MQTTFlags = (MQTTFlags | SERVER_ERROR | MQTT_ERROR) & (~SERVER_CONNECT) & (~MQTT_CONNECT);
                    ESP_LOGE(EC20_TAG, "MQTT Connect Failed. Network reset needed.");
                }

#endif
            chkEnd:
                ESP_LOGI(EC20_TAG, "Got data: %s", incomingData);
            }
            taskYIELD();
        }
    }

    bool waitForOk(uint64_t messageDelay)
    {
        int64_t startWait = esp_timer_get_time();
        while ((connectFlags & GOT_OK) == 0)
        {
            vTaskDelay(10 / portTICK_RATE_MS);
            if (esp_timer_get_time() - startWait >= messageDelay)
                return false;
            if ((connectFlags & GOT_ERROR) > 0)
                return false;
            taskYIELD();
        }
        connectFlags = connectFlags & (~GOT_OK);
        return true;
    }

    bool sendAT(char *data, bool isCritical = true, uint64_t messageDealy = 1000000L)
    {
        uint16_t dataLen = strlen(data);
        char sendBuffer[dataLen + 5];
        sprintf(sendBuffer, "AT%s\r\n", data);
        ESP_LOGI(EC20_TAG, "Sending Data: %s", sendBuffer);
        uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
        if (isCritical == true)
        {
            while (!this->waitForOk(messageDealy))
            {
                if ((connectFlags & GOT_ERROR) > 0)
                {
                    connectFlags = connectFlags & (~GOT_ERROR);
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
                if (this->waitForOk(messageDealy))
                    return true;
                if ((connectFlags & GOT_ERROR) > 0)
                {
                    connectFlags = connectFlags & (~GOT_ERROR);
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

    void readNetStatus(char *output, bool LTE)
    {
        ESP_LOGD(EC20_TAG, "Here: %s", output);
        bool enabled = output[8] - '0';
        if (enabled)
        {
            ESP_LOGI(EC20_TAG, "Mode enabled");
            connectFlags = LTE ? connectFlags | LTE_MODE : connectFlags | GSM_MODE;
        }
        else
        {
            ESP_LOGI(EC20_TAG, "Mode not enabled");
            connectFlags = LTE ? connectFlags & (~LTE_MODE) : connectFlags & (~GSM_MODE);
        }
        uint8_t regStatus = output[10] - '0';
        switch (regStatus)
        {
        case 0:
            ESP_LOGI(EC20_TAG, "Not Registered, not searching");
            connectFlags = LTE ? connectFlags & (~LTE_CONNECT) : connectFlags & (~GSM_CONNECT);
            break;
        case 1:
            ESP_LOGI(EC20_TAG, "Registered, Home");
            connectFlags = LTE ? connectFlags | (LTE_CONNECT) : connectFlags | (GSM_CONNECT);
            break;
        case 2:
            ESP_LOGI(EC20_TAG, "Not Registered, Searching");
            connectFlags = LTE ? connectFlags & (~LTE_CONNECT) : connectFlags & (~GSM_CONNECT);
            break;
        case 3:
            ESP_LOGI(EC20_TAG, "Not Registered, Denied");
            connectFlags = LTE ? connectFlags & (~LTE_CONNECT) : connectFlags & (~GSM_CONNECT);
            break;
        case 4:
            ESP_LOGI(EC20_TAG, "Unknown, Sim may not support mode");
            connectFlags = LTE ? connectFlags & (~LTE_CONNECT) : connectFlags & (~GSM_CONNECT);
            break;
        case 5:
            ESP_LOGI(EC20_TAG, "Registered, Roaming");
            connectFlags = LTE ? connectFlags | (LTE_CONNECT) : connectFlags | (GSM_CONNECT);
        default:
            break;
        }
    }

#if CONFIG_EC20_ENABLE_MQTT
    bool waitForMQTTRespone(uint8_t messageCode)
    {
        uint8_t check;
        uint8_t error;
        switch (messageCode)
        {
        case 0:
            check = SERVER_CONNECT;
            error = SERVER_ERROR;
            break;
        case 1:
            check = MQTT_CONNECT;
            error = MQTT_ERROR;
            break;
        default:
            break;
        }

        while ((MQTTFlags & check) == 0)
        {
            vTaskDelay(10 / portTICK_RATE_MS);
            if ((connectFlags & error) > 0)
                return false;
            taskYIELD();
        }
        connectFlags = connectFlags & check;
        return true;
    }

    void connect()
    {
        pauseGPS = true;
        this->sendAT("+QMTDISC=0");
        this->sendAT("+QMTCFG=\"keepalive\",0,30");
        this->sendAT("+QMTCFG=\"session\",0,1");
        this->sendAT("+QMTCFG=\"will\",0,0,0,0");
        this->sendAT("+QMTCFG=\"recv/mode\",0,0,1");
        this->sendAT("+QMTCFG=\"send/mode\",0,0");
        this->reconnect();
        pauseGPS = false;
    }

    void reconnect()
    {
        char messageBuffer[BUF_SIZE];
        sprintf(messageBuffer, "AT+QMTOPEN=0,\"%s\",%d\r\n", TELEMETRY_DOMAIN, TELEMETRY_PORT);
        uart_write_bytes(EC20_PORT_NUM, messageBuffer, strlen(messageBuffer));
        if (!waitForMQTTRespone(0))
        {
            ESP_LOGE(EC20_TAG, "Connect Failed");
            return;
        }
        memset(messageBuffer, 0, BUF_SIZE);
        sprintf(messageBuffer, "AT+QMTCONN=0,\"%s\",\"%s\"\r\n", TELEMETRY_DEVICE_NAME, TELEMETRY_USERNAME);
        uart_write_bytes(EC20_PORT_NUM, messageBuffer, strlen(messageBuffer));
        if (!waitForMQTTRespone(1))
        {
            ESP_LOGE(EC20_TAG, "Connect Failed");
            return;
        }
        ESP_LOGI(EC20_TAG, "Thingsboard connected");
    }

    bool sendTelemetry(char *data, char *topic)
    {
        if (MQTTFlags & MQTT_CONNECT)
        {
            char sendBuffer[BUF_SIZE];
            sprintf(sendBuffer, "AT+QMTPUBEX=0,0,0,0,\"%s\",%d\r\n", topic, strlen(data));
            uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
            this->waitForReady();
            uart_write_bytes(EC20_PORT_NUM, data, strlen(data));
            uart_write_bytes(EC20_PORT_NUM, "\r\n", 2);
            this->publishResponse();
            return true;
        }
        return false;
    }

    void waitForReady()
    {
        while (1)
        {
            if ((MQTTFlags & SEND_READY) > 0)
                break;
            taskYIELD();
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    void publishResponse()
    {
        while (1)
        {
            if ((MQTTFlags & SEND_SUCCESS) > 0)
                break;
            if ((MQTTFlags & MQTT_ERROR) > 0)
            {
                ESP_LOGE(EC20_TAG, "Publish Failed. Need to Reconnect");
            }
            taskYIELD();
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
#endif

#if CONFIG_EC20_ENABLE_GPS
    void getGPSData(void *args)
    {
        while (1)
        {
            if (pauseGPS == false)
            {
                if (!sendAT("+QGPSGNMEA=\"RMC\"", false))
                    this->locationValid = false;
            }
            if (stopGPS == true)
            {
                sendAT("+QGPSEND");
                vTaskDelete(NULL);
            }
            vTaskDelay(1000 / portTICK_RATE_MS);
            taskYIELD();
        }
    }

    void readGSPResponse(char *output)
    {
        char *NMEAIn = (char *)memchr(output, '$', strlen(output));
        GPSData = nmea_parse(NMEAIn, strlen(NMEAIn), 0);
        if (GPSData == NULL)
        {
            ESP_LOGE(EC20_TAG, "Failed to parse the sentence!\n  Type: %.5s (%d), %d\n", NMEAIn + 1, nmea_get_type(NMEAIn), strlen(NMEAIn));
            this->locationValid = false;
        }
        else
        {
            if (GPSData->errors != 0)
            {
                ESP_LOGI(EC20_TAG, "WARN: The sentence struct contains parse errors!\n");
            }
            nmea_gprmc_s *pos = (nmea_gprmc_s *)GPSData;
            this->latitude = pos->latitude.degrees + ((pos->latitude.minutes) / 60);
            this->longitude = pos->longitude.degrees + ((pos->longitude.minutes) / 60);
            if (this->latitude != 0.00f && this->longitude != 0.00f)
            {
                this->locationValid = true;
                ESP_LOGI(EC20_TAG, "%f %f", longitude, latitude);
            }
        }
    }

    bool getLocationValid()
    {
        return this->locationValid;
    }

#endif

private:
    uint8_t connectFlags = 0; // {gotOk, gotError, GSM Mode Enabled, LTE mode enabled, GSM Connected, LTE connected}
    uint8_t MQTTFlags = 0;    // {Server Connect, Server Connect Error, MQTT Connect, MQTT Connect Error, ready to send, send Success}
    nmea_s *GPSData;
    bool locationValid = false;
};

#endif