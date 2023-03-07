#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/Networking/WiFi/WiFi.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    xTaskCreate(WiFi::taskWiFiConnect, "Wifi Connect", 4096, NULL, 10, NULL);
    xTaskCreate(WiFi::taskAutoConnect, "Auto Connect", 4096, NULL, 10, NULL);
}
