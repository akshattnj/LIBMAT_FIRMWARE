#include <stdio.h>
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 22   // GPIO pin for I2C clock
#define I2C_MASTER_SDA_IO 21   // GPIO pin for I2C data
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
#define I2C_MASTER_FREQ_HZ 400000 // I2C clock frequency

#define PN532_I2C_ADDRESS 0x48 // I2C address of the PN532 card reader

esp_err_t i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.scl_io_num = 22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main()
{
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        printf("Failed to initialize I2C master!\n");
        return;
    }

    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PN532_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    printf("Received data: %02X\n", data);
}
