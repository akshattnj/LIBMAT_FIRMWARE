#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "definations.h"
#include "WiFiHandler.h"

#include <sys/param.h>
#include <esp_netif.h>
#include <esp_http_server.h>

httpd_handle_t start_webserver(void);

#endif