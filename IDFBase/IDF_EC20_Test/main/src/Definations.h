#ifndef DEFINATIONS_H
#define DEFINATIONS_H

// General EC20 Definations
#define EC20_TXD (gpio_num_t) 26
#define EC20_RXD (gpio_num_t) 25
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define EC20_PORT_NUM (uart_port_t) 1
#define EC20_BAUD_RATE 115200
#define EC20_STACK_SIZE 4096

#define BUF_SIZE (1024)

// Telemetry Definations
#define TELEMETRY_DOMAIN "3.111.8.114"
#define TELEMETRY_PORT 1883
#define TELEMETRY_USERNAME "ASDF"
#define TELEMETRY_DEVICE_NAME "IFWSi8vM1HWhJFWtVLCn"
#define TELEMETRY_TOPIC "v1/devices/me/telemetry"

// Cellular Flags
#define GOT_OK      BIT7
#define GOT_ERROR   BIT6
#define GSM_MODE    BIT5
#define LTE_MODE    BIT4
#define GSM_CONNECT BIT3
#define LTE_CONNECT BIT2

// MQTT Flags
#define SERVER_CONNECT  BIT7
#define SERVER_ERROR    BIT6
#define MQTT_CONNECT    BIT5
#define MQTT_ERROR      BIT4
#define SEND_READY      BIT3
#define SEND_SUCCESS    BIT2

#define MAIN_TAG "Main"
#define EC20_TAG "UART Handler"
#define TELEMETRY_TAG "Telemetry"

#endif