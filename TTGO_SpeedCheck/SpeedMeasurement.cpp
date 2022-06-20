#include "SpeedMeasurement.h"

SpeedMeasurement::SpeedMeasurement(uint8_t pin) : pin(pin)
{
    pinMode(pin, INPUT);
}
SpeedMeasurement::SpeedMeasurement(uint8_t pin, bool config) : pin(pin), config(config)
{
    pinMode(pin, INPUT);
}

void SpeedMeasurement::measureSpeed(void *parameters)
{
    while (1)
    {
        SpeedMeasurement::currentState = (digitalRead(SpeedMeasurement::pin) == SpeedMeasurement::config);
        if (SpeedMeasurement::currentState && !SpeedMeasurement::previousState)
        {
            SpeedMeasurement::current = millis();
            if (SpeedMeasurement::previous != 0 && SpeedMeasurement::current != SpeedMeasurement::previous)
            {
                SpeedMeasurement::rps = (1000.00 / (SpeedMeasurement::current - SpeedMeasurement::previous));
                SpeedMeasurement::rpm = SpeedMeasurement::rps * 60;
                ESP_LOGI("TAG", "RPM: %u, %f, %d, %d", SpeedMeasurement::rpm, SpeedMeasurement::rps, SpeedMeasurement::previousState, SpeedMeasurement::currentState);
            }
            SpeedMeasurement::previous = SpeedMeasurement::current;
        }
        SpeedMeasurement::previousState = SpeedMeasurement::currentState;
        delay(2);
    }
}