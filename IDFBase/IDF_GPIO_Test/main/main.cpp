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
<<<<<<< HEAD

    // while (true)
    // {
    //     Commons::animationSelection[0] = 0;
    //     Commons::animationSelection[1] = 0;
    //     Commons::animationSelection[2] = 0;
    //     Commons::animationSelection[3] = 0;
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);

    //     Commons::animationSelection[0] = 0;
    //     Commons::animationSelection[1] = 2;
    //     Commons::animationSelection[2] = 2;
    //     Commons::animationSelection[3] = 0;
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);

    //     Commons::animationSelection[0] = 1;
    //     Commons::animationSelection[1] = 0;
    //     Commons::animationSelection[2] = 0;
    //     Commons::animationSelection[3] = 1;
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);

    //     Commons::animationSelection[0] = 2;
    //     Commons::animationSelection[1] = 2;
    //     Commons::animationSelection[2] = 2;
    //     Commons::animationSelection[3] = 2;
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);

    //     Commons::animationSelection[0] = 1;
    //     Commons::animationSelection[1] = 10;
    //     Commons::animationSelection[2] = 0;
    //     Commons::animationSelection[3] = 2;
    //     vTaskDelay(10000 / portTICK_PERIOD_MS);
    // }
=======
>>>>>>> poc_hub_connection_test
}
