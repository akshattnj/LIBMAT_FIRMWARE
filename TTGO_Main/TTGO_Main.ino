/**
 * @file TTGO_Main.ino
 * @author Antony Jose Kuruvilla (ajosekuruvilla@gmail.com)
 * @brief Arduino Core script for Movio Mobility vehicle controller
 *
 * @version 0.3
 * @date 2022-04-30
 *
 * @copyright Copyright (c) 2022
 *
 */

/*
I2C Addresses:
    AXP: 0x34
    MPU: 0x68
Connections:

    MPU6050:
        Connected to Wire in same line as AXP
        SCL: GPIO22
        SDA: GPIO21
    GPS:
        RX: 15
        TX: 13
*/

#define MODEM_TX 27
#define MODEM_RX 26
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 25
#define I2C_SDA 21
#define I2C_SCL 22
#define GPS_RX 13
#define GPS_TX 15
#define TINY_GSM_MODEM_SIM800
#define SerialGPS Serial2
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_YIELD_MS 10

#include <axp20x.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <TinyGsmClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <sys/time.h>
#include <ThingsBoard.h>
#include <MPU6050_light.h>
#include <TinyGPSPlus.h>
#include <Adafruit_ADS1X15.h>

#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);

AXP20X_Class axp;
// TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
ThingsBoard tb(client);
StaticJsonDocument<1024> doc;
MPU6050 mpu(Wire);
TinyGPSPlus gps;
Adafruit_ADS1015 ads;
struct tm timeInfo;
struct timeval tv;

// These variables are no longer used once serial is handed over to modem
char *c;
bool messageOK = false;
bool error = false;
bool handOverSerial = false;
int8_t gotCNTP = -1;
uint8_t netStatus = 0;

// Thingsboard and GSM control variables
char charTelemetry[1024];
time_t currentTime;
bool thingsboardConnected = false;

// Constants
const char *TAG = "MAIN";
const char *apn = "airtelgprs.com";
const char *THINGSBOARD_SERVER = "demo.thingsboard.io";
const char *TOKEN = "1234567898765432123456789";
const int THINGSBOARD_PORT = 1883;
const float A3_offset = 0.060;
const float bakBatR1 = 96.31;
const float bakBatR2 = 19.70;

/**
 * @brief Setup for PMU. Meant for the TTGO T-Call V2
 *
 * @return true PMU setup Success
 * @return false PMU setup fail
 */
bool setupPMU()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);

    if (ret == AXP_FAIL)
    {
        ESP_LOGE(TAG, "AXP Power begin failed");
        return false;
    }

    //! Turn off unused power
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    float vbus_v = axp.getVbusVoltage();
    float vbus_c = axp.getVbusCurrent();
    float batt_v = axp.getBattVoltage();
    ESP_LOGI(TAG, "VBUS:%.2f mV %.2f mA ,BATTERY: %.2f\n", vbus_v, vbus_c, batt_v);

    return true;
}

/**
 * @brief Setup process mor modem attached on serial port 1
 *
 */
void setupModem()
{
    if (setupPMU() == false)
    {
        ESP_LOGE(TAG, "Setting power error");
    }

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);
}

/**
 * @brief Wait until the modem provides an OK message
 *
 */
void waitforOK()
{
    while (!messageOK)
    {
        if (error)
        {
            error = false;
            ESP_LOGE(TAG, "Something went wrong");
            return;
        }
        delay(100);
        yield();
    }
    messageOK = false;
}

/**
 * @brief Get the Network Status using AT commands
 *
 */
void getNetworkStatus()
{
    SerialAT.println("AT+CREG?");
    waitforOK();

    switch (netStatus)
    {
    case 0:
        ESP_LOGI(TAG, "Not registered, not currently searching a new operator to register to");
        break;
    case 1:
        ESP_LOGI(TAG, "Registered, home network ");
        break;
    case 2:
        ESP_LOGI(TAG, "Not registered, currently searching a new operator to register to");
        break;
    case 3:
        ESP_LOGI(TAG, "Registration denied");
        break;
    case 4:
        ESP_LOGI(TAG, "Unknown");
        break;
    case 5:
        ESP_LOGI(TAG, "Registered, roaming ");
        break;

    default:
        ESP_LOGI(TAG, "Unknown Status Code");
        break;
    }
}

