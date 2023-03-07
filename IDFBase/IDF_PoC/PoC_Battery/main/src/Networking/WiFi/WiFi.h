#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "../../definations.h"
#include "../../Commons/Commons.h"

extern "C"
{
    #include <esp_wifi.h>
}

namespace WiFi
{
    void initialiseWiFiSTA();
    void disconnectWiFi();
    void taskAutoConnect(void *args);
    void taskWiFiConnect(void *args);
    void setNetCred();
    void stopWiFi();
}

#endif