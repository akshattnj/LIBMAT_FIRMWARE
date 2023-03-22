#include <ESP32CAN.h>
#include <CAN_config.h>
#include <cJSON.h>

// Define CAN bus pins
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

// Define DBC message IDs
#define VOLTAGE_INFO_MSG_ID 2566848976
#define BATTERY_STATE_MSG_ID 2566850000

// Define DBC signals
#define BATTERY_VOLTAGE_SIGNAL 7
#define MIN_VOLTAGE_SIGNAL 39
#define MAX_VOLTAGE_SIGNAL 55
#define EQUIVALENT_CYCLE_SIGNAL 39
#define STATE_OF_HEALTH_SIGNAL 23
#define STATE_OF_CHARGE_SIGNAL 7

// Declare CAN bus object
CAN_device_t CAN_cfg;

// Declare cJSON objects
cJSON *root, *data, *battery_voltage, *min_voltage, *max_voltage, *equivalent_cycle, *state_of_health, *state_of_charge;

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Initialize CAN bus
  CAN_cfg.speed = CAN_SPEED_500KBPS;
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
  CAN_init();
  
  // Initialize cJSON objects
  root = cJSON_CreateObject();
  data = cJSON_CreateObject();
  battery_voltage = cJSON_CreateNumber(0);
  min_voltage = cJSON_CreateNumber(0);
  max_voltage = cJSON_CreateNumber(0);
  equivalent_cycle = cJSON_CreateNumber(0);
  state_of_health = cJSON_CreateNumber(0);
  state_of_charge = cJSON_CreateNumber(0);
}

void loop() {
  // Wait for CAN message to be received
    CAN_frame_t rx_frame;
  if(xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3*portTICK_PERIOD_MS) == pdTRUE) {
    // Check if message is a Voltage Info message
    if(rx_frame.MsgID == VOLTAGE_INFO_MSG_ID) {
      // Extract signal values from message data
      float battery_voltage_value, min_voltage_value, max_voltage_value;
      memcpy(&battery_voltage_value, &rx_frame.data + 3, 4);
      memcpy(&min_voltage_value, &rx_frame.data + 4, 2);
      memcpy(&max_voltage_value, &rx_frame.data + 6, 2);
      
      // Convert signal values to engineering units
      battery_voltage_value *= 0.001;
      min_voltage_value *= 0.001;
      max_voltage_value *= 0.001;
      
      // Update cJSON objects with signal values
      cJSON_SetNumberValue(battery_voltage, battery_voltage_value);
      cJSON_SetNumberValue(min_voltage, min_voltage_value);
      cJSON_SetNumberValue(max_voltage, max_voltage_value);
      
      // Add cJSON objects to root object
      cJSON_AddItemToObject(data, "Battery Voltage", battery_voltage);
      cJSON_AddItemToObject(data, "Min Voltage", min_voltage);
      cJSON_AddItemToObject(data, "Max Voltage", max_voltage);
      cJSON_AddItemToObject(root, "Voltage Info", data);
    }
    // Check if message is a Battery State message
    else if(rx_frame.MsgID == BATTERY_STATE_MSG_ID) {
      // Extract signal values from message data
      uint32_t equivalent_cycle_value;
      uint16_t state_of_health_value, state_of_charge_value;
      memcpy(&equivalent_cycle_value, &rx_frame.data + 4, 4);
      memcpy(&state_of_health_value, &rx_frame.data + 2, 2);
      memcpy(&state_of_charge_value, &rx_frame.data, 2);
  
  // Convert signal values to engineering units
  equivalent_cycle_value *= 1;
  state_of_health_value *= 0.01;
  state_of_charge_value *= 0.01;
  
  // Update cJSON objects with signal values
//  cJSON_SetNumberValue(equivalent_cycle, equivalent_cycle_value);
//  cJSON_SetNumberValue(state_of_health, state_of_health_value);
  cJSON_SetNumberValue(state_of_charge, state_of_charge_value);
  
  // Add cJSON objects to root object
//  cJSON_AddItemToObject(data, "Equivalent Cycle", equivalent_cycle);
//  cJSON_AddItemToObject(data, "State of Health", state_of_health);
  cJSON_AddItemToObject(data, "State of Charge", state_of_charge);
  cJSON_AddItemToObject(root, "Battery State", data);
}

// Print JSON object to serial monitor
char *json_str = cJSON_Print(root);
Serial.println(json_str);
free(json_str);

// Delete cJSON objects and reset root object
cJSON_Delete(root);
root = cJSON_CreateObject();
data = cJSON_CreateObject(); 
  }}
