#include <Arduino.h>
#include <esp_log.h>
#include <ArduinoJson.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <Adafruit_ADS1X15.h>
#include "FileManager.h"

class TelemetryScanner
{
public:
    TelemetryScanner(MPU6050 *mpuPointer, TinyGPSPlus *gpsPointer, Adafruit_ADS1015 *adsPointer)
    {
        mpu = mpuPointer;
        gps = gpsPointer;
        ads = adsPointer;

        enableMPU = (mpu != NULL) ? true : false;
        enableGPS = (gps != NULL) ? true : false;
        enableADS = (ads != NULL) ? true : false;
    }

    void initialiseTelemetry()
    {
        fs.connectSDCard();
        initialiseMPU6050();
    }

private:
    MPU6050 *mpu;
    TinyGPSPlus *gps;
    Adafruit_ADS1015 *ads;
    StaticJsonDocument<1024> doc;
    FileManager fs;
    StaticJsonDocument<96> MPUData;

    const float A3_offset = 0.060;
    const float bakBatR1 = 96.31;
    const float bakBatR2 = 19.70;

    bool enableMPU = true;
    bool enableGPS = true;
    bool enableADS = true;
    bool SDMountStatus = false;

    char c[1024];

    /**
     * @brief Setup the MPU6050
     *
     */
    void initialiseMPU6050()
    {
        if (!enableMPU)
        {
            ESP_LOGI("Telemetry", "Skipping MPU6050 setup");
            return;
        }
        uint8_t attempts = 0;
        uint8_t status = 255;
        while (status != 0)
        {
            status = mpu->begin();
            if (status != 0)
            {
                ESP_LOGE(TAG, "Error in communication with MPU6050. Status code: %d", status);
                if (attempts > 4)
                {
                    ESP_LOGE("Telemetry", "Unable to reach MPU. Please check connections");
                    return;
                }
                ESP_LOGI(TAG, "Retrying in 2 seconds");
                attempts++;
            }
            delay(2000);
            yield();
        }
        ESP_LOGI(TAG, "MPU6050 connection success. Status: %d", status);

        if (!fs.readFile("/MPUOffsets.txt", c, 1024))
        {
            mpu->calcOffsets();
            MPUData["accXOff"] = mpu->getAccXoffset();
            MPUData["accYOff"] = mpu->getAccYoffset();
            MPUData["accZOff"] = mpu->getAccZoffset();
            MPUData["gyroXOff"] = mpu->getGyroXoffset();
            MPUData["gyroYOff"] = mpu->getGyroYoffset();
            MPUData["gyroZOff"] = mpu->getGyroZoffset();
            memset(c, 0, 1024);
            serializeJson(MPUData, c);
            fs.writeFile("/MPUOffsets.txt", c);
        }
        else
        {
            deserializeJson(MPUData, c);
            mpu->setAccOffsets(MPUData["accXOff"], MPUData["accYOff"], MPUData["accZOff"]);
            mpu->setGyroOffsets(MPUData["gyroXOff"], MPUData["gyroYOff"], MPUData["gyroZOff"]);
        }
    }
};