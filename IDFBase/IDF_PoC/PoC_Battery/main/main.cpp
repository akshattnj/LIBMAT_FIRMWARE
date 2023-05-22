#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/Networking/BLE/BLEHandler.h"
#include "src/Networking/EC20/EC20.h"
#include "src/i2cAHT/I2CHandler.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    BLE::initialiseBLE();
    CANHandler::startTWAI();
    LED::init();

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

    //********** I2C **********//
    I2CHandler handleI2C(I2C_NUM_0, SDA_0_PIN, SCL_0_PIN, I2C_0_CLOCK);
    AHT::setup();
    xTaskCreate([](void *parameters)
                { AHT::updateI2C(parameters); },
                "I2C Updater", 2048, NULL, 12, NULL);
}
