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
        LEDLock = false;
    }

    void batteryBar(float volts)
    {
        if (LEDLock)
            return;
        if (volts < 42.2)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, pixels->numPixels());
            pixels->show();
            return;
        }
        int i = map(volts, 42.2, 53.2, 0, 255);
        int j = 255 - i;
        int LEDsToFill = map(volts, 42.2, 53.2, pixels->numPixels(), 0);

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