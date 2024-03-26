#include <stdio.h>
#include <esp_http_server.h>

#include "esp_log.h"
#include "constants.h" // Include static files.
#include "sensors.h" // Include sensor functions.

const char* TAG = "endpoints";

esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);

void start_webserver();
void stop_webserver();

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
    char* data;
};


typedef enum {
    WS_GET_READINGS = 0,
    BROADCAST = 1
} ws_event_t;

/**
 * @brief structure to hold all connected WebSockets.
 * 
 */
struct ws_ll {
    int fd;
    struct ws_ll *next;
};


struct ws_storage {
    httpd_handle_t hd;
    struct ws_ll *head;
};

httpd_handle_t server = NULL;

ws_storage* tracked_ws = NULL;


size_t count_ws_amt() {
    struct ws_ll* current = tracked_ws->head;

    size_t i = 0;
    while (current != NULL) {
        i++;
        current = current->next;
    }

    return i;
}

/**
 * @brief Provide utility to send to all connected WS.
 * 
 * @param arg 
 */
void broadcast_message(char *data) {
    struct ws_ll *current = tracked_ws->head;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    while (current != NULL) {
        ESP_LOGI(TAG, "sent message!");
        httpd_ws_send_frame_async(tracked_ws->hd, current->fd, &ws_pkt);
        current = current->next;
    }

    // free(data); // rely on the caller to free the data. (this may not be the case.)
}


void remove_ws(int fd) {
    // check tracked_ws 
    struct ws_ll* current = tracked_ws->head;
    struct ws_ll* prev = NULL;
    while (current != NULL) {
        if (current->fd == fd) {
            if (prev != NULL) {
                prev->next = current->next;
                free(current);
            } else {
                tracked_ws->head = current->next;
                free(current);
            }
        }
        prev = current;
        current = current->next;
    }

    ESP_LOGE(TAG, "websocket was not in tracked1");
}

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
    struct async_resp_arg *resp_arg = (async_resp_arg*) arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    char* data = resp_arg->data;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    // free(resp_arg->data); // if I don't free this, I rely on ^^ to free.
    free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req, char* data)
{
    struct async_resp_arg *resp_arg = (async_resp_arg*) malloc(sizeof(struct async_resp_arg));
    if (resp_arg == NULL) {
        return ESP_ERR_NO_MEM;
    }
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    resp_arg->data = data;
    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK) {
        free(resp_arg->data);
        free(resp_arg);
    }
    return ret;
}


void track_ws_handle(httpd_handle_t handle) {
    // add the new connection to the linked list

    struct ws_storage *ws_storage = (struct ws_storage*) malloc(sizeof(struct ws_storage));
    ws_storage->hd = handle;
}


