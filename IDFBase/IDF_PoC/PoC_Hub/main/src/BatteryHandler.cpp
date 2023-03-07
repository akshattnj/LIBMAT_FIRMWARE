#include "BatteryHandler.h"

namespace BatteryHandler
{
    const gpio_num_t doors[2] = {GPIO_NUM_2, GPIO_NUM_4};
    const gpio_num_t chargers[2] = {GPIO_NUM_22, GPIO_NUM_23};
    const gpio_num_t sensors[2] = {GPIO_NUM_32, GPIO_NUM_33};

    void init()
    {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = GPIO_SEL_2 | GPIO_SEL_4 | GPIO_SEL_22 | GPIO_SEL_23;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);

        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = GPIO_SEL_32 | GPIO_SEL_33;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpio_config(&io_conf);

        setGPIO(chargers[1], 1);
    }


    void setGPIO(gpio_num_t pin, uint8_t value)
    {
        gpio_set_level(pin, value);
    }

    bool getGPIO(gpio_num_t pin)
    {
        return gpio_get_level(pin);
    }

    void handleDoor()
    {
        if (Commons::activeDoor)
        {
            uint8_t command = 0x01;
            setGPIO(doors[0], 1);
            xQueueSend(Commons::queueCAN, &command, 0);
            xSemaphoreTake(Commons::semaphoreCAN, portMAX_DELAY);
            setGPIO(doors[0], 0);
            while(1)
            {
                if (getGPIO(sensors[0]) == 1)
                {
                    break;
                }
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
            setGPIO(chargers[0], 1);
            
            setGPIO(chargers[1], 0);
            setGPIO(doors[1], 1);
            vTaskDelay(2500 / portTICK_PERIOD_MS);
            setGPIO(doors[1], 0);
            while(1)
            {
                if (getGPIO(sensors[1]) == 1)
                {
                    break;
                }
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
            Commons::activeDoor = !Commons::activeDoor;
        }
        else
        {
            setGPIO(doors[1], 1);
            vTaskDelay(2500 / portTICK_PERIOD_MS);
            setGPIO(doors[1], 0);
            while(1)
            {
                if (getGPIO(sensors[1]) == 1)
                {
                    break;
                }
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
            setGPIO(chargers[1], 1);

            uint8_t command = 0x02;
            setGPIO(chargers[0], 0);
            setGPIO(doors[0], 1);
            xQueueSend(Commons::queueCAN, &command, 0);
            xSemaphoreTake(Commons::semaphoreCAN, portMAX_DELAY);
            setGPIO(doors[0], 0);
            while(1)
            {
                if (getGPIO(sensors[0]) == 1)
                {
                    break;
                }
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
            Commons::activeDoor = !Commons::activeDoor;
        }
    }
}