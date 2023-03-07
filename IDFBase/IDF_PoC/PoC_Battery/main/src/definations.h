#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define ESP_MAXIMUM_RETRY  5
#define MAX_WIFI_LIST_SIZE 20
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK

#define WS_URI "ws://192.168.4.1/main"

// MQTT Related stuff
#define DEVICE_ID "20ec6300-b54f-11ed-9d5b-213ae9a75e81"
#define DEVICE_ACCESS_ID "cfb62967-c874-486b"
#define MQTT_URI "mqtt://3.111.8.114:1883"

#define BLE_TAG "NimBLE"
#define MAIN_TAG "Main"
#define WIFI_TAG "WiFi"
#define SERV_TAG "Server"
#define GPIO_TAG "GPIO"
#define TWAI_TAG "TWAI"
#define CAN_TAG "CAN"

// BLE Definations
#define BLE_DEVICE_NAME "ESP_COM_4"
#define GATT_SVR_SVC_ALERT_UUID               0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID   0x2A47
#define GATT_SVR_CHR_NEW_ALERT                0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID   0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID      0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT        0x2A44

#define ID_MASTER_PING      0x0B0
#define ID_MASTER_REQUEST   0x0B1
#define ID_MASTER_DONE      0x0B2
#define ID_MASTER_DATA      0x0B3

#define ID_PING_RESP      0x0A0
#define ID_REQUEST_RESP   0x0A1
#define ID_DATA_RESP      0x0A3



#endif