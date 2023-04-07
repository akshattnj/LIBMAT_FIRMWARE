#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include "src/Commons/Commons.h"
#include "src/definations.h"

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
}


#endif