#include <Adafruit_ADS1X15.h>
#include <esp_log.h>

int Vo;
float R1 = 10000;
float logR2, R2, T;
const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

Adafruit_ADS1015 ads;
float A3_offset = 0.060;

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
    float volts0, volts1, volts2, volts3, volts_bkp_batt, volts_ev_batt, degree_celcius;

    adc0 = ads.readADC_SingleEnded(0);
    adc1 = ads.readADC_SingleEnded(1);
    adc2 = ads.readADC_SingleEnded(2);
    adc3 = ads.readADC_SingleEnded(3);

    volts0 = ads.computeVolts(adc0);
    volts1 = ads.computeVolts(adc1);
    volts2 = ads.computeVolts(adc2);
    volts3 = ads.computeVolts(adc3)- A3_offset;
    volts_bkp_batt = calc_batt_voltage(volts3);
    volts_ev_batt = calc_ev_voltage(volts1)-0.80;
    degree_celcius = calc_ntc_temp(adc2);
    ESP_LOGI("TAG", "\nChannel 0 : %0.2f V\nEV Voltage(1): %0.2f V\nTemprature(2) : %0.2f °C\nBackup batt. Voltage(3) : %0.2f V\n", volts0, volts_ev_batt, degree_celcius, volts_bkp_batt);

    delay(1000);
}

float calc_batt_voltage(float vout)
{
  float r1 = 96.31;
  float r2 = 19.70;
  float vin = (vout*(r1+r2))/r2;
  return vin;
}
float calc_ev_voltage(float vout)
{
  float r1 = 99.80;
  float r2 = 4.60;
  float vin = (vout*(r1+r2))/r2;
  return vin;
}
float calc_ntc_temp(int vout)
{
    int Vo;
    float R1 = 1140000;
    float logR2, R2, T;
    const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    Vo = vout;
    R2 = R1 * (1023.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    T = -T + 273.15;
    return T;
}
