#define I2C_SDA 21
#define I2C_SCL 22

#include <esp_log.h>
#include <MPU6050_light.h>
#include "TelemetryScanner.h"

MPU6050 mpu(Wire);

TelemetryScanner ts(&mpu, NULL, NULL);

void setup(void)
{
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);
    ts.initialiseTelemetry();
}

void loop(void)
{
}