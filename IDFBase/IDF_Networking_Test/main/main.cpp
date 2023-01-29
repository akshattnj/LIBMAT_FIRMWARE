extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "src/NimBLE/nimble.h"
#include "src/WiFiClient/WiFiAP.h"
#include "src/WSClient/WSClient.h"
}
#include "src/definations.h"

extern "C" void app_main(void)
{
    startNVS();
    startBLE();
    initWiFiAP();
    
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

    xTaskCreate(wsClientTask, "Websocket", 2048, NULL, 10, NULL);
}
