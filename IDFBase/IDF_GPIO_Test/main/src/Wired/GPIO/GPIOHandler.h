#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

extern "C"
{
#include <driver/gpio.h>
}

#include "../../definations.h"
#include "../../Commons/Commons.h"

namespace BATT
{

    typedef struct door{
        gpio_num_t charge;
        gpio_num_t lock;
        gpio_num_t sense;
    } HubDoor;

    void setupGPIO();
    void openDoor(uint8_t door);
    void doorTask(void *params);
    void doorScanner(void *params);
}

#endif