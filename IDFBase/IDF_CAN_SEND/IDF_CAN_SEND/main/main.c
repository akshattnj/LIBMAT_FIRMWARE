#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/can.h"
#include "esp_log.h"

static const char *TAG = "CAN";

static void twai_receive_callback(void *arg, twai_message_t *message) {
    // Check if the received message has the desired identifier
    if (message->identifier == 0x18FF01D0) {
        // Convert the data to hexadecimal format
        char data_str[20];
        sprintf(data_str, "0x%02x 0x%02x 0x%02x 0x%02x",
                message->data[0], message->data[1], message->data[2], message->data[3]);
                
        // Print the data to the console
        printf("Received message with identifier: 0x%08x\n", message->identifier);
        printf("%s\n", data_str);
        
        // Create a CAN message with the received data
        can_message_t can_message;
        can_message.identifier = message->identifier;
        can_message.flags = CAN_MSG_FLAG_EXTD;
        can_message.data_length_code = 4;
        can_message.data[0] = message->data[0];
        can_message.data[1] = message->data[1];
        can_message.data[2] = message->data[2];
        can_message.data[3] = message->data[3];
        
        // Send the CAN message over the bus
        can_transmit(&can_message, pdMS_TO_TICKS(1000));
    }
}

void app_main() {
    // Configure CAN controller
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    can_driver_install(&g_config, &t_config, &f_config);
    
    // Set up the receive callback function
    twai_receive_callback(twai_receive_callback, NULL);
    
    // Create a CAN message with the desired data
    can_message_t can_message;
    can_message.identifier = 0x18FF01D0;
    can_message.flags = CAN_MSG_FLAG_EXTD;
    can_message.data_length_code = 4;
    can_message.data[0] = 0x00;
    can_message.data[1] = 0x00;
    can_message.data[2] = 0x00;
    can_message.data[3] = 0x00;
    
    // Send the CAN message over the bus
    can_transmit(&can_message, pdMS_TO_TICKS(1000));
    
    // Wait indefinitely
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
