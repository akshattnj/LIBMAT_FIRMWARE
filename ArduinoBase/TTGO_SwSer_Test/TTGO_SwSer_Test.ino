#include <TinyGPSPlus.h>
#include "SoftwareSerial.h"

SoftwareSerial swSer;
TinyGPSPlus gps;

uint32_t timer = 0L;

void setup()
{
    Serial.begin(115200);
    swSer.begin(9600, SWSERIAL_8N1, 34, 14);
}

void loop()
{
    while (swSer.available())
    {
        gps.encode(swSer.read());
    }
    Serial.printf("%f %f\n", gps.location.lat(), gps.location.lng());
}