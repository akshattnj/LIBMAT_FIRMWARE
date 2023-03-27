extern "C"
{
#include <stdio.h>
}

#include "src/Commons/Commons.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/BatteryHandler.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    BatteryHandler::init();
    WiFi::initialiseWiFiSoftAP();
    // WiFi::connectToWiFi("ZOLO~JAZZ", "Z@0J@zZ$");
    WiFi::connectToWiFi("Movio_Mobility", "movio@12334");
    WS::startWebserver();
    CANHandler::startTWAI();
    LED::init();

    xTaskCreate(WS::queueHandlerTask, "Queue handler", 4096, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskReceiveTWAI, "CAN receive", 4096, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskSendTWAI, "CAN send", 4096, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskControlTWAI, "CAN control", 4096, NULL, 10, NULL);
    xTaskCreate(LED::ledAnimationTask, "LED Animation", 4096, NULL, 10, NULL);
}
