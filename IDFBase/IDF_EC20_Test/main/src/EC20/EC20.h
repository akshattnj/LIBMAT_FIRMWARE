#ifndef EC20_H
#define EC20_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>

#include "../definations.h"

namespace EC20
{
    void portListner(void *args);
    void commandControl(void *args);
    void commandRun(void *args);
    void readNetStatus(char *output, bool LTE);
    void setup();
    bool sendATCommand(const char *command, size_t cmdLen, bool isCritical = true, uint32_t timeout = 1000 / portTICK_PERIOD_MS);
    bool waitForATResponse(uint32_t timeout);
    void initialiseMQTT();
    void reconnectMQTT();
    void getGPSData();
    void readGPSData(char *output);
    void publishMQTT(char *data, size_t dataSize);
}

#endif
