#ifndef BATTERY_HANDLER_H
#define BATTERY_HANDLER_H

#include "./Commons/Commons.h"
#include "./definations.h"

extern "C"
{
#include <driver/gpio.h>
}


namespace BatteryHandler
{
    void init();
    void setGPIO(gpio_num_t pin, uint8_t value);
    bool getGPIO(gpio_num_t pin);
    void handleDoor();
}
#endif