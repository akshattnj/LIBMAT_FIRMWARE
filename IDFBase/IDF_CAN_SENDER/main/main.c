#include "driver/can.h"

#define TX_PIN GPIO_NUM_5
#define RX_PIN GPIO_NUM_4
uint8_t variabledata1 = 0x12; 
uint8_t variabledata2 = 0x12; 


void app_main() {
    can_general_config_t g_config = {
        .mode = CAN_MODE_NORMAL,
        .tx_io = TX_PIN,
        .rx_io = RX_PIN,
        .clkout_io = CAN_IO_UNUSED,
        .bus_off_io = CAN_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = CAN_ALERT_NONE,
        .clkout_divider = 0
    };
    can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Initialize CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        can_start();
    } else {
        printf("CAN initialization failed\n");
        return;
    }

    // Continuously send CAN messages
    while (1) {
        // Send CAN message with identifier 0x18FF01D0 and variable data1
        can_message_t can_msg1 = {
            .identifier = 0x18FF01D0,
            .data_length_code = 8,
            .flags = 0,
            .data = {variabledata1, 0, 0, 0, 0, 0, 0, 0}
        };
        can_transmit(&can_msg1, pdMS_TO_TICKS(1000));

        // Send CAN message with identifier 0x18FF05D0 and variable data2
        can_message_t can_msg2 = {
            .identifier = 0x18FF05D0,
            .data_length_code = 8,
            .flags = 0,
            .data = {variabledata2, 0, 0, 0, 0, 0, 0, 0}
        };
        can_transmit(&can_msg2, pdMS_TO_TICKS(1000));
    }
}
