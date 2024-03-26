#include <stdio.h>
#include <esp_http_server.h>

#include "constants.h" // Include static files.

esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);


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
    .user_ctx  = NULL
};

httpd_uri_t index_css_uri = {
    .uri       = "/styles/index.css",
    .method    = HTTP_GET,
    .handler   = index_css_get_handler,
    .user_ctx  = NULL
};


httpd_uri_t test_html_uri = {
    .uri       = "/test",
    .method    = HTTP_GET,
    .handler   = test_html_get_handler,
    .user_ctx  = NULL
};