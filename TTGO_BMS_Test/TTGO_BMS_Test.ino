#include <ModbusClientRTU.h>
#include "BMSHandler.h"

ModbusClientRTU RS485(Serial1);
BMSHandler bms(&RS485);

void setup()
{
    Serial.begin(115200);
    Serial1.begin(19200, SERIAL_8N1, 13, 15);
    bms.initBMS();
    bms.updateBMSTelemetry();
}

void loop()
{
}