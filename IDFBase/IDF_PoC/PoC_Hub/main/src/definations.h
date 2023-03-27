#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define ESP_MAXIMUM_RETRY  50
#define ESP_WIFI_SSID      "HUB_1"
#define ESP_WIFI_PASS      "HUB_1_TEST"
#define ESP_WIFI_CHANNEL   1
#define MAX_STA_CONN       10
#define MAX_SOCKETS        7

#define BLE_TAG "NimBLE"
#define MAIN_TAG "Main"
#define WIFI_TAG "WiFi"
#define SERV_TAG "Server"
#define GPIO_TAG "GPIO"
#define TWAI_TAG "TWAI"
#define CAN_TAG "CAN"

#define ID_MASTER_PING      0x0B0
#define ID_MASTER_REQUEST   0x0B1
#define ID_MASTER_DONE      0x0B2
#define ID_MASTER_DATA      0x0B3

#define ID_PING_RESP      0x0A0
#define ID_REQUEST_RESP   0x0A1
#define ID_DATA_RESP      0x0A3

#define NEOPIXEL_PIN 15
#define NEOPIXEL_NUM 28



#endif