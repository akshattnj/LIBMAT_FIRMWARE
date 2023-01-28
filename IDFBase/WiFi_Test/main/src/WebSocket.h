#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "definations.h"
#include "WiFiHandler.h"

#include <sys/param.h>
#include <esp_netif.h>
#include <esp_http_server.h>

typedef struct asyncRespArgs {
    httpd_handle_t handler;
    int fileDescriptor;
    char *message;
    size_t messageLen;
} WSClient;

WSClient wsClientList[12];


static esp_err_t addToWSQueue(uint8_t clientNum, char *data, size_t messageLen);

httpd_handle_t startWebserver(void);

#endif