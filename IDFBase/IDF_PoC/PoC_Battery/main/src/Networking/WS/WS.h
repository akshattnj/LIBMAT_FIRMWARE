#ifndef WS_H
#define WS_H

#include "../../definations.h"
#include "../../Commons/Commons.h"

extern "C"
{
    #include <esp_netif.h>
    #include <esp_http_server.h>
}

namespace WS
{
    void queueHandlerTask(void *pvParameters);
    httpd_handle_t startWebserver(void);

    extern QueueHandle_t swapQueue;

    class ClientDetails
    {
    public:
        char clientId[9];
        char clientAccessId[21];
        uint8_t clientCanId;
        uint8_t customerId;
        uint8_t clientSlot;
        httpd_handle_t handler;
        int fileDescriptor;
        char message[20];
        size_t messageLen;
    };
}

#endif