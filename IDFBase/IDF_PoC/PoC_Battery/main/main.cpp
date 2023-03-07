#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/CANHandler.h"
#include "src/Networking/MQTT/MQTT.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    CANHandler::startTWAI();
    WiFi::setNetCred();
    MQTT::mqttSetup();
    MQTT::connectMQTT();

    xTaskCreate(WiFi::taskWiFiConnect, "Wifi Connect", 4096, NULL, 10, NULL);
    xTaskCreate(WiFi::taskAutoConnect, "Auto Connect", 4096, NULL, 10, NULL);
    xTaskCreate(WS::wsClientTask, "WS Client", 2048, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskReceiveTWAI, "CAN Receive", 4096, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskSendTWAI, "CAN Send", 4096, NULL, 10, NULL);
    xTaskCreate(MQTT::mqttPublishTelemetry, "MQTT Publish", 4096, NULL, 10, NULL);
}
