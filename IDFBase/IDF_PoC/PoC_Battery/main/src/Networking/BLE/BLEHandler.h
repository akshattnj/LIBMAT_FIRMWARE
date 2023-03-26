#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#define DEVICE_NAME "ESP32"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#include "../../Commons/Commons.h"
#include "../../definations.h"
#include "NimBLEDevice.h"
#include "NimBLELog.h"

#include <esp_log.h>

namespace BLE
{
    void initialiseBLE();
    void sendData(char *data);
    void telemetryTask(void *params);
}

#endif