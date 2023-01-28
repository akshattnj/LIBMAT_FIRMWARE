#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "../definations.h"
#include "../WiFi/WiFiHandler.h"

#include <sys/param.h>
#include <esp_netif.h>
#include <esp_http_server.h>

typedef struct asyncRespArgs {
    httpd_handle_t handler;
    int fileDescriptor;
    char *message;
    size_t messageLen;
} WSClient;

WSClient wsClientList[MAX_SOCKETS];

void broadcastToAll(void *args);

void scanForDisconnectedDevices(void *args);

httpd_handle_t startWebserver(void);

#endif