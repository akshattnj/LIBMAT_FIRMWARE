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
    void initialiseWiFiSoftAP();
    void connectToWiFi(char *SSID, char *password);
    void disconnectWiFi();
}

#endif