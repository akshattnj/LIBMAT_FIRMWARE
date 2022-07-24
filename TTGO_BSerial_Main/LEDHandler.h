#include <Adafruit_NeoPixel.h>

/**
 * @brief Class to handle LED related animations
 *
 */
class LEDHandler
{
public:
    Adafruit_NeoPixel *pixels;
    bool LEDLock = false;

    LEDHandler(Adafruit_NeoPixel *pixelsPointer)
    {
        pixels = pixelsPointer;
    }

    void initLED()
    {
        pixels->begin();
    }

    void vehicleLockAnimation()
    {
        LEDLock = true;
        pixels->fill(pixels->Color(255, 0, 0), 0, pixels->numPixels());
        pixels->show();
        digitalWrite(32, LOW);
        delay(250);

        pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels());
        pixels->show();
        digitalWrite(32, HIGH);
        delay(250);

        pixels->fill(pixels->Color(255, 0, 0), 0, pixels->numPixels());
        pixels->show();
        digitalWrite(32, LOW);
        delay(250);

        pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels());
        pixels->show();
        digitalWrite(32, HIGH);
        delay(250);
        resetLED();
    }

    void vehicleUnlockAnimation()
    {
        LEDLock = true;
        digitalWrite(32, LOW);
        pixels->fill(pixels->Color(0, 255, 0), 0, pixels->numPixels());
        pixels->show();
        delay(250);

        digitalWrite(32, HIGH);
        pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels());
        pixels->show();
        delay(250);

        digitalWrite(32, LOW);
        pixels->fill(pixels->Color(0, 255, 0), 0, pixels->numPixels());
        pixels->show();
        delay(250);

        digitalWrite(32, HIGH);
        pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels());
        pixels->show();
        delay(250);
        LEDLock = false;
    }

    void batteryUnlockAnimation()
    {
        LEDLock = true;
        pixels->fill(pixels->Color(0, 0, 255), 0, pixels->numPixels());
        pixels->show();
        delay(3000);
        pixels->clear();
        LEDLock = false;
    }

    long mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
    {
        const float dividend = out_max - out_min;
        const float divisor = in_max - in_min;
        const float delta = x - in_min;
        if (divisor == 0)
        {
            log_e("Invalid map input range, min == max");
            return -1; // AVR returns -1, SAM returns 0
        }
        return (long)((delta * dividend + (divisor / 2.00)) / divisor + out_min);
    }

    void batteryBar(float batteryPercent)
    {
        if (LEDLock)
            return;
        int i = mapFloat(batteryPercent, 0, 100, 0, 255);
        int j = 255 - i;
        int LEDsToFill = mapFloat(batteryPercent, 0, 100, 0, pixels->numPixels());

        pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels() - LEDsToFill);
        pixels->fill(pixels->Color(j, i, 0), pixels->numPixels() - LEDsToFill, LEDsToFill);
        pixels->show();
    }

    void resetLED()
    {
        pixels->fill(pixels->Color(0, 0, 0), 0, 6);
        pixels->show();
    }

private:
};