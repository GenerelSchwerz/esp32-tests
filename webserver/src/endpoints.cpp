#include <stdio.h>
#include <esp_http_server.h>

#include "esp_log.h"
#include "constants.h" // Include static files.
#include "sensors.h" // Include sensor functions.

const char* TAG = "endpoints";

esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);


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
    WS_GET_READINGS = 0
} ws_event_t;



/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
    struct async_resp_arg *resp_arg = (async_resp_arg*) arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    const char* data = resp_arg->data;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg->data);
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




esp_err_t recv_ws_handler(httpd_req_t *req) {
   if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
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
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

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
            ESP_LOGI(TAG, "Received request for readings");

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


    

    // TODO: impl w/ json.


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
        .is_websocket = true

};
