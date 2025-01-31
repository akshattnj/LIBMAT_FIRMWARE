#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define BLE_TAG "NimBLE"
#define MAIN_TAG "Main"
#define WIFI_TAG "WiFi Client"
#define WS_TAG "Websocket Client"

#define BLE_DEVICE_NAME "ESP_COM_16"

#define GATT_SVR_SVC_ALERT_UUID               0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID   0x2A47
#define GATT_SVR_CHR_NEW_ALERT                0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID   0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID      0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT        0x2A44

#define ESP_WIFI_SSID      "SSID"
#define ESP_WIFI_PASS      "Password"
#define ESP_MAXIMUM_RETRY  5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK

#define WS_URI CONFIG_WS_URI

#endif