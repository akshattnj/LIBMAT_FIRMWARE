#include "BMSHandler.h"

void setup()
{
    Serial.begin(115200);
    Serial1.begin(19200, SERIAL_8N1, 13, 15);
    initBMS();
}

void loop()
{
    updateBMSTelemetry();
    getBMSTelemetry();
    delay(1000);
}