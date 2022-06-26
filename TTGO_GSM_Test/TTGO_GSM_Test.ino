#include <axp20x.h>
#include "PowerManagement.h"
#include "GSMHandler.h"
#include "Secrets.h"

AXP20X_Class axp;
PowerManagement power(&axp);
GSMHandler GSM(&Serial, &Serial1);

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    power.powerSetup();
    xTaskCreate([](void *parameters)
                { ESP_LOGD("TAG", "GSM response"); GSM.GSMResponseScan(parameters); },
                "GSM Response", 2048, NULL, 10, NULL);
    xTaskCreate([](void *parameters)
                { ESP_LOGD("TAG", "GSM handler"); GSM.handleGSM(parameters); },
                "GSM Handle", 4096, NULL, 10, NULL);
}

void loop()
{
}