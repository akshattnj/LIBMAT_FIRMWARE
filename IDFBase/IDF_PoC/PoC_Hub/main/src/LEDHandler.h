#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "./Commons/Commons.h"
#include "./definations.h"

extern "C"
{
#include <driver/rmt.h>
#include <led_strip.h>
}

namespace LED
{
    void setPixel(uint8_t pixelNum, uint32_t red, uint32_t green, uint32_t blue);
    void clear(uint8_t slot);
    void init();
    void ledAnimationTask(void *pvParameter);
    void startupAnimation();
    void normalAnimation(uint8_t batteryPercentage, uint8_t slot);
    void chargingAnimation(uint8_t batteryPercentage, bool lastBlink, uint8_t slot);
    void swappingAnimation(bool blink, uint8_t slot);
}

#endif