/**
 * @brief Sync time using AT commands
 *
 */
void syncTime()
{
    SerialAT.println("AT+CNTP");
    waitforOK();
    while (gotCNTP < 0)
    {
        delay(100);
        yield();
    }
    switch (gotCNTP)
    {
    case 1:
        ESP_LOGI("Server sync success");
        SerialAT.println("AT+CCLK?");
        break;
    case 61:
        ESP_LOGE("Network Error");
        break;
    case 62:
        ESP_LOGE("DNS Resolution Error");
        break;
    case 63:
        ESP_LOGE("Connection Error");
        break;
    case 64:
        ESP_LOGE("Service response error");
        break;
    case 65:
        ESP_LOGE("Service Response Timeout");
        break;

    default:
        break;
    }
    gotCNTP = -1;
}

/**
 * @brief Get the Date and Time using internal RTC
 *
 * @return uint64_t the date and time in ms accuracy
 */
uint64_t getDateTime()
{
    getLocalTime(&timeInfo);
    return mktime(&timeInfo) * 1000LL;
}

/**
 * @brief Connect to thingsboard in case of connection failure
 *
 */
void reconnect()
{
    while (!tb.connected())
    {
        ESP_LOGI(TAG, "Connecting to ThingsBoard node ...");
        // Attempt to connect (clientId, username, password)
        modem.gprsConnect(apn, "", "");
        if (tb.connect(THINGSBOARD_SERVER, TOKEN))
        {
            ESP_LOGI(TAG, "[DONE]");
        }
        else
        {
            ESP_LOGE(TAG, "[FAILED] : retrying now");
        }
    }
}

/**
 * @brief Task to handle all control of modem including MQTT
 *
 * @param parameters Any parameters passed to the task
 */
void modemControl(void *parameters)
{
    SerialAT.println("AT");
    waitforOK();

    getNetworkStatus();

    SerialAT.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    waitforOK();

    SerialAT.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
    waitforOK();

    // Wait a bit
    delay(1000);
    yield();
    delay(1000);
    yield();

    SerialAT.println("AT+SAPBR=1,1");
    waitforOK();

    SerialAT.println("AT+CNTPCID=1");
    waitforOK();

    SerialAT.println("AT+CNTP=\"pool.ntp.org\",\"0\"");
    waitforOK();
    syncTime();
    getNetworkStatus();

    SerialAT.println("AT+CIPMUX=1");
    waitforOK();

    handOverSerial = true;

    // modem.gprsConnect(apn, "", "");

    while (1)
    {
        if (!tb.connected())
        {
            thingsboardConnected = false;
            reconnect();
        }
        thingsboardConnected = true;

        ESP_LOGI(TAG, "Sending Data");
        memset(charTelemetry, 0, 1024);
        serializeJson(doc, charTelemetry);
        ESP_LOGD(TAG, "%s", charTelemetry);
        bool s = tb.sendTelemetryJson(charTelemetry);
        Serial.println(s);
        doc.clear();
        delay(3000);
        yield();
    }
}

float getBackupBatteryVoltage()
{
    int16_t bakBatRaw = ads.readADC_SingleEnded(3);
    float bakBatVolt = ads.computeVolts(bakBatRaw) - A3_offset;
    return (bakBatRaw * (bakBatR1 + bakBatR2)) / bakBatR2;
}

/**
 * @brief Task that runs to log telemetry
 * @todo Actually log telemetry here
 *
 * @param parameters Any parameters passed to the task
 */
void logTelemetry(void *parameters)
{
    while (1)
    {

        ESP_LOGI(TAG, "Logging data");
        ESP_LOGI(TAG, "Pitch: %.2f| Roll: %.2f | TimeStamp: %llu | Chars Processed : %lu", mpu.getAngleX(), mpu.getAngleY(), getDateTime(), gps.charsProcessed());
        doc["pitch"] = mpu.getAngleX();
        doc["roll"] = mpu.getAngleY();
        doc["backupVolt"] = getBackupBatteryVoltage();

        if (gps.location.isValid())
        {
            ESP_LOGI(TAG, "Location Data Available");
            doc["latitude"] = gps.location.lat();
            doc["longitude"] = gps.location.lng();
        }
        else
        {
            ESP_LOGI(TAG, "No Location data yet");
        }

        delay(1000);
        yield();
    }
}

