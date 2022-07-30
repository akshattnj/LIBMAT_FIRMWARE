#include <axp20x.h>
#include <Arduino.h>
#include <esp_log.h>

class PowerManagement
{
public:
    AXP20X_Class *axp;
    const uint8_t I2C_SDA = 21;
    const uint8_t I2C_SCL = 22;
    const uint8_t modemPowerKey = 4;
    const uint8_t modemPowerOn = 25;

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
     * @brief Construct a new Power Management handler with default I2C pins
     *
     * @param axpNew Power management handeler class
     * @param key Modem power key
     * @param pwr Modem power on
     */
    PowerManagement(AXP20X_Class *axpNew, uint8_t key, uint8_t pwr) : modemPowerKey(key), modemPowerOn(pwr)
    {
        axp = axpNew;
    }

    /**
     * @brief Construct a new Power Management handler
     *
     * @param axpNew Power management handeler class
     * @param sda I2C Serial data pin
     * @param scl I2C Serial clock pin
     * @param key Modem power key
     * @param pwr Modem power on
     */
    PowerManagement(AXP20X_Class *axpNew, uint8_t sda, uint8_t scl, uint8_t key, uint8_t pwr) : I2C_SDA(sda), I2C_SCL(scl), modemPowerKey(key), modemPowerOn(pwr)
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
        Wire.begin(I2C_SDA, I2C_SCL);
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
        pinMode(modemPowerKey, OUTPUT);
        pinMode(modemPowerOn, OUTPUT);
        // Turn on the Modem power first
        digitalWrite(modemPowerOn, HIGH);

        // Pull down PWRKEY for more than 1 second according to manual requirements
        digitalWrite(modemPowerKey, HIGH);
        delay(100);
        digitalWrite(modemPowerKey, LOW);
        delay(1000);
        digitalWrite(modemPowerKey, HIGH);
    }
};