#include "./WS.h"

namespace WS
{
    esp_websocket_client_handle_t client;
    const char* message = "DATA{\"id\":\"asdf\",\"aid\":\"1234\",\"cid\":2}}";

    void startWSClient(void)
    {
        esp_websocket_client_config_t websocket_cfg = {};
        websocket_cfg.uri = WS_URI;
        ESP_LOGI(WS_TAG, "Connecting to %s...", websocket_cfg.uri);
        client = esp_websocket_client_init(&websocket_cfg);
        esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, wsEventHandler, (void *)client);
        esp_websocket_client_start(client);
        while (1)
        {
            char data[10];
            sprintf(data, "CONNECT");
            if (esp_websocket_client_is_connected(client))
            {
                ESP_LOGI(WS_TAG, "Sending %s", data);
                esp_websocket_client_send_text(client, data, 7, portMAX_DELAY);
                break;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    void wsEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
        switch (event_id)
        {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(WS_TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(WS_TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            Commons::wsFlags = Commons::wsFlags | BIT1;
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(WS_TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGI(WS_TAG, "Received opcode=%d", data->op_code);
            if (data->op_code == 0x08 && data->data_len == 2)
            {
                ESP_LOGW(WS_TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
                Commons::wsFlags = Commons::wsFlags | BIT1;
            }
            else
            {
                ESP_LOGW(WS_TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
                if (strncmp("DATA", data->data_ptr, 4) == 0)
                {
                    sendWSMessage((char *)message, strlen(message));
                }
            }
            ESP_LOGW(WS_TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(WS_TAG, "WEBSOCKET_EVENT_ERROR");
            break;
        }
    }

    /**
     * Disconnect the websocket client
     */
    void destroyWSClient(void)
    {
        if (esp_websocket_client_is_connected(client))
            esp_websocket_client_close(client, portMAX_DELAY);
        ESP_LOGI(WS_TAG, "Websocket Stopped");
        esp_websocket_client_destroy(client);
    }

    /**
     * Send a message to Websocket server
     * @param data Data to be sent
     * @param dataLen Length of data to be sent
     */
    void sendWSMessage(char *data, size_t dataLen)
    {
        while (1)
        {
            if (esp_websocket_client_is_connected(client))
            {
                char toTransmit[dataLen + 5];
                sprintf(toTransmit, "%s", data);
                ESP_LOGI(WS_TAG, "Sending %s", toTransmit);
                esp_websocket_client_send_text(client, toTransmit, dataLen, portMAX_DELAY);
                break;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    void wsClientTask(void *args)
    {
        while (1)
        {
            if ((Commons::wsFlags & BIT0) > 0)
            {
                Commons::wsFlags = Commons::wsFlags & (~BIT0);

                startWSClient();
            }
            if ((Commons::wsFlags & BIT1) > 0)
            {
                Commons::wsFlags = Commons::wsFlags & (~BIT1);
                destroyWSClient();
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}