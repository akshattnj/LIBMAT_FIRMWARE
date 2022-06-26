#define MODEM_TX 27
#define MODEM_RX 26
#define TINY_GSM_MODEM_SIM800
#define MAX_ARRAY_WIDTH 1024

#include <axp20x.h>
#include "PowerManagement.h"

AXP20X_Class axp;
PowerManagement power(&axp);

void setup()
{
    Serial.begin(115200);
    power.powerSetup();
}

void loop()
{
}