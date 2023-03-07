extern "C"
{
#include <stdio.h>
}

#include "src/Commons/Commons.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/BatteryHandler.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    BatteryHandler::init();
    WiFi::initialiseWiFiSoftAP();
    WiFi::connectToWiFi("ZOLO~JAZZ", "Z@0J@zZ$");
    WS::startWebserver();



    xTaskCreate(WS::queueHandlerTask, "Queue handler", 4096, NULL, 10, NULL);
}
