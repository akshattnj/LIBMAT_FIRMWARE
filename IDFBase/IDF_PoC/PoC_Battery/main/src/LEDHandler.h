#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "src/Commons/Commons.h"
#include "src/definations.h"

extern "C"
{
#include <driver/rmt.h>
#include <led_strip.h>
extern xQueueHandle queueHandle;
}

namespace LED
{
    void setPixel(uint8_t pixelNum, uint32_t red, uint32_t green, uint32_t blue);
    void clear();
    void init();
    void ledAnimationTask(void *pvParameter);
    void startupAnimation();
    void normalAnimation(uint8_t batteryPercentage);
    void chargingAnimation(uint8_t batteryPercentage, bool lastBlink);
    void swappingAnimation(bool blink);
    void lowBatteryAnimation();
    void noCanDataAnimation(bool blink);
}

#endif