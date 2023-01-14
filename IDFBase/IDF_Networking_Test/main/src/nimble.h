#ifndef NIMBLE_H
#define NIMBLE_H

extern bool notify_state;
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
void sendNotification(char *notification, size_t notificationLen);
void startBLE();
void stopBLE();
void startNVS();

#endif



