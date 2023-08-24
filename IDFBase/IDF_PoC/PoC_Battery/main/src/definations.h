#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define ESP_MAXIMUM_RETRY  5
#define MAX_WIFI_LIST_SIZE 20
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK

#define WS_URI "ws://192.168.4.1/main"

// EC20 Definations
#define EC20_TXD 17
#define EC20_RXD 16
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define EC20_PORT_NUM (uart_port_t) 1
#define EC20_BAUD_RATE 115200
#define EC20_STACK_SIZE 4096

#define BUFFER_LENGTH (1024)

// Telemetry Definations
#define TELEMETRY_DOMAIN "122.180.246.214"
#define TELEMETRY_PORT 1883
#define TELEMETRY_USERNAME "movio001"
#define TELEMETRY_DEVICE_NAME "pulse_1"
#define TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define RPC_TOPIC "v1/devices/me/rpc/request/+"
#define RPC_RESPONSE_TOPIC "v1/devices/me/rpc/response/"

#define TWAI_TX (gpio_num_t)26
#define TWAI_RX (gpio_num_t)25

#define BLE_TAG "NimBLE"
#define MAIN_TAG "Main"
#define WIFI_TAG "WiFi"
#define WS_TAG "Websocket Client"
#define I2C_TAG "I2C"
#define MQTT_TAG "MQTT"
#define TWAI_TAG "TWAI"
#define EC20_TAG "EC20"
#define I2C_TAG "I2C"

#define NEOPIXEL_PIN 23
#define NEOPIXEL_NUM 5

#define ID_MASTER_PING      0x0B0
#define ID_MASTER_REQUEST   0x0B1
#define ID_MASTER_DONE      0x0B2
#define ID_MASTER_DATA      0x0B3

#define ID_PING_RESP      0x0A0
#define ID_REQUEST_RESP   0x0A1
#define ID_DATA_RESP      0x0A3

#define BMS_STATE_ID 0x18FF05D0
#define BMS_VOLTAGE_ID 0x18FF01D0

#define MQTTDelay 30000000

#define SDA_0_PIN 21
#define SCL_0_PIN 22
#define I2C_0_CLOCK 100000
    
// AHT related definations
#define AHT_ADDRESS 0x38

#define AHT_ENABLED BIT0
#define AHT_BUSY BIT7
#define AHT_CALIB BIT3
#define AHT_READ_BUFFER 10


#endif