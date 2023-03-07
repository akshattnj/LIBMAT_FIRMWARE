#include "./Commons.h"

namespace Commons 
{
    uint8_t WiFiFlags = 0;
    uint8_t WSFlags = 0;
    bool activeDoor = true;

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

    esp_err_t httpEventHandler(esp_http_client_event_handle_t evt)
    {
        switch (evt->event_id)
        {
        case HTTP_EVENT_ON_DATA:
            printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
            break;

        default:
            break;
        }
        return ESP_OK;
    }

    void sendPostMessage()
    {
        esp_http_client_config_t postConfig = {
        .url = "http://3.111.8.114:5000/swapDevice",
        .cert_pem = NULL,
        .method = HTTP_METHOD_POST,
        .event_handler = httpEventHandler};
        
        esp_http_client_handle_t client = esp_http_client_init(&postConfig);

        char  *post_data = "{\"oldDeviceId\": \"d9b1f7f0\", \"newDeviceId\": \"21d930c0\", \"customerId\": 2}";
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
    }
}