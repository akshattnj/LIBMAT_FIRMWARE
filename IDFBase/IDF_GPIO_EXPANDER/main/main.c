#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define MCP23017_ADDR 0x20
#define MCP23017_IODIRA 0x00
#define MCP23017_GPIOA 0x12

#define I2C_MASTER_SCL_IO 22		/*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21		/*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0	/*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000	/*!< I2C master clock frequency */

void mcp23017_write_register(uint8_t reg, uint8_t val)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg, true);
	i2c_master_write_byte(cmd, val, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK)
	{
		printf("Error: Failed to write MCP23017 register: %d\n", ret);
	}
	else{
		printf("Success: Wrote MCP23017 register\n");
	}
}

void turn_on_channel(uint8_t channel)
{
	uint16_t gpio_state = (1 << channel);
	mcp23017_write_register(MCP23017_GPIOA, gpio_state);
	printf("Turned on channel %d\n", channel);
}

void turn_off_channel(uint8_t channel)
{
	uint16_t gpio_state = ~(1 << channel);
	mcp23017_write_register(MCP23017_GPIOA, gpio_state);
	printf("Turned off channel %d\n", channel);
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

	// Configure MCP23017
	mcp23017_write_register(MCP23017_IODIRA, 0x00); // Set all pins to output

	// Turn on all channels one by one
	while(1){
	for (uint8_t i = 0; i < 16; i++)
	{
		turn_on_channel(i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	// Turn off all channels one by one
	for (uint8_t i = 0; i < 16; i++)
	{
		turn_off_channel(i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	}
}
