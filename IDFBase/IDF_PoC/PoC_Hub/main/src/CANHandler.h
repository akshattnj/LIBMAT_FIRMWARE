#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include "./Commons/Commons.h"
#include "./definations.h"

extern "C"
{
    #include <driver/twai.h>
}

namespace CANHandler
{
    esp_err_t startTWAI();
    esp_err_t endTWAI();

    void taskReceiveTWAI(void *params);
    void taskSendTWAI(void *params);
    void taskControlTWAI(void *params);
}


#endif