void handleGPS(void *parameters)
{
    while (true)
    {
        while (SerialGPS.available())
            gps.encode(SerialGPS.read());
        delay(1);
        yield();
    }
}

/**
 * @brief Setup the MPU6050
 *
 */
void initialiseMPU6050()
{
    uint8_t status = 255;
    while (status != 0)
    {
        status = mpu.begin();
        if (status != 0)
        {
            ESP_LOGE(TAG, "Error in communication with MPU6050. Status code: %d", status);
            ESP_LOGI(TAG, "Retrying in 2 seconds");
        }
        delay(2000);
        yield();
    }
    ESP_LOGI(TAG, "MPU6050 connection success. Status: %d", status);
    ESP_LOGI(TAG, "Calculating offsets for first time setup. Please keep the device horizontal");
    mpu.calcOffsets();
    ESP_LOGI(TAG, "Offsets Calculated");
}

void initialiseADC()
{
    ESP_LOGI("TAG", "Setting up ADC");

    while (!ads.begin())
    {
        ESP_LOGE("ADC setup failed. Retrying in 2 sec");
        delay(2000);
    }
    ESP_LOGI(TAG, "ADC setup complete");
}

/**
 * @brief Setup task
 *
 */
void setup()
{
    Serial.begin(115200);
    setupModem();
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    SerialGPS.begin(9800, SERIAL_8N1, GPS_RX, GPS_TX);
    initialiseMPU6050();
    initialiseADC();
    ESP_LOGI(TAG, "Serial setup complete");

    delay(10000);

    xTaskCreate(modemControl, "modemControl", 4096, NULL, 10, NULL);
    xTaskCreate(logTelemetry, "logTelemetry", 2048, NULL, 10, NULL);
    xTaskCreate(handleGPS, "handleGPS", 2048, NULL, 12, NULL);
}

/**
 * @brief Idle task
 *
 */
void loop()
{
    mpu.update();
    while (SerialAT.available() && !handOverSerial)
    {
        c = (char *)malloc(1024);
        memset(c, 0, 1024);
        int i = 0;
        while (1)
        {
            c[i] = char(SerialAT.read());
            if (c[i] > 127)
            {
                c[i] = 0;
                continue;
            }
            if (c[i] == '\n')
                break;
            i++;
        }
        ESP_LOGI(TAG, "%s", c);
        if (!strncmp(c, "OK", 2))
            messageOK = true;
        else if (!strncmp(c, "ERROR", 5))
            error = true;
        else if (!strncmp(c, "+CREG: 0,", 9))
            netStatus = c[9] - '0';
        else if (!strncmp(c, "+CNTP", 5))
            gotCNTP = c[7] - '0';
        else if (!strncmp("+CCLK", c, 5))
        {
            char temp[3];
            temp[2] = '\0';
            temp[0] = c[8];
            temp[1] = c[9];
            timeInfo.tm_year = atoi(temp) + 100;
            temp[0] = c[11];
            temp[1] = c[12];
            timeInfo.tm_mon = atoi(temp) - 1;
            temp[0] = c[14];
            temp[1] = c[15];
            timeInfo.tm_mday = atoi(temp);
            temp[0] = c[17];
            temp[1] = c[18];
            timeInfo.tm_hour = atoi(temp);
            temp[0] = c[20];
            temp[1] = c[21];
            timeInfo.tm_min = atoi(temp);
            temp[0] = c[23];
            temp[1] = c[24];
            timeInfo.tm_sec = atoi(temp);
            ESP_LOGI(TAG, "Got Date/Time: %d-%d-%d | %d:%d:%d", timeInfo.tm_mday, timeInfo.tm_mon, timeInfo.tm_year, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
            currentTime = mktime(&timeInfo);
            tv.tv_sec = currentTime;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            getDateTime();
        }
        free(c);
        c = NULL;
    }
}