#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <esp_http_server.h>


// Handler function prototypes
esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t index_css_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);


// URI configurations (extern declarations)
extern httpd_uri_t index_html_uri;
extern httpd_uri_t index_css_uri;
extern httpd_uri_t test_html_uri;


#endif /* WEB_HANDLERS_H */
