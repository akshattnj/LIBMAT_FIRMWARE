/**
 * @file TelemetryScanner.h
 * @author Antony (ajosekuruvilla@gmail.com)
 * @brief .h File to handle all telemetry related functions. Depends on FileManager.h and LEDHandler.h
 * @version 0.5
 * @date 2022-07-04
 *
 * @copyright Movio Mobility Pvt. Ltd. (c) 2022
 *
 */
#include <Arduino.h>
#include <esp_log.h>
#include <ArduinoJson.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <Adafruit_ADS1X15.h>
#include <SoftwareSerial.h>

#include "LEDHandler.h"
#include "FileManager.h"

class TelemetryScanner
{
public:
    MPU6050 *mpu;
    TinyGPSPlus *gps;
    Adafruit_ADS1115 *ads;
    SoftwareSerial *gpsSerial;
    LEDHandler *led;
    StaticJsonDocument<512> telemetryDoc;

    bool enableMPU = true;
    bool enableGPS = true;
    bool enableADS = true;

    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3, volts_bkp_batt, volts_ev_batt, degree_celcius, current;
    float *batteryPercent;

    TelemetryScanner(MPU6050 *mpuPointer, TinyGPSPlus *gpsPointer, SoftwareSerial *gpsSer, Adafruit_ADS1115 *adsPointer, LEDHandler *ledPointer, float *remainingPower)
    {
        mpu = mpuPointer;
        gps = gpsPointer;
        gpsSerial = gpsSer;
        ads = adsPointer;
        led = ledPointer;
        batteryPercent = remainingPower;

        enableMPU = (mpu != NULL) ? true : false;
        enableGPS = (gps != NULL) ? true : false;
        enableADS = (ads != NULL) ? true : false;
    }

    void initialiseTelemetry()
    {
        fs.connectSDCard();
        initialiseMPU6050();
        initialiseADS();
    }

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
                    enableMPU = false;
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
            ESP_LOGD("Telemetry", "Got offset data: \n%f\n%f\n%f\n%f\n%f\n%f", MPUData["accXOff"], MPUData["accYOff"],
                     MPUData["accZOff"], MPUData["gyroXOff"], MPUData["gyroYOff"], MPUData["gyroZOff"]);
            mpu->setAccOffsets(MPUData["accXOff"], MPUData["accYOff"], MPUData["accZOff"]);
            mpu->setGyroOffsets(MPUData["gyroXOff"], MPUData["gyroYOff"], MPUData["gyroZOff"]);
        }
    }

    void initialiseADS()
    {
        if (!enableADS)
        {
            ESP_LOGI("Telemetry", "Skipping ADS1115 setup");
            return;
        }

        uint8_t attempts = 0;
        while (!ads->begin())
        {
            ESP_LOGE("ADC setup failed. Retrying in 2 sec");
            attempts++;
            if (attempts > 5)
            {
                ESP_LOGE("TAG", "Unable to communicate with ADS");
                enableADS = false;
                break;
            }
            delay(2000);
        }
    }

    void handleGPS(void *parameters)
    {
        if (!enableGPS)
            return;
        while (true)
        {
            while (gpsSerial->available())
                gps->encode(gpsSerial->read());
            delay(10);
            yield();
        }
    }

    void handleI2CTelemetry(void *parameters)
    {
        while (1)
        {
            if (enableADS)
            {
                adc0 = ads->readADC_SingleEnded(0);
                adc1 = ads->readADC_SingleEnded(1);
                adc2 = ads->readADC_SingleEnded(2);
                adc3 = ads->readADC_SingleEnded(3);

                volts0 = ads->computeVolts(adc0);
                volts1 = ads->computeVolts(adc1);
                volts2 = ads->computeVolts(adc2);
                volts3 = ads->computeVolts(adc3) - A3_offset;

                volts_bkp_batt = calc_batt_voltage(volts3);
                volts_ev_batt = calc_ev_voltage(volts1) - 0.80;
                degree_celcius = calc_ntc_temp(adc2);
                current = ev_current(adc0);

                led->batteryBar(*batteryPercent);
            }

            delay(1);
            yield();

            if (enableMPU)
            {
                mpu->update();
            }

            delay(1);
            yield();
        }
    }

    void getTelemetry()
    {
        telemetryDoc.clear();
        if (enableADS)
        {
            ESP_LOGI("Telemetry", "Current Draw(0) : %0.2f A\nEV Voltage(1): %0.2f V\nTemprature(2) : %0.2f °C\nBackup batt. Voltage(3) : %0.2f V\nRaw Data: %u %u %u %u", current, volts_ev_batt, degree_celcius, volts_bkp_batt, adc0, adc1, adc2, adc3);
            telemetryDoc["CurrentDraw(ADC)"] = current;
            telemetryDoc["EV Voltage(ADC)"] = volts_ev_batt;
            telemetryDoc["Temprature(ADC)"] = degree_celcius;
            telemetryDoc["BackupVoltage(ADC)"] = volts_bkp_batt;
        }
        if (enableGPS)
        {
            ESP_LOGI("Telemetry", "GPS Chars Processed: %lu", gps->charsProcessed());
            telemetryDoc["Latitude"] = gps->location.lat();
            telemetryDoc["Longitude"] = gps->location.lng();
        }
        if (enableMPU)
        {
            ESP_LOGI("Telemetry", "Angles: %f %f\nAccl: %f %f %f\nGyro: %f %f %f", mpu->getAngleX(), mpu->getAngleY(), mpu->getAccX(), mpu->getAccY(), mpu->getAccZ(), mpu->getGyroX(), mpu->getGyroY(), mpu->getGyroZ());
            telemetryDoc["pitch"] = mpu->getAngleX();
            telemetryDoc["roll"] = mpu->getAngleY();
            telemetryDoc["yaw"] = mpu->getAngleZ();
        }
    }

private:
    StaticJsonDocument<1024> doc;
    FileManager fs;
    StaticJsonDocument<96> MPUData;

    const float A3_offset = 0.060;
    const float bakBatR1 = 96.31;
    const float bakBatR2 = 19.70;

    bool SDMountStatus = false;

    float calc_batt_voltage(float vout)
    {
        float r1 = 96.31;
        float r2 = 19.70;
        float vin = (vout * (r1 + r2)) / r2;
        return vin;
    }
    float calc_ev_voltage(float vout)
    {
        float r1 = 99.80;
        float r2 = 4.60;
        float vin = (vout * (r1 + r2)) / r2;
        return vin;
    }

    float calc_ntc_temp(int vout)
    {
        int Vo;
        float R1 = 11400;
        float logR2, R2, T;
        const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
        Vo = vout;
        R2 = R1 * (1023.0 / (float)Vo - 1.0);
        logR2 = log(R2);
        T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
        T = -T + 273.15;
        return T;
    }

    float ev_current(float adc)
    {
        return ((adc - 8688) * 0.3052 / 40.00);
    }
    char c[1024];
};