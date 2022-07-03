#define I2C_SDA 21
#define I2C_SCL 22
#define GPS_RX 34
#define GPS_TX 14

#include <esp_log.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <Adafruit_ADS1X15.h>
#include <SoftwareSerial.h>

#include "TelemetryScanner.h"

MPU6050 mpu(Wire);
TinyGPSPlus gps;
Adafruit_ADS1115 ads;
SoftwareSerial SerialGPS;

TelemetryScanner ts(&mpu, &gps, &SerialGPS, &ads);

void setup(void)
{
    Serial.begin(115200);
    SerialGPS.begin(9800, SWSERIAL_8N1, GPS_RX, GPS_TX);
    Wire.begin(I2C_SDA, I2C_SCL);
    ts.initialiseTelemetry();

    if (ts.enableGPS)
        xTaskCreate([](void *parameters)
                    { ts.handleGPS(parameters); },
                    "GPS Handler", 2048, NULL, 12, NULL);
    xTaskCreate([](void *parameters)
                { ts.handleI2CTelemetry(parameters); },
                "ADC Scanner", 2048, NULL, 10, NULL);
}

void loop(void)
{
    ts.getTelemetry();
    delay(1000);
}