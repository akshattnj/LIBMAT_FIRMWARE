#define MODEM_TX             27
#define MODEM_RX             26
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       25
#define I2C_SDA              21
#define I2C_SCL              22

#include <axp20x.h>         //https://github.com/lewisxhe/AXP202X_Library

AXP20X_Class axp;

bool setupPMU()
{
// For more information about the use of AXP192, please refer to AXP202X_Library https://github.com/lewisxhe/AXP202X_Library
    Wire.begin(I2C_SDA, I2C_SCL);
    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);

    if (ret == AXP_FAIL) {
        Serial.println("AXP Power begin failed");
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
    Serial.printf("VBUS:%.2f mV %.2f mA ,BATTERY: %.2f\n", vbus_v, vbus_c, batt_v);

    return true;
}

void setupModem()
{
    if (setupPMU() == false) {
        Serial.println("Setting power error");
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


void setup()
{
  
  Serial.begin(115200);
  setupModem();
  Serial.println("ESP32+SIM800C AT CMD Test");
  delay(10000);
  
  Serial2.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
}


void loop() {
  while (Serial2.available()) {
    Serial.print(char(Serial2.read()));
  }
  while (Serial.available()) {
     Serial2.print(char(Serial.read()));
  }
}
