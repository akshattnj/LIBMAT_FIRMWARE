#ifndef NIMBLE_H
#define NIMBLE_H

#include <stdbool.h>
#include <nimble/ble.h>
#include <modlog/modlog.h>

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;
extern bool notify_state;

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);
void print_bytes(const uint8_t *bytes, int len);
void print_addr(const void *addr);

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
void sendNotification(char *notification, size_t notificationLen);
void startBLE();
void stopBLE();
void startNVS();

#endif



