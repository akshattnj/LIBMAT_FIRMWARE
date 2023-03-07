#include "./WS.h"

namespace WS
{
    cJSON *json = NULL;
    ClientDetails clientDetails;
    QueueHandle_t swapQueue = xQueueCreate(7, sizeof(ClientDetails));

    static void sendMessageToClient(void *args)
    {
        ClientDetails client = *(ClientDetails *)args;
        httpd_ws_frame_t wsPacket;
        memset(&wsPacket, 0, sizeof(httpd_ws_frame_t));
        uint8_t *buffer = (uint8_t *)calloc(1, client.messageLen + 5);
        size_t len = sprintf((char *)buffer, "%s", (char *)client.message);
        wsPacket.payload = buffer;
        wsPacket.len = len;
        wsPacket.type = HTTPD_WS_TYPE_TEXT;
        httpd_ws_send_frame_async(client.handler, client.fileDescriptor, &wsPacket);
        free(buffer);
    }

    /**
     * Sends a Web Socket disconnect message to a specified client
     */
    static void sendDisconnect(void *args)
    {
        ClientDetails client = *(ClientDetails *)args;
        httpd_ws_frame_t wsPacket;
        memset(&wsPacket, 0, sizeof(httpd_ws_frame_t));
        wsPacket.payload = NULL;
        wsPacket.len = 0;
        wsPacket.type = HTTPD_WS_TYPE_CLOSE;
        httpd_ws_send_frame_async(client.handler, client.fileDescriptor, &wsPacket);
    }


    static esp_err_t addToWsAsyncQueue(ClientDetails client, int fileDescriptor, char *data, size_t messageLen, bool disconnect)
    {
        if(disconnect)
        {
            httpd_queue_work(client.handler, sendDisconnect, &client);
        }
        ESP_LOGI(SERV_TAG, "Queuing job for fd: %d", client.fileDescriptor);
        httpd_queue_work(client.handler, sendMessageToClient, &client);
        return ESP_OK;
    }


    void queueHandlerTask(void *pvParameters)
    {
        ClientDetails client;
        while (1)
        {
            if (xQueueReceive(swapQueue, &client, portMAX_DELAY))
            {
                ESP_LOGI(SERV_TAG, "Got data from queue");
                strncpy(client.message, "SWAP", 5);
                client.messageLen = 5;
                addToWsAsyncQueue(client, client.fileDescriptor, client.message, client.messageLen, false);
                BatteryHandler::handleDoor();
                addToWsAsyncQueue(client, client.fileDescriptor, client.message, client.messageLen, true);
                httpd_sess_trigger_close(client.handler, client.fileDescriptor);
                Commons::sendPostMessage();
            }
        }
    }

    /**
     * Handler for incoming websocket requests
     *  @param req Pointer to incoming request (httpd_req_t)
     */
    static esp_err_t requestHandler(httpd_req_t *req)
    {
        // Response on first connect
        if (req->method == HTTP_GET)
        {
            ESP_LOGI(SERV_TAG, "Handshake done, the new connection was opened");
            return ESP_OK;
        }

        // Get incoming packet length
        httpd_ws_frame_t wsPacket;
        memset(&wsPacket, 0, sizeof(httpd_ws_frame_t));
        wsPacket.type = HTTPD_WS_TYPE_TEXT;
        esp_err_t ret = httpd_ws_recv_frame(req, &wsPacket, 0);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SERV_TAG, "Issue with getting frame length");
            return ret;
        }
        ESP_LOGI(SERV_TAG, "Got frame length %d", wsPacket.len);

        // Validate packet
        if (wsPacket.len > 200 || wsPacket.len <= 0)
        {
            ESP_LOGE(SERV_TAG, "Invalid message length");
            return ESP_FAIL;
        }

        // Get packet contents
        uint8_t *buffer = (uint8_t *)calloc(1, wsPacket.len + 1);
        wsPacket.payload = buffer;
        ret = httpd_ws_recv_frame(req, &wsPacket, wsPacket.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SERV_TAG, "Something went wrong with getting data frame");
            return ret;
        }
        ESP_LOGI(SERV_TAG, "Got Data: %s", wsPacket.payload);

        // Process packet contents
        if ((strncmp("CONNECT", (char *)wsPacket.payload, 7) == 0))
        {
            free(buffer);
            buffer = (uint8_t *)calloc(1, 20);
            wsPacket.payload = buffer;
            
            sprintf((char *)wsPacket.payload, "DATA");
            wsPacket.len = 6;
            ret = httpd_ws_send_frame(req, &wsPacket);
            goto finish;
        }

        if((strncmp("DATA", (char *)wsPacket.payload, 4) == 0))
        {
            memcpy(wsPacket.payload, wsPacket.payload + 4, wsPacket.len - 4);
            json = cJSON_Parse((char *)wsPacket.payload);
            if (json == NULL)
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    ESP_LOGE(SERV_TAG, "Error before: %s", error_ptr);
                }
                goto finish;
            }
            strncpy(clientDetails.clientId, cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "id")), strlen(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "id"))));
            strncpy(clientDetails.clientAccessId, cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "aid")), strlen(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "aid"))));
            clientDetails.customerId = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "cid"));
            clientDetails.handler = req->handle;
            clientDetails.fileDescriptor = httpd_req_to_sockfd(req);
            xQueueSend(swapQueue, &clientDetails, portMAX_DELAY);
            cJSON_Delete(json);
            goto finish;
        }

        // Track all connections that responded with OK and mark as alive
        if (strncmp("OK", (char *)wsPacket.payload, 2) == 0)
        {
            goto finish;
        }

        sprintf((char *)wsPacket.payload, "INVALID");
        wsPacket.len = 7;
        ret = httpd_ws_send_frame(req, &wsPacket);

    finish:
        // Clean up
        free(buffer);
        return ret;
    }

    static const httpd_uri_t mainURI = {
        .uri = "/main",
        .method = HTTP_GET,
        .handler = requestHandler,
        .user_ctx = NULL,
        .is_websocket = true
    };
    
    httpd_handle_t startWebserver(void)
    {
        httpd_handle_t server = NULL;
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.max_open_sockets = MAX_SOCKETS;

        ESP_LOGI(SERV_TAG, "Starting server on port: '%d'", config.server_port);
        if (httpd_start(&server, &config) == ESP_OK)
        {
            ESP_LOGI(SERV_TAG, "Registering URI handlers");
            httpd_register_uri_handler(server, &mainURI);
            return server;
        }

        ESP_LOGI(SERV_TAG, "Error starting server!");
        return NULL;
    }
}