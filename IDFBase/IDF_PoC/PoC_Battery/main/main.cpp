#include "src/definations.h"
#include "src/Commons/Commons.h"
#include "src/CANHandler.h"
#include "src/LEDHandler.h"
#include "src/Networking/WiFi/WiFi.h"
#include "src/Networking/WS/WS.h"
#include "src/Networking/MQTT/MQTT.h"
#include "src/Networking/BLE/BLEHandler.h"


extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSTA();
    BLE::initialiseBLE();
    CANHandler::startTWAI();
    MQTT::mqttSetup();
    MQTT::connectMQTT();
    LED::init();

    xTaskCreate(WiFi::taskWiFiConnect, "Wifi Connect", 4096, NULL, 10, NULL);
    xTaskCreate(WiFi::taskAutoConnect, "Auto Connect", 4096, NULL, 10, NULL);
    xTaskCreate(WS::wsClientTask, "WS Client", 2048, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskReceiveTWAI, "CAN Receive", 4096, NULL, 10, NULL);
    xTaskCreate(CANHandler::taskSendTWAI, "CAN Send", 4096, NULL, 10, NULL);
    xTaskCreate(MQTT::mqttPublishTelemetry, "MQTT Publish", 4096, NULL, 10, NULL);
    xTaskCreate(LED::ledAnimationTask, "LED Animation", 4096, NULL, 10, NULL);
    xTaskCreate(BLE::telemetryTask, "BLE Telemetry", 4096, NULL, 10, NULL);
}
