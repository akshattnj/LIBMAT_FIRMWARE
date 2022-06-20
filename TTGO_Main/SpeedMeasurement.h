#include <esp_log.h>
#include <Arduino.h>

class SpeedMeasurement
{
public:
    const uint8_t pin;
    const bool config = true;
    uint16_t rpm = 0;
    float rps = 0.00;

    SpeedMeasurement(uint8_t pin) : pin(pin)
    {
        pinMode(pin, INPUT);
    }

    SpeedMeasurement(uint8_t pin, bool config) : pin(pin), config(config)
    {
        pinMode(pin, INPUT);
    }

    void measureSpeed(void *parameters)
    {
        while (1)
        {
            currentState = (digitalRead(pin) == config);
            if (currentState && !previousState)
            {
                current = millis();
                if (previous != 0 && current != previous)
                {
                    rps = (1000.00 / (current - previous));
                    rpm = rps * 60;
                    //ESP_LOGI("TAG", "RPM: %u, %f, %d, %d", rpm, rps, previousState, currentState);
                }
                previous = current;
            }
            previousState = currentState;
            delay(2);
        }
    }

private:
    uint32_t current = 0L;
    uint32_t previous = 0L;
    bool previousState = false;
    bool currentState = false;
};
