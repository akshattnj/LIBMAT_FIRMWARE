#ifndef NIMBLE_H
#define NIMBLE_H

#include "../definations.h"

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <esp_nimble_hci.h>
#include <nimble/ble.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <host/ble_uuid.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>

#include <nvs_flash.h>
#include <console/console.h>

#include <esp_log.h>
#include <modlog/modlog.h>

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;
extern bool notify_state;

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);
void sendNotification(char *notification, size_t notificationLen);
void startBLE();
void stopBLE();

#endif



