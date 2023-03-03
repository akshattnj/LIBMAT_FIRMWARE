extern "C"
{
#include <stdio.h>
}

#include "src/WiFiHandler.h"
#include "src/MQTT.h"


extern "C" void app_main(void)
{
    WiFi::startNVS();
    WiFi::initialiseWiFiSTA();
    WiFi::connectWiFi();
    MQTT::mqttSetup();
    MQTT::connectMQTT();
}
