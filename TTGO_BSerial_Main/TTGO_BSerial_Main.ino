#define LED_PIN 33
#define BUZZER_PIN 32
#define NUMPIXELS 6
#define I2C_SDA 21
#define I2C_SCL 22
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <Adafruit_ADS1X15.h>
#include <MPU6050_light.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_log.h>

#include "TelemetryScanner.h"

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
BluetoothSerial SerialBT;
Adafruit_ADS1115 ads;
MPU6050 mpu(Wire);
LEDHandler led(&pixels);
TelemetryScanner ts(&mpu, NULL, NULL, &ads, &led);
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

float A3_offset = 0.060;
int received;      // received value will be stored in this variable
char receivedChar; // received value will be stored as CHAR in this variable
const char turnON = '1';
const char turnOFF = '0';
const char lockON = '3';
const int ignition = 25;
const int lockpin = 4;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/**
 * @brief Send data via BLE. Uses global variables deviceConnected and pCharacteristic.
 *
 * @param data Data to be sent (unsigned char*)
 * @param len Length of data (size_t)
 */
void sendData(char *data)
{
    if (deviceConnected)
    {
        pCharacteristic->setValue((uint8_t *)data, strlen(data));
        pCharacteristic->notify();
    }
}

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

/**
 * @brief Extends class BLECharacteristicCallbacks and implements the method onWrite.
 *
 */
class MyCallbacks : public BLECharacteristicCallbacks
{
    /**
     * @brief What to do when data is written to the BLE.
     *
     * @param pCharacteristic Input used by library.
     */
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
                if (rxValue[i] == turnON)
                {
                    sendData("Cycle Unlocked:\n");     // write on BT app
                    Serial.println("Cycle Unlocked:"); // write on serial monitor
                    digitalWrite(ignition, HIGH);      // turn the LED ON
                    led.vehicleUnlockAnimation();
                }
                if (rxValue[i] == turnOFF)
                {
                    sendData("Cycle Locked:\n");     // write on BT app
                    Serial.println("Cycle Locked:"); // write on serial monitor
                    digitalWrite(ignition, LOW);     // turn the LED off
                    led.vehicleLockAnimation();
                }
                if (rxValue[i] == lockON)
                {
                    sendData("Battery Unlocked :\n"); // write on BT app
                    Serial.println("Lock OFF:");      // write on serial monitor
                    digitalWrite(lockpin, HIGH);      // turn the LED off
                    led.batteryUnlockAnimation();
                    digitalWrite(lockpin, LOW); // turn the LED off
                    pixels.clear();
                }
            }
        }
    }
};

void setup()
{
    pinMode(33, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(lockpin, OUTPUT);
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

    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,
                                                     BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
                                                         BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");

    xTaskCreate([](void *parameters)
                { ts.handleI2CTelemetry(parameters); },
                "ADC Scanner", 2048, NULL, 10, NULL);
}

void loop()
{
    ts.getTelemetry();
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}