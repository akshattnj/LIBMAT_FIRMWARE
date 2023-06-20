#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/Networking/BLE/BLEHandler.h"
#include "src/Networking/EC20/EC20.h"
#include "src/I2CHandler/I2CHandler.h"

// Task that restarts the ESP every 2 minutes
void restartTask(void* pvParameters)
{
    const TickType_t restartDelay = pdMS_TO_TICKS(2 * 60 * 1000);  // 2 minutes in milliseconds

    while (true) {
        vTaskDelay(restartDelay);
        esp_restart();
    }
}

extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    BLE::initialiseBLE();
    CANHandler::startTWAI();
    LED::init();
    I2C::setup();

    // Hardcode GPIO 19 to High
    gpio_config_t tempConfig = {
        .pin_bit_mask = GPIO_SEL_19,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&tempConfig);
    gpio_set_level(GPIO_NUM_19, 1);

    xTaskCreate(WS::wsClientTask, "WS Client", 2048, NULL, 10, NULL);
    xTaskCreate(EC20::commandControl, "Command Control", EC20_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(restartTask, "Restart Task", 2048, NULL, 5, NULL);
}
