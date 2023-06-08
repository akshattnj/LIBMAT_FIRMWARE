#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/Networking/BLE/BLEHandler.h"
#include "src/Networking/EC20/EC20.h"
#include "src/I2CHandler/I2CHandler.h"

// void OLEDTask(void* pvParameters);

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
    I2C::setup();
    //errors here
    xTaskCreate(I2C::updateI2C, "I2C Updater", 2048, NULL, 12, NULL);
    // xTaskCreate(OLEDTask, "OLED", 4096, NULL, 12, NULL);
}

// void OLEDTask(void* pvParameters)
// {
//     // Initialize OLED
//     SSD1306_t dev;
//     int center, top, bottom;
//     char lineChar[20];
//     i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
//     ssd1306_init(&dev, 128, 64);
//     ssd1306_clear_screen(&dev, false);
//     ssd1306_contrast(&dev, 0xFF);
//     top = 2;
//     center = 3;
//     bottom = 8;
//     int SoCC = 100;
//     char* line1 = "%";
//     char* pre= "";
//     char final[12];
//     sprintf(final, "%s%d%s", pre,SoCC,line1);
//     while (true)
//     {
//         // Update OLED display
//         ssd1306_clear_screen(&dev, false);
//         ssd1306_display_text_x3(&dev, top, final, 16, false);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         // Additional OLED operations here
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
