#include <Adafruit_ADS1X15.h>
#include <esp_log.h>

Adafruit_ADS1015 ads;

void setup(void)
{
    Serial.begin(115200);
    ESP_LOGI("TAG", "Setting up ADC");

    while (!ads.begin())
    {
        ESP_LOGE("ADC setup failed. Retrying in 2 sec");
        delay(2000);
    }
}

void loop(void)
{
    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3;

    adc0 = ads.readADC_SingleEnded(0);
    adc1 = ads.readADC_SingleEnded(1);
    adc2 = ads.readADC_SingleEnded(2);
    adc3 = ads.readADC_SingleEnded(3);

    volts0 = ads.computeVolts(adc0);
    volts1 = ads.computeVolts(adc1);
    volts2 = ads.computeVolts(adc2);
    volts3 = ads.computeVolts(adc3);

    ESP_LOGI("TAG", "Channel 0 : %0.2f V\nChannel 1 : %0.2f V\nChannel 2 : %0.2f V\nChannel 3 : %0.2f V\n", volts0, volts1, volts2, volts3);

    delay(1000);
}
