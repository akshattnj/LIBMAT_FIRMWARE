#include <ModbusClientRTU.h>
#include <esp_log.h>

class BMSHandler
{
public:
    BMSHandler(ModbusClientRTU *RS485Client)
    {
        RS485 = RS485Client;
    }

    BMSHandler(ModbusClientRTU *RS485Client, uint8_t slaveID) : BMSSlaveID(slaveID)
    {
        RS485 = RS485Client;
    }

    void initBMS()
    {
        RS485->onDataHandler(&handleData);
        RS485->onErrorHandler(&handleError);
        RS485->begin();
    }

    void updateBMSTelemetry()
    {
        err = RS485->addRequest(0x12345678, BMSSlaveID, READ_HOLD_REGISTER, 0x9C42, 0x0007);
        if (err != SUCCESS)
        {
            ModbusError e(err);
            ESP_LOGE("BMS Handler", "Error creating request: %02X - %s\n", err, (const char *)e);
        }
        else
        {
        }
        delay(1000);
        err = RS485->addRequest(0x12345678, BMSSlaveID, READ_HOLD_REGISTER, 0x9C41, 0x001A);
        if (err != SUCCESS)
        {
            ModbusError e(err);
            ESP_LOGE("BMS Handler", "Error creating request: %02X - %s\n", err, (const char *)e);
        }
        else
        {
        }
    }

private:
    ModbusClientRTU *RS485;
    Error err;
    const uint8_t BMSSlaveID = 2;

    float cellVolts[16];
    float current;
    float temperature[6];
    float capah;
    uint8_t BMSState;

    float chargingVoltage;
    float chargingCurrent;
    float dischargingVoltage;
    float dischargingCurrent;
    float totalCellVoltage;
    float remainingPower;

    static void handleData(ModbusMessage msg, uint32_t token)
    {
        ESP_LOGI("BMS Handler", "Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", msg.getServerID(), msg.getFunctionCode(), token, msg.size());
        for (auto &byte : msg)
        {
            Serial.printf("%02X ", byte);
        }
        Serial.println("");
    }

    static void handleError(Error error, uint32_t token)
    {
        // ModbusError wraps the error code and provides a readable error message for it
        ModbusError me(error);
        ESP_LOGE("BMS Handler", "Error response: %02X - %s\n", error, (const char *)me);
    }
};