#define MODEM_TX 27
#define MODEM_RX 26
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

class GSMHandler
{
public:
    GSMHandler(HardwareSerial *debugPort, HardwareSerial *GSM, bool debugMode = false)
    {
        gsmPort = GSM;
        debug = debugPort;
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
        ESP_LOGD("TAG", "AT");
        gsmPort->println("AT");
        waitforOK();

        getNetworkStatus();

        ESP_LOGD("TAG", "AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
        gsmPort->println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
        waitforOK();

        ESP_LOGD("TAG", "AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
        gsmPort->println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
        waitforOK();

        // Wait a bit
        delay(1000);
        yield();
        delay(1000);
        yield();

        ESP_LOGD("TAG", "AT+SAPBR=1,1");
        gsmPort->println("AT+SAPBR=1,1");
        waitforOK();

        ESP_LOGD("TAG", "AT+CNTPCID=1");
        gsmPort->println("AT+CNTPCID=1");
        waitforOK();

        ESP_LOGD("TAG", "AT+CNTP=\"pool.ntp.org\",\"0\"");
        gsmPort->println("AT+CNTP=\"pool.ntp.org\",\"0\"");
        waitforOK();
        syncTime();
        getNetworkStatus();

        ESP_LOGD("TAG", "AT+CIPMUX=1");
        gsmPort->println("AT+CIPMUX=1");
        waitforOK();

        handOverSerial = true;
        while (1)
        {
            delay(10);
        }
    }

    void GSMResponseScan(void *parameters)
    {
        while (1)
        {
            delay(10);
            if (handOverSerial)
                vTaskDelete(NULL);

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
    bool thingsBoardConnected = false;

    char *c;
    bool messageOK = false;
    bool error = false;
    bool handOverSerial = false;
    int8_t gotCNTP = -1;
    uint8_t netStatus = 0;

    void syncTime()
    {
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
};