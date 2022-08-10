#define BUFFER_SIZE 300

#include "src/Telemetry.pb.h"

#include <pb_common.h>
#include <pb.h>
#include <pb_encode.h>
#include <mbedtls/base64.h>

#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_log.h>

StaticJsonDocument<48> doc;
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool pauseTelemetry = false;
const uint8_t LEDPin = 12;

uint8_t buffer[BUFFER_SIZE];
ESPTelemetry_data message = ESPTelemetry_data_init_zero;
pb_ostream_t stream = pb_ostream_from_buffer(buffer, BUFFER_SIZE);
uint8_t b64Encoded[600];
size_t b64Size;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BLESendData
{
public:
    static void sendData(const char *data)
    {
        if (deviceConnected)
        {
            pCharacteristic->setValue((uint8_t *)data, strlen(data));
            pCharacteristic->notify();
        }
    }

    static void sendData(uint8_t *data, size_t len)
    {
        if (deviceConnected)
        {
            pCharacteristic->setValue(data, len);
            pCharacteristic->notify();
        }
    }
};

class MyServerCallbacks : public BLEServerCallbacks
{
    /**
     * @brief Callback function for when the BLE connects
     *
     * @param pServer
     */
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    /**
     * @brief Callback function for when the BLE disconnects
     *
     * @param pServer
     */
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
     * @brief Callback functoin for BLE Notify
     *
     * @param pCharacteristic
     */
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            ESP_LOGI("TAG", "%s", rxValue);
            doc.clear();
            DeserializationError error = deserializeJson(doc, rxValue);
            if (error)
            {
                ESP_LOGE("TAG", "deserializeJson() failed: %s", error.c_str());
                BLESendData::sendData("{\"error\":1}");
                return;
            }
            else
            {
                JsonVariant data = doc["lock"];
                if (!data.isNull())
                {
                    digitalWrite(LEDPin, !data.as<bool>());
                    BLESendData::sendData("{\"error\":0}\n");
                    return;
                }
                data = doc["battery"];
                if (!data.isNull())
                {
                    BLESendData::sendData("{\"error\":0}\n");
                    for (int i = 0; i < 3; i++)
                    {
                        digitalWrite(LEDPin, LOW);
                        delay(100);
                        digitalWrite(LEDPin, HIGH);
                        delay(100);
                    }
                    return;
                }
                data = doc["telemetry"];
                if (!data.isNull())
                {
                    pauseTelemetry = data.as<bool>();
                    BLESendData::sendData("{\"error\":0}\n");
                    return;
                }
                BLESendData::sendData("{\"error\":99}\n");
            }
        }
    }
};

void telemetryTask(void *parameters)
{
    while (1)
    {
        if (!pauseTelemetry)
        {
          BLESendData::sendData(b64Encoded, b64Size);
        }
        delay(1000);
    }
}

void setup()
{
    Serial.begin(115200);

    memset(buffer, 0, sizeof(buffer));
    memset(b64Encoded, 0, 600);

    message.currentADC = 13.22;
    message.voltsEVADC = 44.8;
    message.ambientTempADC = 24;
    message.backupVoltADC = 12.5;
    message.pitchMPU = 0.22;
    message.rollMPU = 0.22;
    message.capacity = 12.5;
    message.voltage = 44.8;
    message.current = 13.21;
    message.remaining = 98.99;
    message.state = 1;

    message.cell01 = 3.22;
    message.cell02 = 3.22;
    message.cell03 = 3.22;
    message.cell04 = 3.22;
    message.cell05 = 3.22;
    message.cell06 = 3.22;
    message.cell07 = 3.22;
    message.cell08 = 3.22;
    message.cell09 = 3.22;
    message.cell10 = 3.22;
    message.cell11 = 3.22;
    message.cell12 = 3.22;
    message.cell13 = 3.22;

    message.temperature1 = 26;
    message.temperature2 = 26;
    message.temperature3 = 26;
    message.temperature4 = 26;
    message.temperature5 = 26;
    message.temperature6 = 26;

    bool status = pb_encode(&stream, ESPTelemetry_data_fields, &message);
    if (!status)
    {
        Serial.println("Failed to encode");
        return;
    }
    Serial.println(stream.bytes_written);
    for (int i = 0; i < stream.bytes_written; i++)
    {
        Serial.printf("%02X", buffer[i]);
    }
    Serial.print('\n');
    mbedtls_base64_encode(b64Encoded, 600, &b64Size, buffer, stream.bytes_written);
    Serial.println((char *)b64Encoded);

    pinMode(LEDPin, OUTPUT);
    digitalWrite(LEDPin, HIGH);
    // Create the BLE Device
    BLEDevice::init("ESP32-001");
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

    xTaskCreate(telemetryTask, "telemetry", 2048, NULL, 10, NULL);
}

void loop()
{
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