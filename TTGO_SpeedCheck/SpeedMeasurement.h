#include <esp_log.h>
#include <Arduino.h>

class SpeedMeasurement
{
public:
    const uint8_t pin;
    const bool config = true;
    uint16_t rpm = 0;
    float rps = 0.00;
    SpeedMeasurement(uint8_t pin);
    SpeedMeasurement(uint8_t pin, bool config);
    void measureSpeed(void *parameters);

private:
    uint32_t current = 0L;
    uint32_t previous = 0L;
    bool previousState = false;
    bool currentState = false;
};