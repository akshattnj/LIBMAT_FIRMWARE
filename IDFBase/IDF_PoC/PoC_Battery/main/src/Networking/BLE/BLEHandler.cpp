#include "BLEHandler.h"

namespace BLE
{
    class ServerCallbacks : public NimBLEServerCallbacks
    {
        void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
        {
            ESP_LOGI(BLE_TAG, "Client address: %s\n", connInfo.getAddress().toString().c_str());
        };

        void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
        {
            ESP_LOGI(BLE_TAG, "Client disconnected with reason: %d\n", reason);
            NimBLEDevice::startAdvertising();
        };
    };

    class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
    {
        void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
        {
            std::string value = pCharacteristic->getValue();
            ESP_LOGI(BLE_TAG, "Received value: %s", value.c_str());
        }
    };

    class DescriptorCallbacks : public NimBLEDescriptorCallbacks
    {
        void onWrite(NimBLEDescriptor *pDescriptor, NimBLEConnInfo &connInfo)
        {
            std::string dscVal = pDescriptor->getValue();
            printf("Descriptor witten value: %s\n", dscVal.c_str());
        };

        void onRead(NimBLEDescriptor *pDescriptor, NimBLEConnInfo &connInfo)
        {
            printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
        };
    };

    static NimBLEServer *pServer;
    static NimBLECharacteristic *pCharacteristic;
    static NimBLEService *pService;

    void initialiseBLE()
    {
        NimBLEDevice::init(DEVICE_NAME);
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
        NimBLEDevice::setSecurityAuth(false, true, true);

        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        pService = pServer->createService(SERVICE_UUID);
        pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, (NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY));
        pCharacteristic->setValue("Hello World");
        pCharacteristic->setCallbacks(new CharacteristicCallbacks());

        NimBLE2904 *char2904 = (NimBLE2904 *)pCharacteristic->createDescriptor("2904");
        char2904->setFormat(NimBLE2904::FORMAT_UTF8);
        char2904->setCallbacks(new DescriptorCallbacks());

        pService->start();

        NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);

        pAdvertising->setScanResponse(false);
        pAdvertising->start();
    }

    void sendData(char *data, size_t len)
    {
        if (pServer->getConnectedCount() == 0)
            return;
            
        pCharacteristic->setValue(data);
        pCharacteristic->notify();
    }

    void telemetryTask(void *params)
    {
        while (true)
        {
            char buffer[50];
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "{\"ba%%\":%d}\n", Commons::batteryPercentage);
            sendData(buffer);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}