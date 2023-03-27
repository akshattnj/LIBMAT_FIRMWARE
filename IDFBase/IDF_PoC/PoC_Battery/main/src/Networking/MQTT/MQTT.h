#ifndef MQTT_H
#define MQTT_H

#include "../WiFi/WiFi.h"
#include "../../Commons/Commons.h"
#include "../../definations.h"

extern "C"
{
    #include <mqtt_client.h>
}

namespace MQTT
{
    void mqttSetup();
    void connectMQTT();
    void mqttPublishTelemetry(void *args);
}

#endif