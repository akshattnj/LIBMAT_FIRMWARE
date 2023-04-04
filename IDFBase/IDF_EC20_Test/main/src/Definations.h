#ifndef DEFINATIONS_H
#define DEFINATIONS_H

// General EC20 Definations
#define EC20_TXD 26
#define EC20_RXD 25
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define EC20_PORT_NUM (uart_port_t) 1
#define EC20_BAUD_RATE 115200
#define EC20_STACK_SIZE 4096

#define BUFFER_LENGTH (1024)

// Telemetry Definations
#define TELEMETRY_DOMAIN "3.111.8.114"
#define TELEMETRY_PORT 1883
#define TELEMETRY_USERNAME "NHYWSw3xplNZCFjdtddz"
#define TELEMETRY_DEVICE_NAME "615651"
#define TELEMETRY_TOPIC "v1/devices/me/telemetry"


#define MAIN_TAG "Main"
#define EC20_TAG "UART Handler"

#endif