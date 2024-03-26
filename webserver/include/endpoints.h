#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

// #ifdef __cplusplus
// extern "C" {
// #endif

// Handler function prototypes
esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t index_css_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);




// URI configurations (extern declarations)
extern  httpd_uri_t index_html_uri;
extern  httpd_uri_t index_css_uri;
extern httpd_uri_t index_js_uri;
extern  httpd_uri_t test_html_uri;
extern httpd_uri_t ws_uri;

// do not expose server, no point.
void start_webserver();
void stop_webserver();

// expose broadcast function
void broadcast_message(char *data);

// #ifdef __cplusplus
// }
// #endif

#endif /* WEB_HANDLERS_H */
