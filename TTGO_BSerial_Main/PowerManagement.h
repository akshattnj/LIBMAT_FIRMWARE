#include <axp20x.h>
#include <Arduino.h>
#include <esp_log.h>

#include "Definations.h"

class PowerManagement
{
public:
    AXP20X_Class *axp;

    /**
     * @brief Construct a new Power Management handler with all defaults
     *
     * @param axpNew Power management handeler class
     */
    PowerManagement(AXP20X_Class *axpNew)
    {
        axp = axpNew;
    }

    /**
     * @brief Setup for the Power Management unit
     *
     * @return true Power management setup success
     * @return false Power management setup failed
     */
    bool setupPMU()
    {
        int ret = axp->begin(Wire, AXP192_SLAVE_ADDRESS);

        if (ret == AXP_FAIL)
        {
            ESP_LOGE(TAG, "AXP Power begin failed");
            return false;
        }

        //! Turn off unused power
        axp->setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
        axp->setPowerOutPut(AXP192_LDO2, AXP202_OFF);
        axp->setPowerOutPut(AXP192_LDO3, AXP202_OFF);
        axp->setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
        axp->setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
        axp->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

        float vbus_v = axp->getVbusVoltage();
        float vbus_c = axp->getVbusCurrent();
        float batt_v = axp->getBattVoltage();
        ESP_LOGI(TAG, "VBUS:%.2f mV %.2f mA ,BATTERY: %.2f\n", vbus_v, vbus_c, batt_v);

        return true;
    }

    /**
     * @brief Setup for modem power
     *
     */
    void powerSetup()
    {
        ESP_LOGI("TAG", "Began power setup");
        while (setupPMU() == false)
            ;
        ESP_LOGI("TAG", "PMU setup complete");
        pinMode(MODEM_POWER_KEY, OUTPUT);
        pinMode(MODEM_POWER_ON, OUTPUT);
        // Turn on the Modem power first
        digitalWrite(MODEM_POWER_ON, HIGH);

        // Pull down PWRKEY for more than 1 second according to manual requirements
        digitalWrite(MODEM_POWER_KEY, HIGH);
        delay(100);
        digitalWrite(MODEM_POWER_KEY, LOW);
        delay(1000);
        digitalWrite(MODEM_POWER_KEY, HIGH);
    }
};