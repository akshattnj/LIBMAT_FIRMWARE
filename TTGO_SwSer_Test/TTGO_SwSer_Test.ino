#include "SoftwareSerial.h"
SoftwareSerial swSer;

uint32_t timer = 0L;

void setup()
{
    Serial.begin(9600);
    swSer.begin(9600, SWSERIAL_8N1, 34, 14);
    timer = millis();
}

void loop()
{
    char c[100];
    int i = 0;
    while (1)
    {
        char s = swSer.read();
        if (s > 0 && s < 127)
            Serial.println((int)s);
        if (millis() - timer > 1000L)
        {
            swSer.print("Hello");
            timer = millis();
        }
    }
}