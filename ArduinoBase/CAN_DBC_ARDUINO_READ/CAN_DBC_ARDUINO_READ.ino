#include <CAN.h>
#include <cJSON.h>

// DBC message definitions
#define VOLTAGE_INFO_MSG_ID 2566848976
#define BATTERY_STATE_MSG_ID 2566850000

// CAN bus settings
#define CAN_SPEED CAN_SPEED_500KBPS
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

void setup() {
  Serial.begin(115200);

  // Initialize CAN bus
  if (!CAN.begin(CAN_SPEED, CAN_TX_PIN, CAN_RX_PIN)) {
    Serial.println("Failed to initialize CAN bus");
    while (1) {}
  }

  // Set filter to receive only messages with the defined IDs
  CAN.filterExtended(VOLTAGE_INFO_MSG_ID, 0xFFFFFFFF);
  CAN.filterExtended(BATTERY_STATE_MSG_ID, 0xFFFFFFFF);
}

void loop() {
  // Check if there is a new CAN message
  if (CAN.available()) {
    // Read the message
    CANMessage msg;
    CAN.read(msg);

    // Check the message ID to determine which DBC message it belongs to
    if (msg.id == VOLTAGE_INFO_MSG_ID) {
      // Parse the message using cJSON
      cJSON *root = cJSON_Parse(msg.data);

      // Check if parsing was successful
      if (root != NULL) {
        // Get the values of the signals
        double battery_voltage = cJSON_GetObjectItem(root, "Battery_Voltage")->valuedouble;
        double min_voltage = cJSON_GetObjectItem(root, "Min_Voltage")->valuedouble;
        double max_voltage = cJSON_GetObjectItem(root, "Max_Voltage")->valuedouble;

        // Print the values
        Serial.print("Battery voltage: ");
        Serial.println(battery_voltage);
        Serial.print("Minimum voltage: ");
        Serial.println(min_voltage);
        Serial.print("Maximum voltage: ");
        Serial.println(max_voltage);

        // Free the cJSON object
        cJSON_Delete(root);
      } else {
        Serial.println("Failed to parse message");
      }
    } else if (msg.id == BATTERY_STATE_MSG_ID) {
      // Parse the message using cJSON
      cJSON *root = cJSON_Parse(msg.data);

      // Check if parsing was successful
      if (root != NULL) {
        // Get the values of the signals
        double equivalent_cycle = cJSON_GetObjectItem(root, "Equivalent_Cycle")->valuedouble;
        int state_of_health = cJSON_GetObjectItem(root, "State_of_Health")->valueint;
        int state_of_charge = cJSON_GetObjectItem(root, "State_of_Charge")->valueint;

        // Print the values
        Serial.print("Equivalent cycle: ");
        Serial.println(equivalent_cycle);
        Serial.print("State of health: ");
        Serial.println(state_of_health);
        Serial.print("State of charge: ");
        Serial.println(state_of_charge);

        // Free the cJSON object
        cJSON_Delete(root);
      } else {
        Serial.println("Failed to parse message");
      }
    }
  }
}
