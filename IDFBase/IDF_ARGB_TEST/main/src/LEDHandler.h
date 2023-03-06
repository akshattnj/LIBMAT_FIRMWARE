#ifndef LED_HANDLER_H
#define LED_HANDLER_H

extern "C"
{
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/rmt.h>
#include <led_strip.h>
}

#include "definations.h"

class LEDHandler
{
private:
    led_strip_t *strip = nullptr;
    uint8_t batteryPercentage = 0;
    uint8_t animationSelection = 0xFF;
    /**
     * @brief Set the color of a pixel on the strip
     * @param pixelNum number of the pixel to set
     * @param red value of red color
     * @param green value of green color
     * @param blue value of blue color
    */
    void setPixel(uint8_t pixelNum, uint32_t red, uint32_t green, uint32_t blue)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, pixelNum, red, green, blue));
    }

    /**
     * @brief Clear value of all pixels on the strip
    */
    void clear()
    {
        ESP_ERROR_CHECK(strip->clear(strip, 1000));
    }
public:
    /**
     * @brief Initialize the LED strip
    */
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

    /**
     * @brief Set the battery percentage
    */
    void setBatteryPercentage(uint8_t percentage)
    {
        batteryPercentage = percentage;
    }

    /**
     * @brief Set the animation selection
    */
    void setAnimationSelection(uint8_t selection)
    {
        animationSelection = selection;
    }
    

    void ledAnimationTask(void *pvParameter)
    {
        while (true)
        {
            switch (animationSelection)
            {
            case 0:
                normalAnimation(batteryPercentage);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            case 1:
                chargingAnimation(batteryPercentage, false);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                chargingAnimation(batteryPercentage, true);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            case 2:
                swappingAnimation(false);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                swappingAnimation(true);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            default:
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;
            }
        }
    }

    void startupAnimation()
    {
        for (int i = 0; i < 10; i++)
        {
            normalAnimation(i * 10);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        animationSelection = 0;
    }

    void normalAnimation(uint8_t batteryPercentage)
    {
        uint8_t green = (uint8_t)((float)batteryPercentage * 2.55);
        uint8_t red = 255 - green;
        uint8_t ledsToFill = (uint8_t)((float)batteryPercentage * 0.01 * NEOPIXEL_NUM);
        for (int i = 0; i < ledsToFill; i++)
        {
            setPixel(i, red, green, 0);
        }
        for(int i = ledsToFill; i < NEOPIXEL_NUM; i++)
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
        if(ledsToFill == 0) ledsToFill = 1;
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
        for(int i = ledsToFill; i < NEOPIXEL_NUM; i++)
        {
            setPixel(i, 0, 0, 0);
        }
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
    }

    void swappingAnimation(bool blink)
    {
        if (blink)
        {
            for (int i = 0; i < NEOPIXEL_NUM; i++)
            {
                setPixel(i, 0, 0, 0);
            }
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
    
};

#endif