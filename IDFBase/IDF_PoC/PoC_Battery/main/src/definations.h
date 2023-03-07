#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define ESP_MAXIMUM_RETRY  5
#define MAX_WIFI_LIST_SIZE 20
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK

#define WS_URI "ws://192.168.4.1/main"

// MQTT Related stuff
#define DEVICE_ID "21d930c0-baf1-11ed-b4d9-13b68cb12c7e"
#define DEVICE_ACCESS_ID "IFWSi8vM1HWhJFWtVLCn"
#define MQTT_URI "mqtt://3.111.8.114:1883"

#define SSID "Movio_Mobility"
#define PASSWORD "movio@12334"

#define BLE_TAG "NimBLE"
#define MAIN_TAG "Main"
#define WIFI_TAG "WiFi"
#define WS_TAG "Websocket Client"
#define I2C_TAG "I2C"
#define MQTT_TAG "MQTT"
#define TWAI_TAG "TWAI"

#define NEOPIXEL_PIN 16
#define NEOPIXEL_NUM 5

#define ID_MASTER_PING      0x0B0
#define ID_MASTER_REQUEST   0x0B1
#define ID_MASTER_DONE      0x0B2
#define ID_MASTER_DATA      0x0B3

#define ID_PING_RESP      0x0A0
#define ID_REQUEST_RESP   0x0A1
#define ID_DATA_RESP      0x0A3



#endif