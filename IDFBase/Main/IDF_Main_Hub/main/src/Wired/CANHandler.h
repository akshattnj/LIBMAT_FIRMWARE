#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/twai.h>
#include <set>
#include <cstring>

#include "../definations.h"

namespace CAN
{

    enum Instructions {
        PING,
        GENERAL_PING,
        DATA_SEND,
        DATA_RECV,
        DISCONNECT
    };

    class ControlQueueParams
    {
    public:
        Instructions instruction;
        char *data;
        size_t dataLen;
    };

    extern QueueHandle_t controlQueue;

    extern std::set<uint8_t> detectedIDs;

    void intitialiseTWAI();

    void recieveTask(void *args);

    void transmitTask(void *args);

    void controlTask(void *args);
}

#endif