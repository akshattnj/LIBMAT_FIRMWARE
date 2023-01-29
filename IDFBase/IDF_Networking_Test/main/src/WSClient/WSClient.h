#ifndef WS_CLIENT
#define WS_CLIENT

#include "../WiFiClient/WiFiAP.h"

#include <freertos/semphr.h>
#include <esp_websocket_client.h>

extern uint8_t wsFlags;

void wsEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void startWSClient(void);
void sendWSMessage(char *data, size_t dataLen);
void destroyWSClient(void);
void wsClientTask(void *args);

#endif