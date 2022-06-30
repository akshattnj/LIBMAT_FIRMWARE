
// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Evandro Copercini - 2018
//
// This example creates a bridge between Serial and Classical Bluetooth (SPP)
// and also demonstrate that SerialBT have the same functionalities of a normal Serial
#include <Adafruit_NeoPixel.h>
#define PIN 33
#define NUMPIXELS 6
#include "BluetoothSerial.h"
#include <Adafruit_ADS1X15.h>
#include <esp_log.h>
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 2000 // Time (in milliseconds) to pause between pixels
#define ani_speed 5
BluetoothSerial SerialBT;
Adafruit_ADS1015 ads;
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
    pixels.begin();
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
}

void loop()
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
            SerialBT.println("Cycle Unlocked:"); // write on BT app
            Serial.println("Cycle Unlocked:");   // write on serial monitor
            digitalWrite(ignition, HIGH);        // turn the LED ON
            unlock_rgb();
        }
        if (receivedChar == turnOFF)
        {
            SerialBT.println("Cycle Locked:"); // write on BT app
            Serial.println("Cycle Locked:");   // write on serial monitor
            digitalWrite(ignition, LOW);       // turn the LED off
            lock_rgb();
        }
        if (receivedChar == lockON)
        {
            SerialBT.println("Battery Unlocked :"); // write on BT app
            Serial.println("Lock OFF:");            // write on serial monitor
            digitalWrite(lockpin, HIGH);            // turn the LED off
            batt_lock_rgb();
            delay(3000);
            digitalWrite(lockpin, LOW); // turn the LED off
            pixels.clear();
        }
    }
    delay(20);
    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3, volts_bkp_batt, volts_ev_batt, degree_celcius, current;

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
    battery_bar(volts_ev_batt);
    degree_celcius = calc_ntc_temp(adc2);
    current = ev_current(adc0);
    ESP_LOGI("TAG", "\nCurrent Draw(0) : %0.2f V\nEV Voltage(1): %0.2f V\nTemprature(2) : %0.2f °C\nBackup batt. Voltage(3) : %0.2f V\n", current, volts_ev_batt, degree_celcius, volts_bkp_batt);
    SerialBT.print("-----------------------------------------------\n");
    SerialBT.print("Backup_battery :");
    SerialBT.print(volts_bkp_batt);
    SerialBT.print(" V \n");

    SerialBT.print("Current :");
    SerialBT.print(volts0);
    SerialBT.print(" A \n");

    SerialBT.print("Cycle Battery  :");
    SerialBT.print(volts_ev_batt);
    SerialBT.print(" V \n");

    SerialBT.print("Ambient Temp :");
    SerialBT.print(degree_celcius);
    SerialBT.print(" °C \n");

    SerialBT.print("-----------------------------------------------");
    delay(1000);
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
void battery_bar(float volts)
{
    if (volts >= 51 && volts < 53.2)
    {
        int i = map(volts, 51, 53.2, 10, 255);
        pixels.fill(pixels.Color(0, i, 0), 0, 6);
        pixels.show();
        delay(ani_speed);
    }

    if (volts >= 48.8 && volts < 51)
    {
        int i = map(volts, 48.8, 51, 153, 204);
        pixels.fill(pixels.Color(0, 0, 0), 0, 1);
        pixels.fill(pixels.Color(10, i, 0), 1, 5);
        pixels.show();
    }

    if (volts >= 46.6 && volts < 48.8)
    {
        int i = map(volts, 46.6, 48.8, 102, 153);
        pixels.fill(pixels.Color(0, 0, 0), 0, 2);
        pixels.fill(pixels.Color(51, i, 0), 2, 4);
        pixels.show();
    }
    if (volts >= 44.4 && volts < 46.6)
    {
        int i = map(volts, 44.4, 46.6, 51, 102);
        pixels.fill(pixels.Color(0, 0, 0), 0, 3);
        pixels.fill(pixels.Color(102, i, 0), 3, 3);
        pixels.show();
    }
    if (volts >= 42.2 && volts < 44.4)
    {
        int i = map(volts, 46.6, 48.8, 0, 51);
        pixels.fill(pixels.Color(0, 0, 0), 0, 4);
        pixels.fill(pixels.Color(160, i, 0), 4, 2);
        pixels.show();
    }
    if (volts >= 40 && volts < 42.2)
    {
        // int i= map(volts,46.6,48.8,0,255);
        pixels.fill(pixels.Color(0, 0, 0), 0, 5);
        pixels.fill(pixels.Color(254, 0, 0), 5, 1);
        pixels.show();
    }
    if (volts >= 5 && volts < 40)
    {
        pixels.fill(pixels.Color(0, 0, 0), 0, 6);
        pixels.show();
    }
}
void unlock_rgb()
{
    digitalWrite(32, LOW);
    pixels.fill(pixels.Color(0, 255, 0), 0, 6);
    pixels.show();
    delay(250);

    digitalWrite(32, HIGH);
    pixels.fill(pixels.Color(0, 0, 0), 0, 6);
    pixels.show();
    delay(250);

    digitalWrite(32, LOW);
    pixels.fill(pixels.Color(0, 255, 0), 0, 6);
    pixels.show();
    delay(250);

    digitalWrite(32, HIGH);
    pixels.fill(pixels.Color(0, 0, 0), 0, 6);
    pixels.show();
    delay(250);
}
void lock_rgb()
{
    pixels.fill(pixels.Color(255, 0, 0), 0, 6);
    pixels.show();
    digitalWrite(32, LOW);
    delay(250);

    pixels.fill(pixels.Color(0, 0, 0), 0, 6);
    pixels.show();
    digitalWrite(32, HIGH);
    delay(250);

    pixels.fill(pixels.Color(255, 0, 0), 0, 6);
    pixels.show();
    digitalWrite(32, LOW);
    delay(250);

    pixels.fill(pixels.Color(0, 0, 0), 0, 6);
    pixels.show();
    digitalWrite(32, HIGH);
    delay(250);
}
void batt_lock_rgb()
{
    pixels.fill(pixels.Color(0, 0, 255), 0, 6);
    pixels.show();
}
float ev_current(float adc)
{
    return ((adc - 543) * 4.88 / 40.00);
}
void buzz()
{
    digitalWrite(32, LOW);
    delay(1000);
    digitalWrite(32, HIGH);
    delay(1000);
}
