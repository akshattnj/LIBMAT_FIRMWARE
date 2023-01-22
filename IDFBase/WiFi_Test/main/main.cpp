/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "src/definations.h"

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "src/WiFiHandler.h"
}

extern "C" void app_main(void)
{
    initialiseNVS();
    initialiseWiFiSoftAP();
}
