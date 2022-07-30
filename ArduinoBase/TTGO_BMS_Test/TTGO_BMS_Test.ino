#include "BMSHandler.h"

void setup()
{
    Serial.begin(115200);
    Serial1.begin(19200, SERIAL_8N1, 13, 15);
    initBMS();
    xTaskCreate(updateBMSTelemetry, "BMS Telemetry", 2048, NULL, 15, NULL);
}

void loop()
{
    getBMSTelemetry();
    delay(1000);
}