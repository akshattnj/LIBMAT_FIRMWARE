#include "GPIOHandler.h"


namespace BATT
{
    const HubDoor doors[4] = {
        {
            .charge = DOOR_0_CHARGER,
            .lock = DOOR_0_LOCK,
            .sense = DOOR_0_SENSE,
        },
        {
            .charge = GPIO_NUM_NC,
            .lock = GPIO_NUM_NC,
            .sense = GPIO_NUM_NC,
        },
        {
            .charge = DOOR_2_CHARGER,
            .lock = DOOR_2_LOCK,
            .sense = DOOR_2_SENSE,
        },
        {
            .charge = GPIO_NUM_NC,
            .lock = GPIO_NUM_NC,
            .sense = GPIO_NUM_NC,
        },
    };

    void setupGPIO()
    {
        gpio_config_t ioConfig;
        ioConfig.intr_type = GPIO_INTR_DISABLE;
        ioConfig.mode = GPIO_MODE_OUTPUT;
        ioConfig.pin_bit_mask = GPIO_SEL_2 | GPIO_SEL_4 | GPIO_SEL_22 | GPIO_SEL_23;
        ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
        ioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&ioConfig);

        ioConfig.mode = GPIO_MODE_INPUT;
        ioConfig.pin_bit_mask = GPIO_SEL_32 | GPIO_SEL_33;
        ioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpio_config(&ioConfig);

        gpio_set_level(doors[0].charge, 0);
        gpio_set_level(doors[2].charge, 1);
        gpio_set_level(doors[0].lock, 0);
        gpio_set_level(doors[2].lock, 0);
    }

    void openDoor(uint8_t door)
    {
        if(gpio_get_level(doors[door].sense) == 0)
            return;
        ESP_LOGI(GPIO_TAG, "Opening door %d", door);
        gpio_set_level(doors[door].lock, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(doors[door].lock, 0);
    }

    void doorTask(void *params)
    {
        while(1)
        {
            openDoor(2);
            openDoor(0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        
    }

    void doorScanner(void *params)
    {
        while(1)
        {

            if(gpio_get_level(doors[0].sense) == 0)
                Commons::animationSelection[0] = 2;
            else
                Commons::animationSelection[0] = 1;

            if(gpio_get_level(doors[2].sense) == 0)
                Commons::animationSelection[2] = 2;
            else
                Commons::animationSelection[2] = 1;

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}