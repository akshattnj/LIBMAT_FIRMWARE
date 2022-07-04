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
        pixels->fill(pixels->Color(255, 0, 0), 0, 6);
        pixels->show();
        digitalWrite(32, LOW);
        delay(250);

        pixels->fill(pixels->Color(0, 0, 0), 0, 6);
        pixels->show();
        digitalWrite(32, HIGH);
        delay(250);

        pixels->fill(pixels->Color(255, 0, 0), 0, 6);
        pixels->show();
        digitalWrite(32, LOW);
        delay(250);

        pixels->fill(pixels->Color(0, 0, 0), 0, 6);
        pixels->show();
        digitalWrite(32, HIGH);
        delay(250);
        LEDLock = false;
    }

    void vehicleUnlockAnimation()
    {
        LEDLock = true;
        digitalWrite(32, LOW);
        pixels->fill(pixels->Color(0, 255, 0), 0, 6);
        pixels->show();
        delay(250);

        digitalWrite(32, HIGH);
        pixels->fill(pixels->Color(0, 0, 0), 0, 6);
        pixels->show();
        delay(250);

        digitalWrite(32, LOW);
        pixels->fill(pixels->Color(0, 255, 0), 0, 6);
        pixels->show();
        delay(250);

        digitalWrite(32, HIGH);
        pixels->fill(pixels->Color(0, 0, 0), 0, 6);
        pixels->show();
        delay(250);
        LEDLock = false;
    }

    void batteryUnlockAnimation()
    {
        LEDLock = true;
        pixels->fill(pixels->Color(0, 0, 255), 0, 6);
        pixels->show();
        LEDLock = false;
    }

    void batteryBar(float volts)
    {
        if (LEDLock)
            return;
        int i = map(volts, 42.2, 53.2, 0, 255);
        if (volts >= 51 && volts < 53.2)
        {
            pixels->fill(pixels->Color(0, i, 0), 0, 6);
            pixels->show();
        }

        if (volts >= 48.8 && volts < 51)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 1);
            pixels->fill(pixels->Color(10, i, 0), 1, 5);
            pixels->show();
        }

        if (volts >= 46.6 && volts < 48.8)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 2);
            pixels->fill(pixels->Color(51, i, 0), 2, 4);
            pixels->show();
        }
        if (volts >= 44.4 && volts < 46.6)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 3);
            pixels->fill(pixels->Color(102, i, 0), 3, 3);
            pixels->show();
        }
        if (volts >= 42.2 && volts < 44.4)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 4);
            pixels->fill(pixels->Color(160, i, 0), 4, 2);
            pixels->show();
        }
        if (volts >= 40 && volts < 42.2)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 5);
            pixels->fill(pixels->Color(254, 0, 0), 5, 1);
            pixels->show();
        }
        if (volts >= 5 && volts < 40)
        {
            pixels->fill(pixels->Color(0, 0, 0), 0, 6);
            pixels->show();
        }
    }

private:
};