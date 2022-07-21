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
#include <esp_log.h>

StaticJsonDocument<48> doc;
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool pauseTelemetry = false;
const uint8_t LEDPin = 12;

const char *BMSDummy = "{\
  \"type\": 1,\
  \"Cell_Voltages\": [\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0,\
    0\
  ],\
  \"Temperature\": [\
    0,\
    0,\
    0,\
    0,\
    0,\
    0\
  ],\
  \"Current\": 0,\
  \"Capacity\": 0,\
  \"BMS_State\": 0,\
  \"Charging_V\": 0,\
  \"Charging_I\": 0,\
  \"Discharging_V\": 0,\
  \"Discharging_C\": 0,\
  \"Voltage\": 0,\
  \"Battery_Percent\": 0\
}\n";

const char *sensorDummy = "{\
  \"type\": 1,\
  \"CurrentDraw(ADC)\": -40.4084816,\
  \"EV Voltage(ADC)\": 13.15782642,\
  \"Temprature(ADC)\": null,\
  \"BackupVoltage(ADC)\": 3.250635147,\
  \"Latitude\": 0,\
  \"Longitude\": 0,\
  \"pitch\": -15.73036289,\
  \"roll\": 53.15324402,\
  \"yaw\": -11.05370426\
}\n";

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/**
 * @brief Send data via BLE. Uses global variables deviceConnected and pCharacteristic.
 *
 * @param data Data to be sent (unsigned char*)
 * @param len Length of data (size_t)
 */
void sendData(const char *data)
{
    if (deviceConnected)
    {
        pCharacteristic->setValue((uint8_t *)data, strlen(data));
        pCharacteristic->notify();
    }
}

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
                sendData("{\"error\":1}");
                return;
            }
            else
            {
                JsonVariant data = doc["lock"];
                if (!data.isNull())
                {
                    digitalWrite(LEDPin, !data.as<bool>());
                    sendData("{\"error\":0}\n");
                    return;
                }
                data = doc["battery"];
                if (!data.isNull())
                {
                    sendData("Battery Unlocked\n{\"error\":0}\n");
                    return;
                }
                data = doc["telemetry"];
                if (!data.isNull())
                {
                    pauseTelemetry = data.as<bool>();
                    sendData("{\"error\":0}\n");
                    return;
                }
                sendData("{\"error\":99}\n");
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
            sendData(BMSDummy);
            sendData(sensorDummy);
        }
        delay(1000);
    }
}

void setup()
{
    Serial.begin(115200);

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
