#ifndef WS_H
#define WS_H

#include "../../definations.h"
#include "../../Commons/Commons.h"

extern "C"
{
    #include <esp_websocket_client.h>
}

namespace WS
{
    void wsEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

    /**
     * @brief Send Data to Websocket Server
    */
    void sendWSMessage(char *data, size_t dataLen);

    /**
     * @brief Start Websocket Client
    */
    void startWSClient(void);

    /**
     * @brief Destroy Websocket Client
    */
    void destroyWSClient(void);

    /**
     * @brief Websocket connection control task
    */
    void wsClientTask(void *args);
}

#endif