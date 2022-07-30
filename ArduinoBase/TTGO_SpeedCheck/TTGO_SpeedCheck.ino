#include "SpeedMeasurement.h"

SpeedMeasurement speed(34);

void setup()
{
    Serial.begin(115200);
    xTaskCreate([](void *parameters)
                { speed.measureSpeed(parameters); },
                "Speed Measurement", 2048, NULL, 10, NULL);
}

void loop()
{
}
