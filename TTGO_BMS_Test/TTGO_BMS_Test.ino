#include "ModbusClientRTU.h"

ModbusClientRTU RS485(Serial1);

void handleData(ModbusMessage msg, uint32_t token)
{
    Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", msg.getServerID(), msg.getFunctionCode(), token, msg.size());
    for (auto &byte : msg)
    {
        Serial.printf("%02X ", byte);
    }
    Serial.println("");
}

void handleError(Error error, uint32_t token)
{
    // ModbusError wraps the error code and provides a readable error message for it
    ModbusError me(error);
    Serial.printf("Error response: %02X - %s\n", error, (const char *)me);
}

void setup()
{
    Serial.begin(115200);
    Serial1.begin(19200, SERIAL_8N1, 13, 15);

    // Set up ModbusClientRTU client.
    // - provide onData and onError handler functions
    RS485.onDataHandler(&handleData);
    RS485.onErrorHandler(&handleError);

    // Start ModbusClientRTU background task
    RS485.begin();

    Error err = RS485.addRequest(0x12345678, 2, READ_HOLD_REGISTER, 0x9C42, 0x0007);
    if (err != SUCCESS)
    {
        ModbusError e(err);
        Serial.printf("Error creating request: %02X - %s\n", err, (const char *)e);
    }
}

void loop()
{
}