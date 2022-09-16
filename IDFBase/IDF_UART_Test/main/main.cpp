#include "src/Definations.h"
#include "src/UARTHandler.h"

extern "C"
{
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
}

UARTHandler testUART(UART_PORT_NUM, UART_RXD, UART_TXD);

extern "C" void app_main(void)
{
    testUART.begin();
    xTaskCreate([](void *args)
                { testUART.readData(); },
                "UART Reader", UART_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate([](void *args)
                {
        while(1) {
            testUART.writeData("AT\r\n");
            vTaskDelay(1000/portTICK_RATE_MS);
        } },
                "UART Writer", UART_STACK_SIZE, NULL, 10, NULL);
}
