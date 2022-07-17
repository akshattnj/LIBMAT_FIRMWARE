#include <ModbusClientRTU.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <Arduino.h>

uint8_t batteryState;

float cellVolts[16];
float current;
uint8_t temperature[6];
float capah;
uint8_t BMSState;

float chargingVoltage;
float chargingCurrent;
float dischargingVoltage;
float dischargingCurrent;
float totalCellVoltage;
float remainingPower;

StaticJsonDocument<1024> BMSDoc;
ModbusClientRTU RS485(Serial1);
Error err;
uint8_t BMSSlaveID = 2;

void decodeBatGeneralInfo(ModbusMessage msg)
{
    chargingVoltage = ((msg[3] * 100.00) + msg[4]) / 10.0;
    chargingCurrent = ((msg[5] * 100.00) + msg[6]) / 10.0;
    dischargingVoltage = ((msg[7] * 100.00) + msg[8]) / 10.0;
    dischargingCurrent = ((msg[9] * 100.00) + msg[10]) / 10.0;
    totalCellVoltage = ((msg[11] * 100.00) + msg[12]) / 10.0;
    remainingPower = ((msg[13] * 100.00) + msg[14]) / 100.0;
}

void decodeBatDetail(ModbusMessage msg)
{
    for (int i = 2; i < 17; i++)
    {
        cellVolts[i - 2] = ((msg[2 * i - 1] * 100.00) + msg[(2 * i)]) / 1000.0;
    }
    current = ((msg[35] * 100.00) + msg[36]) / 10.0;
    for (int i = 38; i < 49; i = i + 2)
    {
        temperature[(i - 38) / 2] = msg[i];
    }
    capah = ((msg[49] * 100.00) + msg[50]) / 100.0;
    BMSState = msg[52];
}

void handleData(ModbusMessage msg, uint32_t token)
{
    ESP_LOGI("BMS Handler", "Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", msg.getServerID(), msg.getFunctionCode(), token, msg.size());
    for (int i = 0; i < msg.size(); i++)
    {
        Serial.printf("%02X ", msg[i]);
    }
    Serial.println("");
    if (token == 0x01)
    {
        decodeBatGeneralInfo(msg);
    }
    else if (token == 0x02)
    {
        decodeBatDetail(msg);
    }
}

void handleError(Error error, uint32_t token)
{
    // ModbusError wraps the error code and provides a readable error message for it
    ModbusError me(error);
    ESP_LOGE("BMS Handler", "Error response: %02X - %s\n", error, (const char *)me);
    if (token == 0x01)
    {
        chargingVoltage = 0;
        chargingCurrent = 0;
        dischargingVoltage = 0;
        dischargingCurrent = 0;
        totalCellVoltage = 0;
        remainingPower = 0;
    }
    else if (token == 0x02)
    {
        memset(cellVolts, 0, 16);
        current = 0;
        memset(temperature, 0, 6);
        capah = 0;
        BMSState = 0;
    }
}

void initBMS()
{
    RS485.onDataHandler(&handleData);
    RS485.onErrorHandler(&handleError);
    RS485.begin();
}

void updateBMSTelemetry(void *parameters)
{
    while (1)
    {
        err = RS485.addRequest(0x01, BMSSlaveID, READ_HOLD_REGISTER, 0x9C42, 0x0007);
        if (err != SUCCESS)
        {
            ModbusError e(err);
            ESP_LOGE("BMS Handler", "Error creating request: %02X - %s\n", err, (const char *)e);
            chargingVoltage = 0;
            chargingCurrent = 0;
            dischargingVoltage = 0;
            dischargingCurrent = 0;
            totalCellVoltage = 0;
            remainingPower = 0;
        }

        delay(500);

        if (chargingVoltage > 0 && dischargingVoltage > 0)
        {
            ESP_LOGE("BMS Hadler", "Something is very wrong");
        }
        else if (dischargingVoltage > 0)
        {
            batteryState = 1;
            ESP_LOGI("BMS Handler", "Battery discharging");
        }
        else if (chargingVoltage > 0)
        {
            batteryState = 2;
            ESP_LOGI("BMS Handler", "Battery charging");
        }
        else
        {
            batteryState = 0;
            ESP_LOGI("BMS Handler", "Battery disconnected");
        }

        err = RS485.addRequest(0x02, BMSSlaveID, READ_HOLD_REGISTER, 0x9C41, 0x001A);
        if (err != SUCCESS)
        {
            ModbusError e(err);
            ESP_LOGE("BMS Handler", "Error creating request: %02X - %s\n", err, (const char *)e);
            memset(cellVolts, 0, 16);
            current = 0;
            memset(temperature, 0, 6);
            capah = 0;
            BMSState = 0;
        }
        delay(500);
    }
}

void getBMSTelemetry()
{

    ESP_LOGI("TAG", "Cells\n%0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f",
             cellVolts[0], cellVolts[1], cellVolts[2], cellVolts[3], cellVolts[4], cellVolts[5], cellVolts[6], cellVolts[7],
             cellVolts[8], cellVolts[9], cellVolts[10], cellVolts[11], cellVolts[12], cellVolts[13], cellVolts[14]);
    ESP_LOGI("TAG", "Temperature\n%d %d %d %d %d %d", temperature[0], temperature[1], temperature[2],
             temperature[3], temperature[4], temperature[5]);
    ESP_LOGI("TAG", "Current : %0.2f\nCapah : %0.2f\nState : %d", current, capah, BMSState);
    ESP_LOGI("TAG", "Charging V : %0.2fV\nCharging A : %0.2fA\nDischarging V : %0.2fV\nDischarging A : %0.2fA",
             chargingVoltage, chargingCurrent, dischargingVoltage, dischargingCurrent);
    ESP_LOGI("TAG", "Total Cell Voltage: %0.2f\nRemaining: %0.2f", totalCellVoltage, remainingPower);

    BMSDoc.clear();
    JsonArray cellVoltages = BMSDoc.createNestedArray("Cell_Voltages");
    cellVoltages.add(cellVolts[0]);
    cellVoltages.add(cellVolts[1]);
    cellVoltages.add(cellVolts[2]);
    cellVoltages.add(cellVolts[3]);
    cellVoltages.add(cellVolts[4]);
    cellVoltages.add(cellVolts[5]);
    cellVoltages.add(cellVolts[6]);
    cellVoltages.add(cellVolts[7]);
    cellVoltages.add(cellVolts[8]);
    cellVoltages.add(cellVolts[9]);
    cellVoltages.add(cellVolts[10]);
    cellVoltages.add(cellVolts[11]);
    cellVoltages.add(cellVolts[12]);
    cellVoltages.add(cellVolts[13]);
    cellVoltages.add(cellVolts[14]);
    JsonArray temperatures = BMSDoc.createNestedArray("Temperature");
    temperatures.add(temperature[0]);
    temperatures.add(temperature[1]);
    temperatures.add(temperature[2]);
    temperatures.add(temperature[3]);
    temperatures.add(temperature[4]);
    temperatures.add(temperature[5]);
    BMSDoc["Current"] = current;
    BMSDoc["Capacity"] = capah;
    BMSDoc["BMS_State"] = BMSState;
    BMSDoc["Charging_V"] = chargingVoltage;
    BMSDoc["Charging_I"] = chargingCurrent;
    BMSDoc["Discharging_V"] = dischargingVoltage;
    BMSDoc["Discharging_C"] = dischargingCurrent;
    BMSDoc["Voltage"] = totalCellVoltage;
    BMSDoc["Battery_Percent"] = remainingPower;
}
