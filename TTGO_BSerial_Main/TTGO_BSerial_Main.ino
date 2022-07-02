#define PIN 33
#define NUMPIXELS 6

#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <Adafruit_ADS1X15.h>
#include <esp_log.h>
#include "LEDHandler.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
BluetoothSerial SerialBT;
Adafruit_ADS1115 ads;
LEDHandler led(&pixels);

float A3_offset = 0.060;
int received;      // received value will be stored in this variable
char receivedChar; // received value will be stored as CHAR in this variable
int buzzer = 32;
const char turnON = '1';
const char turnOFF = '0';
const char lockON = '3';
const int ignition = 25;
const int lockpin = 4;

void setup()
{
    pinMode(33, OUTPUT);
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH);
    led.initLED();

    Serial.begin(115200);
    SerialBT.begin("Movio_E-Cycle"); // Bluetooth device name
    Serial.println("The device started, now you can pair it with bluetooth!");
    Serial.println("To Unlock send: 1"); // print on serial monitor
    Serial.println("To Lock send: 2");   // print on serial monitor

    pinMode(ignition, OUTPUT);
    pinMode(lockpin, OUTPUT);
    ESP_LOGI("TAG", "Setting up ADC");

    while (!ads.begin())
    {
        ESP_LOGE("ADC setup failed. Retrying in 2 sec");
        delay(2000);
    }

    xTaskCreate(bluetoothHander, "Bluetooth", 2048, NULL, 10, NULL);
    xTaskCreate(adcScanner, "ADC Handler", 2048, NULL, 10, NULL);
}

void loop()
{
    delay(10);
}

void adcScanner(void *parameters)
{
    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3, volts_bkp_batt, volts_ev_batt, degree_celcius, current;

    while (1)
    {
        adc0 = ads.readADC_SingleEnded(0);
        adc1 = ads.readADC_SingleEnded(1);
        adc2 = ads.readADC_SingleEnded(2);
        adc3 = ads.readADC_SingleEnded(3);

        volts0 = ads.computeVolts(adc0);
        volts1 = ads.computeVolts(adc1);
        volts2 = ads.computeVolts(adc2);
        volts3 = ads.computeVolts(adc3) - A3_offset;

        volts_bkp_batt = calc_batt_voltage(volts3);
        volts_ev_batt = calc_ev_voltage(volts1) - 0.80;
        degree_celcius = calc_ntc_temp(adc2);
        current = ev_current(adc0);

        led.batteryBar(volts_ev_batt);

        ESP_LOGI("TAG", "\nCurrent Draw(0) : %0.2f A\nEV Voltage(1): %0.2f V\nTemprature(2) : %0.2f °C\nBackup batt. Voltage(3) : %0.2f V\n %u %u %u %u", current, volts_ev_batt, degree_celcius, volts_bkp_batt, adc0, adc1, adc2, adc3);
        SerialBT.print("-----------------------------------------------\n");
        SerialBT.printf("Backup_battery : %0.2f V\n", volts_bkp_batt);
        SerialBT.printf("Current : %f A\n", current);
        SerialBT.printf("Cycle Battery : %f V\n", volts_ev_batt);
        SerialBT.printf("Ambient Temp : %f °C\n", degree_celcius);
        SerialBT.print("-----------------------------------------------");

        delay(1000);
    }
}

void bluetoothHander(void *parameters)
{
    while (1)
    {
        receivedChar = (char)SerialBT.read();

        if (Serial.available())
        {
            SerialBT.write(Serial.read());
        }
        if (SerialBT.available())
        {

            SerialBT.print("Received:");    // write on BT app
            SerialBT.println(receivedChar); // write on BT app
            Serial.print("Received:");      // print on serial monitor
            Serial.println(receivedChar);   // print on serial monitor
            // SerialBT.println(receivedChar);//print on the app
            // SerialBT.write(receivedChar); //print on serial monitor
            if (receivedChar == turnON)
            {
                led.LEDLock = true;
                SerialBT.println("Cycle Unlocked:"); // write on BT app
                Serial.println("Cycle Unlocked:");   // write on serial monitor
                digitalWrite(ignition, HIGH);        // turn the LED ON
                led.vehicleUnlockAnimation();
            }
            if (receivedChar == turnOFF)
            {
                led.LEDLock = true;
                SerialBT.println("Cycle Locked:"); // write on BT app
                Serial.println("Cycle Locked:");   // write on serial monitor
                digitalWrite(ignition, LOW);       // turn the LED off
                led.vehicleLockAnimation();
            }
            if (receivedChar == lockON)
            {
                led.LEDLock = true;
                SerialBT.println("Battery Unlocked :"); // write on BT app
                Serial.println("Lock OFF:");            // write on serial monitor
                digitalWrite(lockpin, HIGH);            // turn the LED off
                led.batteryUnlockAnimation();
                delay(3000);
                digitalWrite(lockpin, LOW); // turn the LED off
                pixels.clear();
            }
        }
        led.LEDLock = false;
        delay(20);
    }
}

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
void buzz()
{
    digitalWrite(32, LOW);
    delay(1000);
    digitalWrite(32, HIGH);
    delay(1000);
}
