#include "WebSocket.h"

static uint8_t wsClientListSize;
static uint8_t clientStatus;

/**
 * Sends a Web Socket message to a specified client
 * @param args Arguments needed by the function to send a websocket request.
 * In this case, the struct asyncRespArgs that is defined in the .h file
 */
static void sendMessageToClient(void *args)
{
    WSClient *clientArgs = (WSClient *)args;
    httpd_ws_frame_t wsPacket;
    memset(&wsPacket, 0, sizeof(httpd_ws_frame_t));
    uint8_t *buffer = calloc(1, clientArgs->messageLen + 5);
    size_t len = sprintf((char *)buffer, "%s %d", (char *)clientArgs->message, clientArgs->fileDescriptor);
    wsPacket.payload = buffer;
    wsPacket.len = len;
    wsPacket.type = HTTPD_WS_TYPE_TEXT;
    httpd_ws_send_frame_async(clientArgs->handler, clientArgs->fileDescriptor, &wsPacket);
    free(buffer);
}

static void sendDisconnect(void *args) {
    WSClient *clientArgs = (WSClient *)args;
    httpd_ws_frame_t wsPacket;
    memset(&wsPacket, 0, sizeof(httpd_ws_frame_t));
    wsPacket.payload = NULL;
    wsPacket.len = 0;
    wsPacket.type = HTTPD_WS_TYPE_CLOSE;
    httpd_ws_send_frame_async(clientArgs->handler, clientArgs->fileDescriptor, &wsPacket);
}

/**
 * Adds a message send request to the http Websocket request queue
 * @param clientNum The client number as per the variable wsClientList defined in the .h file
 * @param data Pointer to the data to be sent
 * @param messageLen Size of the data to be sent
 */
static esp_err_t addToWSQueue(uint8_t clientNum, char *data, size_t messageLen)
{
    if (clientNum >= wsClientListSize)
    {
        ESP_LOGE(SERV_TAG, "Invalid Client");
        return ESP_ERR_INVALID_ARG;
    }
    wsClientList[clientNum].message = data;
    wsClientList[clientNum].messageLen = messageLen;
    ESP_LOGI(SERV_TAG, "Queuing job for fd: %d", wsClientList[clientNum].fileDescriptor);
    httpd_queue_work(wsClientList[clientNum].handler, sendMessageToClient, &wsClientList[clientNum]);
    return ESP_OK;
}

void broadcastToAll(void *args)
{
    while (true)
    {
        if (wsClientListSize > 0)
        {
            for (uint8_t i = 0; i < wsClientListSize; i++)
            {
                addToWSQueue(i, "Hello", 5);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Remove inactive devices and move the queue
 * @param devicePos position of the device
*/
void removeDeviceFromQueue(uint8_t devicePos)
{
    httpd_queue_work(wsClientList[devicePos].handler, sendDisconnect, &wsClientList[devicePos]);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    httpd_sess_trigger_close(wsClientList[devicePos].handler, wsClientList[devicePos].fileDescriptor);
    for(int i = devicePos + 1; i < MAX_SOCKETS; i++) {
        wsClientList[i - 1] = wsClientList[i];
    }
    wsClientListSize--;
}

/**
 * Task to determine which websockets have become non responsive and remove from queue accordingly
 * @param args task arguments
*/
void scanForDisconnectedDevices(void *args)
{
    while (true)
    {
        clientStatus = 0x00;
        for (int i = 0; i < wsClientListSize; i++)
        {
            addToWSQueue(i, "TEST", 4);
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        uint16_t testVariable = 0x01;
        for(int i = 0; i < wsClientListSize; i++)
        {
            if((testVariable & clientStatus) == 0)
            {
                ESP_LOGE(SERV_TAG, "Client at %d is non responsive. Removing", i);
                removeDeviceFromQueue(i);
            }
            testVariable = testVariable << 1;
        }
    }
}

/**
 * Finds where the WS device is in queue by File Descriptor
 * @param fileDescriptor File Descriptor of websocket 
*/
int8_t scanForDevice(int fileDescriptor) {
    for(int i = 0; i < wsClientListSize; i++)
    {
        if(wsClientList[i].fileDescriptor == fileDescriptor)
        {
            return i;
        }
    }
    return -1;
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
    uint8_t *buffer = calloc(1, wsPacket.len + 1);
    wsPacket.payload = buffer;
    ret = httpd_ws_recv_frame(req, &wsPacket, wsPacket.len);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SERV_TAG, "Something went wrong with getting data frame");
        return ret;
    }
    ESP_LOGI(SERV_TAG, "Got Data: %s", wsPacket.payload);

    // Process packet contents
    if (strncmp("CONNECT", (char *)wsPacket.payload, 7) == 0 && wsClientListSize < 12)
    {
        free(buffer);
        buffer = calloc(1, 10);
        wsPacket.payload = buffer;
        if (wsClientListSize < 12)
        {
            // Make sure device is not already registered
            if(scanForDevice(httpd_req_to_sockfd(req)) != -1) {
                sprintf((char *)wsPacket.payload, "INVALID");
                wsPacket.len = 7;
                ret = httpd_ws_send_frame(req, &wsPacket);
                goto finish;
            }
            // Register new connection
            ESP_LOGI(SERV_TAG, "CONNECT ACCEPT");
            wsClientList[wsClientListSize].handler = req->handle;
            wsClientList[wsClientListSize].fileDescriptor = httpd_req_to_sockfd(req);
            clientStatus = clientStatus | (0x01 << wsClientListSize);
            wsClientListSize++;

            // Sent acceptance response
            sprintf((char *)wsPacket.payload, "ACCEPT");
            wsPacket.len = 6;
            ret = httpd_ws_send_frame(req, &wsPacket);
            goto finish;
        }
        else
        {
            // Hub is full
            sprintf((char *)wsPacket.payload, "FULL");
            wsPacket.len = 4;
            ret = httpd_ws_send_frame(req, &wsPacket);
            goto finish;
        }
    }
    // Track all connections that responded with OK and mark as alive
    if(strncmp("OK", (char *)wsPacket.payload, 2) == 0)
    {
        int8_t device = scanForDevice(httpd_req_to_sockfd(req));
        if(device == -1)
        {
            ESP_LOGE(SERV_TAG, "Could not find device");
            goto finish;
        }
        uint8_t flag = 0x01 << device;
        ESP_LOGI(SERV_TAG, "Setting device tag %X", flag);
        clientStatus = clientStatus | flag;
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
    .is_websocket = true};

/**
 * Start the Web Socket server and register all relavent URIs
 */
httpd_handle_t startWebserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = MAX_SOCKETS;
    wsClientListSize = 0;
    clientStatus = 0;

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