
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"

static const char *TAG = "v1.0";

#define RMT_TX_CHANNEL RMT_CHANNEL_0

#define EXAMPLE_CHASE_SPEED_MS (100) //speed of the blinking
//#define CONFIG_EXAMPLE_STRIP_LED_NUMBER=4 //number of leds

void app_main(void)
{
    uint32_t red = 255;
    uint32_t green = 255;
    uint32_t blue = 255;
    uint16_t start_rgb = 0;

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 1000));
    // Show simple rainbow chasing pattern
    ESP_LOGI(TAG, "LED Rainbow Chase Start");
    while (true) {
        
            /* for (int j = 0; j < CONFIG_EXAMPLE_STRIP_LED_NUMBER8; j += 1) {
                // Write RGB values to strip driver
                ESP_ERROR_CHECK(strip->set_pixel(strip, j, red, green, blue));
            } */

            //set rgb values
            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 255,0)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 0, 255,0));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 0, 255,0)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 3, 0, 255,0));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            strip->clear(strip, 500);
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));

            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 255,0,0)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 255,0,0));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 255,0,0)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 3, 255,0,0));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            strip->clear(strip, 500);
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));

            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, 255)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 0, 0, 255));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 0, 0, 255)); 
            ESP_ERROR_CHECK(strip->set_pixel(strip, 3, 0, 0, 255));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            strip->clear(strip, 500);
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        }
}
