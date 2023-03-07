#include "./Commons.h"

namespace Commons 
{
    uint8_t WiFiFlags = 0;
    uint8_t wsFlags = 0;

    SemaphoreHandle_t semaphoreCAN = xSemaphoreCreateBinary();
    QueueHandle_t queueCAN = xQueueCreate(1, sizeof(uint8_t));

    void startNVS()
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
    }
}