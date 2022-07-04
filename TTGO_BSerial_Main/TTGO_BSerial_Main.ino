#define LED_PIN 33
#define BUZZER_PIN 32
#define NUMPIXELS 6
#define I2C_SDA 21
#define I2C_SCL 22

#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <Adafruit_ADS1X15.h>
#include <MPU6050_light.h>
#include <esp_log.h>

#include "TelemetryScanner.h"

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
BluetoothSerial SerialBT;
Adafruit_ADS1115 ads;
MPU6050 mpu(Wire);
LEDHandler led(&pixels);
TelemetryScanner ts(&mpu, NULL, NULL, &ads, &led);

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

    Wire.begin(I2C_SDA, I2C_SCL);
    ts.initialiseTelemetry();

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
    xTaskCreate([](void *parameters)
                { ts.handleI2CTelemetry(parameters); },
                "ADC Scanner", 2048, NULL, 10, NULL);
}

void loop()
{
    ts.getTelemetry();
    delay(1000);
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
                SerialBT.println("Cycle Unlocked:"); // write on BT app
                Serial.println("Cycle Unlocked:");   // write on serial monitor
                digitalWrite(ignition, HIGH);        // turn the LED ON
                led.vehicleUnlockAnimation();
            }
            if (receivedChar == turnOFF)
            {
                SerialBT.println("Cycle Locked:"); // write on BT app
                Serial.println("Cycle Locked:");   // write on serial monitor
                digitalWrite(ignition, LOW);       // turn the LED off
                led.vehicleLockAnimation();
            }
            if (receivedChar == lockON)
            {
                SerialBT.println("Battery Unlocked :"); // write on BT app
                Serial.println("Lock OFF:");            // write on serial monitor
                digitalWrite(lockpin, HIGH);            // turn the LED off
                led.batteryUnlockAnimation();
                delay(3000);
                digitalWrite(lockpin, LOW); // turn the LED off
                pixels.clear();
            }
        }
        delay(20);
    }
}
