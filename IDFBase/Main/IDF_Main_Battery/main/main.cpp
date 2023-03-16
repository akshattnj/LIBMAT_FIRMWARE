#define MAIN_TAG "main"


#include <stdio.h>

#include "src/Wireless/BLE/BLE.h"


extern "C" void app_main(void)
{
    BLE::initialiseBLE();
    while(1)
    {
        BLE::sendData("Hello World");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
