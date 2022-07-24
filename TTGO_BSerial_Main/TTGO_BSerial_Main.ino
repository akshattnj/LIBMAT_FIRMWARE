#define LED_PIN 33
#define BUZZER_PIN 32
#define NUMPIXELS 10

#define I2C_SDA 21
#define I2C_SCL 22

#define GPS_RX 34
#define GPS_TX 14

#define BMS_RX 13
#define BMS_TX 15

#define BAT_LOCK 23
#define BAT_CHK 36

#define IGNITION 19

#define SerialBMS Serial1

#define DEVICE_NAME "ESP_32-COM4"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <Adafruit_ADS1X15.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_log.h>

#include "TelemetryScanner.h"
#include "BMSHandler.h"
#include "ErrorCodes.h"

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_ADS1115 ads;
MPU6050 mpu(Wire);
TinyGPSPlus gps;
SoftwareSerial SerialGPS;
LEDHandler led(&pixels);
TelemetryScanner ts(&mpu, &gps, &SerialGPS, &ads, &led, &remainingPower);
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
StaticJsonDocument<600> incoming;
StaticJsonDocument<32> errorDoc;

TaskHandle_t i2cTask;
TaskHandle_t bmsTask;
TaskHandle_t telemetryTask;

float A3_offset = 0.060;

char telemetryJSON[1024];
char BMSTelemetry[512];
char sensorTelemetry[512];
char errorJSON[32];

bool vehicleState = false;
bool sendTelemetry = true;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/**
 * @brief Send data via BLE.
 * Global variables used:
 * - deviceConnected
 * - pCharacteristic
 *
 * @param data Data to be sent (unsigned char*)
 * @param len Length of data (size_t)
 *
 */
void sendData(char *data)
{
    if (deviceConnected)
    {
        pCharacteristic->setValue((uint8_t *)data, strlen(data));
        pCharacteristic->notify();
    }
}

/**
 * @brief Merge JSON documents
 *
 * @param dest The destination JSON object
 * @param src The source JSON object
 */
void mergeJSON(JsonObject dest, JsonObjectConst src)
{
    for (auto kvp : src)
    {
        dest[kvp.key()] = kvp.value();
    }
}

void transmitTelemetry(void *parameters)
{
    while (true)
    {
        ts.getTelemetry();
        getBMSTelemetry();
        if (sendTelemetry)
        {
            memset(BMSTelemetry, 0, 512);
            memset(sensorTelemetry, 0, 512);
            memset(telemetryJSON, 0, 1024);

            serializeJsonPretty(BMSDoc, BMSTelemetry);
            BMSTelemetry[strlen(BMSTelemetry)] = '\n';
            serializeJsonPretty(ts.telemetryDoc, sensorTelemetry);
            sensorTelemetry[strlen(sensorTelemetry)] = '\n';
            sendData(BMSTelemetry);
            sendData(sensorTelemetry);

            mergeJSON(BMSDoc.as<JsonObject>(), ts.telemetryDoc.as<JsonObject>());
            serializeJson(BMSDoc, telemetryJSON);

            ESP_LOGI("TAG", "%s", telemetryJSON);
        }
        delay(1000);
    }
}

class MyServerCallbacks : public BLEServerCallbacks
{
    /**
     * @brief Callback function for connect event
     * Global variables used:
     * - deviceConnected
     *
     * @param pServer
     */
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    /**
     * @brief Callback event for disconnect event
     * Global variables used:
     * - deviceConnected
     * - vehicleState
     * - led
     * - PIN IGNITION
     *
     * @param pServer
     */
    void onDisconnect(BLEServer *pServer)
    {
        vehicleState = false;
        digitalWrite(IGNITION, LOW);
        led.vehicleLockAnimation();
        deviceConnected = false;
    }
};

/**
 * @brief Sends response
 *
 * @param ERROR_CODE
 */
void sendResponse(int ERROR_CODE)
{
    errorDoc.clear();
    errorDoc["error"] = ERROR_CODE;
    serializeJson(errorDoc, errorJSON);
    errorJSON[strlen(errorJSON)] = '\n';
    sendData(errorJSON);
}

/**
 * @brief Extends class BLECharacteristicCallbacks and implements the method onWrite.
 *
 */
class MyCallbacks : public BLECharacteristicCallbacks
{
    /**
     * @brief Callback function for write event on BLE
     *
     * @param pCharacteristic Input used by library.
     */
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            ESP_LOGI("TAG", "%s", rxValue);

