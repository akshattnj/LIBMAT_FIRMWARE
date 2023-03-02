extern "C"
{
#include <stdio.h>
}

#include "src/WiFiHandler.h"


extern "C" void app_main(void)
{
    WiFi::startNVS();
    WiFi::initialiseWiFiSTA();
    WiFi::connectWiFi();
}
