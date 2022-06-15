struct SpeedMeasurement
{
    const uint8_t pin;
    uint32_t current;
    uint32_t previous;
    uint16_t rpm;
    float rps;
    bool previousState;
    bool currentState;
    const bool config;
};

SpeedMeasurement speed = {34, 0L, 0L, 0, false, false, true};

void setup()
{
    Serial.begin(115200);
    pinMode(speed.pin, INPUT);
}

/**
 * @todo Run speed calculations based on RPM/RPS
 *
 */
void loop()
{

    speed.currentState = (digitalRead(speed.pin) == speed.config);
    if (speed.currentState && !speed.previousState)
    {
        speed.current = millis();
        if (speed.previous != 0 && speed.current != speed.previous)
        {
            speed.rps = (1000.00 / (speed.current - speed.previous));
            speed.rpm = speed.rps * 60;
            ESP_LOGI("TAG", "RPM: %u, %f, %d, %d", speed.rpm, speed.rps, speed.previousState, speed.currentState);
        }
        speed.previous = speed.current;
    }
    speed.previousState = speed.currentState;
    delay(1);
}
