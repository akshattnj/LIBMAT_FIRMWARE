#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* WIFI_SSID = "Movio_Mobility";
const char* WIFI_PASSWORD = "movio@12334";

// ThingsBoard credentials
const char* THINGSBOARD_SERVER = "demo.thingsboard.io";
const char* ACCESS_TOKEN = "86cOh0fqLUd0r5agYIDh";
const int THINGSBOARD_PORT = 1883;

// MQTT topics
const char* RPC_TOPIC = "v1/devices/me/rpc/request/+";
const char* ATTRIBUTE_TOPIC = "v1/devices/me/attributes";

// GPIO pins
#define GPIO0 0
#define GPIO2 2

#define GPIO0_PIN 3
#define GPIO2_PIN 5

boolean gpioState[] = {false, false};

// MQTT client
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Function prototypes
void connectToWiFi();
void connectToThingsBoard();
void onMessage(char* topic, byte* payload, unsigned int length);
String getGpioStatus();
void setGpioStatus(int pin, bool enabled);

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  connectToThingsBoard();
  client.setCallback(onMessage);
}

void loop() {
  if (!client.connected()) {
    connectToThingsBoard();
  }
  client.loop();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi");
}

void connectToThingsBoard() {
  if (client.connect("ESP32", ACCESS_TOKEN, "", 0, 0, 0, "", true)) {
    Serial.println("Connected to ThingsBoard");

    // Subscribe to RPC topic
    client.subscribe(RPC_TOPIC);
    Serial.println("Subscribed to RPC topic");

    // Publish GPIO status as attributes
    String gpioStatus = getGpioStatus();
    client.publish(ATTRIBUTE_TOPIC, gpioStatus.c_str());
    Serial.println("Published GPIO status as attributes");
  } else {
    Serial.print("Failed to connect to ThingsBoard, rc=");
    Serial.println(client.state());
  }
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  Serial.println("Received message:");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  payload: ");
  Serial.write(payload, length);
  Serial.println();

  // Decode JSON request
  DynamicJsonDocument jsonBuffer(200);
  deserializeJson(jsonBuffer, payload, length);
  JsonObject data = jsonBuffer.as<JsonObject>();

  // Check request method
  String methodName = data["method"].as<String>();
  if (methodName == "getGpioStatus") {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    String gpioStatus = getGpioStatus();
    client.publish(responseTopic.c_str(), gpioStatus.c_str());
    Serial.println("Replied with GPIO status");
  } else if (methodName == "setGpioStatus") {
    // Update GPIO status and reply
    int pin = data["params"]["pin"].as<int>();
    bool enabled = data["params"]["enabled"].as<bool>();
    setGpioStatus(pin, enabled);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    String responsePayload = "{\"pin\": " + String(pin) +", \"enabled\": " + String(enabled) + "}";
    client.publish(responseTopic.c_str(), responsePayload.c_str());
    Serial.println("Updated GPIO status and replied");
  }
}

String getGpioStatus() {
  DynamicJsonDocument jsonBuffer(200);
  JsonObject data = jsonBuffer.to<JsonObject>();
  data["gpio0"] = gpioState[0];
  data["gpio2"] = gpioState[1];
  String jsonStr;
  serializeJson(data, jsonStr);
  return jsonStr;
}

void setGpioStatus(int pin, bool enabled) {
  if (pin == GPIO0_PIN) {
    gpioState[0] = enabled;
    digitalWrite(GPIO0, enabled ? HIGH : LOW);
  } else if (pin == GPIO2_PIN) {
    gpioState[1] = enabled;
    digitalWrite(GPIO2, enabled ? HIGH : LOW);
  }
}
