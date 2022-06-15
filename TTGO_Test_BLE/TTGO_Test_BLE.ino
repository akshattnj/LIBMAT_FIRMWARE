/**
 * @file TTGO_Test_BLE.ino
 * @author Antony Kuruvilla (ajosekuruvilla@gmail.com)
 * @brief Test script to check BLE functionality of TTGO V2. Notifies the value "hello every 1 second".
 * Default state of onboard LED is off. Responds to JSON key "LEDStatus" with a binary value.
 * @version 1
 * @date 2022-06-15
 *
 * @copyright None
 *
 */
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define MBEDTLS_CIPHER_MODE_WITH_PADDING
#define ARRAY_WIDTH 1024

static const uint8_t key_128[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const uint8_t IV[] = {
    0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09,
    0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};

StaticJsonDocument<48> doc;
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
const uint8_t LEDPin = 12;
uint8_t temp[] = {'h', 'e', 'l', 'l', 'o', '\n'};

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

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
                Serial.print(rxValue[i]);
            Serial.println();
            DeserializationError error = deserializeJson(doc, rxValue);
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }
            digitalWrite(LEDPin, doc["LEDStatus"]);
        }
    }
};

/**
 * @brief Send data via BLE. Uses global variables deviceConnected and pCharacteristic.
 *
 * @param data Data to be sent (unsigned char*)
 * @param len Length of data (size_t)
 */
void sendData(uint8_t *data, size_t len)
{
    if (deviceConnected)
    {
        pCharacteristic->setValue(data, len);
        pCharacteristic->notify();
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(LEDPin, OUTPUT);
    digitalWrite(LEDPin, HIGH);
    // Create the BLE Device
    BLEDevice::init("ESP32");
    Serial.println("ESP Setup Complete");
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,
                                                     BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
                                                         BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void loop()
{

    sendData(temp, temp - *(&temp - 1));
    delay(1000);
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
}
