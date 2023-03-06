extern "C"
{
#include <stdio.h>
}

#include "src/Commons/Commons.h"
#include "src/Networking/WiFi/WiFi.h"

extern "C" void app_main(void)
{
    Commons::startNVS();
    WiFi::initialiseWiFiSoftAP();
    WiFi::connectToWiFi("ZOLO~JAZZ", "Z@0J@zZ$");
}
