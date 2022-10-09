#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define EC20_TXD (CONFIG_EC20_TXD)
#define EC20_RXD (CONFIG_EC20_RXD)
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define EC20_PORT_NUM      (CONFIG_EC20_PORT_NUM)
#define EC20_BAUD_RATE     (CONFIG_EC20_BAUD_RATE)
#define EC20_STACK_SIZE    (CONFIG_EC20_STACK_SIZE)

#define BUF_SIZE (1024)

#define TELEMETRY_DOMAIN (CONFIG_MQTT_DOMAIN)
#define TELEMETRY_PORT (CONFIG_MQTT_PORT)
#define TELEMETRY_USERNAME (CONFIG_MQTT_USER)
#define TELEMETRY_DEVICE_NAME (CONFIG_MQTT_DEVICE)
#define TELEMETRY_TOPIC (CONFIG_MQTT_TOPIC)

#define MAIN_TAG "Main"
#define EC20_TAG "UART Handler"
#define TELEMETRY_TAG "Telemetry"


#endif