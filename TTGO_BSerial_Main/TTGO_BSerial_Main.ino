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

#define DEVICE_NAME "ESP_32-Prototype"
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

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
BluetoothSerial SerialBT;
Adafruit_ADS1115 ads;
MPU6050 mpu(Wire);
TinyGPSPlus gps;
SoftwareSerial SerialGPS;
LEDHandler led(&pixels);
TelemetryScanner ts(&mpu, &gps, &SerialGPS, &ads, &led, &remainingPower);
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

float A3_offset = 0.060;
int received;      // received value will be stored in this variable
char receivedChar; // received value will be stored as CHAR in this variable
char telemetryJSON[1024];
char BMSTelemetry[512];
char sensorTelemetry[512];
const char turnON = '1';
const char turnOFF = '0';
const char lockON = '3';
bool vehicleState = false;
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
        vehicleState = false;
        digitalWrite(IGNITION, LOW);
        led.vehicleLockAnimation();
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
            ESP_LOGI("TAG", "%s", rxValue);
            for (int i = 0; i < rxValue.length(); i++)
            {
                bool pinStatus = digitalRead(BAT_CHK);
                ESP_LOGI("TAG", "%d", rxValue[i]);
                if (rxValue[i] == turnON && ts.volts_ev_batt > 30 && pinStatus)
                {
                    vehicleState = true;
                    sendData("Cycle Unlocked:\n");
                    ESP_LOGI("TAG", "Cycle Unlocked:");
                    digitalWrite(IGNITION, HIGH);
                    led.vehicleUnlockAnimation();
                }
                else if (rxValue[i] == turnON && ts.volts_ev_batt < 30)
                {
                    sendData("{\"err\":\"Please plug in battery before attempting to start the vehicle\"}");
                }
                else if (rxValue[i] == turnON && !pinStatus)
                {
                    sendData("{\"err\":\"Error seating battery. Please make sure the battrey is properly inserted into the holder\"}");
                }
                else if (rxValue[i] == turnON && batteryState == 2)
                {
                    sendData("{\"err\":\"Please do not attempt to start the vehicle while the batery is charging\"}");
                }

                if (rxValue[i] == turnOFF)
                {
                    vehicleState = false;
                    sendData("Cycle Locked:\n");
                    ESP_LOGI("TAG", "Cycle Locked:");
                    digitalWrite(IGNITION, LOW);
                    led.vehicleLockAnimation();
                }

                if (rxValue[i] == lockON && ts.volts_ev_batt < 30 && !vehicleState)
                {
                    sendData("Battery Unlocked :\n");
                    ESP_LOGI("TAG", "Lock OFF:");
                    digitalWrite(BAT_LOCK, HIGH);
                    led.batteryUnlockAnimation();
                    digitalWrite(BAT_LOCK, LOW);
                    pixels.clear();
                }
                else if (rxValue[i] == lockON && vehicleState)
                {
                    sendData("{\"err\":\"Please turn off vehicle and remove battery cable before attempting to unlock battery\"}");
                }
                else if (rxValue[i] == lockON)
                {
                    sendData("{\"err\":\"Please remove battery cable before attempting to unlock battery\"}");
                }
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
    SerialGPS.begin(9800, SWSERIAL_8N1, GPS_RX, GPS_TX);

    SerialBMS.begin(19200, SERIAL_8N1, BMS_RX, BMS_TX);
    initBMS();

    ESP_LOGI("TAG", "Serial Setup Complete");

    SerialBT.begin("Movio_E-Cycle"); // Bluetooth device name
    Serial.println("The device started, now you can pair it with bluetooth!");
    Serial.println("To Unlock send: 1"); // print on serial monitor
    Serial.println("To Lock send: 2");   // print on serial monitor

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
                "I2C Scanner", 2048, NULL, 10, NULL);

    xTaskCreate([](void *parameters)
                { ts.handleGPS(parameters); },
                "GPS Handler", 2048, NULL, 12, NULL);

    xTaskCreate(updateBMSTelemetry, "BMS Telemetry", 2048, NULL, 15, NULL);

    ESP_LOGI("TAG", "Task Generation Complete");
}

void mergeJSON(JsonObject dest, JsonObjectConst src)
{
    for (auto kvp : src)
    {
        dest[kvp.key()] = kvp.value();
    }
}

void loop()
{
    ts.getTelemetry();
    getBMSTelemetry();

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
    delay(1000);
}