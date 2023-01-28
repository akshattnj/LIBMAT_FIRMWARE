#include "src/definations.h"

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "src/WiFiHandler.h"
#include "src/WebSocket.h"
}

extern "C" void app_main(void)
{
    initialiseNVS();
    initialiseWiFiSoftAP();
    startWebserver();
    xTaskCreate(scanForDisconnectedDevices, "Device Scanner", 2048, NULL, 10, NULL);
}
