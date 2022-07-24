#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_YIELD_MS 10

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <time.h>
#include <sys/time.h>
#include <StreamDebugger.h>
#include <ThingsBoard.h>

#include "Secrets.h"
#include "Definations.h"

#include <esp_log.h>

class GSMHandler
{
public:
    time_t currentTime;
    GSMHandler(HardwareSerial *debugPort, HardwareSerial *GSM, char *package1Pointer, char *package2Pointer, char *package3Pointer, bool debugMode = false)
    {
        gsmPort = GSM;
        debug = debugPort;
        package1 = package1Pointer;
        package2 = package2Pointer;
        package3 = package3Pointer;
        if (debugMode)
        {
            debugger = new StreamDebugger(*gsmPort, *debug);
            modem = new TinyGsm(*debugger);
        }
        else
        {
            modem = new TinyGsm(*GSM);
        }
        client = new TinyGsmClient(*modem);
        tb = new ThingsBoard(*client);
    }

    void handleGSM(void *parameters)
    {
        delay(10000);
        ESP_LOGI("TAG", "AT");
        gsmPort->println("AT");
        waitforOK();

        getNetworkStatus();

        ESP_LOGI("TAG", "AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
        gsmPort->println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
        waitforOK();

        ESP_LOGI("TAG", "AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
        gsmPort->println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
        waitforOK();

        // Wait a bit
        delay(1000);
        yield();
        delay(1000);
        yield();

        ESP_LOGI("TAG", "AT+SAPBR=1,1");
        gsmPort->println("AT+SAPBR=1,1");
        waitforOK();

        ESP_LOGI("TAG", "AT+CNTPCID=1");
        gsmPort->println("AT+CNTPCID=1");
        waitforOK();

        ESP_LOGI("TAG", "AT+CNTP=\"pool.ntp.org\",\"0\"");
        gsmPort->println("AT+CNTP=\"pool.ntp.org\",\"0\"");
        waitforOK();
        syncTime();
        getNetworkStatus();

        ESP_LOGI("TAG", "AT+CIPMUX=1");
        gsmPort->println("AT+CIPMUX=1");
        waitforOK();

        handOverSerial = true;
        while (1)
        {
            if (!tb->connected())
            {
                thingsboardConnected = false;
                reconnect();
            }
            thingsboardConnected = true;

            ESP_LOGI(TAG, "Sending Data");
            bool s = tb->sendTelemetryJson(package1);
            Serial.println(s);
            s = tb->sendTelemetryJson(package2);
            Serial.println(s);
            s = tb->sendTelemetryJson(package3);
            Serial.println(s);
            delay(3000);
            yield();
        }
    }

    void GSMResponseScan(void *parameters)
    {
        while (1)
        {
            delay(1);
            if (handOverSerial)
            {
                ESP_LOGD("TAG", "Ending response scanner task");
                vTaskDelete(NULL);
            }

            while (gsmPort->available())
            {
                delay(10);
                c = (char *)malloc(1024);
                memset(c, 0, 1024);
                int i = 0;
                while (1)
                {
                    delay(10);
                    c[i] = char(gsmPort->read());
                    ESP_LOGV("TAG", "%s", c);
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
    }

private:
    HardwareSerial *gsmPort;
    HardwareSerial *debug;
    StreamDebugger *debugger;
    TinyGsm *modem;
    TinyGsmClient *client;
    ThingsBoard *tb;
    bool thingsboardConnected = false;

    char *c;
    char *package1;
    char *package2;
    char *package3;
    bool messageOK = false;
    bool error = false;
    bool handOverSerial = false;
    int8_t gotCNTP = -1;
    uint8_t netStatus = 0;

    struct tm timeInfo;
    struct timeval tv;

    void syncTime()
    {
        ESP_LOGI("TAG", "AT+CNTP");
        gsmPort->println("AT+CNTP");
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
            gsmPort->println("AT+CCLK?");
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

    void getNetworkStatus()
    {
        ESP_LOGI("TAG", "AT+CREG?");
        gsmPort->println("AT+CREG?");
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
        while (!tb->connected())
        {
            ESP_LOGI(TAG, "Connecting to ThingsBoard node ...");
            // Attempt to connect (clientId, username, password)
            modem->gprsConnect(APN, "", "");
            if (tb->connect(SECRET_SERVER, SECRET_TOKEN))
            {
                ESP_LOGI(TAG, "[DONE]");
            }
            else
            {
                ESP_LOGE(TAG, "[FAILED] : retrying now");
            }
        }
    }
};