esp_err_t recv_ws_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "received any message!");
  
   if (req->method == HTTP_GET) {

        ESP_LOGI(TAG, "Handshake done, the new connection was opened! %d sockets connected.", count_ws_amt());
        // add the new connection to the linked list (tracked_ws)
        // tracked_ws will never be null here.

        if (tracked_ws == NULL) {
            ESP_LOGE(TAG, "Tracked WS is null");
            return ESP_FAIL;
        }

        if (tracked_ws->head == NULL) {
            struct ws_ll *new_ws = (struct ws_ll*) malloc(sizeof(struct ws_ll));
            new_ws->fd = httpd_req_to_sockfd(req);
            new_ws->next = NULL;
            tracked_ws->head = new_ws;
        } else {
            struct ws_ll *current = tracked_ws->head;
            struct ws_ll *new_ws = (struct ws_ll*) malloc(sizeof(struct ws_ll));
            new_ws->fd = httpd_req_to_sockfd(req);
            new_ws->next = current;
            tracked_ws->head = new_ws;    
        }
        
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    // ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = (uint8_t*) calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
    }


    if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {

        ESP_LOGI(TAG, "Websocket disconnected! %d left!", count_ws_amt());
        remove_ws(httpd_req_to_sockfd(req));
        free(buf);
        return ESP_OK;
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        char* tokens;

        tokens = strtok((char*) ws_pkt.payload, ":");
        if (tokens == NULL) {
            ESP_LOGE(TAG, "Invalid message format");
            return ESP_FAIL;
        }

        int ws_code = atoi(tokens);
        
        ESP_LOGI(TAG, "Received message with code: %d", ws_code);

        // shift by ws_code + 1 for the colon
        // this is the data of the message.
        // char* data1 = (char*) ws_pkt.payload + strlen(tokens) + 1; 

        switch (ws_code) {
            case WS_GET_READINGS: {
                // ESP_LOGI(TAG, "Received request for readings");

                // format a string
                char data[128];
                sprintf(data, "{\"distance\": %.2f}", current_distance);

                httpd_ws_frame_t ret_ws_pkt;
                memset(&ret_ws_pkt, 0, sizeof(httpd_ws_frame_t));
                ret_ws_pkt.payload = (uint8_t*)data;
                ret_ws_pkt.len = strlen(data);
                ret_ws_pkt.type = HTTPD_WS_TYPE_TEXT;

                ret = httpd_ws_send_frame(req, &ret_ws_pkt);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
                }
                free(buf);

                return ret;

                // return trigger_async_send(req->handle, req, data); // frees data, so we're good.
            }
            default: {
                ESP_LOGE(TAG, "Unknown WS code: %d", ws_code);
                return ESP_FAIL;
            }      
        }
    }
   
   return ESP_OK;
}

// Handler implementations
esp_err_t index_html_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, (char*) index_html_start, index_html_length);
    return ESP_OK;
}

esp_err_t index_css_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (char*) index_css_start, index_css_length);
    return ESP_OK;
}

esp_err_t index_js_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (char*) index_js_start, index_js_length);
    return ESP_OK;
}

esp_err_t test_html_get_handler(httpd_req_t *req) {
    // Handle another endpoint here
    const char* resp_str = "This is another endpoint!";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

void websocket_disconnect_handler(httpd_req_t *req, void *args) {
    ESP_LOGI(TAG, "WebSocket client disconnected");
    // Handle WebSocket disconnect event
}


// Define URI configurations
 httpd_uri_t index_html_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_html_get_handler,
    .user_ctx  = NULL,
    .is_websocket = false
};

 httpd_uri_t index_css_uri = {
    .uri       = "/styles/index.css",
    .method    = HTTP_GET,
    .handler   = index_css_get_handler,
    .user_ctx  = NULL,
    .is_websocket = false
};

 httpd_uri_t index_js_uri = {
    .uri       = "/scripts/index.js",
    .method    = HTTP_GET,
    .handler   = index_js_get_handler,
    .user_ctx  = NULL,
    .is_websocket = false
};


 httpd_uri_t test_html_uri = {
    .uri       = "/test",
    .method    = HTTP_GET,
    .handler   = test_html_get_handler,
    .user_ctx  = NULL,
    .is_websocket = false
};

 httpd_uri_t ws_uri = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = recv_ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = true
        

};



void start_webserver()
{
    if (server != NULL) {
        ESP_LOGE(TAG, "Webserver already started");
        stop_webserver();
        free(server); // free the memory
    }


    server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
 

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_register_uri_handler(server, &index_html_uri);
        httpd_register_uri_handler(server, &index_css_uri);
        httpd_register_uri_handler(server, &index_js_uri);
        httpd_register_uri_handler(server, &test_html_uri);
        httpd_register_uri_handler(server, &ws_uri);

        // Initialize the linked list
        tracked_ws = (struct ws_storage*) malloc(sizeof(struct ws_storage));
        tracked_ws->hd = server;
        tracked_ws->head = NULL;



    }


    return;
}

void stop_webserver()
{
    if (server) {
        // Stop the httpd server
        httpd_stop(server);
    }
}
