#define I2C_SDA 21
#define I2C_SCL 22
#define GPS_RX 34
#define GPS_TX 14
#define BMS_RX 13
#define BMS_TX 15
#define SerialBMS Serial1

#include <esp_log.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <Adafruit_ADS1X15.h>
#include <SoftwareSerial.h>

#include "TelemetryScanner.h"
#include "BMSHandler.h"

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

    SerialBMS.begin(19200, SERIAL_8N1, BMS_RX, BMS_TX);
    initBMS();

    ESP_LOGI("TAG", "Serial Setup Complete");

    xTaskCreate([](void *parameters)
                { ts.handleGPS(parameters); },
                "GPS Handler", 2048, NULL, 12, NULL);

    xTaskCreate([](void *parameters)
                { ts.handleI2CTelemetry(parameters); },
                "ADC Scanner", 2048, NULL, 10, NULL);

    xTaskCreate(updateBMSTelemetry, "BMS Telemetry", 2048, NULL, 15, NULL);

    ESP_LOGI("TAG", "Tasl Generation Complete");
}

void loop(void)
{
    ts.getTelemetry();
    getBMSTelemetry();
    delay(1000);
}