            incoming.clear();
            DeserializationError error = deserializeJson(incoming, rxValue);
            if (error)
            {
                ESP_LOGE("TAG", "deserializeJson() failed: %s", error.c_str());
                sendResponse(JSON_ERROR);
                return;
            }
            else
            {
                bool pinStatus = digitalRead(BAT_CHK);
                JsonVariant data = incoming["lock"];
                if (!data.isNull())
                {
                    bool content = data.as<bool>();
                    if (content)
                    {
                        if (ts.volts_ev_batt > 30 && pinStatus && !vehicleState && batteryState != 2)
                        {
                            vehicleState = true;
                            ESP_LOGI("TAG", "Cycle Unlocked:");
                            digitalWrite(IGNITION, HIGH);
                            sendResponse(ALL_OK);
                            led.vehicleUnlockAnimation();
                            return;
                        }
                        else if (vehicleState)
                        {
                            sendResponse(VEHICLE_ON);
                            return;
                        }
                        else if (ts.volts_ev_batt < 30)
                        {
                            sendResponse(NO_BATTERY);
                            return;
                        }
                        else if (!pinStatus)
                        {
                            sendResponse(BAD_BATTERY_SEATING);
                            return;
                        }
                        else if (batteryState == 2)
                        {
                            sendResponse(BATTERY_CHARGING);
                            return;
                        }
                    }
                    else
                    {
                        if (vehicleState)
                        {
                            vehicleState = false;
                            sendData("Cycle Locked:\n");
                            ESP_LOGI("TAG", "Cycle Locked:");
                            digitalWrite(IGNITION, LOW);
                            sendResponse(ALL_OK);
                            led.vehicleLockAnimation();
                            return;
                        }
                        else
                        {
                            sendResponse(VEHICLE_OFF);
                            return;
                        }
                    }
                }
                data = incoming["battery"];
                if (!data.isNull())
                {
                    if (ts.volts_ev_batt < 30 && !vehicleState)
                    {
                        sendResponse(ALL_OK);
                        ESP_LOGI("TAG", "Lock OFF:");
                        digitalWrite(BAT_LOCK, HIGH);
                        led.batteryUnlockAnimation();
                        digitalWrite(BAT_LOCK, LOW);
                        return;
                    }
                    else if (vehicleState)
                    {
                        sendResponse(VEHICLE_ON);
                        return;
                    }
                    else
                    {
                        sendResponse(BATTERY_CABLE_ATTACHED);
                        return;
                    }
                }
                data = incoming["telemetry"];
                if (!data.isNull())
                {
                    sendTelemetry = data.as<bool>();
                    sendResponse(ALL_OK);
                    return;
                }

                data = incoming["resetMPU"];
                if (!data.isNull())
                {
                    ts.resetMPU = true;
                    sendResponse(ALL_OK);
                    return;
                }
                sendResponse(INVALID);
            }
        }
    }
};

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);
    pinMode(BAT_CHK, INPUT);

    led.initLED();
    led.LEDLock = true;
    led.resetLED();

    Wire.begin(I2C_SDA, I2C_SCL);
    ts.initialiseTelemetry();

    Serial.begin(115200);
    SerialGPS.begin(9600, SWSERIAL_8N1, GPS_RX, GPS_TX);

    SerialBMS.begin(19200, SERIAL_8N1, BMS_RX, BMS_TX);
    initBMS();

    ESP_LOGI("TAG", "Serial Setup Complete");

    pinMode(IGNITION, OUTPUT);
    pinMode(BAT_LOCK, OUTPUT);

    BLEDevice::init(DEVICE_NAME);
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
    ESP_LOGI("TAG", "BLE Setup complete");

    xTaskCreate([](void *parameters)
                { ts.handleI2CTelemetry(parameters); },
                "I2C Scanner", 2048, NULL, 10, &i2cTask);

    xTaskCreate(updateBMSTelemetry, "BMS Telemetry", 2048, NULL, 15, &bmsTask);

    xTaskCreatePinnedToCore(transmitTelemetry, "Telemetry transmitter", 4096, NULL, 10, &telemetryTask, 1);

    ESP_LOGI("TAG", "Task Generation Complete");
}

void loop()
{
    if (SerialGPS.available())
        gps.encode(SerialGPS.read());

    if ((batteryState == 2 || ts.volts_ev_batt < 20) && vehicleState)
    {
        digitalWrite(IGNITION, LOW);
        led.vehicleLockAnimation();
        vehicleState = false;
    }
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