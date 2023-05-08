#include "LEDHandler.h"

namespace LED
{
    led_strip_t *strip = nullptr;

    void setPixel(uint8_t pixelNum, uint32_t red, uint32_t green, uint32_t blue)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, pixelNum, red, green, blue));
    }

    void clear()
    {
        ESP_ERROR_CHECK(strip->clear(strip, 1000));
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

        xTaskCreate(ledAnimationTask, "LED Animation", 4096, NULL, 10, NULL);
    }

    void ledAnimationTask(void *pvParameter)
    {
        xQueueHandle queueHandle = (xQueueHandle)pvParameter;
        while (true)
        {
            int animationSelection;
            animationSelection=Commons::animationSelection;
            if (xQueueReceive(queueHandle, &animationSelection, portMAX_DELAY) == pdTRUE)
            switch (animationSelection)
            {
            case 0:
                if (Commons::batteryPercentage < 10)
                {
                    lowBatteryAnimation();
                }
                else
                {
                    normalAnimation(Commons::batteryPercentage);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                break;
            case 1:
                chargingAnimation(Commons::batteryPercentage, false);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                chargingAnimation(Commons::batteryPercentage, true);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            case 2:
                swappingAnimation(false);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                swappingAnimation(true);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            case 3:
                noCanDataAnimation(false);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                noCanDataAnimation(true);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                break;
            default:
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;
            }
            uint32_t message = 1234;
            xQueueSend(queueHandle, &message, portMAX_DELAY);
        }
    }

    void lowBatteryAnimation()
    {
        normalAnimation(0);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        clear();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        normalAnimation(0);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        clear();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        normalAnimation(0);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        clear();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }

    void noCanDataAnimation(bool blink)
    {
        if(blink)
        {
            clear();
        }
        else
        {
            for (int i = 0; i < NEOPIXEL_NUM; i++)
            {
                setPixel(i, 143, 0, 255);
            }
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void startupAnimation()
    {
        for (int i = 0; i <= 10; i++)
        {
            normalAnimation(i * 10);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Commons::animationSelection = 0;
    }

    void normalAnimation(uint8_t batteryPercentage)
    {
        uint8_t green = (uint8_t)((float)batteryPercentage * 2.55);
        uint8_t red = 255 - green;
        uint8_t ledsToFill = (uint8_t)((float)batteryPercentage * 0.01 * NEOPIXEL_NUM);
        if(ledsToFill == 0)
            ledsToFill = 1;
        for (int i = 0; i < ledsToFill; i++)
        {
            setPixel(i, red, green, 0);
        }
        for (int i = ledsToFill; i < NEOPIXEL_NUM; i++)
        {
            setPixel(i, 0, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void chargingAnimation(uint8_t batteryPercentage, bool lastBlink)
    {
        uint8_t green = (uint8_t)((float)batteryPercentage * 2.55);
        uint8_t red = 255 - green;
        uint8_t ledsToFill = (uint8_t)((float)batteryPercentage * 0.01 * NEOPIXEL_NUM);
        if (ledsToFill == 0)
            ledsToFill = 1;
        for (int i = 0; i < ledsToFill - 1; i++)
        {
            setPixel(i, red, green, 0);
        }
        if (lastBlink)
        {
            setPixel(ledsToFill - 1, 0, 0, 0);
        }
        else
        {
            setPixel(ledsToFill - 1, red, green, 0);
        }
        for (int i = ledsToFill; i < NEOPIXEL_NUM; i++)
        {
            setPixel(i, 0, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void swappingAnimation(bool blink)
    {
        if (blink)
        {
            clear();
        }
        else
        {
            for (int i = 0; i < NEOPIXEL_NUM; i++)
            {
                setPixel(i, 0, 0, 255);
            }
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }
}