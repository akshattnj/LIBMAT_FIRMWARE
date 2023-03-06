#include "./WS.h"

namespace WS
{

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
            
            sprintf((char *)wsPacket.payload, "ACCEPT");
            wsPacket.len = 6;
            ret = httpd_ws_send_frame(req, &wsPacket);
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