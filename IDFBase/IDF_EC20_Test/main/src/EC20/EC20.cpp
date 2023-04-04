#include "EC20.h"

namespace EC20
{
    typedef struct command
    {
        uint8_t command;
        char data[BUFFER_LENGTH];
    } Command;

    int16_t incomingDataLen = 0;
    char incomingData[BUFFER_LENGTH];
    uint8_t flagsMQTT = 0x00; // {BIT0 : Server connect, BIT1 : Server error, BIT2 : MQTT connect, BIT3 : MQTT error, BIT4 : Send ready, BIT5 : Send success, BIT6 : Send error, BIT7 : reconnect}
    uint8_t flagsEC20 = 0x00; // {BIT0 : GSM mode, BIT1 : LTE mode, BIT2 : GSM connect, BIT3 : LTE connect, BIT4 : Error, BIT5 : OK, BIT6 : Valid Coordinates}

    double longitude;
    double latitude;

    SemaphoreHandle_t responseAT = xSemaphoreCreateBinary();
    SemaphoreHandle_t responseMQTT = xSemaphoreCreateBinary();
    SemaphoreHandle_t responseGPS = xSemaphoreCreateBinary();

    QueueHandle_t commandQueue = xQueueCreate(10, sizeof(command));

    const uart_config_t portConfig = {
        .baud_rate = EC20_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    void commandControl(void *args)
    {
        setup();
        initialiseMQTT();
        Command cmd;
        cmd.command = 0x03;
        xQueueSend(commandQueue, &cmd, 0);

        int64_t delayGPS = esp_timer_get_time();
        int64_t delayMQTT = esp_timer_get_time();
        xTaskCreate(commandRun, "Command Runtime Handler", EC20_STACK_SIZE, NULL, 10, NULL);

        while (1)
        {
            if(esp_timer_get_time() - delayGPS > 500000)
            {
                cmd.command = 0x01;
                xQueueSend(commandQueue, &cmd, 0);
                delayGPS = esp_timer_get_time();
            }
            if(esp_timer_get_time() - delayMQTT > 3000000)
            {
                char buffer[100];
                memset(cmd.data, 0, sizeof(cmd.data));
                strcat(cmd.data, "{\'Hello\': \'Hi\'");
                if(flagsEC20 & BIT6)
                    sprintf(buffer, ",\'latitude\': %.4f, \'longitude\': %.4f}", latitude, longitude);
                else
                    sprintf(buffer, "}");
                strcat(cmd.data, buffer);
                cmd.command = 0x02;
                ESP_LOGI(EC20_TAG, "Sending: %s", cmd.data);
                xQueueSend(commandQueue, &cmd, 0);
                delayMQTT = esp_timer_get_time();
            }
            vTaskDelay(100);
        }
    }

    void commandRun(void *args)
    {
        Command cmd;
        while (1)
        {
            if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY))
            {
                switch (cmd.command)
                {
                case 0x01:
                    getGPSData();
                    break;
                case 0x02:
                    if(flagsMQTT & BIT2)
                        publishMQTT(cmd.data, strlen(cmd.data));
                    else
                        ESP_LOGI(EC20_TAG, "MQTT not connected");
                    break;
                case 0x03:
                    reconnectMQTT();
                    break;
                default:
                    break;
                }
            }
        }
    }

    /**
     * @brief Setup UART port for EC20 and send some initial commands
     */
    void setup()
    {
        ESP_ERROR_CHECK(uart_driver_install(EC20_PORT_NUM, BUFFER_LENGTH * 2, 0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(EC20_PORT_NUM, &portConfig));
        ESP_ERROR_CHECK(uart_set_pin(EC20_PORT_NUM, EC20_TXD, EC20_RXD, UART_RTS, UART_CTS));

        xTaskCreate(portListner, "UART Handler", EC20_STACK_SIZE, NULL, 15, NULL);

        sendATCommand("", 0);
        sendATCommand("V1", 2);
        sendATCommand("E0", 2);
        sendATCommand("+CMEE=2", 7);
        sendATCommand("+CPIN?", 6);
        sendATCommand("+CSQ", 4);
        sendATCommand("+CREG=0", 7);  // Older circuit switched connectivity
        sendATCommand("+CGREG=0", 8); // For 2G connectivity
        sendATCommand("+CEREG=0", 8); // For 3G/4G/5G connectivity
        sendATCommand("+QGPSCFG=\"nmeasrc\",1", 20);
        sendATCommand("+QGPSEND", 8, false);
        sendATCommand("+QGPS=1", 7);
        sendATCommand("+CEREG?", 7);
        sendATCommand("+CGREG?", 7);
    }

    /**
     * @brief Set up initial paramteters for MQTT connection
    */
    void initialiseMQTT()
    {
        sendATCommand("+QMTDISC=0", 10);
        sendATCommand("+QMTCFG=\"keepalive\",0,30", 24);
        sendATCommand("+QMTCFG=\"session\",0,1", 21);
        sendATCommand("+QMTCFG=\"will\",0,0,0,0", 22);
        sendATCommand("+QMTCFG=\"recv/mode\",0,0,1", 25);
        sendATCommand("+QMTCFG=\"send/mode\",0,0", 23);
    }

    /**
     * @brief Connect to MQTT broker
    */
    void reconnectMQTT()
    {
        char messageBuffer[BUFFER_LENGTH];
        size_t dataLen = sprintf(messageBuffer, "+QMTOPEN=0,\"%s\",%d", TELEMETRY_DOMAIN, TELEMETRY_PORT);
        sendATCommand(messageBuffer, dataLen, true);
        xSemaphoreTake(responseMQTT, portMAX_DELAY);
        ESP_LOGI(EC20_TAG, "Thingsboard opened");

        memset(messageBuffer, 0, BUFFER_LENGTH);
        dataLen = sprintf(messageBuffer, "+QMTCONN=0,\"%s\",\"%s\"", TELEMETRY_DEVICE_NAME, TELEMETRY_USERNAME);
        sendATCommand(messageBuffer, dataLen, true);
        xSemaphoreTake(responseMQTT, portMAX_DELAY);
        ESP_LOGI(EC20_TAG, "Thingsboard connected");
    }

    /**
     * @brief Publish data to MQTT broker
     * @param data Data to be published
     * @param dataSize Size of data
    */
    void publishMQTT(char *data, size_t dataSize)
    {
        char messageBuffer[BUFFER_LENGTH];
        size_t dataLen = sprintf(messageBuffer, "AT+QMTPUBEX=0,0,0,0,\"%s\",%d\r\n", TELEMETRY_TOPIC, dataSize);
        uart_write_bytes(EC20_PORT_NUM, messageBuffer, dataLen);
        xSemaphoreTake(responseMQTT, portMAX_DELAY);
        memset(messageBuffer, 0, BUFFER_LENGTH);
        dataLen = sprintf(messageBuffer, "%s\r\n", data);
        ESP_LOGI(EC20_TAG, "Sending: %s", messageBuffer);
        uart_write_bytes(EC20_PORT_NUM, messageBuffer, dataLen);
        xSemaphoreTake(responseMQTT, portMAX_DELAY);
        ESP_LOGI(EC20_TAG, "Message published");
    }

    /**
     * @brief Get GPS data from EC20
    */
    void getGPSData()
    {
        char messageBuffer[BUFFER_LENGTH];
        size_t dataLen = sprintf(messageBuffer, "+QGPSGNMEA=\"RMC\"");
        sendATCommand(messageBuffer, dataLen, false);
        xSemaphoreTake(responseGPS, portMAX_DELAY);
        ESP_LOGI(EC20_TAG, "GPS data received");
    }

    void readGPSData(char *output)
    {
        char* ptr = strtok(output, ",");
        uint8_t commaCount = 0;
        while (ptr != NULL) 
        {
            commaCount++;
            if (commaCount == 4) 
            {
                double latDegree = atoi(ptr) / 100;
                double latMinute = atof(ptr + 2) / 60;
                latitude = latDegree + latMinute;
                if (latitude < -90 || latitude > 90) 
                {
                    ESP_LOGI(EC20_TAG, "Invalid latitude");
                    flagsEC20 = flagsEC20 & (~BIT6);    
                    return;
                }
            } 
            else if (commaCount == 6) 
            {
                double lonDegree = atoi(ptr) / 100;
                double lonMinute = atof(ptr + 3) / 60;
                longitude = lonDegree + lonMinute;
                if (longitude < -180 || longitude > 180) 
                {
                    ESP_LOGI(EC20_TAG, "Invalid longitude");
                    flagsEC20 = flagsEC20 & (~BIT6);
                    return;
                }
                flagsEC20 = flagsEC20 | BIT6;
            }
            ptr = strtok(NULL, ",");
        }
        if(longitude == 0.00 && latitude == 0.00)
        {
            ESP_LOGE(EC20_TAG, "Invalid GPS data");
            flagsEC20 = flagsEC20 & (~BIT6);
        }
    }

    /**
     * @brief Send AT command to EC20
     * @param command Command to send
     * @param cmdLen Length of command
     * @param isCritical Is the command critical
     * @param timeout Timeout in ms
    */
    bool sendATCommand(const char *command, size_t cmdLen, bool isCritical, uint32_t timeout)
    {
        char sendBuffer[cmdLen + 6];
        sprintf(sendBuffer, "AT%s\r\n", command);
        ESP_LOGI(EC20_TAG, "Sending: %s", sendBuffer);
        uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
        if (isCritical)
        {
            while (1)
            {
                if (waitForATResponse(timeout))
                {
                    return true;
                }
                else
                {
                    ESP_LOGE(EC20_TAG, "Communication Error with EC20. Sending Again");
                    ESP_LOGI(EC20_TAG, "Sending: %s", sendBuffer);
                    uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
                }
            }
            return false;
        }
        else
        {
            for(int i = 0; i < 5; i++)
            {
                if (waitForATResponse(timeout))
                {
                    return true;
                }
                else
                {
                    ESP_LOGE(EC20_TAG, "Communication Error with EC20. Send Attempt: %d", i);
                    uart_write_bytes(EC20_PORT_NUM, sendBuffer, strlen(sendBuffer));
                }
            }
        }
        return false;
    }

    /**
     * @brief Wait for response from EC20
     * @param timeout Timeout in ms
     * 
     * The code does the following:
     * 1. Check that the semaphore is available. If not, block the task until the semaphore becomes available.
     * 2. If the semaphore is available, check if the BIT4 flag is set. If so, clear the flag and return false.
     * 3. If the semaphore is available, check if the BIT5 flag is set. If so, clear the flag and return true.
     * 4. If the semaphore is available, but neither BIT4 or BIT5 are set, then the task was blocked for longer than the timeout, so return false. 
    */
    bool waitForATResponse(uint32_t timeout)
    {
        if (xSemaphoreTake(responseAT, timeout) == pdTRUE)
        {
            if(flagsEC20 & BIT4)
            {
                flagsEC20 = flagsEC20 & (~BIT4);
                ESP_LOGE(EC20_TAG, "EC20 Error");
                return false;
            }
            else if(flagsEC20 & BIT5)
            {
                flagsEC20 = flagsEC20 & (~BIT5);
                ESP_LOGI(EC20_TAG, "EC20 OK");
                return true;
            }
            return false;
        }
        else
        {
            ESP_LOGE(EC20_TAG, "EC20 response timed out");
            return false;
        }
    }

    /**
     * @brief Read Network status response from EC20
     * @param output Response from EC20
     * @param LTE LTE mode or GSM mode
     */
    void readNetStatus(char *output, bool LTE)
    {
        ESP_LOGD(EC20_TAG, "Here: %s", output);
        bool enabled = output[8] - '0';
        if (enabled)
        {
            ESP_LOGI(EC20_TAG, "Mode enabled");
            flagsEC20 = LTE ? flagsEC20 | BIT1 : flagsEC20 | BIT0;
        }
        else
        {
            ESP_LOGI(EC20_TAG, "Mode not enabled");
            flagsEC20 = LTE ? flagsEC20 & (~BIT1) : flagsEC20 & (~BIT0);
        }
        uint8_t regStatus = output[10] - '0';
        switch (regStatus)
        {
        case 0:
            ESP_LOGI(EC20_TAG, "Not Registered, not searching");
            flagsEC20 = LTE ? flagsEC20 & (~BIT3) : flagsEC20 & (~BIT2);
            break;
        case 1:
            ESP_LOGI(EC20_TAG, "Registered, Home");
            flagsEC20 = LTE ? flagsEC20 | (BIT3) : flagsEC20 | (BIT2);
            break;
        case 2:
            ESP_LOGI(EC20_TAG, "Not Registered, Searching");
            flagsEC20 = LTE ? flagsEC20 & (~BIT3) : flagsEC20 & (~BIT2);
            break;
        case 3:
            ESP_LOGI(EC20_TAG, "Not Registered, Denied");
            flagsEC20 = LTE ? flagsEC20 & (~BIT3) : flagsEC20 & (~BIT2);
            break;
        case 4:
            ESP_LOGI(EC20_TAG, "Unknown, Sim may not support mode");
            flagsEC20 = LTE ? flagsEC20 & (~BIT3) : flagsEC20 & (~BIT2);
            break;
        case 5:
            ESP_LOGI(EC20_TAG, "Registered, Roaming");
            flagsEC20 = LTE ? flagsEC20 | (BIT3) : flagsEC20 | (BIT2);
        default:
            break;
        }
    }

    /**
     * @brief Task to listen to EC20 and set flags accordingly
     * 
     * The code above does the following:
     *  1. If the current line is "OK", then set the flagsEC20 bit 5 and give the semaphore responseAT.
     *  2. If the current line is "ERROR", then set the bit 4 and give the semaphore responseAT.
     *  3. If the current line is "+CGREG:", then call the function readNetStatus with the current line as the first argument and false as the second argument to specify GSM.
     *  4. If the current line is "+CEREG:", then call the function readNetStatus with the current line as the first argument and true as the second argument to specigy LTE.
     *  5. If the current line is "+QGPSGNMEA:", then print the respoonse to console and give the semaphore responseGPS
     *  6. If the current line is "+QMTOPEN:", then set the flagsMQTT bit 0 and clear the flagsMQTT bit 1 if the 12th character of the current line is 0 and set the flagsMQTT bit 1 and clear the flagsMQTT bit 0 if the 12th character of the current line is not 0.
     *  7. If the current line is "+QMTCONN", then set the flagsMQTT bit 2 and clear the flagsMQTT bit 3 if the 12th character of the current line is 0 and set the flagsMQTT bit 3 and clear the flagsMQTT bit 2 if the 12th character of the current line is not 0.
     *  8. If the current line is ">", then set the flagsMQTT bit 4.
     *  9. If the current line is "+QMTPUBEX", then set the flagsMQTT bit 5 if the 15th character of the current line is 0 and set the flagsMQTT bit 1 and clear the flagsMQTT bit 0 if the 15th character of the current line is not 0.
     *  10. If the current line is "+QMTSTAT", then set the flagsMQTT bit 1, flagsMQTT bit 3, clear the flagsMQTT bit 0, and clear the flagsMQTT bit 2. 
    */
    void portListner(void *args)
    {
        while (1)
        {
            incomingDataLen = uart_read_bytes(EC20_PORT_NUM, incomingData, (BUFFER_LENGTH - 1), 20 / portTICK_RATE_MS);
            if (incomingDataLen > 0)
            {
                incomingData[incomingDataLen] = '\0';
                char *output = strstr(incomingData, "OK");
                if (output)
                {
                    flagsEC20 = flagsEC20 | BIT5;
                    xSemaphoreGive(responseAT);
                }
                output = strstr(incomingData, "ERROR");
                if (output)
                {
                    flagsEC20 = flagsEC20 | BIT4;
                    ESP_LOGE(EC20_TAG, "Got Error communication with EC20:\n%s", incomingData);
                    xSemaphoreGive(responseAT);
                }
                output = strstr(incomingData, "+CGREG:");
                if (output)
                {
                    readNetStatus(output, false);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+CEREG:");
                if (output)
                {
                    readNetStatus(output, true);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QGPSGNMEA:");
                if (output)
                {
                    ESP_LOGI(EC20_TAG, "GPS Response: %s", output);
                    readGPSData(output);
                    xSemaphoreGive(responseGPS);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QMTOPEN:");
                if (output)
                {
                    if (output[12] == '0') 
                        flagsMQTT = (flagsMQTT | BIT0) & (~BIT1);
                    else
                        flagsMQTT = (flagsMQTT & (~BIT0)) | BIT1;
                    xSemaphoreGive(responseMQTT);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QMTCONN");
                if (output)
                {
                    if (output[12] == '0')
                        flagsMQTT = (flagsMQTT | BIT2) & (~BIT3);
                    else
                        flagsMQTT = (flagsMQTT & (~BIT2)) | BIT3;
                    xSemaphoreGive(responseMQTT);
                    goto chkEnd;
                }
                output = strstr(incomingData, ">");
                if (output)
                {
                    flagsMQTT = flagsMQTT | BIT4;
                    xSemaphoreGive(responseMQTT);
                    goto chkEnd;
                }
                output = strstr(incomingData, "+QMTPUBEX");
                if (output)
                {
                    if (output[15] == '0')
                        flagsMQTT = flagsMQTT | BIT5;
                    else
                    {
                        ESP_LOGE(EC20_TAG, "Failed to sent with code %c", output[15]);
                        flagsMQTT = (flagsMQTT | BIT1) & (~BIT0);
                    }
                    xSemaphoreGive(responseMQTT);
                }
                output = strstr(incomingData, "+QMTSTAT");
                if (output)
                {
                    flagsMQTT = (flagsMQTT | BIT1 | BIT3) & (~BIT0) & (~BIT2);
                    ESP_LOGE(EC20_TAG, "MQTT Connect Failed. Network reset needed.");
                    Command cmd = {.command = 0x03};
                    xQueueSend(commandQueue, &cmd, 0);
                }
            chkEnd:
                ESP_LOGI(EC20_TAG, "Got data: %s", incomingData);
            }
            taskYIELD();
        }
    }
} // namespace EC20
