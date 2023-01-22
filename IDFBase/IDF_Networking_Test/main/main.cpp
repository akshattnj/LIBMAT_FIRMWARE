extern "C"
{
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_nimble_hci.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <console/console.h>
#include <services/gap/ble_svc_gap.h>
#include "src/nimble.h"
}
#include <string.h>

extern "C" void app_main(void)
{
    startNVS();
    startBLE();
    xTaskCreate([](void *args) {
        uint16_t x = 0;
        char buffer[50];
        while(1){
            if(notify_state){
                sprintf(buffer, "Tester %d\n", x);
                sendNotification(buffer, strlen(buffer));
                x++;
            }
            vTaskDelay(1000 / portTICK_RATE_MS);
        } 
    }, "Task", 2048, NULL, 10, NULL);
}
