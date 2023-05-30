#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define PCF8575_ADDR_BASE 0x20   // Base address of the PCF8575 devices

#define I2C_MASTER_SCL_IO 22       /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21       /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0    /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0    /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000  /*!< I2C master clock frequency */

#define NUM_DEVICES 3   // Number of PCF8575 devices connected

uint8_t device_addresses[NUM_DEVICES] = {0x20, 0x21, 0x22};   // I2C addresses of each PCF8575 device

void pcf8575_write_register(uint8_t device, uint16_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addresses[device] << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, val & 0xFF, true);
    i2c_master_write_byte(cmd, (val >> 8) & 0xFF, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        printf("Error: Failed to write PCF8575 register: %d\n", ret);
    }
    else{
        printf("Success: Wrote PCF8575 register\n");
    }
}

void update_gpio_state(uint8_t device, uint8_t pin, uint8_t state)
{
    uint16_t gpio_state = 0;
    gpio_state |= (1 << pin);   // Set the bit for the specified pin

    if (state == 0)
    {
        gpio_state = ~gpio_state;   // Invert the state if turning off the pin
    }

    pcf8575_write_register(device, gpio_state);
}

void turn_on_pin(uint8_t device, uint8_t pin)
{
    update_gpio_state(device, pin, 1);
    printf("Turned on pin %d of device %d\n", pin, device);
}

void turn_off_pin(uint8_t device, uint8_t pin)
{
    update_gpio_state(device, pin, 0);
    printf("Turned off pin %d of device %d\n", pin, device);
}

void app_main(void)
{
    // Configure I2C master
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ,
        },
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    // Turn on all pins of all devices one by one
    while (1)
    {
        for (uint8_t i = 0; i < NUM_DEVICES; i++)
        {
            for (uint8_t j = 0; j < 16; j++)
            {
                turn_on_pin(i, j);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }

        // Turn off all pins of all devices one by one
        for (uint8_t i = 0; i < NUM_DEVICES; i++)
        {
            for (uint8_t j = 0; j < 16; j++)
            {
                turn_off_pin(i, j);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
    }
}
