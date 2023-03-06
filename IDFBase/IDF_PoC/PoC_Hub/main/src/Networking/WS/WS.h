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
    httpd_handle_t startWebserver(void);
}

#endif