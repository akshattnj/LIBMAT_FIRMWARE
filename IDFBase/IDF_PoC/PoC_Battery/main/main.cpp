#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/Networking/BLE/BLEHandler.h"
#include "src/Networking/EC20/EC20.h"


extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    BLE::initialiseBLE();
    CANHandler::startTWAI();
    LED::init();

    xTaskCreate(WS::wsClientTask, "WS Client", 2048, NULL, 10, NULL);
    xTaskCreate(EC20::commandControl, "Command Control", EC20_STACK_SIZE, NULL, 10, NULL);
}
