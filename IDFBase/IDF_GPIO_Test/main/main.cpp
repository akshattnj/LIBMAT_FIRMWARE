#include <stdio.h>

#include "src/Commons/Commons.h"
#include "src/Wired/LED/LEDHandler.h"
#include "src/Wired/GPIO/GPIOHandler.h"
#include "src/definations.h"

extern "C" void app_main(void)
{
    BATT::setupGPIO();
    LED::init();
    xTaskCreate(LED::ledAnimationTask, "LED Animation", 4096, NULL, 10, NULL);
    xTaskCreate(BATT::doorTask, "Door", 2048, NULL, 10, NULL);
    xTaskCreate(BATT::doorScanner, "Door Scanner", 2048, NULL, 10, NULL);
}
