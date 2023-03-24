#include "LEDHandler.h"

namespace LED
{
    led_strip_t *strip = nullptr;

    void setPixel(uint8_t pixelNum, uint32_t red, uint32_t green, uint32_t blue)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, pixelNum, red, green, blue));
    }

    void clear(uint8_t slot)
    {
        for (int i = (7 * slot); i < (7 * slot) + 7; i++)
        {
            setPixel(i, 0, 0, 0);
        }
    }

    void init()
    {
        rmt_config_t config = RMT_DEFAULT_CONFIG_TX((gpio_num_t)NEOPIXEL_PIN, RMT_CHANNEL_0);
        config.clk_div = 2;
        ESP_ERROR_CHECK(rmt_config(&config));
        ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
        led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(NEOPIXEL_NUM, (led_strip_dev_t)config.channel);
        strip = led_strip_new_rmt_ws2812(&strip_config);
        if (!strip)
        {
            ESP_LOGE("LEDHandler", "install WS2812 driver failed");
        }
        ESP_ERROR_CHECK(strip->clear(strip, 1000));
        startupAnimation();
    }

    void ledAnimationTask(void *pvParameter)
    {
        while (true)
        {
            for (int i = 0; i < 4; i++)
            {
                switch (Commons::animationSelection[i])
                {
                case 0:
                    normalAnimation(Commons::batteryPercentage[i], i);
                    break;
                case 1:
                    chargingAnimation(Commons::batteryPercentage[i], false, i);
                    break;
                case 2:
                    swappingAnimation(false, i);
                    break;
                default:
                    noBatteryAnimation(i);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    break;
                }
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);

            for (int i = 0; i < 4; i++)
            {
                switch (Commons::animationSelection[i])
                {
                case 0:
                    normalAnimation(Commons::batteryPercentage[i], i);
                    break;
                case 1:
                    chargingAnimation(Commons::batteryPercentage[i], true, i);
                    break;
                case 2:
                    swappingAnimation(true, i);
                    break;
                default:
                    noBatteryAnimation(i);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    break;
                }
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    void noBatteryAnimation(uint8_t slot)
    {
        for (int i = 7 * slot; i < (7 * slot) + 7; i++)
        {
            setPixel(i, 255, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void startupAnimation()
    {
        for (int i = 0; i < 10; i++)
        {
            normalAnimation(i * 10, 0);
            normalAnimation(i * 10, 1);
            normalAnimation(i * 10, 2);
            normalAnimation(i * 10, 3);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Commons::animationSelection[0] = 0xFF;
        Commons::animationSelection[1] = 0x01;
        Commons::animationSelection[2] = 0x01;
        Commons::animationSelection[3] = 0x01;
    }

    void normalAnimation(uint8_t batteryPercentage, uint8_t slot)
    {
        uint8_t green = (uint8_t)((float)batteryPercentage * 2.55);
        uint8_t red = 255 - green;
        uint8_t ledsToFill = (uint8_t)((float)batteryPercentage * 0.01 * 7);

        // ESP_LOGI(GPIO_TAG, "Normal: %d %d %d %d", ledsToFill, (7 * slot) + ledsToFill, batteryPercentage, slot);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
        for (int i = 7 * slot; i < (7 * slot) + ledsToFill; i++)
        {
            setPixel(i, red, green, 0);
        }
        for (int i = (7 * slot) + ledsToFill; i < (7 * slot) + 7; i++)
        {
            setPixel(i, 0, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }
    void chargingAnimation(uint8_t batteryPercentage, bool lastBlink, uint8_t slot)
    {
        uint8_t green = (uint8_t)((float)batteryPercentage * 2.55);
        uint8_t red = 255 - green;
        uint8_t ledsToFill = (uint8_t)((float)batteryPercentage * 0.01 * 7);
        if (ledsToFill == 0)
            ledsToFill = 1;

        // ESP_LOGI(GPIO_TAG, "Charging: %d %d %d %d", ledsToFill, 7 * slot, batteryPercentage, slot);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
        for (int i = 7 * slot; i < (7 * slot) + ledsToFill; i++)
        {
            setPixel(i, red, green, 0);
        }
        if (lastBlink)
        {
            setPixel((7 * slot) + ledsToFill - 1, 0, 0, 0);
        }
        else
        {
            setPixel((7 * slot) + ledsToFill - 1, red, green, 0);
        }
        for (int i = (7 * slot) + ledsToFill; i < (7 * slot) + 7; i++)
        {
            setPixel(i, 0, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void swappingAnimation(bool blink, uint8_t slot)
    {
        // ESP_LOGI(GPIO_TAG, "Swapping: %d %d", 7 * slot, slot);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
        if (blink)
        {
            for (int i = (7 * slot); i < (7 * slot) + 7; i++)
            {
                setPixel(i, 0, 0, 0);
            }
        }
        else
        {
            for (int i = (7 * slot); i < (7 * slot) + 7; i++)
            {
                setPixel(i, 0, 0, 255);
            }
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }
}